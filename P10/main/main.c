#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t colaDatos;

void tareaProductora(void *pvParameters)
{
    int valor;

    while (1)
    {
        valor = rand() % 101;

        if (xQueueSend(colaDatos, &valor, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            printf("Productor (Core %d): dato enviado = %d\n",
                   xPortGetCoreID(),
                   valor);
        }
        else
        {
            printf("Productor: cola llena, dato descartado\n");
        }

        // Espera 500 ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void tareaConsumidora(void *pvParameters)
{
    int valorRecibido;

    while (1)
    {
        if (xQueueReceive(colaDatos, &valorRecibido, portMAX_DELAY) == pdTRUE)
        {
            printf("Consumidor (Core %d): valor recibido = %d\n",
                xPortGetCoreID(),
                valorRecibido);
            
            printf("Procesando...\n");

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    colaDatos = xQueueCreate(10, sizeof(int));

    if (colaDatos == NULL)
    {
        printf("Error al crear la cola\n");
        return;
    }

    printf("Cola creada correctamente\n");

    // Crea tarea productora en Core 1
    xTaskCreatePinnedToCore(
        tareaProductora,    
        "Productor",        
        2048,              
        NULL,               
        2,     
        NULL,       
        1          
    );

    // Crea tarea consumidora en Core 0
    xTaskCreatePinnedToCore(
        tareaConsumidora,
        "Consumidor",
        2048,
        NULL,
        1,
        NULL,
        0                    
    );
}