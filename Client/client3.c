/***
	Advanced operating systems Project 1
	Author : Amal Roy
	
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 4455

#define Server1 "10.176.66.71"
#define Server2 "10.176.66.72"
#define Server3 "10.176.66.73"

#define Client1 "10.176.66.75"
#define Client2 "10.176.66.76"
#define Client3 "10.176.66.77"
#define Client4 "10.176.66.78"
#define Client5 "10.176.66.79"

#define File1 "Files/1.txt"
#define File2 "Files/2.txt"
#define File3 "Files/3.txt"
#define File4 "Files/4.txt"
#define File5 "Files/5.txt"

#define CID "Client3"
#define Client Client3
#define CPORT 5000
#define no_of_clients 5
#define no_of_servers 3
#define Log "Logs/client3.txt"

typedef struct {
    int priority;
    char *data;
} node_t;
 
typedef struct {
    node_t *nodes;
    int len;
    int size;
} heap_t;

char fname[100];
int clientSocket;
struct sockaddr_in serverAddr;
char buffer[256];
volatile int timestamp = 0;
volatile int running_threads = 100;
char *Client_ip[]={Client1,Client2,Client3,Client4,Client5};
char *Server_ip[]={Server1,Server2,Server3};

heap_t *pq[6] = {0};

/* Logger is used to keep track of activities done by the client. */
void logger(char* fname,char* mode,char* data){
	FILE *fp;
	int n;
	
	char log_data[500]; 
	fp = fopen(Log, "a"); 
	if(fp!=NULL)
	{
		n=sprintf (log_data, "[T-%d] %s || %s || %s \r\n", timestamp,fname,mode,data);
		printf("[+]Logger :%s\n",log_data);
		fwrite(log_data, 1,strlen(log_data),fp);
	}
	fclose(fp);
	printf("[+]Log Writing Completed\n");
	
}

int getFileIndex(char *fname){
	if(strcmp(fname,File1)==0){
		return 1;
	}else if(strcmp(fname,File2)==0){
		return 2;
	}else if(strcmp(fname,File3)==0){
		return 3;
	}else if(strcmp(fname,File4)==0){
		return 4;
	}else if(strcmp(fname,File5)==0){
		return 5;
	}
	return 0;
}

void push ( heap_t *q,int priority, char *data) {
    if (q->len + 1 >= q->size) {
        q->size = q->size ? q->size * 2 : 4;
        q->nodes = (node_t *)realloc(q->nodes, q->size * sizeof (node_t));
    }
    int i = q->len + 1;
    int j = i / 2;
    while (i > 1 && q->nodes[j].priority > priority) {
        q->nodes[i] = q->nodes[j];
        i = j;
        j = j / 2;
    }
    q->nodes[i].priority = priority;
    q->nodes[i].data = data;
    q->len++;
}
 
char *pop (heap_t *q) {
    int i, j, k;
    if (!q->len) {
        return NULL;
    }
    char *data = q->nodes[1].data;
 
    q->nodes[1] = q->nodes[q->len];
    int priority = q->nodes[1].priority;
 
    q->len--;
 
    i = 1;
    while (1) {
        k = i;
        j = 2 * i;
        if (j <= q->len && q->nodes[j].priority < priority) {
            k = j;
        }
        if (j + 1 <= q->len && q->nodes[j + 1].priority < q->nodes[k].priority) {
            k = j + 1;
        }
        if (k == i) {
            break;
        }
        q->nodes[i] = q->nodes[k];
        i = k;
    }
    q->nodes[i] = q->nodes[q->len + 1];
    return data;

}

char *peek(heap_t *q){

	if (!q->len) {
        return NULL;
    }
    char *data = q->nodes[1].data;
    int priority = q->nodes[1].priority;
 	return data;
}

