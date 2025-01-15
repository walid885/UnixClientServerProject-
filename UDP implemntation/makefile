# Compiler settings
CC = gcc
CFLAGS = -Wall

# Target executables
all: clientUDP serveurUDP

clientUDP: clientUDP.c
	$(CC) $(CFLAGS) -o clientUDP clientUDP.c

serveurUDP: serveurUDP.c
	$(CC) $(CFLAGS) -o serveurUDP serveurUDP.c

clean:
	rm -f clientUDP serveurUDP

.PHONY: all clean