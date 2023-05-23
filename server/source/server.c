#include <stdio.h>
#include <string.h>
#include <pthread.h>
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

pthread_mutex_t recordMutex = PTHREAD_MUTEX_INITIALIZER;






void*workerFunction(){

	while(1){
		pthread_mutex_lock(&mutex);
		while(QueueSize(&clientQueue) == 0)
			pthread_cond_wait(&nonEmptyCondition,&mutex);
		int newSocket = *(int*)QueueFront(&clientQueue);
		QueuePop(&clientQueue);
		pthread_cond_signal(&nonFullCondition);
		pthread_mutex_unlock(&mutex);



		write(newSocket,"SEND NAME PLEASE\n", strlen("SEND NAME PLEASE\n"));
		char name[1000];
		char party[1000];
		int i=0;
		while(read(newSocket,name+i,1)>0){
			if(name[i]=='\n')break;
			i++;
		}
		name[i]='\0';
		
		write(newSocket,"SEND VOTE PLEASE\n", strlen("SEND VOTE PLEASE\n"));
		i=0;
		while(read(newSocket,party+i,1)>0){
			if(party[i]=='\n')break;
			i++;
		}
		party[i]='\0';
		write(newSocket,"DONE\n", strlen("DONE\n"));

		pthread_mutex_lock(&recordMutex);




		pthread_mutex_unlock(&recordMutex);
		

		//Serve Client
		close(newSocket);
	}









	pthread_exit(NULL);
}






int main(int argc, char* argv[]){
	if(argc < 6){
		printf("Insufficient number of arguments!\n");
		return 0;
	}
	int portnum = atoi(argv[1]);
	int numWorkerthreads = atoi(argv[2]);
	int bufferSize = atoi(argv[3]);
	char* poll_log = argv[4];
	char* poll_stats = argv[5];

	//Initialize Queue
	QueueInitialize(&clientQueue,sizeof(int));


	//Create Worker Threads
	pthread_t* workerThread = malloc(sizeof(pthread_t)*numWorkerthreads);
	for(int i=0; i < numWorkerthreads; i++){
		pthread_create(workerThread+i,NULL,&workerFunction,NULL);
	}



	//Initialize Server Variables
	struct sockaddr_in server;
	struct sockaddr* serverptr = (struct sockaddr*)&server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(portnum);


	struct sockaddr_in client;
	struct sockaddr* clientptr = (struct sockaddr*)&client;


	int mainSocket = socket(AF_INET,SOCK_STREAM,0);

	bind(mainSocket,serverptr,sizeof(server));

	listen(mainSocket,5);

	while(1){
		socklen_t clientlen = sizeof(client);
		int newSocket = accept(mainSocket,clientptr,&clientlen);

		pthread_mutex_lock(&mutex);
		while(QueueSize(&clientQueue) == bufferSize)
			pthread_cond_wait(&nonFullCondition,&mutex);
		QueueInsert(&clientQueue,&newSocket);
		pthread_cond_signal(&nonEmptyCondition);
		pthread_mutex_unlock(&mutex);
	}




	return 0;
}