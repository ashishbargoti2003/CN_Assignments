#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void start_client(const char *server_ip, int server_port, int client_id) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    // Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    printf("Client %d connected to %s:%d\n", client_id, server_ip, server_port);

    // Send a message to the server
    sprintf(buffer, "Requesting top CPU processes from client %d", client_id);
    send(sock, buffer, strlen(buffer), 0);
    printf("Message sent from client %d\n", client_id);

    // Receive server response
    int valread = read(sock, buffer, BUFFER_SIZE);
    buffer[valread] = '\0';
    printf("Client %d received: %s\n", client_id, buffer);

    // Close socket
    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <Server IP> <Number of Clients>\n", argv[0]);
        return -1;
    }

    const char *server_ip = argv[1];
    int server_port = 8080;
    int num_clients = atoi(argv[2]);

    for (int i = 0; i < num_clients; i++) {
        start_client(server_ip, server_port, i + 1);
        sleep(1);  // Optional: Sleep to stagger client connections
    }

    return 0;
}
