#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[0]);

        printf("Child PID: %d\n", getpid());
        printf("Child writing to pipe...\n");
        fflush(stdout);

        write(pipefd[1], "Security event\n", 15);

        sleep(60);

        close(pipefd[1]);
        _exit(0);
    }

    close(pipefd[1]);

    printf("Parent PID: %d\n", getpid());
    printf("Child PID: %d\n", pid);
    printf("Parent reading from pipe...\n");
    fflush(stdout);

    char buffer[100];
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);

    if (n > 0) {
        buffer[n] = '\0';
        printf("Received: %s", buffer);
        fflush(stdout);
    }

    sleep(60);

    close(pipefd[0]);
    wait(NULL);

    return 0;
}
