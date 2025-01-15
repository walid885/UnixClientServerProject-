CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

all: server client

server: MultiServer.c
	$(CC) $(CFLAGS) -o server MultiServer.c $(LDFLAGS)

client: Multiclient.c
	$(CC) $(CFLAGS) -o client Multiclient.c

clean:
	rm -f server client