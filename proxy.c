// msheuh
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>
	/*get to hostname information*/
int takehostname(const char* buff,char* hostname){
	char* first=strstr(buff,"Host: ");
	if(first==NULL)
		return 0;
	first+=strlen("Host: ");
	char* last=strstr(first,"\r\n");
	strncpy(hostname,first,strlen(first)-strlen(last));
	hostname[strlen(first)-strlen(last)]='\0';
	return 1;
}
	/*convert the Proxy-connection:keep-alive to close*/
void settherequest(char* request){
	char buff[100000];
	char* pch;
	char *first,*second,*third;
	int len;
	strcpy(buff,request);
	request[0]='\0';
	first=buff;
	pch=strstr(buff,"\n");
	if(pch==NULL)
		return;
	else
		*pch='\0';
	while(1){
		if((strstr(first,"Proxy-Connection:"))){
			strcat(request,"Proxy-Connection: close\n");
		}
		else{
			strcat(request,first);
			strcat(request,"\n");
		}
		first=pch+1;
		pch=strstr(first,"\n");
		if(pch==NULL)
			break;
		else
			*pch='\0';
	}
	return;
}


void request (void* data){
	int serverfd=*(int*)data;
	int clientfd;
	struct sockaddr_in clit;
	struct hostent* hp;
	char buff[100000];
	char buff2[100000];
	char hostname[512];
	char get[40960];
	int byte;
	int getbyte;
	int sendbyte;
	if( (clientfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		exit(-1);
	}

	bzero(&buff,sizeof(buff));
	/*set the structure, and set port to 80*/ 
	clit.sin_family=AF_INET;
	clit.sin_port=htons(80);
	//printf("beforeread\n");
	byte=read(serverfd,buff,sizeof(buff));
	//printf("%s\n",buff);
	if(byte<=0){
		close(clientfd);
		close(serverfd);
		return ;
	}
	printf("%s\n",buff);
	//printf("beforegethostname\n");
	
	/*to take hostname in the HEADER*/
	if(takehostname(buff,hostname)==0){
		close(clientfd);
		close(serverfd);
		return ;
	}
	/*convert the Proxy-connection:keep-alive to close*/
	settherequest(buff);
	//printf("buff is \n%s",buff);
	if( (hp=(struct hostent*)gethostbyname(hostname))==NULL){
		printf("hello\n");
		perror("gethostbyname");
		return;
		exit(-1);
	}
	clit.sin_addr=*((struct in_addr*)hp->h_addr);
	printf("server IP %s\n",inet_ntoa(clit.sin_addr));
	if( (connect(clientfd,(struct sockaddr*)&clit,sizeof(clit)))<0){
		perror("connect");
		exit(-1);
	}
	/*pass the request from browser to server*/
	write(clientfd,buff,byte);
	/*read the information from web server and send it to browser*/
	while(1){
		getbyte=read(clientfd,get,sizeof(get));
		if(getbyte<=0)
			break;
		sendbyte=write(serverfd,get,getbyte);
	}
	close(clientfd);
	close(serverfd);
	return ;
}


int main(int argc,char* argv[]){
	int serverfd;
	struct sockaddr_in serv;
	int sockfd;
	struct sockaddr_in server_addr;
	int addrlen=sizeof(server_addr);
	pthread_t th;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&serv,sizeof(serv));
	/*set the structure and port defined as argv[1]*/
	serv.sin_family=AF_INET;
	serv.sin_port=htons(atoi(argv[1]));
	inet_pton(AF_INET,"127.0.0.1",&serv.sin_addr);
//	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	printf("address :%s\n",inet_ntoa(serv.sin_addr));
	bind(sockfd,(struct sockaddr*)&serv,sizeof(serv));
	if(listen(sockfd,1000)<0){
		perror("listen");
		exit(-1);
	}
	/*continously get the request from the browser 
	 * and build the thread to run each request*/
	while(1){
		printf("wait request\n");
		serverfd=accept(sockfd,(struct sockaddr*)&server_addr,&addrlen);
		if(serverfd<=0)
			break;
		pthread_create(&th,NULL,&request,(void*)&serverfd);
	}
	close(sockfd);
	return 0;
}







