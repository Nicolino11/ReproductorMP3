#include "freertos/FreeRTOS.h"
#include "mi_queue.h"

typedef void* task_handle_t;

task_handle_t mi_task_c_start(QueueHandle_t queue);
