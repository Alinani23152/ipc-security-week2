#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define ITERATIONS 100000

struct shared_data {
    int counter;
};

int main(void)
{
    struct shared_data *shared;

    shared = mmap(NULL, sizeof(struct shared_data),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS,
                  -1, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    shared->counter = 0;

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            munmap(shared, sizeof(struct shared_data));
            return 1;
        }

        if (pid == 0) {
            for (int j = 0; j < ITERATIONS; j++) {
                shared->counter++;
            }

            _exit(0);
        }
    }

    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }

    printf("Final counter: %d\n", shared->counter);
    printf("Expected counter: %d\n", 2 * ITERATIONS);

    munmap(shared, sizeof(struct shared_data));

    return 0;
}
