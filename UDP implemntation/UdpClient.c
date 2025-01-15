#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define NMAX 100  // Maximum random number

int main(int argc, char *argv[]) {
    // Validate command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Initialize server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    // Generate random number between 1 and NMAX
    srand(time(NULL));
    int random_num = (rand() % NMAX) + 1;
    printf("Generated random number: %d\n", random_num);

    // Send number to server
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%d", random_num);
    if (sendto(sock, buffer, strlen(buffer), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        exit(1);
    }

    // Receive response from server
    int numbers[NMAX];
    socklen_t server_len = sizeof(server_addr);
    ssize_t received = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                               (struct sockaddr *)&server_addr, &server_len);
    
    if (received < 0) {
        perror("Receive failed");
        exit(1);
    }
    buffer[received] = '\0';

    // Display received numbers
    printf("Received from server: %s\n", buffer);

    close(sock);
    return 0;
}