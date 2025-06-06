#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "mi_led.h"
#include "mi_delay.h"
#include "mi_touch.h"
#include "mi_wifi_ap.h"
#include "mi_web_server.h"
#include "mi_queue.h"
#include "mi_task_b.h"
#include "mi_task_c.h"
#include "mi_task_a.h"

//--> Variables globales compartidas
led_color_t global_color = {255, 0, 0};
SemaphoreHandle_t mutex_color;
led_strip_t *strip;

void app_main(void)
{
    led_rgb_init(&strip); 


    mutex_color = xSemaphoreCreateMutex();
    // Inicializar la queue
    QueueHandle_t queue = mi_queue_init(10);

    // Lanzar productor y consumidor
    mi_task_b_start(queue);
    mi_task_c_start(queue);
    //mi_task_a_start(NULL);

}