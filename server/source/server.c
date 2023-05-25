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
Queue clientQueue;
int stopFlag;


pthread_mutex_t recordMutex = PTHREAD_MUTEX_INITIALIZER;
votersRecord voteRecordStructure;




void*workerFunction(){
	
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set,SIGINT);
	pthread_sigmask(SIG_BLOCK,&signal_set,NULL);


	while(1){
		pthread_mutex_lock(&mutex);
		
		while(QueueSize(&clientQueue) == 0){
			pthread_cond_wait(&nonEmptyCondition,&mutex);
			if(stopFlag){
				pthread_cond_signal(&nonEmptyCondition);
				pthread_exit(NULL);
			}
		}
		int newSocket = *(int*)QueueFront(&clientQueue);
		QueuePop(&clientQueue);
		pthread_cond_signal(&nonFullCondition);
		pthread_mutex_unlock(&mutex);


		char* sendNamePlease = "SEND NAME PLEASE\n"; 
		char* sendVotePlease = "SEND VOTE PLEASE\n";

		write(newSocket,sendNamePlease, strlen(sendNamePlease));
		char name[1000];
		char party[1000];
		int i=0;
		while(read(newSocket,name+i,1)>0){
			if(name[i]=='\n')break;
			i++;
		}
		name[i]='\0';
		
		write(newSocket,sendVotePlease, strlen(sendVotePlease));
		i=0;
		while(read(newSocket,party+i,1)>0){
			if(party[i]=='\n')break;
			i++;
		}
		party[i]='\0';

		char doneMessage[100]="VOTE FOR PARTY ";
		strcat(doneMessage,party);
		strcat(doneMessage," RECORDED\n");


		write(newSocket,doneMessage, strlen(doneMessage));
		close(newSocket);
		pthread_mutex_lock(&recordMutex);
		InsertRecord(&voteRecordStructure,name,party);
		pthread_mutex_unlock(&recordMutex);
	}
}

void setSockAsReuseable(int sock){
	const int enable = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int))<0){
		//error
	}
}


pthread_t* workerThread;
int numWorkerthreads;
int mainSocket;

void signalHandler(int sigval){
	if(sigval == SIGINT){
		pthread_mutex_lock(&mutex);
		stopFlag = 1;
		pthread_cond_signal(&nonEmptyCondition);
		pthread_mutex_unlock(&mutex);

		for(int i=0;i<numWorkerthreads;i++){
			pthread_join(workerThread[i],NULL);
		}

		pthread_mutex_lock(&recordMutex);
		saveToPollLog(&voteRecordStructure);
		saveToPollStats(&voteRecordStructure);
		DestructRecord(&voteRecordStructure);
		pthread_mutex_unlock(&recordMutex);


		close(mainSocket);
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
	InitializeRecord(&voteRecordStructure,poll_log,poll_stats);


	//Create Worker Threads
	workerThread = malloc(sizeof(pthread_t)*numWorkerthreads);
	for(int i=0; i < numWorkerthreads; i++){
		pthread_create(workerThread+i,NULL,&workerFunction,NULL);
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


	mainSocket = socket(AF_INET,SOCK_STREAM,0);
	setSockAsReuseable(mainSocket);


	bind(mainSocket,serverptr,sizeof(server));

	listen(mainSocket,5);

	while(1){
		socklen_t clientlen = sizeof(client);
		int newSocket = accept(mainSocket,clientptr,&clientlen);
		setSockAsReuseable(newSocket);
		pthread_mutex_lock(&mutex);
		while(QueueSize(&clientQueue) == bufferSize)
			pthread_cond_wait(&nonFullCondition,&mutex);
		QueueInsert(&clientQueue,&newSocket);
		pthread_cond_signal(&nonEmptyCondition);
		pthread_mutex_unlock(&mutex);
	}




	return 0;
}
