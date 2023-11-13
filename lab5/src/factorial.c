#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int k;
    int pnum;
    int mod;
    int* result;
    pthread_mutex_t* mutex;
} FactorialArgs;

// Функция для вычисления факториала
void* Factorial(void* args) {
    FactorialArgs* fa = (FactorialArgs*)args;
    int k = fa->k;
    int mod = fa->mod;
    int pnum = fa->pnum;
    int* result = fa->result;
    pthread_mutex_t* mutex = fa->mutex;

    int local_result = 1;
    for (int i = k; i > 0; i -= pnum) {
        local_result *= i;
        local_result %= mod;
    }

    pthread_mutex_lock(mutex);
    *result *= local_result;
    *result %= mod;
    pthread_mutex_unlock(mutex);

    return NULL;
}

int main(int argc, char** argv) {

    if (argc != 7) {
        printf("Usage: %s -k <number> --pnum=<number of threads> --mod=<modulus>\n", argv[0]);
        return 1;
    }

    int k, pnum, mod;
    sscanf(argv[2], "%d", &k);
    sscanf(argv[4], "%d", &pnum);
    sscanf(argv[6], "%d", &mod);

    // Создаем массив потоков и структуру для передачи параметров
    pthread_t threads[pnum];
    FactorialArgs args[pnum];
    int result = 1;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    // Запускаем потоки
    for (int i = 0; i < pnum; i++) {
        args[i].k = k - i;
        args[i].pnum = pnum;
        args[i].mod = mod;
        args[i].result = &result;
        args[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, Factorial, (void*)&args[i]);
    }

    // Ожидаем завершения потоков
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%d! mod %d = %d\n", k, mod, result);

    pthread_mutex_destroy(&mutex);

    return 0;
}