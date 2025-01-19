# Compiler settings
CC = gcc
CFLAGS = -Wall

# Target executables
all: UdpClient UdpServer

clientUDP: UdpClient.c
	$(CC) $(CFLAGS) -o clientUDP UdpClient.c

serveurUDP: UdpServer.c
	$(CC) $(CFLAGS) -o serveurUDP UdpServer.c

clean:
	rm -f UdpServer UdpClient

.PHONY: all clean