void request_queue(heap_t *pq[]){

   int i,n,j;
    
	for(j=1;j<6;j++){
		n=pq[j]->len;
	    i=1;
		while(i<=n){
			printf("[%d]-%s\n", pq[j]->nodes[i].priority,pq[j]->nodes[i].data);
			i++;
		}
	}

}
bool get_approval(char* fname){
	int t,sockfd,n;
	int i = 0;
	char client_ip[20];
	char sendBuff[100];
	char recvBuff[100];
	bool permission=false;
	char* str[5];
	
	timestamp++;
	push(pq[getFileIndex(fname)],timestamp,CID);
	while(i<no_of_clients){
		
		if(strcmp(Client_ip[i],Client)==0){
			i++;
			continue;
		}
		sockfd=getConnection(Client_ip[i],CPORT);
		
		if(sockfd<0){
			permission=false;
			break;
		}
		
		t=sprintf(sendBuff,"REQUEST_%s_%d_%s",CID,timestamp,fname);
		printf("[+]Sending Approval Request : %s\n",sendBuff);
		send(sockfd, sendBuff,100,0);

		if(recv(sockfd, recvBuff, 256,0) > 0)
		{
			printf("[+]Received From Other client : %s\n",recvBuff);
			str[0] = strtok (recvBuff,"_");
			if(strcmp(str[0],"REPLY")==0){
				str[1] = strtok (NULL, "_");
				str[2] = strtok (NULL, "_");
				str[3]  = strtok (NULL, "_");
				n = getFileIndex(str[3]);
				t=atoi(str[2]);
				if(timestamp<=t){
					timestamp = t+1;
				}
			}
		}
		close(sockfd);
		i++;
	}
	i=getFileIndex(fname);
	while(1){
		
		if(strcmp(CID,peek(pq[i]))==0){
			printf("Permisiion granted!");
			printf("PEEK : %s || ClientID: %s\n",peek(pq[i]),CID );
			permission=true;
			pop(pq[i]);
			break;
		}
	}

	return permission;
}

void release(char* fname){
	int t,sockfd;
	int i = 0;
	char client_ip[20];
	char sendBuff[100];
	timestamp++;

	while(i<no_of_clients){
		
		if(strcmp(Client_ip[i],Client)==0){
			i++;
			continue;
		}
		sockfd=getConnection(Client_ip[i],CPORT);
		if(sockfd<0){
			break;
		}
		
		t=sprintf(sendBuff,"RELEASE_%s_%d_%s",CID,timestamp,fname);
		printf("[+]Sending Release message : %s\n",sendBuff);
		send(sockfd, sendBuff,100,0);
		close(sockfd);
		i++;
	}
}

/* Client replies for critical section request. */
void *com_client(void *args)
{
	int client_sock=*(int*)args;
	char* str[5];
	char recvBuff[100];
	char sendBuff[100];
	int n,t;

	printf("[+]Connection Established for receiving request.\n");
	n = recv(client_sock, recvBuff, 256,0);
	if( n> 0)
	{
		timestamp++;
		printf("[+]Received Request : %s\n",recvBuff);
		str[0] = strtok (recvBuff,"_");
		if(strcmp(str[0],"REQUEST")==0){
			str[1] = strtok (NULL, "_"); // Client ID
			str[2] = strtok (NULL, "_"); // Timestamp
			str[3]  = strtok (NULL, "_"); // Filename
			n = getFileIndex(str[3]);
			t=atoi(str[2]);
			if(timestamp<=t){
				timestamp = t+1;
			}
			n=sprintf(sendBuff,"REPLY_%s_%d_%s",CID,timestamp,str[3]);
			send(client_sock, sendBuff,100,0);
			printf("[+]Reply Sent : %s\n",sendBuff);
			push(pq[getFileIndex(str[3])],t,str[1]);
		}
		else if(strcmp(str[0],"RELEASE")==0){
			str[1] = strtok (NULL, "_"); // Client ID
			str[2] = strtok (NULL, "_"); // Timestamp
			str[3]  = strtok (NULL, "_");
			n = getFileIndex(str[3]);
			t=atoi(str[2]);
			if(timestamp<=t){
				timestamp = t+1;
			}
			printf("[+]Release message received and Pop-> %s\n",pop(pq[getFileIndex(str[3])]));
		}
	}
	
	close(client_sock);
}

/* Write to server Functionality */
void write_to_server(char* fname){
	
	char tmp[10];
	int n,i=0;
	int clientfd;

	while(i<no_of_servers){

		clientfd=getConnection(Server_ip[i],PORT);
		send(clientfd, "W",100,0);
		send(clientfd, fname,100,0);
		n=sprintf (buffer, "ClientID:%s,Timestamp:%d", CID,timestamp);
		send(clientfd, buffer,256,0);
		printf("File Name : %s ; File Entry: %s\n; Server:%d",fname,buffer,i+1);
		i++;
	}
	logger(fname,"W",buffer);
	release(fname);
	close(clientfd);
}

