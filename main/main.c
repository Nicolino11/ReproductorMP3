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


led_strip_t *strip; //--> Parametro que maneja al LED
int R = 255, G = 0, B = 0; //--> Colores del LED

void vTaskA(void *pvParameters){
    while(1){ //--> Loop para hacer parpadear al LED
        turn_led_on(strip, R, G, B);
        vTaskDelay(pdMS_TO_TICKS(500));
        turn_led_off(strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    led_rgb_init(&strip); //--> Iniciamos el LED

    // Inicializar la queue
    QueueHandle_t queue = mi_queue_init(10);

    // Lanzar productor y consumidor
    mi_task_b_start(queue);
    mi_task_c_start(queue);

    //xTaskCreate( //--> Creamos vTaskA para hacer parpadear al LED
    //    vTaskA,
    //    "ParpadeoLED",
    //    1800, //--> Con menos de 1800 se rompe todo
    //    NULL,
    //    1,
    //    NULL
    //);
}