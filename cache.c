/* Ming-Hsiung Hsueh msheuh
 * Shu-Hao Yu shuhaoy
 */


#include "cache.h"

c_data** CacheHead;
int globalcounter;
int hashNum;
int CachedSize;
void Cache_init(){
    int i;
    hashNum = 20;
    CacheHead = (c_data**)malloc(hashNum * sizeof(c_data*));
    for(i = 0 ; i < hashNum ;i++){
        CacheHead[i] = NULL;
    }
    globalcounter = 0;
    CachedSize = 0;
}
void Cache_checker(){
    int i;
    c_data* curr;
    for(i = 0 ; i < hashNum ; i++){
        curr = CacheHead[i];
        while(curr != NULL){
            if(curr->url == NULL){
                printf("URL is NULL\n");
            }
            else{
                printf("cached URL :%s\n",curr->url);
            }
            if(curr->data == NULL){
                printf("cache data is NULL\n");
            }
            printf("cached size:%d\n",curr->size);
            printf("cached counter:%d\n",curr->counter);
            curr = curr->next_cache;
        }
    }
    fflush(stdout);
    return;
}



int hashfunction(char* url){
    int hash = 0;
    while(*url != '\0'){
        hash = 0x3FFFFFFF & (hash + *url + (*(url+1)<<7));
        url++;
    }
    return (hash % hashNum);
}
int DeleteLRUCache(){
    int i;
    c_data* curr;
    c_data* LRU_cache;
    c_data** LRU_head;
    printf("DELETE LRU CACHE!!!!!!!!!\n");
    int minimunCounter = globalcounter;
    for(i = 0 ; i < hashNum ;i++){
        curr = CacheHead[i];
        while(curr != NULL){
            if(curr->counter < minimunCounter){
                minimunCounter = curr->counter;
                LRU_head = &CacheHead[i];
                LRU_cache = curr;
            }
            curr = curr->next_cache;
        }
    }
    if(LRU_cache->prev_cache == NULL){
        if(LRU_cache->next_cache !=NULL){
            LRU_cache->next_cache->prev_cache = NULL;
            *LRU_head = LRU_cache->next_cache;
        }
        else{
            *LRU_head = NULL;
        }
    }
    else if(LRU_cache->next_cache == NULL){
        LRU_cache->prev_cache->next_cache = NULL;
    }
    else{
        LRU_cache->prev_cache->next_cache = LRU_cache->next_cache;
        LRU_cache->next_cache->prev_cache = LRU_cache->prev_cache;
    }
    printf("size :%d LRU URL:%s\n",LRU_cache->size,LRU_cache->url);

    CachedSize -= LRU_cache->size;
    //    free(LRU_cache->url);
    //  free(LRU_cache->data);
    free(LRU_cache);
    return 1;
}
int Create_cache( char *url, char *data, int size){

    while (CachedSize + size > MAX_CACHE_SIZE){
        DeleteLRUCache();
    }



    c_data* cache = (c_data*)malloc(sizeof(c_data));
    int n = hashfunction(url);
    cache->url = url;
    cache->data = data;
    cache->size = size;
    cache->counter = globalcounter++;
    cache->prev_cache = NULL;
    cache->next_cache = NULL;
    if(CacheHead[n] == NULL){
        CacheHead[n] = cache;
    }
    else{
        CacheHead[n]->prev_cache = cache;
        cache->next_cache = CacheHead[n];
        CacheHead[n] = cache;
    }
    CachedSize += size;
    return 1;
}
int Get_cachedata(char *url, int browser_fd){
    int n = hashfunction(url);
    c_data* curr = CacheHead[n];
    while(curr!=NULL){
        if(!strcmp(url,curr->url)){
            write(browser_fd,curr->data,curr->size);
            return 1;
        }
        curr = curr->next_cache;
    }
    return 0;
}
