#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){
	int i = 0 ;
	int lengthSet[5] = {86,73,32,19,25};
	char ** headerSet;
	headerSet = (char**)malloc(5*sizeof(char*));
	for(i = 0 ; i < 5 ;i++){
		headerSet[i] = (char*)malloc(lengthSet[i]*sizeof(char));
	}
	strcpy(headerSet[0],"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n");
	strcpy(headerSet[1],"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
	strcpy(headerSet[2],"Accept-Encoding: gzip, deflate\r\n");
	strcpy(headerSet[3],"Connection: close\r\n");
	strcpy(headerSet[4],"Proxy-Connection: close\r\n");
	for(i = 0 ; i < 5 ;i++){
		printf("%s\n",headerSet[i]);
	}
/*	
	printf("%d\n",strlen("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n"));
	printf("%d\n",strlen("Accept-Encoding: gzip, deflate\r\n"));
	printf("%d\n",strlen("Connection: close\r\n"));
	printf("%d\n",strlen("Proxy-Connection: close\r\n"));
*/
}
