CC=gcc
CFLAGS=-Wall

all: server client

server: TcpMonoserveur.c
	$(CC) $(CFLAGS) -o server TcpMonoserveur.c

client: TcpMultiClient.c
	$(CC) $(CFLAGS) -o client TcpMultiClient.c

clean:
	rm -f server client *.o
