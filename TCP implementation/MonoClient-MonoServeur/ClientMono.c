#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 1024

void print_menu() {
    printf("\nAvailable services:\n");
    printf("1. Get server date and time\n");
    printf("2. List directory contents\n");
    printf("3. Read file content\n");
    printf("4. Get connection duration\n");
    printf("5. Exit\n");
    printf("Choose service (1-5): ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER] = {0};
    char username[50], password[50];
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    // Authentication
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    sprintf(buffer, "%s %s", username, password);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, MAX_BUFFER, 0);
    
    if (strncmp(buffer, "Authentication failed", 19) == 0) {
        printf("Authentication failed\n");
        close(sock);
        return 1;
    }
    
    printf("Authentication successful\n");
    
    while (1) {
        print_menu();
        
        int choice;
        scanf("%d", &choice);
        
        if (choice == 5) {
            send(sock, "exit", 4, 0);
            break;
        }
        
        sprintf(buffer, "%d", choice);
        send(sock, buffer, strlen(buffer), 0);
        
        if (choice == 3) {
            printf("Enter filename: ");
            scanf("%s", buffer);
            send(sock, buffer, strlen(buffer), 0);
        }
        
        memset(buffer, 0, MAX_BUFFER);
        recv(sock, buffer, MAX_BUFFER, 0);
        printf("\nServer response:\n%s\n", buffer);
    }
    
    close(sock);
    return 0;
}