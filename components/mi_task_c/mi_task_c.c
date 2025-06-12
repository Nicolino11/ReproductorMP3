#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "mi_task_c.h"
#include "esp_log.h"
#include "mi_queue.h"

static const char *TAG = "mi_task_c";

//--> Variables globales
static led_color_t *shared_color = NULL;
static SemaphoreHandle_t shared_mutex = NULL;

static void timer_callback(TimerHandle_t xTimer) {
    led_color_t *color = (led_color_t *)pvTimerGetTimerID(xTimer);

    if (shared_mutex != NULL && color != NULL) {
        if (xSemaphoreTake(shared_mutex, portMAX_DELAY) == pdTRUE) {
            shared_color->r = color->r;
            shared_color->g = color->g;
            shared_color->b = color->b;

            ESP_LOGI(TAG, "Color actualizado desde timer: R=%d G=%d B=%d", color->r, color->g, color->b);
            xSemaphoreGive(shared_mutex);
        }
    }
    free(color);
}
static void vTaskC(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    mi_evento_t evento;

    while (1){
        if (mi_queue_receive(queue, &evento, portMAX_DELAY) == pdTRUE){
            ESP_LOGI(TAG, "Consumido: R=%d G=%d B=%d T=%dms",
                    evento.r, evento.g, evento.b, evento.tiempo_ms);
                     
            led_color_t *nuevo_color = malloc(sizeof(led_color_t));
            if (nuevo_color == NULL) {
                ESP_LOGE(TAG, "Error al asignar memoria dinámica");
                continue;
            }
            nuevo_color->r = evento.r;
            nuevo_color->g = evento.g;
            nuevo_color->b = evento.b;
            
            TimerHandle_t timer = xTimerCreate(
                "ColorTimer",
                pdMS_TO_TICKS(evento.tiempo_ms),
                pdFALSE, //--> one-shot
                (void *)nuevo_color,
                timer_callback);
            if (timer == NULL) {
                ESP_LOGE(TAG, "Error al crear el timer");
                free(nuevo_color);
                continue;
            }
            //--> Iniciar el timer
            if (xTimerStart(timer, 0) != pdPASS) {
                ESP_LOGE(TAG, "Error al iniciar el timer");
                free(nuevo_color);
                xTimerDelete(timer, 0);
            }
        }
    }
}

void task_c_set_shared_resources(led_color_t *color, SemaphoreHandle_t mutex){
    shared_color = color;
    shared_mutex = mutex;
}

task_handle_t mi_task_c_start(QueueHandle_t queue){
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskC, "ConsumidorC", 4096, (void *)queue, 2, &handle);
    return handle;
}