#include "mi_task_b.h"
#include "esp_log.h"
#include "mi_queue.h"

static void vTaskB(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    mi_evento_t evento;
    int i = 0;
    while (1) {
        // Generar color RGB y tiempo
        evento.r = (i * 50) % 256;
        evento.g = (i * 80) % 256;
        evento.b = (i * 120) % 256;
        evento.tiempo_ms = 500 + (i % 5) * 100;
        mi_queue_send(queue, &evento, portMAX_DELAY);
        ESP_LOGI("mi_task_b", "Producido: R=%d G=%d B=%d T=%d", evento.r, evento.g, evento.b, evento.tiempo_ms);
        vTaskDelay(pdMS_TO_TICKS(1000));
        i++;
    }
}

TaskHandle_t mi_task_b_start(QueueHandle_t queue) {
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskB, "ConsumidorB", 2048, (void*)queue, 2, &handle);
    return handle;
}
