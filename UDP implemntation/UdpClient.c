#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define NMAX 100
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_host> <port>\n", argv[0]);
        exit(1);
    }

    // Initialize random seed with current time and process ID
    srand(time(NULL) ^ (getpid()<<16));
    
    // Generate random number between 1 and NMAX
    int number = (rand() % NMAX) + 1;
    
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Get server information
    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "Unknown host %s\n", argv[1]);
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(atoi(argv[2]));
    
    printf("Generated random number: %d\n", number);
    
    // Convert number to string and send to server
    sprintf(buffer, "%d", number);
    if (sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Sendto failed");
        exit(1);
    }
    
    // Set timeout for receiving response (5 seconds)
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Set socket timeout failed");
        exit(1);
    }
    
    // Receive response from server
    socklen_t server_len = sizeof(server_addr);
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&server_addr, &server_len);
    
    if (n < 0) {
        perror("Recvfrom failed");
        exit(1);
    }
    
    buffer[n] = '\0';
    printf("Received from server: %s\n", buffer);
    
    close(sockfd);
    return 0;
}