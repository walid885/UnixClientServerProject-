#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void display_menu() {
    printf("\nAvailable services:\n");
    printf("1. Get current date and time\n");
    printf("2. List directory contents\n");
    printf("3. Read file content\n");
    printf("4. Exit\n");
    printf("Choose a service: ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char username[50], password[50];
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    // Connect to server
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    // Authentication
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    write(sock, username, strlen(username));
    write(sock, password, strlen(password));
    
    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);
    
    if (strncmp(buffer, "Authentication failed", 19) == 0) {
        close(sock);
        return 1;
    }
    
    // Service loop
    while (1) {
        display_menu();
        
        char choice[2];
        scanf("%s", choice);
        write(sock, choice, strlen(choice));
        
        if (strcmp(choice, "4") == 0) {
            break;
        }
        
        if (strcmp(choice, "3") == 0) {
            printf("Enter filename: ");
            char filename[100];
            scanf("%s", filename);
            write(sock, filename, strlen(filename));
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("\nServer response:\n%s\n", buffer);
    }
    
    close(sock);
    return 0;
}