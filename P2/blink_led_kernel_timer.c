#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE  // Para que reconozca usleep

#include <signal.h>
#include <pigpio.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define WAIT_TIME 1
#define LED_PIN 24

timer_t timerid;

volatile sig_atomic_t led_status = 0, keepRunning = 1;

void timer_handler(int sig) {
    led_status = !led_status;
    //printf("\nLED %s\n", led_status ? "ON " : "OFF");
    gpioWrite(LED_PIN, led_status);
}

void loop_exit(int sig) {
    keepRunning = 0;
}

void set_timer() {
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN, &sa, NULL);

    
    sev.sigev_notify = SIGEV_SIGNAL; 
    sev.sigev_signo = SIGRTMIN;      
    sev.sigev_value.sival_ptr = &timerid;

    
    timer_create(CLOCK_MONOTONIC, &sev, &timerid);

    
    its.it_value.tv_sec = WAIT_TIME;
    its.it_value.tv_nsec = 0;
    
    its.it_interval.tv_sec = WAIT_TIME;
    its.it_interval.tv_nsec = 0;


    timer_settime(timerid, 0, &its, NULL);
}

int main() {

    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error: Failed to initialize pigpio\n");
        return 1;
    }
    gpioSetMode(LED_PIN, PI_OUTPUT);


    set_timer();
    signal(SIGINT, loop_exit);

    int puntos = 0;
    while(keepRunning) {
        printf("\rRunning");
        for (int i = 0; i < 3; i++) {
            if (i < puntos) printf(".");
            else printf(" ");
        }
        fflush(stdout);
        puntos = (puntos + 1) % 4;
        usleep(500000);
    }
    gpioTerminate();
    return 0;
}