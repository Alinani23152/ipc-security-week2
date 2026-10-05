#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    /* 1. Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    /* Allow quick restart after testing */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(PORT);

    /* 3. Bind socket */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    /* 4. Listen for connections */
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Security server started.\n");
    printf("Listening on 127.0.0.1:%d\n", PORT);
    printf("Listening FD: %d\n", server_fd);
    fflush(stdout);

    /* 5. Accept a client */
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");
    printf("Connected Client FD: %d\n", client_fd);
    fflush(stdout);

    /* 6. Receive request */
    ssize_t n = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (n == -1) {
        perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    buffer[n] = '\0';

    printf("Received request: %s\n", buffer);

    /* 7. Provide a small, explicitly defined service */
    if (strcmp(buffer, "STATUS") == 0) {

        const char *response =
            "SECURITY_STATUS: OK\n";

        send(client_fd,
             response,
             strlen(response),
             0);

    } else if (strcmp(buffer, "EVENTS") == 0) {

        const char *response =
            "EVENTS: LOGIN_SUCCESS LOGIN_FAILED SECURITY_ALERT\n";

        send(client_fd,
             response,
             strlen(response),
             0);

    } else {

        const char *response =
            "ERROR: Invalid request\n";

        send(client_fd,
             response,
             strlen(response),
             0);
    }

    /* 8. Close descriptors */
    close(client_fd);
    close(server_fd);

    printf("Server closed.\n");

    return 0;
}
