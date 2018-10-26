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
#include <pthread.h>
#include <time.h>
#include <malloc.h>
#include "header.h"


/*
 * Global variables
 */

//locks that I will use to synchronize my threads
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;

int running=1;

void* clientStuff(void* arg)
{
	CPackage* package=(CPackage *)arg;
	int *ptr=package->active;
	Account* accounts=package->account;
	int length=package->length;

	char *message;

	int o=0;
	char buffer[556];
	char command[10];
	char name[502];
	int n;
	int session=0;
	int f=0;
	int a;
	//printf("Thread is executing\n");


	while(1)
	{

		memset( buffer, 0, sizeof(buffer));
		memset( command, 0, sizeof(command));
		memset( name, 0, sizeof(name));
		//printf("accounts[%d].name is %s\n", a,accounts[a].name);

		//reads only the first sizeof(buffer) characters, so that the last character is null terminating
		n = read(package->id, buffer, sizeof(buffer)-sizeof(char));


		if(n < 0)
		{
			fprintf(stderr,"Error reading from client.\n");

		}
		if(buffer[0] == 0)
		{
			fprintf(stderr,"Error. Client closed unexpectedly.\n");
			if(session==1)
			{
				accounts[a].inSession=0;
			}
			break;
		}
		
		//puts characters from the fullLine variable into 
		//the command and name char arrays
		extractInfo(buffer,command,name);

	//	printf("command is %s\n",command);
	//	printf("name is %s\n",name);
		if(session==0)
		{
			if(strcmp(command,"open")==0)
			{

				if(strlen(name)>100)
				{
					message="Error. The name you provided was too long.";
				}
				else
				{
					//so that only one account can opened at one time 
					pthread_mutex_lock(&lock2);
					f=0;
					for(a=0;a<length;a++)
					{
						//printf("%d\n",a);
						if(strlen(accounts[a].name)==0)
						{
							f=1;
							break;
						}
						if( strcmp(name,accounts[a].name)==0)
						{
							f=0;
							break;
						}
					}

					//opening new account
					if(f==1)
					{
						//printf("%s\n",name);
						strcpy(accounts[a].name,name);	
						message="You opened a new account.";	
					}

					else
					{
						if(a==length)
						{
							message="Error. the maxium number of bank accounts have been opened.";

						}
						else
						{
							message="Error. A bank account with that name already exists.";

						}
					}
					pthread_mutex_unlock(&lock2);

				}

			}


			else if(strcmp(command,"start")==0)
			{
				f=0;
				for(a=0;a<length;a++)
				{
					if(strlen(accounts[a].name)==0)
					{
						f=0;
						break;
					}
					if( strcmp(name,accounts[a].name)==0)
					{
						f=1;

						pthread_mutex_lock(&lock3);
						if(accounts[a].inSession==1)
						{
							message="Error. The account you are trying to access. Is already being accessed elsewhere";

						}
						else
						{
							accounts[a].inSession=1;
							session=1;
							message="customer session started";
						}
						pthread_mutex_unlock(&lock3);
						break;
					}
				}
				if(f==0)
				{

					message="Error. A bank accounts with that name does not exist.";

				}

			}
			else if(strcmp(command,"exit")==0)
			{
				n = write(package->id, "You have been disconnected from the server",100 );

				break;
			}
			else 
			{
				message="Error. Invalid command.";
			}	

		}
		else
		{
			if(strcmp(command,"credit")==0)
			{
				float f1=atof(name);
				accounts[a].balance=f1+accounts[a].balance;
				message="money successfully deposited"; 		
			}
			else if(strcmp(command,"debit")==0)
			{
				float f1=atof(name);
				if(accounts[a].balance-f1<0)
					message="Error. requested amount is greater than current balance.";
				else
				{

					accounts[a].balance=accounts[a].balance-f1;
					message="money successfully deducted";
				}
			}
			else if(strcmp(command,"balance")==0)
			{
				sprintf(buffer,"%.2f",accounts[a].balance);
				n = write(package->id,buffer,strlen(buffer));
				o=1;

			}
			else if(strcmp(command,"finish")==0)
			{
				session=0;
				accounts[a].inSession=0;
				message="customer session ended.";
			}
			else 
			{
				message="Error. Invalid command.";
			}	


		}
		if(o==0)
			n = write(package->id, message,strlen(message));

		o=0;
	}



	//printf("ptr is %d\n",ptr);
	change(ptr,length ,package->index);
//	printf("thread is exiting\n");	
	pthread_exit(0);
}

