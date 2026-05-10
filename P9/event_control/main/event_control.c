#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#define PIN_BUTTON GPIO_NUM_26
#define PIN_LED GPIO_NUM_2

SemaphoreHandle_t sem = NULL;
int64_t last_event = 0;

static void IRAM_ATTR isr(void* arg) {
    BaseType_t high_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(sem, &high_priority_task_woken);
    if (high_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void data_processing(void *pvParameter) {
    bool led_state = false;
    while(1) {
        if (xSemaphoreTake(sem, portMAX_DELAY) == pdTRUE) {
            int64_t current_time = esp_timer_get_time();
            int64_t delta = (current_time - last_event) / 1000;

            led_state = !led_state;
            gpio_set_level(PIN_LED, led_state);
            
            printf("Evento detectado! Delta: %lld ms\n", delta);
            last_event = current_time;
        }
    }
}

void telemetry(void *pvParameter) {
    while(1) {
        printf("Sistema Operativo Saludable\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {

    gpio_reset_pin(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(PIN_BUTTON);
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
    gpio_set_intr_type(PIN_BUTTON, GPIO_INTR_NEGEDGE);
    gpio_pullup_en(PIN_BUTTON);

    sem = xSemaphoreCreateBinary();

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_BUTTON, isr, (void*) PIN_BUTTON);
 
    xTaskCreatePinnedToCore(data_processing, "Procesamiento", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(telemetry, "Telemetria", 2048, NULL, 1, NULL, 1);
}