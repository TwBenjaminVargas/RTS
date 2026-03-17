#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pigpio.h>
#include <signal.h>

#define LED_PIN    24
#define BUTTON_PIN 23
#define DEBOUNCE_TIME 50

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signum) {
    keepRunning = 0;
}


void interrupt_handler(int gpio, int level, uint32_t tick) {
    gpioWrite(LED_PIN, !level);
    
    if (level == 0) {
        printf("\rBUTTON PRESSED        ");
    } else {
        printf("\rBUTTON NOT PRESSED    ");
    }
    fflush(stdout);
}

int main() {
    signal(SIGINT, signalHandler);
    if (gpioInitialise() < 0)
    {
        printf("Error: Failed to initialise pigpio\n");
        return 1;
    }

    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    //gpioSetPullUpDown(BUTTON_PIN, PI_PUD_DOWN);

    // CONFIGURACIÓN DE LA INTERRUPCIÓN
    // EITHER_EDGE: detecta cuando sube (1) y cuando baja (0)
    // 300: Tiempo de debounce por hardware (en milisegundos)
    // interrupt_handler: la función que se ejecuta al detectar el evento
    gpioSetISRFunc(BUTTON_PIN, EITHER_EDGE, DEBOUNCE_TIME, interrupt_handler);

    printf("System ready with Interrupts!\n");

    while (keepRunning) {

        gpioDelay(100000); 
    }

    gpioWrite(LED_PIN, 0);
    gpioTerminate();
    return 0;
}