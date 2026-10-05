#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define PORT 8080
#define MAX_REQUEST 64

static int send_response(int fd, const char *response)
{
    size_t total = 0;
    size_t length = strlen(response);

    while (total < length) {
        ssize_t sent = send(fd, response + total, length - total, 0);

        if (sent < 0) {
            perror("send");
            return -1;
        }

        total += (size_t)sent;
    }

    return 0;
}

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_REQUEST + 1];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Secure security server started.\n");
    printf("Listening on 127.0.0.1:%d\n", PORT);
    printf("Listening FD: %d\n", server_fd);
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");
    printf("Connected Client FD: %d\n", client_fd);
    fflush(stdout);

    /*
     * Read one extra byte so we can detect requests
     * that exceed MAX_REQUEST.
     */
    ssize_t n = recv(client_fd, buffer, MAX_REQUEST + 1, 0);

    if (n < 0) {
        perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    if (n > MAX_REQUEST) {
        const char *response = "ERROR: Request too long\n";

        printf("Rejected oversized request.\n");
        send_response(client_fd, response);

        close(client_fd);
        close(server_fd);
        return 0;
    }

    buffer[n] = '\0';

    printf("Received request: %s\n", buffer);

    if (strcmp(buffer, "STATUS") == 0) {
        send_response(client_fd, "SECURITY_STATUS: OK\n");
    }
    else if (strcmp(buffer, "EVENTS") == 0) {
        send_response(
            client_fd,
            "EVENTS: LOGIN_SUCCESS LOGIN_FAILED SECURITY_ALERT\n"
        );
    }
    else {
        send_response(client_fd, "ERROR: Invalid request\n");
    }

    close(client_fd);
    close(server_fd);

    printf("Server closed.\n");

    return 0;
}
