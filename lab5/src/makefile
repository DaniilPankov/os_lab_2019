CC=gcc

all: without_mutex with_mutex factorial deadlock

without_mutex:
	$(CC) -o without_mutex without_mutex.c -lpthread

with_mutex:
	$(CC) -o with_mutex with_mutex.c -lpthread

factorial:
	$(CC) -o factorial factorial.c -lpthread

deadlock:
	$(CC) -o deadlock deadlock.c -lpthread

clean:
	rm without_mutex with_mutex factorial deadlock