/* Write to server Functionality */
void read_from_server(char* fname){
	int recvflag;
	char recvBuff[256]={0};
	size_t length;
	int i=rand()%3;
	int clientfd;

	clientfd=getConnection(Server_ip[i],PORT);
	
	send(clientfd, "R",100,0);
	send(clientfd, fname,100,0);
	if((recvflag = recv(clientfd, recvBuff, 256,0)) > 0)
	{
		printf("[+]Received File Entry %s from server%d\n",recvBuff,i);
		if( (length =strlen(recvBuff) ) >0){
		   if(recvBuff[length-1] == '\n')
		        recvBuff[length-1] ='\0';
		}
	}
	close(clientfd);
	logger(fname,"R",recvBuff);
	release(fname);
}

void enquire_from_server(){
	int recvflag;
	char recvBuff[256];
	int i=rand()%3;
	int clientfd;
	clientfd=getConnection(Server_ip[i],PORT);

	send(clientfd, "E",100,0);
	if((recvflag = recv(clientfd, recvBuff, 256,0)) > 0){
		printf("[+]List Of files in server%d : %s\n",i,recvBuff);
	}
	logger("File List :","E",recvBuff);
	close(clientfd);
}

void do_operation(){
	char fname[100];
	int i=20;
	int op;
	int ft;
	int sp;
	while(i>0){
		timestamp++;
		ft = (rand()%5)+1;
		op = (rand()%4)+1;
		
		switch (ft){
			case 1 : strcpy(fname,File1);
				break;
			case 2 : strcpy(fname,File2);
				break;
			case 3 : strcpy(fname,File3);
				break;
			case 4 : strcpy(fname,File4);
				break;
			case 5 : strcpy(fname,File5);
				break;
			default : sleep(1);
				break;
		}

		printf("[%d]Operation Selected : %d and File Selected :%s Timestamp:%d\n",i,op,fname,timestamp);

		switch (op){
			case 1 : 
				if(get_approval(fname))
					write_to_server(fname);
				break;
			case 2 : 
				if(get_approval(fname))
					read_from_server(fname);
				break;
			case 3 : 
				enquire_from_server();
				break;
			default : sleep(1);
				break;
		}

		sp = (rand()%5)+1;
		sleep(sp);	
		i--;
	}
}

/* Create and Return new connection socket */
int getConnection(char* server_ip,int port){  
	
	int clientSocket;
	
	clientSocket = socket(AF_INET,SOCK_STREAM,0);
	printf("[+]Client Socket Created Successfully\n");
	memset(&serverAddr,'\0',sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(server_ip);
	
	if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0)
    {
        printf("[+]Server Error : Connect Failed \n");
		return -1;
    }
	printf("[+]Connected to the Server\n");
	return clientSocket;
	
	
}

void *start_listening(){
	int sockfd;
	int client_sock;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	pthread_t t[2];
	int i;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
	  printf("Error in Listening socket creation\n");
	}
	printf("[+]Listening Socket Created Successfully...\n");

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(CPORT);
	serverAddr.sin_addr.s_addr=inet_addr(Client);

	if(bind(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
		printf("[+]BindFailed\n");
	}

	listen(sockfd, 10);
	
	while(1){
		for(i=0;i<1;i++)      //can support 1 clients at a time
		{
			printf("[+]Client is listening..\n");
			
			client_sock=accept(sockfd,(struct sockaddr*)&clientAddr, &addr_size);
			if (client_sock < 0)
			{
				printf("[+]Client Connection Failed !! %d \n",client_sock);
				break;
			}else{
				printf("[+]Connected to client %d \n",client_sock);
				printf("[+]Receiving  From :%s \n",inet_ntoa(clientAddr.sin_addr));
				pthread_create(&t,NULL,com_client,&client_sock);
				running_threads++;
			}
		}
	}	
	close(sockfd);
}

void clear_log(){
	fclose(fopen(Log, "w"));
}

void main(){
	
	int i;
	//Initializing Priority Queues for files.
	for(i=0;i<6;i++){
		pq[i] = (heap_t *)calloc(1, sizeof (heap_t));	
	}
	clear_log();
	pthread_t th[1];
	pthread_create(&th,NULL,start_listening,NULL);
	sleep(5);
	do_operation();
	while (running_threads > 0)
	{
	 sleep(1);
	 running_threads--;
	}
	//request_queue(pq);
}
