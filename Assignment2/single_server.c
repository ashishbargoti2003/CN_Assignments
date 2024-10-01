#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 10


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

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    
    
    get_top_cpu_processes(buffer);

    send(client_socket, buffer, strlen(buffer), 0);
    
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    printf("pid: %d\n", getpid());
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Binding it to a specific port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    listen(server_fd, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue; // Continue to accept next connection
        }
        
        // Handle the client directly
        handle_client(new_socket);
    }
    
    return 0;
}
