#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void print_menu() {
    printf("\n=== Available Services ===\n");
    printf("1. Get server time\n");
    printf("2. List directory contents\n");
    printf("3. Read file content\n");
    printf("4. Get connection duration\n");
    printf("5. Exit\n");
    printf("Choose a service (1-5): ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }
    
    // Authentication
    char username[50], password[50];
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    sprintf(buffer, "%s %s", username, password);
    send(sock, buffer, strlen(buffer), 0);
    
    int valread = recv(sock, buffer, BUFFER_SIZE, 0);
    buffer[valread] = '\0';
    
    if (strncmp(buffer, "Authentication failed", 19) == 0) {
        printf("Authentication failed\n");
        close(sock);
        return -1;
    }
    
    printf("Authentication successful\n");
    
    int choice;
    while (1) {
        print_menu();
        scanf("%d", &choice);
        
        if (choice < 1 || choice > 5) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        
        sprintf(buffer, "%d", choice);
        send(sock, buffer, strlen(buffer), 0);
        
        if (choice == 5) {
            printf("Goodbye!\n");
            break;
        }
        
        if (choice == 3) {
            printf("Enter filename: ");
            scanf("%s", buffer);
            send(sock, buffer, strlen(buffer), 0);
        }
        
        valread = recv(sock, buffer, BUFFER_SIZE, 0);
        buffer[valread] = '\0';
        printf("\nServer response:\n%s\n", buffer);
    }
    
    close(sock);
    return 0;
}