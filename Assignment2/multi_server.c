#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CLIENTS 10

// Structure to hold client data
typedef struct {
    int client_socket;
} client_data_t;

// Function to get the top 2 CPU-consuming processes
void get_top_cpu_processes(char *buffer) {
    FILE *fp;
    char path[1024];
    int pid;
    char comm[256];
    long utime, stime;
    long max_utime = 0, max_stime = 0;
    long second_utime = 0, second_stime = 0;
    int max_pid = 0, second_pid = 0;
    
    for (pid = 1; pid < 32768; pid++) {
        sprintf(path, "/proc/%d/stat", pid);
        fp = fopen(path, "r");
        if (fp) {
            fscanf(fp, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %ld %ld",
                &pid, comm, &utime, &stime);
            fclose(fp);
            
            // Compare for top 2 processes
            if (utime + stime > max_utime + max_stime) {
                second_utime = max_utime;
                second_stime = max_stime;
                second_pid = max_pid;
                
                max_utime = utime;
                max_stime = stime;
                max_pid = pid;
            } else if (utime + stime > second_utime + second_stime) {
                second_utime = utime;
                second_stime = stime;
                second_pid = pid;
            }
        }
    }
    
    sprintf(buffer, "1st Process: PID: %d, CPU: %ld\n2nd Process: PID: %d, CPU: %ld\n",
        max_pid, max_utime + max_stime, second_pid, second_utime + second_stime);
}

void *handle_client(void *arg) {
    client_data_t *data = (client_data_t *)arg;
    int client_socket = data->client_socket;
    char buffer[1024] = {0};
    
    // Getting CPU processes information
    get_top_cpu_processes(buffer);

    send(client_socket, buffer, strlen(buffer), 0);
    
    close(client_socket);
    free(data);
    pthread_exit(NULL);
}


int main() {
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t tid;
    printf("pid: %d\n",getpid());
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Binding it to a specific port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    printf("%d ip\n",address.sin_addr.s_addr);
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    
    listen(server_fd, MAX_CLIENTS);
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        client_data_t *data = (client_data_t *)malloc(sizeof(client_data_t));
        data->client_socket = new_socket;
        pthread_create(&tid, NULL, handle_client, (void *)data);
    }
    
    return 0;
}
