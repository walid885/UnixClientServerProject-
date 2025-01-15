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
    return (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0);
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    pthread_mutex_lock(&clients_mutex);
    client_count++;
    printf("New client connected. Total clients: %d\n", client_count);
    pthread_mutex_unlock(&clients_mutex);

    // Authentication
    read_size = recv(client->sock, buffer, BUFFER_SIZE, 0);
    buffer[read_size] = '\0';

    if (!authenticate(buffer)) {
        send(client->sock, "Authentication failed", 20, 0);
        close(client->sock);
        pthread_mutex_lock(&clients_mutex);
        client_count--;
        pthread_mutex_unlock(&clients_mutex);
        free(client);
        return NULL;
    }

    send(client->sock, "Authentication successful", 23, 0);
    client->connection_time = time(NULL);

    while ((read_size = recv(client->sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';

        if (strcmp(buffer, "1") == 0) {
            get_time(buffer);
        }
        else if (strcmp(buffer, "2") == 0) {
            list_directory(buffer);
        }
        else if (strcmp(buffer, "3") == 0) {
            recv(client->sock, buffer, BUFFER_SIZE, 0);
            read_file(buffer, buffer);
        }
        else if (strcmp(buffer, "4") == 0) {
            time_t now = time(NULL);
            sprintf(buffer, "Connection duration: %ld seconds\n", now - client->connection_time);
        }
        else if (strcmp(buffer, "5") == 0) {
            break;
        }

        send(client->sock, buffer, strlen(buffer), 0);
    }

    close(client->sock);
    pthread_mutex_lock(&clients_mutex);
    client_count--;
    printf("Client disconnected. Total clients: %d\n", client_count);
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