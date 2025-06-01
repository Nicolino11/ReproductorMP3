#include "mi_led.h"
#include "mi_delay.h"
#include "mi_touch.h"
#include "mi_wifi_ap.h"
#include "mi_web_server.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"

led_strip_t *strip;
int R = 255, G = 0, B = 0;

void vTaskA(void *pvParameters){
    while(1){ 
        turn_led_on(strip, R, G, B);
        vTaskDelay(pdMS_TO_TICKS(500));
        turn_led_off(strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    led_rgb_init(&strip);

    xTaskCreate(
        vTaskA,
        "ParpadeoLED",
        1800,
        NULL,
        1,
        NULL
    );
}