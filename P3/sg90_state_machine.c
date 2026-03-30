#include <stdio.h>
#include <pigpio.h>
#include <stdatomic.h>
#include <signal.h>

#define PIN_SG90 21
#define PIN_BUTTON 26
#define DEBOUNCE_TIME 2000
#define MIN_OPERATION_RANGE 500 // 500us = 0 degrees
#define MAX_OPERATION_RANGE 2500 // 2500us = 180 degrees


// volatile not required
atomic_int current_degrees = 0;
atomic_bool keepRunning = 1;

void isr(int gpio, int level, uint32_t tick) {
    if (level == RISING_EDGE) {
        int actual = atomic_load(&current_degrees);
        int next = actual + 30;
        if (next > 180) {
            next = 0;
        }
        atomic_store(&current_degrees, next);
    }

}

int degrees_to_us(int degrees) {
    return MIN_OPERATION_RANGE + (degrees * (MAX_OPERATION_RANGE - MIN_OPERATION_RANGE) / 180);
}

void signalHandler(int signum) {
    atomic_store(&keepRunning, 0);
}

int main() {


    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error: No se pudo inicializar pigpio\n");
        return 1;
    }

    gpioSetMode(PIN_BUTTON, PI_INPUT);
    signal(SIGINT, signalHandler);

    gpioSetPullUpDown(PIN_BUTTON, PI_PUD_DOWN);
    
    gpioGlitchFilter(PIN_BUTTON, DEBOUNCE_TIME);

    gpioSetAlertFunc(PIN_BUTTON, isr);

    gpioServo(PIN_SG90, degrees_to_us(0));

    int degrees = atomic_load(&current_degrees);
    int pulse_width = degrees_to_us(degrees);

    printf("\rDegrees: %3d | Pulse Width: %4dus    ", degrees, pulse_width);
    fflush(stdout);

    while (atomic_load(&keepRunning)) {

        int new_degrees = atomic_load(&current_degrees);

        
        if (degrees != new_degrees) 
        {
            degrees = new_degrees;
            pulse_width = degrees_to_us(degrees);
            
            gpioServo(PIN_SG90, pulse_width);

            printf("\rDegrees: %3d | Pulse Width: %4dus    ", degrees, pulse_width);
            fflush(stdout);

            time_sleep(0.02); // 20ms delay for servo update
        }

        time_sleep(0.01); // 10ms delay to reduce CPU usage
    }
    
    gpioServo(PIN_SG90, 0);
    gpioTerminate();
    
    return 0;

}