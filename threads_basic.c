#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *thread_func(void *arg)
{
    int id = *(int *)arg;

    printf("  Потік #%d: TID = %lu, запущено\n",
           id, (unsigned long)pthread_self());

    /* Імітація корисної роботи */
    for (volatile int i = 0; i < 1000000; i++);

    printf("  Потік #%d: завершую роботу\n", id);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Використання: %s <кількість_потоків>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n <= 0 || n > 64) {
        fprintf(stderr, "Помилка: кількість потоків має бути 1..64\n");
        return EXIT_FAILURE;
    }

    pthread_t *threads = malloc(n * sizeof(pthread_t));
    int *ids = malloc(n * sizeof(int));
    if (!threads || !ids) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    printf("[main] Створюємо %d потоків...\n", n);

    /* Створення потоків у циклі */
    for (int i = 0; i < n; i++) {
        ids[i] = i + 1;
        int rc = pthread_create(&threads[i], NULL, thread_func, &ids[i]);
        if (rc != 0) {
            fprintf(stderr, "Помилка створення потоку %d: %s\n", i + 1, strerror(rc));
            free(threads);
            free(ids);
            return EXIT_FAILURE;
        }
    }

    /* Очікування завершення кожного потоку */
    for (int i = 0; i < n; i++) {
        int rc = pthread_join(threads[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "Помилка очікування потоку %d: %s\n", i + 1, strerror(rc));
        }
    }

    printf("[main] Усі потоки завершено.\n");

    free(threads);
    free(ids);
    return EXIT_SUCCESS;
}
