CC = gcc
CFLAGS = -Wall

client: client.c client.h chatInfo.h
	$(CC) $(CFLAGS) client.c -o client

server: server.c server.h chatInfo.h
	$(CC) $(CFLAGS) server.c -o server

all: server client

clean:
	rm -f *.o server client
	
