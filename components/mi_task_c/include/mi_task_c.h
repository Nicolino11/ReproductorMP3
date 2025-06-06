#include "freertos/FreeRTOS.h"
#include "mi_queue.h"
#include "../../led_strip/include/led_strip.h"
#include "../../mi_delay/include/mi_delay.h"

typedef void* task_handle_t;

task_handle_t mi_task_c_start(QueueHandle_t queue);

void turn_led_on(led_strip_t *strip, int a, int b, int c);

void turn_led_off(led_strip_t *strip);

void set_led_brightness(led_strip_t *strip, int base_r, int base_g, int base_b, float brightness);

