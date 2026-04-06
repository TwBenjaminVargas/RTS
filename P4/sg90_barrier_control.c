#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <pigpio.h>
#include <stdatomic.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define PIN_SG90 21
#define PIN_BUTTON 26
#define PIN_LED 20
#define DEBOUNCE_TIME 2000
#define MIN_OPERATION_RANGE 500 // 500us = 0 degrees
#define MAX_OPERATION_RANGE 2500 // 2500us = 180 degrees
#define STEP_DEGREES 20 // step size for incrementing position
#define REPORT_INTERVAL_SEC 1 // interval for reporting servo position

int degrees_to_us(int degrees);


// volatile not required
atomic_int current_degrees = 0;
atomic_bool keepRunning = 1, keepRunningServo = 1;


int degrees_to_us(int degrees) {
    return MIN_OPERATION_RANGE + (degrees * (MAX_OPERATION_RANGE - MIN_OPERATION_RANGE) / 180);
}

void timer_handler(union sigval sv) {
    printf("\r[INFO] Posición actual del servo: %d grados, Sistema en modo: %s                 ", atomic_load(&current_degrees), atomic_load(&keepRunningServo) ? "Seguro" : "Alerta");
    fflush(stdout);
}

void* servo_thread(void* arg) 
{
    int position = 0; // initial position
    int step = STEP_DEGREES; 
    while (atomic_load(&keepRunning))
    {
        if(!atomic_load(&keepRunningServo)) {
            time_sleep(0.02);
            continue;
        }
        gpioServo(PIN_SG90, degrees_to_us(position));
        atomic_store(&current_degrees, position);
        position += step;
        if (position >= 180 || position <= 0) step = -step;
        time_sleep(0.02); 
    }
    return NULL;
    
}
//warning: bussy wait implementation.
void* button_thread(void* arg) 
{
    int alert_state = 0;
    int button_state = 0, button_state_prev = 0;
    while (atomic_load(&keepRunning))
    {
        button_state = gpioRead(PIN_BUTTON);
        if (button_state == 1 && button_state_prev == 0)
        {
            alert_state = !alert_state;
            gpioWrite(PIN_LED, alert_state);
            atomic_store(&keepRunningServo, !alert_state);
            // do not abuse of printf in high priority threads, but for demonstration purposes is ok.
            if(alert_state){
                printf("\r[ALERTA] Parada de emergencia activada - Latencia detectada                 ");
                fflush(stdout);
            }   
        }
        button_state_prev = button_state;

        // short sleep to prevent starvation on servo thread, but still responsive to button presses.
        time_sleep(0.0001);
    }
    return NULL;  
}

void signalHandler(int signum) {
    atomic_store(&keepRunning, 0);
}

int main() {


    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error: No se pudo inicializar pigpio\n");
        return 1;
    }
    signal(SIGINT, signalHandler);

    gpioSetMode(PIN_BUTTON, PI_INPUT);
    gpioSetMode(PIN_LED, PI_OUTPUT);
    
    gpioServo(PIN_SG90, degrees_to_us(0));
    
    gpioSetPullUpDown(PIN_BUTTON, PI_PUD_DOWN);
    
    gpioGlitchFilter(PIN_BUTTON, DEBOUNCE_TIME);

    pthread_t thread_high, thread_low;

    pthread_attr_t attr_high, attr_low;
    
    struct sched_param param;


    pthread_attr_init(&attr_high);
    pthread_attr_setschedpolicy(&attr_high, SCHED_FIFO);
    param.sched_priority = 80;                                                // high priority for the button control thread
    pthread_attr_setschedparam(&attr_high, &param);
    pthread_attr_setinheritsched(&attr_high, PTHREAD_EXPLICIT_SCHED);
    
    pthread_attr_init(&attr_low);
    pthread_attr_setschedpolicy(&attr_low, SCHED_FIFO);
    param.sched_priority = 10;                                               // low priority for the servo control thread
    pthread_attr_setinheritsched(&attr_low, PTHREAD_EXPLICIT_SCHED);
    
    gpioServo(PIN_SG90, degrees_to_us(0));
    
    //timer setup

    struct sigevent sev;
    struct itimerspec its;
    timer_t timer_id;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = timer_handler;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = &timer_id;

    timer_create(CLOCK_REALTIME, &sev, &timer_id);

    its.it_value.tv_sec = REPORT_INTERVAL_SEC;                            // first report after REPORT_INTERVAL_SEC second
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = REPORT_INTERVAL_SEC;                         // Report every REPORT_INTERVAL_SEC second
    its.it_interval.tv_nsec = 0;

    
    timer_settime(timer_id, 0, &its, NULL);
    pthread_create(&thread_high, &attr_high,button_thread, NULL);
    pthread_create(&thread_low, &attr_low, servo_thread, NULL);
    

    pthread_join(thread_high, NULL);
    pthread_join(thread_low, NULL);

    pthread_attr_destroy(&attr_high);
    pthread_attr_destroy(&attr_low);

    gpioServo(PIN_SG90, 0);
    gpioWrite(PIN_LED, 0);
    timer_delete(timer_id);
    gpioTerminate();
    
    return 0;

}