#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void *joinable_worker(void *arg)
{
    int id = *(int *)arg;
    printf("  [Joinable #%d] Працюю...\n", id);
    usleep(200000);
    printf("  [Joinable #%d] Завершую.\n", id);

    int *result = malloc(sizeof(int));
    if (result) *result = id * 100;
    return (void *)result;
}

void *detached_worker(void *arg)
{
    int id = *(int *)arg;
    printf("  [Detached #%d] Працюю (ресурси звільняться автоматично)...\n", id);
    usleep(100000);
    printf("  [Detached #%d] Завершую.\n", id);
    return NULL;
}

int main(void)
{
    pthread_t t_join, t_detach;
    pthread_attr_t attr;
    int id_join = 1, id_detach = 2;

    /* Ініціалізація атрибутів */
    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("[main] Створюю joinable-потік...\n");
    pthread_create(&t_join, NULL, joinable_worker, &id_join);

    /* Створення detached-потоку */
    printf("[main] Створюю detached-потік...\n");
    pthread_create(&t_detach, &attr, detached_worker, &id_detach);

    pthread_attr_destroy(&attr);

    /* Спроба pthread_join() для detached-потоку — очікуємо помилку EINVAL */
    int rc = 0;
    rc = pthread_join(t_detach, NULL);
    if (rc != 0) {
        printf("[main] pthread_join(detached): помилка %d (%s)\n",
               rc, strerror(rc));
    }

    /* pthread_join() для joinable-потоку — отримання результату */
    int *res = NULL;
    pthread_join(t_join, (void **)&res);
    if (res) {
        printf("[main] Joinable-потік повернув: %d\n", *res);
        free(res);
    }

    usleep(300000);
    printf("[main] Завершено.\n");

    return EXIT_SUCCESS;
}
