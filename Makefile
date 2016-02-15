CC=gcc
GC=g++
CFLAGS=-g
LFLAGS=-pthread
OBJS=app/server.o
OBJS2=client/client.o

all: server client

server: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o app/server

app/server.o: app/server.c
	$(CC) $(CFLAGS) -c app/server.c -o app/server.o


client: $(OBJS2)
	$(GC) $(CFLAGS) $(OBJS2) -o client/client

client/client.o: client/client.cpp
	$(GC) $(CFLAGS) -c client/client.cpp -o client/client.o

