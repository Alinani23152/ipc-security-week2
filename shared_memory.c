#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define ITERATIONS 100000

struct shared_data {
    int counter;
    sem_t lock;
};

int main(void)
{
    struct shared_data *shared;

    shared = mmap(NULL, sizeof(struct shared_data),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    shared->counter = 0;

    if (sem_init(&shared->lock, 1, 1) == -1) {
        perror("sem_init");
        munmap(shared, sizeof(struct shared_data));
        return 1;
    }

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            sem_destroy(&shared->lock);
            munmap(shared, sizeof(struct shared_data));
            return 1;
        }

        if (pid == 0) {
            for (int j = 0; j < ITERATIONS; j++) {
                sem_wait(&shared->lock);

                shared->counter++;

                sem_post(&shared->lock);
            }

            _exit(0);
        }
    }

    int status;
    int failed = 0;

    for (int i = 0; i < 2; i++) {
        if (wait(&status) == -1 ||
            !WIFEXITED(status) ||
            WEXITSTATUS(status) != 0) {
            failed = 1;
        }
    }

    printf("Final counter: %d\n", shared->counter);
    printf("Expected counter: %d\n", 2 * ITERATIONS);

    sem_destroy(&shared->lock);
    munmap(shared, sizeof(struct shared_data));

    return failed;
}
