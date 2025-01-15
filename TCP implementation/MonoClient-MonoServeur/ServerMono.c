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
#define MAX_BUFFER 1024
#define MAX_CLIENTS 5

typedef struct {
    char username[50];
    char password[50];
    time_t connection_time;
} Client;

void get_system_time(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d", 
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
}

void list_directory(char *buffer, const char *path) {
    DIR *d = opendir(path);
    struct dirent *dir;
    buffer[0] = '\0';
    
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strcat(buffer, dir->d_name);
            strcat(buffer, "\n");
        }
        closedir(d);
    }
}

void read_file_content(char *buffer, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        size_t bytes_read = fread(buffer, 1, MAX_BUFFER - 1, file);
        buffer[bytes_read] = '\0';
        fclose(file);
    } else {
        strcpy(buffer, "Error: File not found");
    }
}

int authenticate(const char *username, const char *password) {
    // Simple authentication (in practice, use secure storage)
    return (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER] = {0};
    
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
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (fork() == 0) {
            close(server_fd);
            Client client;
            client.connection_time = time(NULL);
            
            // Authentication
            recv(new_socket, buffer, MAX_BUFFER, 0);
            sscanf(buffer, "%s %s", client.username, client.password);
            
            if (!authenticate(client.username, client.password)) {
                send(new_socket, "Authentication failed", 20, 0);
                close(new_socket);
                exit(0);
            }
            
            send(new_socket, "Authentication successful", 23, 0);
            
            while (1) {
                memset(buffer, 0, MAX_BUFFER);
                recv(new_socket, buffer, MAX_BUFFER, 0);
                
                if (strcmp(buffer, "1") == 0) {
                    get_system_time(buffer);
                } else if (strcmp(buffer, "2") == 0) {
                    list_directory(buffer, ".");
                } else if (strcmp(buffer, "3") == 0) {
                    recv(new_socket, buffer, MAX_BUFFER, 0);
                    read_file_content(buffer, buffer);
                } else if (strcmp(buffer, "4") == 0) {
                    time_t now = time(NULL);
                    sprintf(buffer, "Connection duration: %ld seconds", now - client.connection_time);
                } else if (strcmp(buffer, "exit") == 0) {
                    break;
                }
                
                send(new_socket, buffer, strlen(buffer), 0);
            }
            
            close(new_socket);
            exit(0);
        }
        close(new_socket);
    }
    
    return 0;
}