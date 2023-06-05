#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <netdb.h>

#define ERROR_CHECK(arg)                    \
{                                           \
    if((arg)<0){                            \
        printf("Error in %s\n",#arg);       \
        exit(0);                            \
    }                                       \
}                                           


char* serverName;
int portNum;
FILE* inputFilePointer;




pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;


int countLinesInFile(const char* filename){
    FILE* fileptr = fopen(filename,"r");
    if(!fileptr){
        printf("Error in opening file %s\n",filename);
        exit(0);
    }
    int lineCounter = 0;
    for(char c=getc(fileptr);c!=EOF;c=getc(fileptr)){
        if(c=='\n')lineCounter++;
    }
    fclose(fileptr);
    return lineCounter;
}




/*
    Read response from socket until a newline is found.
    
    If writeBuf is not NULL, write response to writeBuf

    Same as server
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

void setSockAsReuseable(int sock){
    const int enable = 1;
    ERROR_CHECK(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int)));
}



void* swayerFunction(){

    ERROR_CHECK(pthread_mutex_lock(&mutex));
    
    //If inputFilePointer is NULL, all of the lines have been consumed
    if(inputFilePointer == NULL){
        ERROR_CHECK(pthread_mutex_unlock(&mutex));
        pthread_exit(NULL);
    }
    char voterNameBuf[100]="";
    char party[100]="";    

    
    //If any of the next fscanf's return EOF, there exists an error in the input file.
    if(fscanf(inputFilePointer,"%s",voterNameBuf)==EOF){
        fclose(inputFilePointer);
        inputFilePointer = NULL;
        ERROR_CHECK(pthread_mutex_unlock(&mutex));
        pthread_exit(NULL);
    }
    strcat(voterNameBuf," ");
    if(fscanf(inputFilePointer,"%s",voterNameBuf+strlen(voterNameBuf))==EOF){
        fclose(inputFilePointer);
        inputFilePointer = NULL;
        ERROR_CHECK(pthread_mutex_unlock(&mutex));
        pthread_exit(NULL);
    }
    if(fscanf(inputFilePointer,"%s",party)==EOF){
        fclose(inputFilePointer);
        inputFilePointer = NULL;
        ERROR_CHECK(pthread_mutex_unlock(&mutex));
        pthread_exit(NULL);
    }

    
    ERROR_CHECK(pthread_mutex_unlock(&mutex));


    //Initialize Server Variables
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
	int sock;

	//Create Socket
    ERROR_CHECK(sock = socket(AF_INET,SOCK_STREAM,0))
    setSockAsReuseable(sock);


    //Find server address
    if((rem = gethostbyname(serverName)) == NULL){
        printf("Hostname error\n");
    	exit(0);
   	}


   	server.sin_family = AF_INET;
    memcpy(&server.sin_addr,rem->h_addr,rem->h_length);
    server.sin_port = htons(portNum);

    ERROR_CHECK(connect(sock,serverptr,sizeof(server)))
    strcat(voterNameBuf,"\n");
    strcat(party,"\n");

    getResponse(sock,NULL);
    ERROR_CHECK(write(sock,voterNameBuf,strlen(voterNameBuf)));
    getResponse(sock,NULL);
    ERROR_CHECK(write(sock,party,strlen(party)));
    getResponse(sock,NULL);
    ERROR_CHECK(close(sock));
    pthread_exit(NULL);
}


int main(int argc, char*argv[]){

	if(argc<4){
		printf("Insufficient number of arguments!\n");
		return 0;
	}

	serverName = argv[1];
	portNum = atoi(argv[2]);
	char* inputFile = argv[3];

    int numThreads = countLinesInFile(inputFile);

    inputFilePointer = fopen(inputFile,"r");
    if(!inputFilePointer){
        printf("Error in opening file %s\n",inputFile);
        exit(0);
    }

	pthread_t* swayer = malloc(sizeof(pthread_t)*numThreads);
	for(int i=0;i<numThreads;i++){
		pthread_create(swayer+i,NULL,&swayerFunction,NULL);
	}

	for(int i=0;i<numThreads;i++){
		pthread_join(swayer[i],NULL);
	}
    if(inputFilePointer)fclose(inputFilePointer);
    free(swayer);


	return 0;
}

