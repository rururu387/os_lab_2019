CC=gcc
CFLAGS=-std=c99 -w

all : tcpClient tcpServer udpClient udpServer

tcpClient : tcpclient.c
	$(CC) -o $@ $^ $(CFLAGS)

tcpServer : tcpserver.c
	$(CC) -o $@ $^ $(CFLAGS)

udpClient : udpclient.c
	$(CC) -o $@ $^ $(CFLAGS)

udpServer : udpserver.c
	$(CC) -o $@ $^ $(CFLAGS)