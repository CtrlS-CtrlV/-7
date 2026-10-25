#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define NUM_TASKS     20
#define POOL_SIZE     4
#define ITERATIONS    50000000

typedef struct {
    int     task_id;
    double  input;
    double  result;
} task_t;

typedef struct {
    task_t  *tasks;
    int     start;
    int     end;
} worker_arg_t;

double heavy_computation(double x)
{
    double result = 0.0;
    for (int i = 0; i < ITERATIONS; i++) {
        result += sin(x + i) * cos(x - i);
    }
    return result;
}

void *worker_func(void *arg)
{
    worker_arg_t *w = (worker_arg_t *)arg;

    for (int i = w->start; i < w->end; i++) {
        w->tasks[i].result = heavy_computation(w->tasks[i].input);
        printf("  Worker: завдання %d завершено (input=%.2f, result=%.4f)\n",
               w->tasks[i].task_id, w->tasks[i].input, w->tasks[i].result);
    }

    return NULL;
}

int main(void)
{
    task_t tasks[NUM_TASKS];
    for (int i = 0; i < NUM_TASKS; i++) {
        tasks[i].task_id = i;
        tasks[i].input = (double)i * 0.5;
        tasks[i].result = 0.0;
    }

    struct timespec t1, t2;

    printf("=== Однопоточне виконання (%d завдань) ===\n", NUM_TASKS);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    for (int i = 0; i < NUM_TASKS; i++) {
        tasks[i].result = heavy_computation(tasks[i].input);
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    double seq_time = (t2.tv_sec - t1.tv_sec)
                    + (t2.tv_nsec - t1.tv_nsec) / 1e9;
    printf("Час: %.4f сек\n\n", seq_time);

    /* Скидання результатів */
    for (int i = 0; i < NUM_TASKS; i++) tasks[i].result = 0.0;

    printf("=== Пул потоків (%d workers, %d завдань) ===\n",
           POOL_SIZE, NUM_TASKS);

    pthread_t pool[POOL_SIZE];
    worker_arg_t wargs[POOL_SIZE];

    int tasks_per_worker = NUM_TASKS / POOL_SIZE;
    int remainder = NUM_TASKS % POOL_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    int offset = 0;
    for (int i = 0; i < POOL_SIZE; i++) {
        wargs[i].tasks = tasks;
        wargs[i].start = offset;
        wargs[i].end = offset + tasks_per_worker + (i < remainder ? 1 : 0);
        offset = wargs[i].end;
        
        pthread_create(&pool[i], NULL, worker_func, &wargs[i]);
    }

    for (int i = 0; i < POOL_SIZE; i++) {
        pthread_join(pool[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    double par_time = (t2.tv_sec - t1.tv_sec)
                    + (t2.tv_nsec - t1.tv_nsec) / 1e9;

    printf("Час: %.4f сек\n", par_time);
    printf("Прискорення: %.2fx\n", seq_time / par_time);

    return EXIT_SUCCESS;
}
