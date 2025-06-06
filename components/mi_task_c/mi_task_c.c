#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mi_task_c.h"
#include "esp_log.h"
#include "../led_strip/include/led_strip.h"
#include "../mi_delay/include/mi_delay.h"
#include "../mi_queue/include/mi_queue.h"

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

            // Espera si se indica un tiempo
            if (evento.tiempo_ms > 0)
            {
                delay(evento.tiempo_ms, 'm');
            }

            // Encendido si al menos uno de los colores es distinto de 0
            if (evento.r > 0 || evento.g > 0 || evento.b > 0)
            {
                turn_led_on(strip, evento.r, evento.g, evento.b);
            }
            else
            {
                turn_led_off(strip);
            }
        }
    }
}

task_handle_t mi_task_c_start(QueueHandle_t queue)
{
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskC, "ConsumidorC", 2048, (void *)queue, 2, &handle);
    return handle;
}
