#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define NUM_HILOS 10

void* imprimir_tid(void* arg) {
    int id_local = *(int*)arg;
    
    // Liberamos la memoria reservada para el argumento
    free(arg);

    // Obtención del TID a nivel de Kernel (específico de Linux)
    pid_t tid = (pid_t)syscall(SYS_gettid);

    printf("Hilo índice [%d]: Mi TID de sistema es %d\n", id_local, tid);

    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[NUM_HILOS];
    int i;

    printf("Iniciando creación de %d hilos...\n", NUM_HILOS);

    for (i = 0; i < NUM_HILOS; i++) {
        // Reservamos memoria para pasar el índice de forma segura
        int* indice = malloc(sizeof(int));
        *indice = i;

        if (pthread_create(&hilos[i], NULL, imprimir_tid, indice) != 0) {
            perror("Error al crear el hilo");
            return 1;
        }
    }

    // Esperar a que todos los hilos finalicen
    for (i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Todos los hilos han terminado su ejecución.\n");

    return 0;
}