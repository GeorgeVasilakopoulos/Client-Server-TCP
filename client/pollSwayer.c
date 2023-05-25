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



char* serverName;
int portNum;
FILE* inputFilePointer;




pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;


int countLinesInFile(const char* filename){
    FILE* fileptr = fopen(filename,"r");
    int lineCounter = 0;
    for(char c=getc(fileptr);c!=EOF;c=getc(fileptr)){
        if(c=='\n')lineCounter++;
    }
    fclose(fileptr);
    return lineCounter;
}




void getResponse(int sock, char* writeBuf){
    char responseBuf[100]="";
    int i=0;
    while(read(sock,responseBuf+i,1)>0){
        if(responseBuf[i]=='\n' || responseBuf[i]=='\0')break;
        i++;
    }
    responseBuf[i]='\0';
    if(writeBuf==NULL)return;
    strcpy(writeBuf,responseBuf);
}



void* swayerFunction(){

    pthread_mutex_lock(&mutex);
    if(inputFilePointer == NULL){
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
    }
    char voterNameBuf[100]="";
    char party[100]="";    

    if(fscanf(inputFilePointer,"%s",voterNameBuf)==EOF){
        //Error
        inputFilePointer = NULL;
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
    }
    if(fscanf(inputFilePointer,"%s",party)==EOF){
        inputFilePointer = NULL;
    }

    

	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
	int sock;

	//Create Socket
    if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("Socket error\n");
    	exit(0);
    }

    pthread_mutex_unlock(&mutex);

    //Find server address
    if((rem = gethostbyname(serverName)) == NULL){
        printf("Hostname error\n");
    	exit(0);
   	}


   	server.sin_family = AF_INET;
    memcpy(&server.sin_addr,rem->h_addr,rem->h_length);
    server.sin_port = htons(portNum);

    if(connect(sock,serverptr,sizeof(server))<0){
    	printf("ERROR\n");
    	exit(0);
    }

    getResponse(sock,NULL);
    write(sock,voterNameBuf,strlen(voterNameBuf));
    write(sock,"\n",1);
    getResponse(sock,NULL);
    write(sock,party,strlen(party));
    write(sock,"\n",1);
    getResponse(sock,NULL);
    close(sock);
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


	pthread_t* swayer = malloc(sizeof(pthread_t)*numThreads);
	for(int i=0;i<numThreads;i++){
		pthread_create(swayer+i,NULL,&swayerFunction,NULL);
	}

	for(int i=0;i<numThreads;i++){
		pthread_join(swayer[i],NULL);
	}
    free(swayer);


	return 0;
}

