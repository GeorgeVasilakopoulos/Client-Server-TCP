#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <unistd.h> 
#include "queue.h"
#include "hashtable.h"
#include "votersRecord.h"



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t nonFullCondition = PTHREAD_COND_INITIALIZER;
pthread_cond_t nonEmptyCondition = PTHREAD_COND_INITIALIZER;

//Use of a FIFO queue structure instead of a buffer...
Queue clientQueue;	

//Will be set to 1 when SIGINT signal is received
int stopFlag=0;


//Mutex to protect the structure that stores the votes
pthread_mutex_t recordMutex = PTHREAD_MUTEX_INITIALIZER;
votersRecord voteRecordStructure;


#define ERROR_CHECK(arg)                    \
{                                           \
    if((arg)<0){                            \
        printf("Error in %s\n",#arg);       \
    	exit(0);							\
    }                                       \
}                                           

/*
	Read response from socket until a newline is found.
	
	If writeBuf is not NULL, write response to writeBuf

	Same as client
*/

void getResponse(int sock, char* writeBuf){
    char responseBuf[100]="";
    int i=0;
    while(read(sock,responseBuf+i,1)>0){
        if(responseBuf[i]=='\n')break;
        i++;
    }
    responseBuf[i]='\0';
    if(writeBuf==NULL)return;
    strcpy(writeBuf,responseBuf);
}



void*workerFunction(){
	

	//Child threads will ignore the SIGINT signal
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set,SIGINT);
	pthread_sigmask(SIG_BLOCK,&signal_set,NULL);


	while(1){
		pthread_mutex_lock(&mutex);

		//If there are no pending requests or stopFlag is set
		while(stopFlag || QueueSize(&clientQueue) == 0){
			if(stopFlag){

				//Unlock the mutex and exit
				pthread_cond_signal(&nonEmptyCondition);
				pthread_mutex_unlock(&mutex);
				pthread_exit(NULL);
			}

			//Wait until queue is non empty
			pthread_cond_wait(&nonEmptyCondition,&mutex);
		}

		//Get newSocket value and pop the front of the queue
		int newSocket = *(int*)QueueFront(&clientQueue);
		QueuePop(&clientQueue);
		pthread_cond_signal(&nonFullCondition);
		pthread_mutex_unlock(&mutex);


		char* sendNamePlease = "SEND NAME PLEASE\n"; 
		char* sendVotePlease = "SEND VOTE PLEASE\n";

		//Send "SEND NAME PLEASE"
		write(newSocket,sendNamePlease, strlen(sendNamePlease));
		

		char name[1000];
		char party[1000];
		

		getResponse(newSocket,name);

		//Send "SEND VOTE PLEASE"
		write(newSocket,sendVotePlease, strlen(sendVotePlease));

		getResponse(newSocket,party);

		//Request access to the record structure
		pthread_mutex_lock(&recordMutex);
		int alreadyVoted = InsertVote(&voteRecordStructure,name,party);
		pthread_mutex_unlock(&recordMutex);
		
		char doneMessage[100];
			


		//If person has already voted, the new vote will not be included in the record structure
		if(alreadyVoted){
			strcpy(doneMessage,"ALREADY VOTED\n");
		}
		else{
			strcpy(doneMessage,"VOTE FOR PARTY ");
			strcat(doneMessage,party);
			strcat(doneMessage," RECORDED\n");
		}
		write(newSocket,doneMessage, strlen(doneMessage));
		ERROR_CHECK(close(newSocket));
	}
}

void setSockAsReuseable(int sock){
	const int enable = 1;
	ERROR_CHECK(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int)));
}


pthread_t* workerThread;
int numWorkerthreads;
int mainSocket;

void signalHandler(int sigval){
	if(sigval == SIGINT){
		pthread_mutex_lock(&mutex);
		stopFlag = 1;

		//Wake up all waiting threads in order to terminate
		pthread_cond_broadcast(&nonEmptyCondition);
		pthread_mutex_unlock(&mutex);


		//Wait for the child threads to terminate
		for(int i=0;i<numWorkerthreads;i++){
			ERROR_CHECK(pthread_join(workerThread[i],NULL));
		}

		//Deallocate threads
		free(workerThread);
		

		//No need to lock any mutex. All child threads have been terminated
		
		// saveToPollLog(&voteRecordStructure);		//No need to do this, because we have enabled realTimeSaving
		saveToPollStats(&voteRecordStructure);
		DestructRecord(&voteRecordStructure);
		


		//If the queue was not empty when the interrupt occured, close the sockets of the queue
		while(QueueSize(&clientQueue)){
			ERROR_CHECK(close(*(int*)QueueFront(&clientQueue)));
			QueuePop(&clientQueue);
		}
		QueueDestruct(&clientQueue);

		ERROR_CHECK(close(mainSocket));
		exit(0);
	}
}









int main(int argc, char* argv[]){
	if(argc < 6){
		printf("Insufficient number of arguments!\n");
		return 0;
	}
	int portnum = atoi(argv[1]);
	numWorkerthreads = atoi(argv[2]);
	int bufferSize = atoi(argv[3]);
	char* poll_log = argv[4];
	char* poll_stats = argv[5];

	//Initialize Queue
	QueueInitialize(&clientQueue,sizeof(int));
	stopFlag = 0;

	//Initialize record structure
	InitializeRecord(&voteRecordStructure,poll_log,poll_stats,1);	//Saving stats in real time


	//Create Worker Threads
	workerThread = malloc(sizeof(pthread_t)*numWorkerthreads);
	for(int i=0; i < numWorkerthreads; i++){
		ERROR_CHECK(pthread_create(workerThread+i,NULL,&workerFunction,NULL));
	}

	signal(SIGINT,&signalHandler);

	//Initialize Server Variables
	struct sockaddr_in server;
	struct sockaddr* serverptr = (struct sockaddr*)&server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(portnum);


	struct sockaddr_in client;
	struct sockaddr* clientptr = (struct sockaddr*)&client;


	ERROR_CHECK(mainSocket = socket(AF_INET,SOCK_STREAM,0));
	setSockAsReuseable(mainSocket);


	ERROR_CHECK(bind(mainSocket,serverptr,sizeof(server)));


	//If requests are too many to be stored in the buffer, an error occurs.
	ERROR_CHECK(listen(mainSocket,4096));

	while(1){
		socklen_t clientlen = sizeof(client);
		int newSocket = accept(mainSocket,clientptr,&clientlen);
		ERROR_CHECK(newSocket);
		setSockAsReuseable(newSocket);
		
		//Lock mutex to access FIFO queue structure
		pthread_mutex_lock(&mutex);
		while(QueueSize(&clientQueue) == bufferSize)
			pthread_cond_wait(&nonFullCondition,&mutex);
		QueueInsert(&clientQueue,&newSocket);
		
		//Signal that the queue is not empty
		pthread_cond_signal(&nonEmptyCondition);
		pthread_mutex_unlock(&mutex);
	}




	return 0;
}
