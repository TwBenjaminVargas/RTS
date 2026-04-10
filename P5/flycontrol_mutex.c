#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdatomic.h>

#define STABILITY_CONTROL_PRIORITY 80
#define NAVIGATION_PRIORITY 40
#define TELEMETRY_PRIORITY 10
#define TELEMETRY_INTERVAL_NSEC 500 * 1000 * 1000 // 500 ms
#define STABILITY_CONTROL_FREQUENCY_NSEC 100 * 1000 * 1000 // 100 ms
#define NAVIGATION_FREQUENCY_NSEC 400 * 1000 * 1000 // 200 ms

#define NAVIGATION_WORK_ITERATIONS 150000000
#define STABILITY_CONTROL_WORK_ITERATIONS 150000000
#define MUTEX_WORK_ITERATIONS 300000000

#define EXECUTION_TIME_SECONDS 10

atomic_int telemetry_iterations = 0, stability_control_iterations = 0, navigation_iterations = 0, keepRunning = 1;

pthread_mutex_t recurso_compartido_mutex = PTHREAD_MUTEX_INITIALIZER;
// Simulamos un recurso (ej. memoria para guardar logs)
int recurso_valor = 0;


void simulate_work(long iterations) {
    volatile long i;
    for (i = 0; i < iterations; i++);
}

void set_priority(pthread_attr_t *attr, int priority, int policy) {
    struct sched_param param;
    pthread_attr_init(attr);
    
    pthread_attr_setschedpolicy(attr, policy);
    
    if (policy == SCHED_OTHER) {
        param.sched_priority = 0;
    } else {
        param.sched_priority = priority;
    }
    
    pthread_attr_setschedparam(attr, &param);
    pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
}

void* stability_control(void* arg) {

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = STABILITY_CONTROL_FREQUENCY_NSEC;

    while(atomic_load(&keepRunning)) {
        simulate_work(STABILITY_CONTROL_WORK_ITERATIONS);
    
        // Intenta usar el recurso compartido
        pthread_mutex_lock(&recurso_compartido_mutex);
        recurso_valor++; // Operación rápida
        pthread_mutex_unlock(&recurso_compartido_mutex);
        
        atomic_fetch_add(&stability_control_iterations, 1);
        nanosleep(&ts, NULL);
    }
}


void* navigation(void* arg) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NAVIGATION_FREQUENCY_NSEC;

    while(atomic_load(&keepRunning)) {
        simulate_work(NAVIGATION_WORK_ITERATIONS);
        atomic_fetch_add(&navigation_iterations, 1);
        nanosleep(&ts, NULL);
    }
}


void* telemetry(void* arg) {
    
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    
    while(atomic_load(&keepRunning)) {
        struct timespec timeout = {1, 0}; // 1 second timeout
        sig = sigtimedwait(&set, NULL, &timeout);

        if (sig == SIGALRM)
        {
            printf("\r[Telemetria] Bloqueando recurso...         ");
            pthread_mutex_lock(&recurso_compartido_mutex); // BLOQUEA
    
            // Simula un procesamiento largo con el recurso (ej. guardar en SD)
            simulate_work(MUTEX_WORK_ITERATIONS); 
            recurso_valor++;

            pthread_mutex_unlock(&recurso_compartido_mutex); // LIBERA
            printf("\r[Telemetria] Recurso liberado.             ");

            atomic_fetch_add(&telemetry_iterations, 1);
            fflush(stdout);
        }
    }
}

void print_table() {
    printf("\n\n");
    printf("====================================================\n");
    printf("     RESULTADOS DE EJECUCION (%d SEGUNDOS)          \n", EXECUTION_TIME_SECONDS);
    printf("====================================================\n");
    printf("| Tarea           | Prioridad | Iteraciones Logradas |\n");
    printf("|-----------------|-----------|----------------------|\n");
    printf("| Estabilidad     |    %d     |       %12d   |\n", STABILITY_CONTROL_PRIORITY, atomic_load(&stability_control_iterations));
    printf("| Navegacion      |    %d     |       %12d   |\n", NAVIGATION_PRIORITY, atomic_load(&navigation_iterations));
    printf("| Telemetria      |    %d     |       %12d   |\n", TELEMETRY_PRIORITY, atomic_load(&telemetry_iterations));
    printf("====================================================\n");
}



int main()
{
    int choice;
    int selected_policy;
    pthread_t stability_control_th, navigation_th, telemetry_th;
    pthread_attr_t stability_control_attr, navigation_attr, telemetry_attr;

    printf("Elija la politica de planificacion (Inversion de prioridad activada):\n");
    printf("1. SCHED_FIFO (Tiempo Real - Requiere sudo)\n");
    printf("2. SCHED_OTHER (Normal/Justa)\n");
    printf("Seleccion: ");
    scanf("%d", &choice);

    if (choice == 1) {
        selected_policy = SCHED_FIFO;
        printf("Iniciando en modo Tiempo Real...\n");
    } else {
        selected_policy = SCHED_OTHER;
        printf("Iniciando en modo Normal...\n");
    }


    set_priority(&stability_control_attr, STABILITY_CONTROL_PRIORITY, selected_policy);
    set_priority(&navigation_attr, NAVIGATION_PRIORITY, selected_policy);
    set_priority(&telemetry_attr, TELEMETRY_PRIORITY, selected_policy);

    // Block SIGALRM in the main thread and all threads created from it
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);


    pthread_create(&stability_control_th, &stability_control_attr, stability_control, NULL);
    pthread_create(&navigation_th, &navigation_attr, navigation, NULL);
    pthread_create(&telemetry_th, &telemetry_attr, telemetry, NULL);

    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;


    timer_create(CLOCK_REALTIME, &sev, &timerid);

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = TELEMETRY_INTERVAL_NSEC;
    its.it_interval.tv_sec = 0;                                        
    its.it_interval.tv_nsec = TELEMETRY_INTERVAL_NSEC;

    timer_settime(timerid, 0, &its, NULL);


    printf("Ejecutando sistema de control por %d segundos...\n", EXECUTION_TIME_SECONDS);
    sleep(EXECUTION_TIME_SECONDS); 
    
    atomic_store(&keepRunning, 0);


    pthread_join(stability_control_th, NULL);
    pthread_join(navigation_th, NULL);
    pthread_join(telemetry_th, NULL);
    timer_delete(timerid);

    print_table();


    return 0;
}
