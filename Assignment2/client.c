#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080


int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <num_clients>\n", argv[0]);
        return -1;
    }

    int num_clients = atoi(argv[2]);
    const char *server_ip = argv[1];

    for (int i = 0; i < num_clients; i++) {
        int sock = 0;
        struct sockaddr_in serv_addr;
        char buffer[1024] = {0};
        
        
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        
        // Converting server IP
        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }
        
        // Connect to server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        
        
        read(sock, buffer, 1024);
        printf("Server response:\n%s\n", buffer);
        
        
        close(sock);
    }

    return 0;
}
