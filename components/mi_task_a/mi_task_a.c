#include "mi_task_a.h"
#include "mi_led.h"

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
TaskHandle_t mi_task_a_start(QueueHandle_t) {
    TaskHandle_t handle = NULL;
    xTaskCreate(vTaskA, "ParpadeoLED", 2048, NULL, 2, &handle);
    return handle;
}