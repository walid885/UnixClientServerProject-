=gcc
CFLAGS=-Wall

all: server client

server: serveurTCP.c
	$(CC) $(CFLAGS) -o server serveurTCP.c

client: clientTCP.c
	$(CC) $(CFLAGS) -o client clientTCP.c

clean:
	rm -f server client