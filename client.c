#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_SOCKET_PATH "/tmp/chat_socket"

int main(int argc, char *argv[]) {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[256];
    char *socket_path = (argc > 1) ? argv[1] : DEFAULT_SOCKET_PATH;

    // Create socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        fprintf(stderr, "Socket failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Configure socket address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // Connect to server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Connect failed: %s\n", strerror(errno));
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type 'exit' to quit.\n");

    // Chat loop
    while (1) {
        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);
        write(client_fd, buffer, strlen(buffer));

        if (strcmp(buffer, "exit\n") == 0) break;

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) break;  // Handle disconnection

        buffer[bytes_read] = '\0'; // Null-terminate for safety
        printf("Server: %s", buffer);
    }

    close(client_fd);
    return 0;
}

