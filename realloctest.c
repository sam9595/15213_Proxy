#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
    char* a = (char*) malloc(10*sizeof(char));
    strcpy(a,"abcds");
    printf("%s\n",a);
    a = realloc(a,5);
    printf("%s\n",a);
}
