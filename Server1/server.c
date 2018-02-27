/***
	Advanced operating systems Project 1
	Author : Amal Roy
	
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 4455
#define Server "10.176.66.71"

/* Write info received from client */
void write_to_file(int client_sock){
	
	FILE *fp;
	char fname[100];
	int bytesReceived = 0;
    char recvBuff[256];
	
	bytesReceived=recv(client_sock, fname, 100,0);
	printf("[+]File Name: %s\n",fname);
	printf("[+]bytesReceived: %d\n",bytesReceived);
   	fp = fopen(fname, "a"); 
	if(fp!=NULL)
	{
		if((bytesReceived = recv(client_sock, recvBuff, 256,0)) > 0)
		{	
			strcat(recvBuff,"\r\n");
			printf("[+]Received Buffer :%s\n",recvBuff);
			fwrite(recvBuff, 1,strlen(recvBuff),fp);
		}
		fclose(fp);
		printf("[+]File Writing Completed\n");
	}else{
		printf("[+]Error opening file\n");
	}
}

/* Return last line of the file from Server*/
void read_from_file(int client_sock){
	
	FILE *fp;
	char fname[100];
	int bytesReceived = 0;
    char recvBuff[256];
	char tmp[50]={0};

	bytesReceived=recv(client_sock, fname, 100,0);
	printf("[+]File Name: %s\n",fname);

	fp = fopen(fname, "r"); 
	if(fp!=NULL)
	{
		while(!feof(fp)){
			fgets(tmp, 50, fp);
		}
		if(strlen(tmp)==0){
			strcpy(tmp,"-No Data-");
		}
		printf("[+]Last Line:%s\n",tmp);
		send(client_sock, tmp,256,0);
	}else{
		printf("[+]Error opening file\n");
	}
}

/* Return the list of files present in the Server*/
void enquire_List_of_Files(int client_sock){
	
	FILE *fp;
	char fname[100];
	int bytesReceived = 0;
    char recvBuff[256];
	char tmp[256];

	system("ls Files > File_List.txt");
   	strcpy(fname,"File_List.txt");

   	fp = fopen(fname, "r"); 
	if(fp!=NULL)
	{
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);  //same as rewind(f);
		char *string = malloc(fsize + 1);	
		while(!feof(fp)){
			fread(string, fsize, 1, fp);
		}
		printf("[+]List Of files :%s\n",string);
		send(client_sock, string,256,0);	
	}else{
		printf("[+]Error opening file\n");
	}
}

/* New Thread is created for the client */
void *spawn_client(void *args)
{
	int client_sock=*(int*)args;
	char operation[100];

	printf("[+]Client thread created..\n");
	
	while(recv(client_sock, operation, 100,0)>0){
		if(strcmp(operation,"W")==0){
			write_to_file(client_sock);
		}
		else if(strcmp(operation,"R")==0){
			read_from_file(client_sock);
		}
		else if(strcmp(operation,"E")==0){
			enquire_List_of_Files(client_sock);
		}
	}
	close(client_sock);
}

void start_server(){
	int i;
	int server_sock;
	struct sockaddr_in serverAddr;

	int client_sock=0;
	struct sockaddr_in clientAddr;
		
	socklen_t addr_size;
	char buffer[1024];
	
	pthread_t t[2];
	printf("[+]Server Starting ..\n");
	server_sock=socket(AF_INET,SOCK_STREAM,0);
	if(server_sock<0)
	{
	  printf("Error in socket creation\n");
	}
	printf("[+]Server Socket Created Successfully...\n");

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(PORT);
	serverAddr.sin_addr.s_addr=inet_addr(Server);

	if(bind(server_sock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
		printf("[+]BindFailed\n");
	}

	if(listen(server_sock, 10) == -1)
    {
        printf("Failed to listen\n");
    }
	addr_size= sizeof(clientAddr);
	printf("[+]Server Listening...\n");
	while(1){
		//for(i=0;i<5i++)      //can support 5 clients at a time
		//{
			printf("[+]Starting Multi-threading..\n");
			
			client_sock=accept(server_sock,(struct sockaddr*)&clientAddr, &addr_size);
			if (client_sock < 0)
			{
				printf("[+]Client Connection Failed !! %d \n",client_sock);
				break;
			}else{
				printf("[+]Connected to client %d \n",client_sock);
				printf("[+]Receiving  From :%s \n",inet_ntoa(clientAddr.sin_addr));
				pthread_create(&t,NULL,spawn_client,&client_sock);
			}
	}	
	close(server_sock);
}

void clear_files(){

	FILE *fp;
	char fname[100];
	int i=1,n;
	while(i<=5){
		n=sprintf (fname, "Files/%d.txt", i);
		fclose(fopen(fname, "w"));
		i++;
	}
}

int main(){
	clear_files();
	start_server();
	return 0;
}

