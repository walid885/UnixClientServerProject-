CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

all: server client

server: TcpMonoserveur.c
	$(CC) $(CFLAGS) -o server TcpMonoserveur.c $(LDFLAGS)

client: TcpMultiClient.c
	$(CC) $(CFLAGS) -o client TcpMultiClient.c

clean:
	rm -f server client