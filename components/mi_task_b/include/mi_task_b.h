#include "freertos/FreeRTOS.h"
#include "mi_queue.h"

// Inicializa y lanza la tarea consumidora
TaskHandle_t mi_task_b_start(QueueHandle_t queue);
