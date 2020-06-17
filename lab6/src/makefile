CC=gcc
CFLAGS=-std=c99 -w -lpthread -g -I./

allc : client server clean

all : client server

client : client.c common.c common.h
	$(CC) -o $@ $^ $(CFLAGS)

server : server.c common.c common.h
	$(CC) -o $@ $^ $(CFLAGS)

clean : 
	rm ./ports.txt