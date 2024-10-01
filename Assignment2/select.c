#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

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

int main() {
    int server_fd, new_socket, client_socket[MAX_CLIENTS], max_sd, sd, activity, valread;
    int max_clients = MAX_CLIENTS;
    int opt = 1;
    int addrlen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address;
    printf("pid: %d\n",getpid());

    // Initialize client socket array to 0 (not checked)
    for (int i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options for the socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    printf("Listening on port %d...\n", PORT);

    fd_set readfds;

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add child sockets to set
        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for an activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // Incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to client array
            for (int i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Handle IO operations on other sockets
        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                // Check if it was for closing, and read the incoming message
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("Received: %s\n", buffer);

                    // Get top CPU processes and send it back to the client
                    get_top_cpu_processes(buffer);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}
