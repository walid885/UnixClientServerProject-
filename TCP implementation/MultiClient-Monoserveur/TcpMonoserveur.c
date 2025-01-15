#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <asm-generic/socket.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define NUM_VALID_USERS (sizeof(valid_users) / sizeof(user_credentials_t))

typedef struct {
    char username[50];
    char password[50];
} user_credentials_t;

user_credentials_t valid_users[] = {
    {"admin", "admin"},
    {"usr1", "usr1"},
    {"usr2", "usr2"}
};

typedef struct {
    int sock;
    struct sockaddr_in address;
    time_t connection_time;
} client_t;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

void get_time(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d", 
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
}

void list_directory(char *buffer) {
    DIR *d;
    struct dirent *dir;
    buffer[0] = '\0';
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strcat(buffer, dir->d_name);
            strcat(buffer, "\n");
        }
        closedir(d);
    }
}

void read_file(char *buffer, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
        buffer[bytes_read] = '\0';
        fclose(file);
    } else {
        strcpy(buffer, "Error: File not found\n");
    }
}

int authenticate(char *credentials) {
    char username[50], password[50];
    sscanf(credentials, "%s %s", username, password);
    
    for (size_t i = 0; i < NUM_VALID_USERS; i++) {
        if (strcmp(username, valid_users[i].username) == 0 && 
            strcmp(password, valid_users[i].password) == 0) {
            return 1;
        }
    }
    return 0;
}


const char* get_username(char *credentials) {
    static char username[50];
    sscanf(credentials, "%s", username);
    return username;
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;
    char username[50] = {0};  // Store username for logging

    pthread_mutex_lock(&clients_mutex);
    client_count++;
    printf("New connection attempt. Total clients: %d\n", client_count);
    pthread_mutex_unlock(&clients_mutex);

    // Authentication
    read_size = recv(client->sock, buffer, BUFFER_SIZE, 0);
    buffer[read_size] = '\0';

    // Extract username before authentication
    sscanf(buffer, "%s", username);

    if (!authenticate(buffer)) {
        printf("Authentication failed for user: %s\n", username);
        send(client->sock, "Authentication failed", 20, 0);
        close(client->sock);
        pthread_mutex_lock(&clients_mutex);
        client_count--;
        pthread_mutex_unlock(&clients_mutex);
        free(client);
        return NULL;
    }

    printf("User %s successfully authenticated\n", username);
    send(client->sock, "Authentication successful", 23, 0);
    client->connection_time = time(NULL);

    // Service handling loop
    while ((read_size = recv(client->sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        
        printf("User %s requested service: ", username);
        
        if (strcmp(buffer, "1") == 0) {
            printf("Get Time\n");
            get_time(buffer);
        }
        else if (strcmp(buffer, "2") == 0) {
            printf("List Directory\n");
            list_directory(buffer);
        }
        else if (strcmp(buffer, "3") == 0) {
            // Get filename
            recv(client->sock, buffer, BUFFER_SIZE, 0);
            printf("Read File: %s\n", buffer);
            read_file(buffer, buffer);
        }
        else if (strcmp(buffer, "4") == 0) {
            printf("Connection Duration\n");
            time_t now = time(NULL);
            sprintf(buffer, "Connection duration: %ld seconds\n", now - client->connection_time);
        }
        else if (strcmp(buffer, "5") == 0) {
            printf("Disconnect Request\n");
            break;
        }
        else {
            printf("Invalid Service Request: %s\n", buffer);
            strcpy(buffer, "Invalid service request");
        }

        send(client->sock, buffer, strlen(buffer), 0);
    }

    // Cleanup and disconnect
    close(client->sock);
    pthread_mutex_lock(&clients_mutex);
    client_count--;
    printf("User %s disconnected. Total clients: %d\n", username, client_count);
    pthread_mutex_unlock(&clients_mutex);
    free(client);
    return NULL;
}
int main() {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (1) {
        client_t *client = malloc(sizeof(client_t));
        int addrlen = sizeof(client->address);

        client->sock = accept(server_fd, (struct sockaddr *)&client->address, (socklen_t*)&addrlen);
        if (client->sock < 0) {
            free(client);
            continue;
        }

        if (pthread_create(&thread_id, NULL, handle_client, (void*)client) < 0) {
            perror("Could not create thread");
            free(client);
            continue;
        }

        pthread_detach(thread_id);
    }

    return 0;
}