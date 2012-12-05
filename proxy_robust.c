/* Ming-Hsiung Hsueh msheuh
 * Shu-Hao Yu shuhaoy
 */
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>
#include "csapp.h"
#include "cache.h"

#define MAXHEADERSIZE 8192
#define MAXHOSTSIZE 1024
#define MAXBUFFERSIZE 4096

int readcnt;
sem_t mutex,w;
char ** headerSet;

void proxy_init(){
    int i;
	int lengthSet[5] = {86,73,32,19,25};

    /* Initial locks to be 1 */
    Sem_init(&mutex,0,1);
    Sem_init(&w,0,1);

    /* Initial Cache space */
/*    
    Cache_init();
*/
    /* Initial pre-set header */
	headerSet = (char**)malloc(5*sizeof(char*));
	for(i = 0 ; i < 5 ;i++){
		headerSet[i] = (char*)malloc(lengthSet[i]*sizeof(char));
	}
	strcpy(headerSet[0],"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n");
	strcpy(headerSet[1],"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
	strcpy(headerSet[2],"Accept-Encoding: gzip, deflate\r\n");
	strcpy(headerSet[3],"Connection: close\r\n");
	strcpy(headerSet[4],"Proxy-Connection: close\r\n");
}
int takeHostUriUrl(char* headerBuff, char* hostname, char* uriname, char* urlname){
    char* first;
    char* hostend;
    char* uriend;
    int urilength;
    char GET[50];
    sscanf(headerBuff,"%*s %s %*s",urlname);
    first = headerBuff;
    
    printf("%s\n",first);
    first += strlen("GET ");
    if(strncmp(first,"http:",5) == 0 ){
        first += strlen("http://");
        hostend = strstr(first,"/");
        if(hostend == NULL){
            perror("hostend null\n");
            return 0 ;
        }
        memcpy(hostname,first,hostend - first);
        hostname[hostend - first] = '\0';
        printf("first :%x hostend :%x length: %d hostname:%s\n",first,hostend,hostend-first,hostname);
        printf("hostname loc:%x hostname[0]:%c\n",hostname,hostname[0]);
        uriend = strstr(hostend," ");
        if(uriend == NULL){
            perror("uriend null\n");
            return 0;
        }
        printf("hostname loc:%x hostname[0]:%c\n",hostname,hostname[0]);
        urilength = (uriend - hostend);
        printf("urilength:%d\n",urilength);
        memcpy(uriname,hostend,urilength);
        printf("hostname loc:%x hostname[0]:%c\n",hostname,hostname[0]);
        uriname[urilength] = '\0';
        printf("hostname:%s\n",hostname);
        printf("hostname loc:%x hostname[0]:%c\n",hostname,hostname[0]);
    }
    else{
        perror("No http header\n");
        return 0 ;
    }

    printf("hostname:%s uriname:%s urlname:%s\n",hostname,uriname,urlname); 
    return 1;
}
int getHostname(char* headerBuff, char* hostname){
    char* first = strstr(headerBuff,"Host: ");
    first += strlen("Host: ");
    char* last = strstr(first,"\r\n");
    memcpy(hostname,first,last - first);
    hostname[last - first] = '\0';
    return 1;
}
int SendRequest(int clientfd, rio_t serverrio_t, char* hostname, char* uriname){
    int i,byte;
    int flag[5] = {0};
    char headerBuff[MAXHEADERSIZE]; 
    char requestLine[MAXHEADERSIZE];

    sprintf(requestLine,"GET %s HTTP/1.0\r\n",uriname);
    rio_writen(clientfd,requestLine,strlen(requestLine));
    printf("%s",requestLine);


    byte = Rio_readlineb(&serverrio_t,headerBuff,sizeof(headerBuff));
    printf("Readin: %s",headerBuff);
    if( strstr(headerBuff,"Host:") != NULL){
        getHostname(headerBuff,hostname);
    }
    sprintf(requestLine,"Host: %s\r\n",hostname);
    rio_writen(clientfd,requestLine,strlen(requestLine));
    printf("%s",requestLine);

    while( (byte = Rio_readlineb(&serverrio_t,headerBuff,sizeof(headerBuff))) >0){
        printf("Readin: %s",headerBuff);
        if(strncmp(headerBuff,"GET",3) == 0){
        }
        else if(strncmp(headerBuff,"Host",4) == 0){
        }
        else if(strncmp(headerBuff,"\r",1) == 0){
            break;
        }
      else if(strncmp(headerBuff,"User-Agent:",11) == 0){
            rio_writen(clientfd,headerSet[0],strlen(headerSet[0]));
            printf("%s",headerSet[0]);
            flag[0] = 1;
        }
        else if(strncmp(headerBuff,"Accept:",7) == 0){
            rio_writen(clientfd,headerSet[1],strlen(headerSet[1]));
            printf("%s",headerSet[1]);
            flag[1] = 1;
        }
        else if(strncmp(headerBuff,"Accept-Encoding:",16) == 0){
            rio_writen(clientfd,headerSet[2],strlen(headerSet[2]));
            printf("%s",headerSet[2]);
            flag[2] = 1;
        }
       else if(strncmp(headerBuff,"Connection:",11) == 0){
            rio_writen(clientfd,headerSet[3],strlen(headerSet[3]));
            printf("%s",headerSet[3]);
            flag[3] = 1;
        }
        else if(strncmp(headerBuff,"Proxy-Connection:",17) == 0){
            rio_writen(clientfd,headerSet[4],strlen(headerSet[4]));
            printf("%s",headerSet[4]);
            flag[4] = 1;
        }
        else{
            rio_writen(clientfd,headerBuff,strlen(headerBuff));
            printf("%s",headerBuff);
        }
    }
    for(i = 0 ; i < 5 ;i++){
        if(flag[i] == 0){
            rio_writen(clientfd,headerSet[i],strlen(headerSet[i]));
            printf("%s",headerSet[i]);
            flag[i] = 1;
        }
    }
    rio_writen(clientfd,"\r\n",strlen("\r\n"));
    printf("%s","\r\n");

}
void* request (void* data){
    pthread_detach(pthread_self());
    Signal(SIGPIPE,SIG_IGN);
	int serverfd=*(int*)data;
	int clientfd;
	char headerBuff[MAXHEADERSIZE],Buffer[MAXBUFFERSIZE];
    char hostname[300],uriname[16284],urlname[16284];
    char* cache_buff;
    int byte,getbyte,error;
    int cachehit = 0;
    int cache_size = 0;
    free(data);
    rio_t serverrio_t;
	printf("to Start read %u\n",pthread_self());
    fflush(stdout);

    Rio_readinitb(&serverrio_t,serverfd);

    byte = Rio_readlineb(&serverrio_t,headerBuff,sizeof(headerBuff));
    printf("Readin: %s",headerBuff);
    if (takeHostUriUrl(headerBuff,hostname,uriname,urlname) ==0){
        perror("Get Host Uri Url error\n");
        close(serverfd);
        pthread_exit(NULL);
    }
    
    if( (clientfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("socket");
        close(serverfd);
        pthread_exit(NULL);
    }
    /*get server's address info*/
    struct addrinfo *hostinfo;
    if( (error=getaddrinfo(hostname,"http",NULL,&hostinfo)) >0){
        perror("hostinfo");
        close(serverfd);
        close(clientfd);
        pthread_exit(NULL);
    }
    fflush(stdout);
    /* connect to the server*/
    if( (connect(clientfd,hostinfo->ai_addr,hostinfo->ai_addrlen))<0){
        perror("connect");
        close(serverfd);
        close(clientfd);
        return NULL;
    }


    SendRequest(clientfd,serverrio_t,hostname,uriname);
    while(1){
        getbyte = Rio_readn(clientfd,Buffer,sizeof(Buffer));
        if(getbyte <= 0 )
            break;
        rio_writen(serverfd,Buffer,getbyte);
    }
    close(clientfd);
    close(serverfd);
    pthread_exit(NULL);
    
}
int main(int argc,char* argv[]){
	int serverfd;
	int sockfd;
	struct sockaddr_in serv;
	struct sockaddr_in server_addr;
    socklen_t length_ptr;
	pthread_t th;
	Signal(SIGPIPE,	SIG_IGN);
    proxy_init();

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&serv,sizeof(serv));

	/*set the structure and port defined as argv[1]*/
	serv.sin_family=AF_INET;
	serv.sin_port=htons(atoi(argv[1]));
    serv.sin_addr.s_addr = INADDR_ANY;
//	inet_pton(AF_INET,"",&serv.sin_addr);

	bind(sockfd,(struct sockaddr*)&serv,sizeof(serv));
	if(listen(sockfd,1000)<0){
		perror("listen");
		exit(-1);
	}

	/*continously get the request from the browser 
	 * and build the thread to run each request*/
	while(1){
		serverfd=accept(sockfd,(struct sockaddr*)&server_addr,&length_ptr);
		if(serverfd<=0){
            perror("Error accept");
			break;
        }
		printf("serverfd:%d\n",serverfd);
        fflush(stdout);
        int *valp = (int*)malloc(sizeof(int));
        *valp = serverfd;
		pthread_create(&th,NULL,request,valp);
	}
	printf("serverfd :%d\n",serverfd);
	fflush(stdout);
	close(sockfd);
	return 0;
}







