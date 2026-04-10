#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


void* imprimir_identificadores(void* arg) {
    // Obtener el Process ID (PID)
    pid_t pid = getpid();

    // Obtener el Thread ID (TID) propio de Linux/POSIX
    // syscall(SYS_gettid) devuelve el ID del hilo a nivel de kernel
    pid_t tid = (pid_t)syscall(SYS_gettid);

    // Obtener el identificador interno de pthread
    pthread_t self = pthread_self();

    printf("--- Información desde el Hilo Secundario ---\n");
    printf("PID (Proceso): %d\n", pid);
    printf("TID (Hilo - Kernel): %d\n", tid);
    printf("pthread_t ID: %lu\n", (unsigned long)self);
    printf("--------------------------------------------\n");

    pthread_exit(NULL);
}

int main() {
    pthread_t hilo_hijo;

    printf("Main: Creando el hilo...\n");

    // Creación del hilo
    if (pthread_create(&hilo_hijo, NULL, imprimir_identificadores, NULL) != 0) {
        perror("Fallo al crear el hilo");
        return 1;
    }

    // Esperar a que el hilo termine su ejecución
    pthread_join(hilo_hijo, NULL);

    printf("Main: El hilo ha finalizado. Terminando programa.\n");

    return 0;
}