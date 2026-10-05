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
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    const char *request = "STATUS";

    /* 1. Create socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    printf("Client socket FD: %d\n", sock_fd);

    /* 2. Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    /* 3. Connect to server */
    if (connect(sock_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("Connected to server.\n");

    /* 4. Send request */
    send(sock_fd, request, strlen(request), 0);

    printf("Sent request: %s\n", request);

    /* 5. Receive response */
    ssize_t n = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);

    if (n == -1) {
        perror("recv");
        close(sock_fd);
        return 1;
    }

    buffer[n] = '\0';

    printf("Server response: %s", buffer);

    /* 6. Close socket */
    close(sock_fd);

    printf("Connection closed.\n");

    return 0;
}
