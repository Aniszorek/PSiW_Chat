CC = gcc
CFLAGS = -Wall

client: inf151805_inf151891_k.c inf151805_inf151891_k.h inf151805_inf151891_chatInfo.h
	$(CC) $(CFLAGS) inf151805_inf151891_k.c -o client

server: inf151805_inf151891_s.c inf151805_inf151891_s.h inf151805_inf151891_chatInfo.h
	$(CC) $(CFLAGS) inf151805_inf151891_s.c -o server

all: server client

clean:
	rm -f *.o server client
	
