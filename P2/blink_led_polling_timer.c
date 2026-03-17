#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pigpio.h>
#include <signal.h>

#define LED_PIN 24
#define WAIT_TIME 500000


volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signum) {
    keepRunning = 0;
}

int main() {
    signal(SIGINT, signalHandler);
    if (gpioInitialise() < 0) {
        printf("Error: Failed to initialise pigpio\n");
        return 1;
    }
    gpioSetMode(LED_PIN, PI_OUTPUT);


    uint32_t tick_led = gpioTick();
    
    int led_status = 0;

    while (keepRunning) {
        
        uint32_t now = gpioTick();

        if (now - tick_led >= WAIT_TIME) {
            led_status = !led_status;
            gpioWrite(LED_PIN, led_status);
            printf("LED %s\n", led_status ? "ON " : "OFF");
            tick_led = now;
        }

        gpioDelay(1000); 
    }

    gpioTerminate();

    return 0;
}