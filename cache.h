# include "csapp.h"

# define CHASHED  1
# define UNCHASED 2
# define MAX_CACHE_SIZE 1049000
# define MAX_OBJECT_SIZE 102400

struct cachedata{
	char *url;
	char *data;
	int size;
	int counter;
	struct cachedata *next_cache;
	struct cachedata *prev_cache;
};

typedef struct cachedata c_data;

void Cache_checker();
void Cache_init();

int Create_cache( char *url, char *data, int size);

int Get_cachedata(char *url, int browser_fd);
