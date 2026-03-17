#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pigpio.h>
#include <signal.h>

#define LED_PIN    24
#define BUTTON_PIN 23
#define DEBOUNCE_TIME 50000 // 50ms en microsegundos

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
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    //gpioSetPullUpDown(BUTTON_PIN, PI_PUD_DOWN); 

    int button_last_status = 0;
    int button_current_status = 0;
    uint32_t last_change_tick = 0;

    printf("System's ready! Press the button\n");
    printf("BUTTON NOT PRESSED");
    fflush(stdout);

    while (keepRunning) {
        int reading = gpioRead(BUTTON_PIN);
        uint32_t now = gpioTick();

        if (reading != button_last_status) {
            last_change_tick = now;
        }

        if ((now - last_change_tick) > DEBOUNCE_TIME) {
            
            if (reading != button_current_status) {
                button_current_status = reading;
                gpioWrite(LED_PIN, !button_current_status);

                if (button_current_status == 1) {
                    printf("\rBUTTON NOT PRESSED        "); 
                } else {
                    printf("\rBUTTON PRESSED    ");
                }
                fflush(stdout);
            }
        }

        button_last_status = reading;
        gpioDelay(1000);
    }

    gpioWrite(LED_PIN, 0);
    gpioTerminate();
    return 0;
}