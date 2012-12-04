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

#define MAXHEADERSIZE 16284
#define MAXHOSTSIZE 1024
#define MAXBUFFERSIZE 8192

int readcnt;
sem_t mutex,w;
char ** headerSet;

/* get url information*/
int takeurlname(const char* buff,char** urlname){
    char* first=strstr(buff,"GET ");
    char* urlend;
    int urlnameLength;
    if(first == NULL)
        return 0;
    first+=strlen("GET ");
    if(strncmp(first,"http:",5) == 0 ){
        first += strlen("http://");
        urlend = strstr(first," ");
        urlnameLength = strlen(first) - strlen(urlend);
        *urlname = (char*)malloc((urlnameLength)*sizeof(char));
        strncpy(*urlname,first,urlnameLength);
        *(*urlname + urlnameLength) = '\0';
        return 1;
    }
    return 0;
}
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
int setGETheader(const char* buff, char* request, char* hostname){
	char* first = strstr(buff,"GET");
	char* hostend;
	char* uriend;
	char* uri;
	char temp_host[100];
	int urilength;
	if(first == NULL){
		perror("No GET in HEADER\n");
		return 0;
	}
	first+=strlen("GET ");
	if(strncmp(first,"http:",5) == 0 ){
		first += strlen("http://");
		hostend = strstr(first,"/");
		strncpy(hostname,first,strlen(first)-strlen(hostend));
		hostname[strlen(first)-strlen(hostend)] = '\0';
		uriend = strstr(hostend," ");
		urilength = strlen(hostend) - strlen(uriend);
		uri = (char*)malloc((urilength+1)*sizeof(char));
		strncpy(uri,hostend,urilength);
        uri[urilength] = '\0';

		strcat(request,"GET ");
		strcat(request,uri);
		strcat(request," HTTP/1.0\r\n");
        free(uri);
	}
	/* Not http request, return zero*/
	else{
		return 0;
	}

    /* Check if the browser has already specified the Host name */
	if( takehostname(buff,temp_host) ){
        strcat(request,"Host: ");
        strcat(request,temp_host);
        strcat(request,"\r\n");
		return 1;
	}
	else{
		strcat(request,"Host: ");
		strcat(request,hostname);
		strcat(request,"\r\n");
		return 1;
	}
}
/*convert the Proxy-connection:keep-alive to close*/
int settherequest(char* request,char* newRequestHeader, char* hostname){

	char* pch;
	char *first;
	int i = 0 ;

	int flag[5] = {0};

	bzero(newRequestHeader,sizeof(newRequestHeader));



	//Get host name and rewrite GET header
	if( setGETheader(request,newRequestHeader,hostname) == 0){
		perror("Error in Header\n");
		return 0;
	}
	printf("set GET header dont\n");



	first=request;
	pch=strstr(request,"\n");
	if(pch==NULL)
		return 0 ;
	else
		*pch='\0';
	while(1){
        /*GET and HOST line has been set*/


		if(strncmp(first,"GET",3) == 0){
		}
		else if(strncmp(first,"Host",4) == 0){
		}
        else if(strncmp(first,"\r",1) == 0){
        }
/*		else if(strncmp(first,"User-Agent:",11) == 0){
			strcat(newRequestHeader,headerSet[0]);
			flag[0] = 1;
		}
		else if(strncmp(first,"Accept:",7) == 0){
			strcat(newRequestHeader,headerSet[1]);
			flag[1] = 1;
		}
		else if(strncmp(first,"Accept-Encoding:",16) == 0){
			strcat(newRequestHeader,headerSet[2]);
			flag[2] = 1;
		}
*/		else if(strncmp(first,"Connection:",11) == 0){
			strcat(newRequestHeader,headerSet[3]);
			flag[3] = 1;
		}
		else if(strncmp(first,"Proxy-Connection:",17) == 0){
			strcat(newRequestHeader,headerSet[4]);
			flag[4] = 1;
		}
		else{
			strcat(newRequestHeader,first);
			strcat(newRequestHeader,"\n");
		}
		first=pch+1;
		pch=strstr(first,"\n");
		if(pch==NULL)
			break;
		else
			*pch='\0';
	}
	for(i = 3 ; i < 5 ;i++){
		if(flag[i] == 0 ){
			strcat(newRequestHeader,headerSet[i]);
			flag[i] = 1;
		}
	}
    strcat(newRequestHeader,"\r\n");
	return 1;
}


