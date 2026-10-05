#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_PRODUCERS 3
#define BUFFER_SIZE 256

int main(void)
{
    int pipefd[2];
    pid_t pid;
    int logfd;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    const char *events[NUM_PRODUCERS] = {
        "LOGIN_SUCCESS",
        "LOGIN_FAILED",
        "SECURITY_ALERT"
    };

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork");
            close(pipefd[0]);
            close(pipefd[1]);
            return 1;
        }

        if (pid == 0) {
            close(pipefd[0]);

            char message[BUFFER_SIZE];
            int len = snprintf(message, sizeof(message),
                               "Producer %d: %s\n", i + 1, events[i]);

            if (len < 0 || len >= BUFFER_SIZE) {
                close(pipefd[1]);
                _exit(1);
            }

            ssize_t written = write(pipefd[1], message, len);

            close(pipefd[1]);
            _exit(written == len ? 0 : 1);
        }
    }

    close(pipefd[1]);

    logfd = open("audit.log",
                 O_WRONLY | O_CREAT | O_APPEND,
                 0640);

    if (logfd == -1) {
        perror("open");
        close(pipefd[0]);

        while (wait(NULL) > 0) {
        }

        return 1;
    }

    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        ssize_t offset = 0;

        while (offset < n) {
            ssize_t written = write(logfd, buffer + offset,
                                    n - offset);

            if (written <= 0) {
                perror("write");
                close(pipefd[0]);
                close(logfd);

                while (wait(NULL) > 0) {
                }

                return 1;
            }

            offset += written;
        }
    }

    if (n < 0) {
        perror("read");
    }

    close(pipefd[0]);
    close(logfd);

    int status;
    int result = 0;

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        if (wait(&status) == -1 ||
            !WIFEXITED(status) ||
            WEXITSTATUS(status) != 0) {
            result = 1;
        }
    }

    printf("Logger finished recording events.\n");

    return result;
}
