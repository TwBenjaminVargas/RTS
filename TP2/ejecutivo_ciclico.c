#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

/**
 * VARIABLE GLOBAL: tics
 * Actúa como el "reloj del sistema". Se incrementa cada 10ms.
 * Se usa 'static' para limitar su alcance al archivo, pero en un entorno 
 * real con señales debería ser 'volatile sig_atomic_t' para mayor seguridad.
 */
static int tics = 0;

/**
 * Tarea 1: Se ejecuta cada 10 tics (aprox. cada 100ms).
 * Utiliza una Máquina de Estados (switch) para gestionar su propia temporización.
 */
void tarea1()
{
    // 'static' permite que estas variables conserven su valor entre llamadas
    static int state = 0, timer;
    
    switch (state)
    {
    case 0: // ESTADO DE INICIALIZACIÓN: Se ejecuta solo la primera vez
        timer = tics + 10; // Programar la próxima ejecución en 10 tics
        state = 1;         // Pasar al estado de ejecución cíclica
        break;
    case 1: // ESTADO DE ESPERA: Compara el tiempo actual con el programado
        if (tics >= timer) {
            printf("Tarea 1 ejecutada en tic %d\n", tics);
            timer = tics + 10; // Reprogramar para dentro de 10 tics
        }
        break;
    }
}

/**
 * Tarea 2: Se ejecuta cada 30 tics (aprox. cada 300ms).
 */
void tarea2()
{
    static int state = 0, timer;
    switch (state)
    {
    case 0:
        timer = tics + 30;
        state = 1;
        break;
    case 1:
        if (tics >= timer) {
            printf("Tarea 2 ejecutada en tic %d\n", tics);
            timer = tics + 30;
        }
        break;
    }
}

/**
 * Tarea 3: Se ejecuta cada 50 tics (aprox. cada 500ms).
 */
void tarea3()
{
    static int state = 0, timer;
    switch (state)
    {
    case 0:
        timer = tics + 50;
        state = 1;
        break;
    case 1:
        if (tics >= timer) {
            printf("Tarea 3 ejecutada en tic %d\n", tics);
            timer = tics + 50;
        }
        break;
    }
}

/**
 * MANEJADOR DE LA SEÑAL (ISR - Interrupt Service Routine)
 * Esta función es llamada automáticamente por el Sistema Operativo cada 10ms.
 */
void timer_handler(int sig) {
    tics++; // Incrementar la base de tiempo global
}

int main() {
    struct itimerval timer;

    // Registrar el manejador para la señal de alarma SIGALRM
    signal(SIGALRM, timer_handler);

    /* * CONFIGURACIÓN DEL TEMPORIZADOR (10ms)
     * tv_usec = 10,000 microsegundos = 10 milisegundos
     */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000; // Intervalo de repetición

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;    // Tiempo para el primer disparo

    // Activar el temporizador real del sistema
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error al configurar el timer");
        return 1;
    }

    /* * BUCLE PRINCIPAL (Super Loop)
     * Ejecuta constantemente las funciones de las tareas. 
     * Cada tarea decide internamente si le toca ejecutarse o no según los 'tics'.
     */
    while(1) {
        tarea1();
        tarea2();
        tarea3();
        // Nota: En un sistema real, aquí se suele poner el CPU en bajo consumo
        // o realizar procesos que no dependan estrictamente del tiempo.
    }

    return 0;
}