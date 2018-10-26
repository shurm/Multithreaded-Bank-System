#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main( int argc, char ** argv)
{
	if(argc!=2)
	{
		fprintf(stderr,"You failed to provide the server's machine name\n");
		return -1;
	}
	/*
	 * The Variables I need to make the socket
	 */

	//the port number the server uses
	char port[5]="5389";

	char buffer[256];

	//file descriptor that will point to my socket
	int mySocket;

	int n;



	struct addrinfo request, *server;

	memset(&request, 0, sizeof(request));
	request.ai_family = AF_INET;
	request.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(argv[1], port, &request, &server) != 0)

	{
		fprintf(stderr,"Error. Could not get address information.\n");
		return -1;

	}



	if((mySocket= socket(server->ai_family,server->ai_socktype, 
					server->ai_protocol)) == -1)
	{
		fprintf(stderr,"Error. Could not create socket.\n");
		return -1;

	}

	while(connect(mySocket, server->ai_addr, server->ai_addrlen) == -1){
		fprintf(stderr,"Error. Could not connect to server.\nTrying again.\n");
		//the client waits 		
		sleep(3);
	}

	printf("connection to server successful.\n");
	printf("please enter a valid command. \n");

	while(1)
	{
		
		memset( buffer, 0, sizeof((buffer)));
		
		//puts the main thread to sleep for 2 seconds
		sleep(2);

		read(0,buffer,sizeof(buffer)-1);
		buffer[strlen(buffer)-1]='\0';
		//printf("you entered %s\n",buffer);
		
		//writes to the server
		n = write(mySocket, buffer, strlen(buffer));



		if(n < 0)
			fprintf(stderr,"Error writing to server.\n");



		memset( buffer, 0, sizeof(buffer)-1);

		//gets messages from the server
		n = read( mySocket, buffer, sizeof(buffer)-1);
		if(n < 0)
			fprintf(stderr,"Error reading from server.\n");
		if(buffer[0] == 0)
		{
			fprintf(stderr,"Error. Server closed unexpectedly.\n");
			break;
		}
		
		printf("%s\n", buffer);
		if(strcmp(buffer, "You have been disconnected from the server") == 0)
			break;
		
	}
	
	//closes socket
	close(mySocket);
	return 0;
}
