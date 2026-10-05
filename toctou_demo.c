#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
    const char *target = "resource.txt";
    const char *replacement = "replacement.txt";
    int pipefd[2];
    struct stat info;

    FILE *file = fopen(target, "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    fprintf(file, "SAFE TEST DATA\n");
    fclose(file);

    file = fopen(replacement, "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    fprintf(file, "CHANGED TEST DATA\n");
    fclose(file);

    if (stat(target, &info) == -1) {
        perror("stat");
        return 1;
    }

    if (S_ISREG(info.st_mode)) {
        printf("CHECK: Resource is a regular file.\n");
    } else {
        printf("CHECK: Resource is not a regular file.\n");
        return 1;
    }

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
        close(pipefd[1]);

        char signal;
        if (read(pipefd[0], &signal, 1) != 1) {
            _exit(1);
        }

        if (rename(replacement, target) == -1) {
            perror("rename");
            _exit(1);
        }

        printf("CHANGE: Resource replaced by another process.\n");
        close(pipefd[0]);
        _exit(0);
    }

    close(pipefd[0]);

    printf("RACE WINDOW: Waiting before using resource.\n");
    fflush(stdout);

    if (write(pipefd[1], "X", 1) != 1) {
        perror("write");
        close(pipefd[1]);
        wait(NULL);
        return 1;
    }

    close(pipefd[1]);

    int status;
    if (waitpid(pid, &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Replacement process failed.\n");
        return 1;
    }

    printf("USE: Opening resource after the check.\n");

    int fd = open(target, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char buffer[100];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';
    printf("READ DATA: %s", buffer);

    close(fd);

    return 0;
}
