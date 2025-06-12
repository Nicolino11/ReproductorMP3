#ifndef MI_TASK_C_H
#define MI_TASK_C_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mi_queue.h"
#include "../../mi_task_a/include/mi_task_a.h"  // Para obtener led_color_t

typedef void* task_handle_t;

task_handle_t mi_task_c_start(QueueHandle_t queue);

void task_c_set_shared_resources(led_color_t *color, SemaphoreHandle_t mutex);

#endif