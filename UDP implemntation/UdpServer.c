#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#define NMAX 100
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    
    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    printf("Server is running on port %s...\n", argv[1]);
    
    // Initialize random seed
    srand(time(NULL));
    
    while (1) {
        socklen_t client_len = sizeof(client_addr);
        
        // Receive request from client
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &client_len);
        
        if (n < 0) {
            perror("Recvfrom failed");
            continue;
        }
        
        buffer[n] = '\0';
        int count = atoi(buffer);
        
        if (count <= 0 || count > NMAX) {
            fprintf(stderr, "Invalid number received: %d\n", count);
            continue;
        }
        
        printf("Received request for %d numbers from client %s:%d\n",
               count, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        // Generate and send random numbers
        char response[BUFFER_SIZE] = "";
        for (int i = 0; i < count; i++) {
            char num[8];
            sprintf(num, "%d ", (rand() % NMAX) + 1);
            strcat(response, num);
        }
        
        // Send response to client
        if (sendto(sockfd, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("Sendto failed");
            continue;
        }
        
        printf("Sent response: %s\n", response);
    }
    
    close(sockfd);
    return 0;
}