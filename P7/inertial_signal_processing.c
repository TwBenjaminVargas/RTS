#define _POSIX_C_SOURCE 200112L
#include <pigpio.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B

#define SCALE_ACCEL 16384.0

#define READ_FREQUENCY_NSEC 10000000 // 10 ms
// FILTER PARAMS
#define FILTER_SIZE 100

volatile sig_atomic_t running = 1;
pthread_mutex_t i2c_mutex;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} RawSensorData;

typedef struct {
    float x;
    float y;
    float z;
} SensorData;

#define QUEUE_NAME  "/mpu6050_queue"
#define MAX_SIZE    10
#define MSG_SIZE   sizeof(RawSensorData)


void moving_average(SensorData* new_reading, SensorData* result);
int MPU6050_init();
void MPU6050_read_raw(int handle, RawSensorData* data);
void MPU6050_read(int handle, RawSensorData* data, SensorData* result);
void MPU6050_process_data(RawSensorData* data, SensorData* result);
void MPU6050_cleanup(int handle);
mqd_t queue_init();
int queue_send(mqd_t mq, RawSensorData* data);
int queue_receive(mqd_t mq, RawSensorData* destiny_buffer);
void queue_cleanup(mqd_t mq);


void moving_average(SensorData* new_reading, SensorData* result) {

    
    static float bufferx[FILTER_SIZE] = {0},
    buffery[FILTER_SIZE] = {0}, bufferz[FILTER_SIZE] = {0};
    
    static int i = 0;

    static float totalxyz[3] = {0};

    static int initial_readings = 0;
    
    totalxyz[0] -= bufferx[i];
    bufferx[i] = new_reading->x;
    totalxyz[0] += new_reading->x;

    totalxyz[1] -= buffery[i];
    buffery[i] = new_reading->y;
    totalxyz[1] += new_reading->y;

    totalxyz[2] -= bufferz[i];
    bufferz[i] = new_reading->z;
    totalxyz[2] += new_reading->z;

    i = (i + 1) % FILTER_SIZE;

    if (initial_readings < FILTER_SIZE) {
        initial_readings++;
    }
    result->x = totalxyz[0] / initial_readings;
    result->y = totalxyz[1] / initial_readings;
    result->z = totalxyz[2] / initial_readings;
}

int MPU6050_init()
{
    pthread_mutex_lock(&i2c_mutex);
    int handle = i2cOpen(1, MPU6050_ADDR, 0);
    if(handle < 0)
    {
        perror("Error initializing MPU6050 sensor");
        pthread_mutex_unlock(&i2c_mutex);
        return handle;

    }
    i2cWriteByteData(handle, PWR_MGMT_1, 0);
    pthread_mutex_unlock(&i2c_mutex);
    return handle;
}

void MPU6050_read_raw(int handle, RawSensorData* data)
{
    uint8_t buffer[6];
    pthread_mutex_lock(&i2c_mutex);
    if (i2cReadI2CBlockData(handle, ACCEL_XOUT_H, (char*)buffer, 6) == 6) {
            
            data->x = (int16_t)( ((uint16_t)buffer[0] << 8) | (uint8_t)buffer[1] ); // Accel X
            data->y = (int16_t)( ((uint16_t)buffer[2] << 8) | (uint8_t)buffer[3] ); // Accel Y
            data->z = (int16_t)( ((uint16_t)buffer[4] << 8) | (uint8_t)buffer[5] ); // Accel Z
        }
    pthread_mutex_unlock(&i2c_mutex);
}

void MPU6050_read(int handle, RawSensorData* data, SensorData* result)
{
    MPU6050_read_raw(handle, data);
    MPU6050_process_data(data, result);
}

void MPU6050_process_data(RawSensorData* data, SensorData* result)
{
    result->x = data->x / SCALE_ACCEL; // Accel X
    result->y = data->y / SCALE_ACCEL; // Accel Y
    result->z = data->z / SCALE_ACCEL; // Accel Z
}
void MPU6050_cleanup(int handle)
{
    pthread_mutex_lock(&i2c_mutex);
    i2cClose(handle);
    pthread_mutex_unlock(&i2c_mutex);
}

int queue_init(mqd_t* mq) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_SIZE;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;
    /*
        - O_CREAT create if not exist
        - O_RDWR read/write
        - 0644 write/read rigths to owner
    */
    *mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (*mq == (mqd_t)-1)
    {
     perror("Error during queue initialization");
     return -1;

    }
    return 0;
}

int queue_send(mqd_t mq, RawSensorData* data) {

    if (mq_send(mq, (const char*)data, MSG_SIZE, 0) == -1) return -1;
    return 0;
}

int queue_receive(mqd_t mq, RawSensorData* destiny_buffer) {
    unsigned int prio;

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_nsec += 50000000; // +50 ms
    if (timeout.tv_nsec >= 1000000000) {
        timeout.tv_sec++;
        timeout.tv_nsec -= 1000000000;
    }

    ssize_t rbytes = mq_timedreceive(mq, (char*)destiny_buffer, MSG_SIZE, &prio, &timeout);
    
    if (rbytes == -1) {
        if (errno == ETIMEDOUT) return 1; 
        return -1;
    }
    return 0;
}
void queue_cleanup(mqd_t mq) {
    if (mq != (mqd_t)-1)
        mq_close(mq);
    mq_unlink(QUEUE_NAME);
}


void* producer(void* arg) {
    int sensor_handle = MPU6050_init();
    if (sensor_handle<0) return NULL;
    
    RawSensorData raw_data;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = READ_FREQUENCY_NSEC;

    mqd_t mq = *(mqd_t*)arg;
    
    while(running) {
        MPU6050_read_raw(sensor_handle, &raw_data);
        queue_send(mq, &raw_data);
        nanosleep(&ts, NULL);
    }

    MPU6050_cleanup(sensor_handle);
    return NULL;

}

void* consumer(void* arg)
{
    mqd_t mq = *(mqd_t*)arg;
    RawSensorData received_data;
    SensorData filtered_data, processed_data;
    while(running) {
        int status = queue_receive(mq, &received_data);
        if( status == 0) {
            MPU6050_process_data(&received_data, &filtered_data);
            moving_average(&filtered_data, &processed_data);
            printf("%.2f\t%.2f\t%.2f\n", processed_data.x, processed_data.y, processed_data.z);
            fflush(stdout);
        }
    }
    return NULL;

}

//TODO - IMPLEMENT ACTUATOR
void* actuator(void* arg) {return NULL;}

void handle_sigint(int sig) {
    running = 0;
}


int main()
{

    signal(SIGINT, handle_sigint);
    
    if (gpioInitialise() < 0) {
        perror("Error during GPIO initialization");
        return 1;
    }
    pthread_mutex_init(&i2c_mutex, NULL);
    
    mq_unlink(QUEUE_NAME);
    mqd_t mq;
    if(queue_init(&mq) < 0)
    {
        queue_cleanup(mq);
        gpioTerminate();
        pthread_mutex_destroy(&i2c_mutex);
        return 1;
    }

    pthread_t producer_thread, consumer_thread, actuator_thread;

    pthread_create(&producer_thread, NULL, producer, (void*)&mq);
    pthread_create(&consumer_thread, NULL, consumer, (void*)&mq);
    pthread_create(&actuator_thread, NULL, actuator, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    pthread_join(actuator_thread, NULL);

    queue_cleanup(mq);
    pthread_mutex_destroy(&i2c_mutex);
    gpioTerminate();

    return 0;
}