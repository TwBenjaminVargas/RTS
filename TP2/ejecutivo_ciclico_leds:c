#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <pigpio.h> // Librería para GPIO en Raspberry Pi

// Definición de pines (puedes cambiarlos según tu conexión)
#define LED1 17
#define LED2 27
#define LED3 22

// Variable atómica para evitar corrupción de datos durante la interrupción
static volatile sig_atomic_t tics = 0;

/**
 * Tarea 1: Conmuta el LED 1 cada 100ms (10 tics)
 */
void tarea1()
{
    static int state = 0, timer;
    switch (state)
    {
    case 0:
        gpioSetMode(LED1, PI_OUTPUT); // Configurar pin como salida
        timer = tics + 10;
        state = 1;
        break;
    case 1:
        if (tics >= timer) {
            // gpioRead lee el estado actual y "!" lo invierte (Toggle)
            gpioWrite(LED1, !gpioRead(LED1)); 
            timer = tics + 10;
        }
        break;
    }
}

/**
 * Tarea 2: Conmuta el LED 2 cada 300ms (30 tics)
 */
void tarea2()
{
    static int state = 0, timer;
    switch (state)
    {
    case 0:
        gpioSetMode(LED2, PI_OUTPUT);
        timer = tics + 30;
        state = 1;
        break;
    case 1:
        if (tics >= timer) {
            gpioWrite(LED2, !gpioRead(LED2));
            timer = tics + 30;
        }
        break;
    }
}

/**
 * Tarea 3: Conmuta el LED 3 cada 500ms (50 tics)
 */
void tarea3()
{
    static int state = 0, timer;
    switch (state)
    {
    case 0:
        gpioSetMode(LED3, PI_OUTPUT);
        timer = tics + 50;
        state = 1;
        break;
    case 1:
        if (tics >= timer) {
            gpioWrite(LED3, !gpioRead(LED3));
            timer = tics + 50;
        }
        break;
    }
}

/**
 * Manejador del timer (Interrupción de hardware cada 10ms)
 */
void timer_handler(int sig) {
    tics++;
}

int main() {
    // 1. Inicializar la librería pigpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error al inicializar pigpio\n");
        return 1;
    }

    // 2. Configurar el Timer del Sistema (SIGALRM)
    struct itimerval timer;
    signal(SIGALRM, timer_handler);

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000; // 10ms
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error configurando itimer");
        gpioTerminate();
        return 1;
    }

    printf("Planificador corriendo a 10ms. Presiona Ctrl+C para detener.\n");

    // 3. Super Loop
    while(1) {
        tarea1();
        tarea2();
        tarea3();
        
        // Pequeño descanso para no saturar la CPU al 100% innecesariamente
        // usleep(1000) permite que el kernel respire sin perder precisión de 10ms
        usleep(1000); 
    }

    // Limpieza (aunque en este caso el while es infinito)
    gpioTerminate();
    return 0;
}