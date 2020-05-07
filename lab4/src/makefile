CC=gcc
CFLAGS=-I. -I../../lab3/src -std=c99
TOLAB3=cd ../../lab3/src
TOLAB4=cd ../../lab4/src

all: utils.o find_min_max.o parallel_sum parallel_min_max clean

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(TOLAB3)
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)
	$(TOLAB4)

utils.o : utils.h
	$(TOLAB3)
	$(CC) -o utils.o -c utils.c $(CFLAGS)
	$(TOLAB4)

find_min_max.o : utils.h find_min_max.h
	$(TOLAB3)
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)
	$(TOLAB4)

parallel_sum : parallel_sum.c
	$(CC) -o parallel_sum parallel_sum.c $(CFLAGS)

clean :
	$(TOLAB3)
	rm utils.o find_min_max.o
	$(TOLAB4)

cleanAll :
	rm parallel_sum
	$(TOLAB3)
	rm utils.o find_min_max.o sequential_min_max parallel_min_max
	$(TOLAB4)