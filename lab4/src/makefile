CC=gcc
CFLAGS=-I.

all : parallel_min_max process_memory

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

process_memory : 
	$(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_sum : lib_sum utils.o utils.h 
	$(CC) -o parallel_sum parallel_sum.c parallel_sum.h utils.o -lpthread -lsum -L=./ $(CFLAGS)

lib_sum : ./SUM/sum.c
	gcc -o ./SUM/sum.o -c ./SUM/sum.c -I./SUM
	ar rcs ./libsum.a ./SUM/sum.o

clean :
	rm utils.o find_min_max.o parallel_min_max process_memory


