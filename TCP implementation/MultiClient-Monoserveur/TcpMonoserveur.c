#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <asm-generic/socket.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
} Auth;

Auth valid_users[] = {
    {"user1", "pass1"},
    {"user2", "pass2"},
    {"user3", "pass3"},
    {"user4", "pass4"},


};

// Function to authenticate user
int authenticate(char *username, char *password) {
    int num_users = sizeof(valid_users) / sizeof(Auth);
    for (int i = 0; i < num_users; i++) {
        if (strcmp(username, valid_users[i].username) == 0 &&
            strcmp(password, valid_users[i].password) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to get current date and time
void get_datetime(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
}

// Function to list directory contents
void list_directory(char *dir_path, char *buffer) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    buffer[0] = '\0';
    
    if (dir == NULL) {
        strcpy(buffer, "Error opening directory");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        strcat(buffer, entry->d_name);
        strcat(buffer, "\n");
    }
    closedir(dir);
}

// Function to read file content
void read_file(char *filename, char *buffer) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        strcpy(buffer, "Error opening file");
        return;
    }
    
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
    
    printf("Server listening on port %d\n", PORT);
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (fork() == 0) {  // Child process
            close(server_fd);
            
            // Handle authentication
            char username[50], password[50];
            read(client_socket, username, sizeof(username));
            read(client_socket, password, sizeof(password));
            
            if (!authenticate(username, password)) {
                write(client_socket, "Authentication failed", 20);
                close(client_socket);
                exit(0);
            }
            write(client_socket, "Authentication successful", 24);
            
            // Handle service requests
            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE);
                
                if (strcmp(buffer, "1") == 0) {  // DateTime service
                    get_datetime(buffer);
                } else if (strcmp(buffer, "2") == 0) {  // List directory
                    list_directory(".", buffer);
                } else if (strcmp(buffer, "3") == 0) {  // Read file
                    read(client_socket, buffer, BUFFER_SIZE);  // Get filename
                    read_file(buffer, buffer);
                } else if (strcmp(buffer, "4") == 0) {  // Exit
                    break;
                }
                
                write(client_socket, buffer, strlen(buffer));
            }
            
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    
    return 0;
}