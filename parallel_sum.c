#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 10000000

typedef struct {
    int     thread_id;
    int     *array;
    int     start;
    int     end;
} task_t;

typedef struct {
    int     thread_id;
    long    partial_sum;
} result_t;

void *compute_partial_sum(void *arg)
{
    task_t *task = (task_t *)arg;

    result_t *res = malloc(sizeof(result_t));
    if (!res) {
        pthread_exit(NULL);
    }

    res->thread_id = task->thread_id;
    res->partial_sum = 0;

    /* Обчислення суми елементів у заданому діапазоні */
    for (int i = task->start; i < task->end; i++) {
        res->partial_sum += task->array[i];
    }

    return (void *)res;
}

int main(int argc, char *argv[])
{
    int num_threads = (argc > 1) ? atoi(argv[1]) : 4;
    if (num_threads <= 0) num_threads = 4;

    /* Створення та ініціалізація масиву */
    int *array = malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 100;
    }

    /* Послідовне обчислення для перевірки та порівняння */
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    long sequential_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sequential_sum += array[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double seq_time = (t_end.tv_sec - t_start.tv_sec)
                    + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;

    /* Паралельне обчислення */
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    task_t *tasks = malloc(num_threads * sizeof(task_t));
    if (!threads || !tasks) {
        perror("malloc threads/tasks");
        free(array);
        free(threads);
        free(tasks);
        return EXIT_FAILURE;
    }

    int chunk_size = ARRAY_SIZE / num_threads;
    int remainder = ARRAY_SIZE % num_threads;
    int current_start = 0;

    for (int i = 0; i < num_threads; i++) {
        tasks[i].thread_id = i + 1;
        tasks[i].array = array;
        tasks[i].start = current_start;
        tasks[i].end = current_start + chunk_size + (i < remainder ? 1 : 0);
        current_start = tasks[i].end;

        pthread_create(&threads[i], NULL, compute_partial_sum, &tasks[i]);
    }

    long parallel_sum = 0;
    for (int i = 0; i < num_threads; i++) {
        result_t *res = NULL;
        pthread_join(threads[i], (void **)&res);
        if (res) {
            parallel_sum += res->partial_sum;
            free(res);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double par_time = (t_end.tv_sec - t_start.tv_sec)
                    + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;

    printf("Послідовна сума: %ld (%.6f сек)\n", sequential_sum, seq_time);
    printf("Паралельна сума: %ld (%.6f сек)\n", parallel_sum, par_time);
    printf("Прискорення:     %.2fx\n", seq_time / par_time);

    free(array);
    free(threads);
    free(tasks);
    return EXIT_SUCCESS;
}
