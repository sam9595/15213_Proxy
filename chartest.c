#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(){
    char a[5] = "abcde";
    char *b;
    b = (char*)malloc(5*sizeof(char));
    *(b+5) = 'f';
//    strncpy(b,"abcde",5);
    strcat(b,"abcde");
    printf("%s\n",a);
    printf("%s\n",b);
}
