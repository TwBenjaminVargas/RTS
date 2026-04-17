
#include <pthread.h>
#include <unistd.h>
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>


// SCHEDULE FIFO PRIORITIES
#define DATA_ADUQISITION_PRIORITY 80
#define LOGIC_CONTROL_PRIORITY 40
#define DATA_ADQUISITION_INTERVAL_SEC 1

// AHT10 
#define AHT10_ADDR 0x38 // I2C address of the AHT10 sensor
#define AHT10_INIT_TIME_MS 20 // Time required for the AHT10 to initialize
#define AHT10_CONVERSION_TIME_MS 80 // Time required for the AHT10 to perform a measurement

// VENTILATION CONTROL
#define VENTILATION_GPIO_PIN 17
#define VENTILATION_TIMEOUT_SEC 60
#define VENTILATION_DURATION_SEC 120

// SYSTEM CONFIGS
#define MESUREMENTS_FREQUENCY_SEC 1
#define SYSTEM_REPORT_FREQUENCY_SEC 5
#define TEMPERATURE_SUPERIOR_THRESHOLD 30.0
#define TEMPERATURE_INFERIOR_THRESHOLD 25.0
#define TEMPERATURE_CHECK_INTERVAL_SEC 10


pthread_mutex_t temp_mutex,state_mutex;
float current_temperature = 0.0;

typedef enum {
    REPOSE,
    ALERT,
    VENTILATION
} state_t;

state_t current_state = REPOSE;

//function headers
void set_priority(pthread_attr_t *attr, int priority);
void ventilate();
int AHT10_init();
void AHT10_read(int handle, float* mesurements);
void AHT10_cleanup(int handle);
void turn_on_ventilation();
void turn_off_ventilation();

void set_priority(pthread_attr_t *attr, int priority) {
    struct sched_param param;
    pthread_attr_init(attr);
    pthread_attr_setschedpolicy(attr, SCHED_FIFO);
    param.sched_priority = priority;
    pthread_attr_setschedparam(attr, &param);
    pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
}

void* data_adquisiton(void* arg) {
    
    struct timespec ts;
    ts.tv_sec = MESUREMENTS_FREQUENCY_SEC;
    ts.tv_nsec = 0;

    int sensor_handle = AHT10_init();
    if(sensor_handle < 0) {
        printf("Error initializing AHT10 sensor: %d\n", sensor_handle);
        exit(1);
    }

    float mesurements[2];

    while(1) {
        AHT10_read(sensor_handle, mesurements);
        pthread_mutex_lock(&temp_mutex);
        current_temperature = mesurements[1];
        pthread_mutex_unlock(&temp_mutex);
        nanosleep(&ts, NULL);
    }

    close(sensor_handle);
    pthread_mutex_destroy(&temp_mutex);
}


void* logic_control(void* arg) {

    struct timespec ts;
    ts.tv_sec = TEMPERATURE_CHECK_INTERVAL_SEC;
    ts.tv_nsec = 0;

    uint32_t last_state_change = 0, tick=0;
    state_t current_state_local;
    float temperature_local = 0.0;
    
    while(1)
    {
        

        pthread_mutex_lock(&temp_mutex);
        temperature_local = current_temperature;
        pthread_mutex_unlock(&temp_mutex);
        
        pthread_mutex_lock(&state_mutex);
        current_state_local = current_state;
        pthread_mutex_unlock(&state_mutex);

        tick = gpioTick();
        
        switch(current_state_local) {
            case REPOSE:
                if(temperature_local > TEMPERATURE_SUPERIOR_THRESHOLD) {
                    current_state_local = ALERT;
                    last_state_change = tick;
                }
                break;
            
            case ALERT:
                if(temperature_local <= TEMPERATURE_SUPERIOR_THRESHOLD) {
                    current_state_local = REPOSE;
                } else if(tick - last_state_change > (uint32_t)VENTILATION_TIMEOUT_SEC * 1000000) {
                    current_state_local = VENTILATION;
                    last_state_change = tick;
                    turn_on_ventilation();
                }
                break;
            
            case VENTILATION:
                if(temperature_local < TEMPERATURE_INFERIOR_THRESHOLD || 
                   tick - last_state_change > (uint32_t)VENTILATION_DURATION_SEC * 1000000) {
                    turn_off_ventilation();
                    current_state_local = REPOSE;
                    last_state_change = tick;
                }
                break;
        }       

        pthread_mutex_lock(&state_mutex);
        current_state = current_state_local;
        pthread_mutex_unlock(&state_mutex);

        nanosleep(&ts, NULL);

    }

}


