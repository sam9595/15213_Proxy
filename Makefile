CC = gcc
#CFLAGS = -Wall -Wextra -Werror -O2 -g -DDRIVER -std=gnu99
CFLAGS = -Wall -g
LDFLAGS = -pthread

all: proxy

mdriver: $(OBJS)
	    $(CC) $(CFLAGS) -o mdriver $(OBJS)
OBJS = proxy.o cache.o csapp.o

csapp.o:	csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c
proxy.o:	proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c
cache.o:	cache.c cache.h csapp.h
	$(CC) $(CFLAGS) -c cache.c


submit:
	(make clean; cd ..; tar cvf proxylab.tar proxylab)

proxy:	proxy.o csapp.o cache.o

all: proxy

clean:
	 rm -f *~ *.o proxy core
