#include <stdio.h>
#include <pigpio.h>
#include <signal.h>

#define LED_PIN 24

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signum) {
    keepRunning = 0;
}

int main() {
    signal(SIGINT, signalHandler);
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error: Failed to initialize pigpio\n");
        return 1;
    }
    gpioSetMode(LED_PIN, PI_OUTPUT);

    while (keepRunning)
    {
        gpioWrite(LED_PIN, 1);
        printf("LED ON\n");
        time_sleep(0.5); // blocking delay
        gpioWrite(LED_PIN, 0);
        printf("LED OFF\n");
        time_sleep(0.5);
    }
    
    
    gpioTerminate(); 

    return 0;
}