void* interface(void* arg) {

    struct timespec ts;
    ts.tv_sec = SYSTEM_REPORT_FREQUENCY_SEC;
    ts.tv_nsec = 0;
    
    uint32_t intial_tick = gpioTick();
    float temperature_local;
    state_t current_state_local;

    const char* state_names[] = {"REPOSE", "ALERT", "VENTILATION"};

    while (1) {
        
        uint32_t current_tick = gpioTick();
        uint32_t time_elapsed = current_tick - intial_tick;

        pthread_mutex_lock(&temp_mutex);
        temperature_local = current_temperature;
        pthread_mutex_unlock(&temp_mutex);
        pthread_mutex_lock(&state_mutex);
        current_state_local = current_state;
        pthread_mutex_unlock(&state_mutex);

        printf("\rTemp: %6.2f °C | State: %-11s | Time: %u s", 
               temperature_local, 
               state_names[current_state_local], 
               time_elapsed / 1000000);
        fflush(stdout);

        nanosleep(&ts, NULL);
        
    }
}

void turn_on_ventilation() {
    gpioWrite(VENTILATION_GPIO_PIN, 1);
}

void turn_off_ventilation() {
    gpioWrite(VENTILATION_GPIO_PIN, 0);
}

int AHT10_init()
{

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = AHT10_INIT_TIME_MS * 1000 * 1000;

    int handle = i2cOpen(1, AHT10_ADDR, 0);
    if (handle < 0) return handle;

    char init_cmd[] = {0xE1, 0x08, 0x00};
    i2cWriteDevice(handle, init_cmd, 3);

    nanosleep(&ts, NULL);

    return handle;
}

void AHT10_read(int handle, float* mesurements)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = AHT10_CONVERSION_TIME_MS * 1000 * 1000;
    
    char trigger_cmd[] = {0xAC, 0x33, 0x00};
    i2cWriteDevice(handle, trigger_cmd, 3);

    nanosleep(&ts, NULL);

    char buffer[6];

    if(i2cReadDevice(handle, buffer, 6) < 0) {
        printf("Error reading from AHT10 sensor\n");
        exit(1);
    }

    /*
    uint32_t humidity_raw = ((buffer[1] << 12) | (buffer[2] << 4) | (buffer[3] >> 4));
    uint32_t temp_raw = (((buffer[3] & 0x0F) << 16) | (buffer[4] << 8) | buffer[5]);
    */
    uint32_t humidity_raw = (((unsigned char)buffer[1] << 12) | ((unsigned char)buffer[2] << 4) | ((unsigned char)buffer[3] >> 4));
    uint32_t temp_raw = ((((unsigned char)buffer[3] & 0x0F) << 16) | ((unsigned char)buffer[4] << 8) | (unsigned char)buffer[5]);
    
    mesurements[0] = (float)humidity_raw * 100 / 1048576;
    mesurements[1] = ((float)temp_raw * 200 / 1048576) - 50;
}

void AHT10_cleanup(int handle)
{
    i2cClose(handle);
    gpioTerminate();
}


int main()
{

    if (gpioInitialise() < 0) return 1;


    pthread_mutex_init(&temp_mutex, NULL);
    pthread_mutex_init(&state_mutex, NULL);

    pthread_t data_adquisition_th, logic_control_th, interface_th;
    pthread_attr_t data_adquisition_attr, logic_control_attr;


    set_priority(&data_adquisition_attr, DATA_ADUQISITION_PRIORITY);
    set_priority(&logic_control_attr, LOGIC_CONTROL_PRIORITY);
    // The interface thread will run with the default priority (SCHED_OTHER)

    pthread_create(&data_adquisition_th, &data_adquisition_attr, data_adquisiton, NULL);
    pthread_create(&logic_control_th, &logic_control_attr, logic_control, NULL);
    pthread_create(&interface_th, NULL, interface, NULL);

    pthread_attr_destroy(&data_adquisition_attr);
    pthread_attr_destroy(&logic_control_attr);

    pthread_detach(data_adquisition_th);
    pthread_detach(logic_control_th);
    pthread_detach(interface_th);

    pthread_exit(NULL);
    return 0;
}
