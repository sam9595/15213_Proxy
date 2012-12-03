#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int takeurlname(const char* buff,char* urlname){
    char* first=strstr(buff,"GET ");
    char* urlend;
    bzero(urlname,sizeof(urlname));
    if(first == NULL)
        return 0;
    first+=strlen("GET ");  
    if(strncmp(first,"http:",5) == 0 ){
        first += strlen("http://");
        urlend = strstr(first," ");
        strncpy(urlname,first,strlen(first) - strlen(urlend));
        urlname[strlen(first) - strlen(urlend)] = '\0';
        return 1;
    }
    return 0;
}
int main(){
    char* buff="GET http://www.gosklfjas;dlfksd savsadfasdfkjs;fs";
    char* buff2="GET https://www.gosklfjas;dlfksd savsadfasdfkjs;fs";
    char* buff3="POST https://www.gosklfjas;dlfksd savsadfasdfkjs;fs";
    char urlname[100];
    int rvalue;
    rvalue = takeurlname(buff,urlname);
    printf("rvalue:%d %s\n",rvalue,urlname);
    rvalue = takeurlname(buff2,urlname);
    printf("rvalue:%d %s\n",rvalue,urlname);
    rvalue = takeurlname(buff3,urlname);
    printf("rvalue:%d %s\n",rvalue,urlname);
}
