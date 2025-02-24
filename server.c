#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>  // <-- Include this for chmod

#define DEFAULT_SOCKET_PATH "/tmp/chat_socket"

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    char buffer[256];
    char *socket_path = (argc > 1) ? argv[1] : DEFAULT_SOCKET_PATH;

    // Create socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "Socket failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Remove existing socket file
    unlink(socket_path);

    // Configure socket address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Bind failed: %s\n", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Set permissions (Ensure any user can connect)
    chmod(socket_path, 0666);  // This requires <sys/stat.h>

    // Listen for connections
    if (listen(server_fd, 5) == -1) {
        fprintf(stderr, "Listen failed: %s\n", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s...\n", socket_path);

    // Accept client connection
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        fprintf(stderr, "Accept failed: %s\n", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    // Chat loop
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) break;  // Handle disconnection

        buffer[bytes_read] = '\0'; // Null-terminate for safety
        if (strcmp(buffer, "exit\n") == 0) break;

        printf("Client: %s", buffer);

        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);
        write(client_fd, buffer, strlen(buffer));
    }

    close(client_fd);
    close(server_fd);
    unlink(socket_path);
    return 0;
}

