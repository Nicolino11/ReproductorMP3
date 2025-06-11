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
{   //--> Iniciamos el LED
    led_rgb_init(&strip); 

    //--> Creamos la queue
    QueueHandle_t queue = mi_queue_init(10);

     //--> Creamos el Semaforo mutex
    mutex_color = xSemaphoreCreateMutex();
    
    //--> Enviamos las variables compartidas a vTaskA
    task_a_set_shared_resources(&global_color, mutex_color, strip);
    task_c_set_shared_resources(&global_color, mutex_color);


    // Lanzar productor y consumidor
    mi_task_b_start(queue);
    mi_task_c_start(queue);

    TaskHandle_t taskA_handle = NULL;
    task_a_start(&taskA_handle);
}