CC=gcc
CFLAGS=-std=c99 -w -lpthread

all : deadlock paralellFactorialMod

deadlock : deadlockDemo.c
	$(CC) -o $@ $^ $(CFLAGS)

paralellFactorialMod : paralellFactorialMod.c
	$(CC) -o $@ $^ $(CFLAGS)