void* request (void* data){
    pthread_detach(pthread_self());
	int serverfd=*(int*)data;
	int clientfd;
	char headerBuff[MAXHEADERSIZE];
    char RequestHeader[MAXHEADERSIZE];
	char hostname[MAXHOSTSIZE];
	char Buffer[MAXBUFFERSIZE];
    char* urlname; 
    char* cache_buff;
    int cache_size = 0;
	int error;
	int byte;
	int getbyte;
    int cachehit = 0;
	Signal(SIGPIPE,	SIG_IGN);
    free(data);
//    rio_t serverrio_t,clientrio_t;
//    Rio_readinitb(&serverrio_t,serverfd);
	/*set the structure, and set port to 80*/ 
	printf("to Start read %u\n",pthread_self());
//	byte=rio_readn(serverfd,headerBuff,sizeof(headerBuff));
	byte=read(serverfd,headerBuff,sizeof(headerBuff));
	printf("finish read byte:%d  %u\n",byte,pthread_self());
	if(byte<=0){
		close(serverfd);
        pthread_exit(NULL);
	}
	printf("%s\n",headerBuff);
	fflush(stdout);

    /*Check if the webpage is cached. if cache hit, no need to connect to server*/
    if(takeurlname(headerBuff,&urlname) == 0){
        perror("Cannot find url name\n");
        close(serverfd);
        pthread_exit(NULL);
    }

	printf("Try to get cached data: %u\n",pthread_self());
    /* Try to get cached data, must check no thread writing the cache,
     * and use FILO architecture to lock when there threads reading cache*/
/*
    P(&mutex);
    readcnt++;
    if(readcnt == 1)
        P(&w);
    V(&mutex);
    if( Get_cachedata(urlname,serverfd) > 0){
        cachehit = 1;
    }

    P(&mutex);
    readcnt--;
    if(readcnt == 0)
        V(&w);
    V(&mutex);
    
    if(cachehit){
        close(serverfd);
        pthread_exit(NULL);
    }
*/
	printf("No cached data try connect: %u\n",pthread_self());
    /* Webpage not cached, retrieve information from web server */

	if( (clientfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
        close(serverfd);
        pthread_exit(NULL);
	} 
//    Rio_readinitb(&clientrio_t,clientfd);


    /* Set the request to the server */ 
	if( settherequest(headerBuff,RequestHeader,hostname) == 0){
		perror("Set Request Error\n");
        close(serverfd);
        pthread_exit(NULL);
	}
	printf("%s\n",RequestHeader);
	fflush(stdout);
   
    /*get server's address info*/
	struct addrinfo *hostinfo;
	if( (error=getaddrinfo(hostname,"http",NULL,&hostinfo)) >0){
		perror("hostinfo");
        close(serverfd);
        pthread_exit(NULL);
	}
    /* connect to the server*/
	if( (connect(clientfd,hostinfo->ai_addr,hostinfo->ai_addrlen))<0){
		perror("connect");
        close(serverfd);
		return NULL;
	}
	printf("Ready to transmitt data: %u\n",pthread_self());
	/*pass the request from browser to server*/
	Rio_writen(clientfd,RequestHeader,byte);
	/*read the information from web server and send it to browser*/

   cache_buff = (char*) malloc(MAX_OBJECT_SIZE*sizeof(char));
    bzero(cache_buff,MAX_OBJECT_SIZE);
	while(1){
		getbyte=read(clientfd,Buffer,sizeof(Buffer));
//		getbyte=Rio_readn(clientfd,Buffer,sizeof(Buffer));
		if(getbyte<=0)
			break;
        if(cache_size + getbyte < MAX_OBJECT_SIZE){
            memcpy(cache_buff + cache_size,Buffer,getbyte);
        }
        cache_size += getbyte;
		Rio_writen(serverfd,Buffer,getbyte);
	}
	printf("Finish transmitt data: %u\n",pthread_self());
    /* if size fit in cache, write into cache, else free the content*/
/*    
    if(cache_size < MAX_OBJECT_SIZE){
        char* new_cache_buff = realloc(cache_buff,cache_size);
        free(cache_buff);
        if(new_cache_buff == NULL){
            perror("Realloc fail\n");
	        close(clientfd);
        	close(serverfd);
            pthread_exit(NULL);
        }

        P(&w);
        Create_cache(urlname,new_cache_buff,cache_size);
        Cache_checker();
        V(&w);
        
    }
    else{
        free(cache_buff);
    }
*/	printf("Finish cached data: %u\n",pthread_self());
	close(clientfd);
	close(serverfd);
    pthread_exit(NULL);
}
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
	inet_pton(AF_INET,"127.0.0.1",&serv.sin_addr);

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
        int  *valp;
        while (  (valp = (int*)malloc(sizeof(int))) ==NULL){
        }
//        int *valp = (int*)malloc(sizeof(int));
        *valp = serverfd;
		pthread_create(&th,NULL,request,valp);
	}
	printf("serverfd :%d\n",serverfd);
	fflush(stdout);
	close(sockfd);
	return 0;
}







