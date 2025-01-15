CC = gcc
CFLAGS = -Wall

all: server client

server: ServerMono.c
	$(CC) $(CFLAGS) -o server ServerMono.c

client: ClientMono.c
	$(CC) $(CFLAGS) -o client ClientMono.c

clean:
	rm -f server client