#include "freertos/FreeRTOS.h"
#include "mi_task_c.h"
#include "esp_log.h"

static void vTaskC(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    mi_evento_t evento;
    while (1) {
        if (mi_queue_receive(queue, &evento, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI("mi_task_c", "Consumido: R=%d G=%d B=%d T=%d", evento.r, evento.g, evento.b, evento.tiempo_ms);
            // Aquí podrías usar los valores RGB y tiempo_ms para controlar el LED
        }
    }
}

task_handle_t mi_task_c_start(QueueHandle_t queue) {
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskC, "ConsumidorC", 2048, (void*)queue, 2, &handle);
    return handle;
}
