#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "include/mi_task_c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "../led_strip/include/led_strip.h"
#include "../mi_delay/include/mi_delay.h"
#include "../mi_queue/include/mi_queue.h"
// NO incluir mi_task_a.h aquí porque ya está incluido en mi_task_c.h

// Variables globales
static led_color_t *shared_color = NULL;
static SemaphoreHandle_t shared_mutex = NULL;

static void vTaskC(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    mi_evento_t evento;

    led_strip_t *strip;
    led_rgb_init(&strip);

    while (1)
    {
        if (mi_queue_receive(queue, &evento, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("mi_task_c", "Consumido: R=%d G=%d B=%d T=%dms",
                     evento.r, evento.g, evento.b, evento.tiempo_ms);

            // Proteger acceso a shared_color
            if (shared_mutex != NULL && shared_color != NULL)
            {
                if (xSemaphoreTake(shared_mutex, portMAX_DELAY) == pdTRUE)
                {
                    shared_color->r = evento.r;
                    shared_color->g = evento.g;
                    shared_color->b = evento.b;
                    xSemaphoreGive(shared_mutex);
                    ESP_LOGI("mi_task_c", "Color global actualizado: R=%d G=%d B=%d", 
                             evento.r, evento.g, evento.b);
                }
            }

            // Aplicar el color al LED
            if (evento.r > 0 || evento.g > 0 || evento.b > 0) {
                turn_led_on(strip, evento.r, evento.g, evento.b);
            } else {
                turn_led_off(strip);
            }

            // Manejar el tiempo de espera si es necesario
            if (evento.tiempo_ms > 0)
            {
                SemaphoreHandle_t timer_semaphore = xSemaphoreCreateBinary();

                void timer_callback(void *arg)
                {
                    SemaphoreHandle_t *sem = (SemaphoreHandle_t *)arg;
                    xSemaphoreGive(*sem);
                }

                const esp_timer_create_args_t timer_args = {
                    .callback = &timer_callback,
                    .arg = (void *)&timer_semaphore,
                    .name = "one_shot_timer"};

                esp_timer_handle_t timer;
                esp_timer_create(&timer_args, &timer);
                esp_timer_start_once(timer, evento.tiempo_ms * 1000); // Convertir a us

                xSemaphoreTake(timer_semaphore, portMAX_DELAY);

                vSemaphoreDelete(timer_semaphore);
                esp_timer_stop(timer);
                esp_timer_delete(timer);
            }
        }
    }
}

void task_c_set_shared_resources(led_color_t *color, SemaphoreHandle_t mutex)
{
    shared_color = color;
    shared_mutex = mutex;
}

task_handle_t mi_task_c_start(QueueHandle_t queue)
{
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskC, "ConsumidorC", 3000, (void *)queue, 2, &handle);
    return handle;
}