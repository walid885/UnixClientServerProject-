#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <bits/mman-linux.h>
#include <asm-generic/socket.h>
#include <sys/mman.h>

#define PORT 8080
#define MAX_SERVERS 4
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
} user_credentials_t;

typedef struct {
    int sock;
    struct sockaddr_in address;
    time_t connection_time;
    char username[50];
    int server_id;
} client_t;

// Shared memory structure
typedef struct {
    int active_clients;
    pid_t server_pids[MAX_SERVERS];
    pthread_mutex_t mutex;
} shared_data_t;

// Global variables
shared_data_t *shared_data;
int server_index = 0;

// Valid users array
user_credentials_t valid_users[] = {
    {"admin", "admin"},
    {"usr1", "usr1"},
    {"usr2", "usr2"},
    {"usr3", "usr3"}
};

#define NUM_VALID_USERS (sizeof(valid_users) / sizeof(user_credentials_t))

// Function to get current time
void get_time(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%02d/%02d/%d %02d:%02d:%02d",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
}

// List directory contents
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

// Read file content
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

// Authentication function
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

// Client handler function
void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;
    
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->active_clients++;
    printf("Server %d: New client connecting. Total clients: %d\n", 
           client->server_id, shared_data->active_clients);
    pthread_mutex_unlock(&shared_data->mutex);

    // Authentication
    read_size = recv(client->sock, buffer, BUFFER_SIZE, 0);
    buffer[read_size] = '\0';
    
    if (!authenticate(buffer)) {
        send(client->sock, "Authentication failed", 20, 0);
        close(client->sock);
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->active_clients--;
        pthread_mutex_unlock(&shared_data->mutex);
        free(client);
        return NULL;
    }

    sscanf(buffer, "%s", client->username);
    send(client->sock, "Authentication successful", 23, 0);
    client->connection_time = time(NULL);
    
    printf("Server %d: User %s authenticated\n", client->server_id, client->username);

    while ((read_size = recv(client->sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        
        printf("Server %d: User %s requested service: %s\n", 
               client->server_id, client->username, buffer);

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
            sprintf(buffer, "Connection duration: %ld seconds\n", 
                    now - client->connection_time);
        }
        else if (strcmp(buffer, "5") == 0) {
            break;
        }

        send(client->sock, buffer, strlen(buffer), 0);
    }

    close(client->sock);
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->active_clients--;
    printf("Server %d: User %s disconnected. Total clients: %d\n", 
           client->server_id, client->username, shared_data->active_clients);
    pthread_mutex_unlock(&shared_data->mutex);
    free(client);
    return NULL;
}

// Server process function
void server_process(int server_id) {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                   &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server process %d ready to accept connections\n", server_id);
    
    while (1) {
        client_t *client = malloc(sizeof(client_t));
        int addrlen = sizeof(client->address);
        
        client->sock = accept(server_fd, (struct sockaddr *)&client->address, 
                            (socklen_t*)&addrlen);
        if (client->sock < 0) {
            free(client);
            continue;
        }
        
        client->server_id = server_id;
        
        if (pthread_create(&thread_id, NULL, handle_client, (void*)client) < 0) {
            perror("Could not create thread");
            free(client);
            continue;
        }
        
        pthread_detach(thread_id);
    }
}

// Signal handler
void handle_signal(int sig) {
    printf("\nReceived signal %d. Shutting down...\n", sig);
    
    for (int i = 0; i < MAX_SERVERS; i++) {
        if (shared_data->server_pids[i] > 0) {
            kill(shared_data->server_pids[i], SIGTERM);
        }
    }
    
    pthread_mutex_destroy(&shared_data->mutex);
    munmap(shared_data, sizeof(shared_data_t));
    exit(0);
}

int main() {
    // Set up shared memory
shared_data = mmap(NULL, sizeof(shared_data_t), 
                  PROT_READ | PROT_WRITE, 
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

if (shared_data == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
}

    
    // Initialize shared data
    shared_data->active_clients = 0;
    memset(shared_data->server_pids, 0, sizeof(pid_t) * MAX_SERVERS);
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_data->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    // Set up signal handlers
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    
    printf("Master server starting up...\n");
    printf("Launching %d server processes\n", MAX_SERVERS);
    
    // Create server processes
    for (int i = 0; i < MAX_SERVERS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {  // Child process
            server_process(i);
            exit(0);
        }
        else {  // Parent process
            shared_data->server_pids[i] = pid;
            printf("Started server process %d with PID %d\n", i, pid);
        }
    }
    
    // Wait for all child processes
    while (1) {
        int status;
        pid_t terminated_pid = wait(&status);
        if (terminated_pid < 0) {
            if (errno == ECHILD) {
                break;
            }
        } else {
            printf("Server process %d terminated\n", terminated_pid);
        }
    }
    
    pthread_mutex_destroy(&shared_data->mutex);
    munmap(shared_data, sizeof(shared_data_t));
    return 0;
}