#include "freertos/FreeRTOS.h"

//--> led_color_t tiene los valores rgb
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

//--> task_a_start crea vTaskA
void task_a_start(TaskHandle_t *task_handle);

//-->task_a_set_shared_resources se trae de main el color, el semaforo y el strip globales de main
void task_a_set_shared_resources(led_color_t *color_ptr, SemaphoreHandle_t sem_ptr, void *strip_ptr);
