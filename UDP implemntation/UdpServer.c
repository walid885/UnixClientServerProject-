#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define NMAX 100

int main(int argc, char *argv[]) {
    // Validate command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Initialize server address structure
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("Server is running on port %s...\n", argv[1]);
    
    // Initialize random number generator
    srand(time(NULL));

    while (1) {
        char buffer[BUFFER_SIZE];
        socklen_t client_len = sizeof(client_addr);

        // Receive number from client
        ssize_t received = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr *)&client_addr, &client_len);
        if (received < 0) {
            perror("Receive failed");
            continue;
        }
        buffer[received] = '\0';
        
        // Convert received string to integer
        int n = atoi(buffer);
        printf("Received number from client: %d\n", n);

        // Generate n random numbers
        char response[BUFFER_SIZE] = "";
        for (int i = 0; i < n; i++) {
            char num_str[20];
            sprintf(num_str, "%d ", (rand() % NMAX) + 1);
            strcat(response, num_str);
        }

        // Send response back to client
        if (sendto(sock, response, strlen(response), 0,
                  (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("Send failed");
        }
    }

    close(sock);
    return 0;
}