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
char* inputFile;

void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}



void* swayerFunction(){

	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
	int sock;

	//Create Socket
    if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("Socket error\n");
    	exit(0);
    }

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

    char buf[100] = "Hey im ";
    tostring(buf+strlen(buf),pthread_self());
    for(int i=0; buf[i]!='\0'; i++){
    	if(write(sock,buf+i,1)<0){
    		printf("Writing error\n");
    		exit(0);
    	}
    }
    close(sock);
    pthread_exit(NULL);
}


int main(int argc, char*argv[]){

	if(argc<4){
		printf("Insufficient number of arguments!\n");
		return 0;
	}

	serverName = argv[1];
	portNum = atoi(argv[2]);
	inputFile = argv[3];	

	pthread_t swayer[10];
	for(int i=0;i<10;i++){
		pthread_create(swayer+i,NULL,&swayerFunction,NULL);
	}

	for(int i=0;i<10;i++){
		pthread_join(swayer[i],NULL);
	}




	return 0;
}