void* acceptingConnections(void* arg)
{
	int np=100;

	//the number of connestions that  the server can handle
	int numberOfConnections=np;

	SPackage* sPackage=(SPackage*)arg;


	Account * accounts=sPackage->accounts;

	int length=sPackage->length;	

	CPackage cPackages[np];
	int a;

	pthread_t clientThread;
	int active[np];

	//printf("accepting thread is running\n");
	for(a=0;a<np;a++)
	{

		active[a]=0;
	}


	//the port number the server uses
	char port[5]="5389";

	//file descriptor that will point to my socke	
	int mySocket;

	int clientId;

	struct sockaddr_storage client;

	struct addrinfo request, *result;

	memset( &request, 0, sizeof(request));
	request.ai_family = AF_INET;
	request.ai_flags = AI_PASSIVE;
	request.ai_socktype = SOCK_STREAM;

	if( getaddrinfo(NULL, port, &request, &result) != 0){
		fprintf(stderr,"Error. Could not get address information.\n");
	running=0;
	pthread_exit(0);

	}

	if((mySocket= socket(request.ai_family,request.ai_socktype, 
					request.ai_protocol)) == -1)
	{
		fprintf(stderr,"Error. Could not create socket.\n");
	running=0;		
pthread_exit(0);


	}

	if(bind(mySocket, result->ai_addr, result->ai_addrlen) == -1){
		fprintf(stderr,"Error. Could not bind socket.\n");

		close(mySocket);
	running=0;	
	pthread_exit(0);

	}



	freeaddrinfo(result);


	while(1)
	{
		//program stops here and waits for clients to connect
		if( listen(mySocket, numberOfConnections) == -1){
			fprintf(stderr,"An error occured while listening\n");
			pthread_exit(0);

		}
		socklen_t len = sizeof(client);

		clientId = accept(mySocket , (struct sockaddr *)&client, &len);
		if(clientId == -1){
			fprintf(stderr,"Error. Socket could not accept connection.\n");
			pthread_exit(0);

		}

		/*
		 *Checks to see how mnay clients are currently interacting with the server
		 */

		a=change(active,np,-1);
		if(a==-1)
		{

			fprintf(stderr,"Error. The maximum number of client threads are currently running.\n");
		}

		else
		{
			printf("connection to client successful.\n");

			cPackages[a].id=clientId;
			cPackages[a].active=active;
			cPackages[a].length=length;
			cPackages[a].account=accounts;
			cPackages[a].index=a;

			//start a new connection thread	
			createThread(&clientThread,clientStuff,&(cPackages[a]));	
		}
	}
	pthread_exit(0);
}
int main(int argc, char ** argv)
{
	// the maximum number of accounts that can be created
	int max =20;

	//an array of Account structs, each one contains a name, a float 
	//representating its current balnce, and an int representing whether 
	//or not the account is currently being accessed  
	Account accounts[max];

	//a variable, which contain a pointer to the array of Accounts, 
	//and the length of the array, which will be passed into the 
	//accepting connections thread. 		
	SPackage package;

	


	/*
	 *	Variables for BankAccounts/Threads
	 *
	 */
	int a=0;



	//intialize my three locks
	if(pthread_mutex_init(&lock,NULL)!=0)
	{
		fprintf(stderr,"Error. mutex init failed.\n");

	}
	if(pthread_mutex_init(&lock2,NULL)!=0)
	{
		fprintf(stderr,"Error. mutex init failed.\n");

	}
	if(pthread_mutex_init(&lock3,NULL)!=0)
	{
		fprintf(stderr,"Error. mutex init failed.\n");

	}
	
	//set the memory of the variables inside each account 
	clearAccounts(accounts,max);

	//passes variables into sPackage struct
	package.accounts=accounts;
	package.length=max;

	//creates thread id
	pthread_t acceptingThread;

	
	//creates the thread
	createThread(&acceptingThread,acceptingConnections,&package);





	while(running)
	{
		//puts the main thread to sleep for 20 seconds
		sleep(20);

		for(a=0;a<max;a++)
		{
			if(strlen(accounts[a].name)>0)
			{
				//locking this mutux variable prevents 
				//new accounts from being opened while 
				//account information is being printed 
				//by the server 
				pthread_mutex_lock(&lock2);

				printf("%s: %.2f", accounts[a].name,accounts[a].balance);
				if(accounts[a].inSession==1)
				{
					printf(" IN SERVICE");
				}
				printf("\n");
				pthread_mutex_unlock(&lock2);

			}

		}
		printf("\n");
	}


	/* wait for the thread to finish */
	pthread_join( acceptingThread,NULL );


	//destroys my three locks
	pthread_mutex_destroy(&lock);	
	pthread_mutex_destroy(&lock2);	
	pthread_mutex_destroy(&lock3);	

	return 0;

}

//puts characters from the fullLine variable into command and name char arrays
void extractInfo(char* fullLine, char* command, char* name)
{
	int length;
	int a;
	int b=0;


	if(strlen(fullLine)<sizeof(command))
		length=strlen(fullLine);
	else
		length=sizeof(command);

	//	printf("length is %d\n",length);
	for(a=0;a<length;a++)
	{
		if(fullLine[a]==' ')
			break;

		command[a]=fullLine[a];	
	}
	//makes the last character null
	command[a]='\0';

	a++;
	length=strlen(fullLine);
	while(a<length)
	{

		name[b]=fullLine[a];
		a++;
		b++;
	}
	name[b]='\0';	
}

int change(int* array, int length,int index)
{
	
	pthread_mutex_lock(&lock);
	//printf("locks\n");
	int r=0;
	//printf("helllo\n");

	if(index>=0)
	{
		array[index]=0;
	}
	else
	{
		for(r=0;r<length;r++)
		{
			if(array[r]==0)
			{
				array[r]=1;
				break;
			}		
		}
		if(r==length)
			r=-1;
		//printf("%d\n",*ptr);
	}
	pthread_mutex_unlock(&lock);
	return r;
}
int createThread(pthread_t* id,Function f,void *arg)
{
	int r;

	// build blank pthread attribute structs and initialize them
	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);


	// set the initialized attribute struct so that the pthreads created will be joinable
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

	// set the initialized attribute struct so that the pthreads created will be kernel threads
	pthread_attr_setscope(&threadAttr, PTHREAD_SCOPE_SYSTEM);






	/****************************8********/
	// build the pthreads
	r=pthread_create(id, &threadAttr,f, arg);






	// destroy the pthread attribute structs, we're done creating the threads,
	//   we don't need them anymore
	pthread_attr_destroy(&threadAttr);

	return r;
}
void clearAccounts(Account * accounts, int max)
{
	int a=0;
	char k[1];
	k[0]='\0';
	for(a=0;a<max;a++)
	{
		strcpy(accounts[a].name,k);
		accounts[a].balance=0;
		accounts[a].inSession=0;
	}		
}
