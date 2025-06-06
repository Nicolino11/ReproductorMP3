#include "mi_task_a.h"
#include "mi_led.h"

static led_strip_t *strip; //--> strip tiene al LED de la placa
static led_color_t *shared_color; //--> shared_color es el puntero a la variable global de color
static SemaphoreHandle_t shared_mutex; //--> shared_mutex es el mutex compartido

void task_a_set_shared_resources(led_color_t *color_ptr, SemaphoreHandle_t sem_ptr, void *strip_ptr) {
    //--> Traemos de main los valores globales necesarios
    shared_color = color_ptr; 
    shared_mutex = sem_ptr;
    strip = strip_ptr;
}

static void task_a(void *arg) {
    led_color_t current_color;
    while (1) {
        if (xSemaphoreTake(shared_mutex, pdMS_TO_TICKS(50))) { //--> Pedimos el semaforo
            current_color = *shared_color; //--> actualizamos el color
            xSemaphoreGive(shared_mutex); //--> Devolvemos el semaforo
        } else {
            printf("Algo salio mal con el mutex del color, apagando el led...");
            current_color.r = 0;
            current_color.g = 0;
            current_color.b = 0; 
            turn_led_on(strip, current_color.r, current_color.g, current_color.b);
            //--> Apagamos el led si algo sale mal para detectarlo
        }
        turn_led_on(strip, current_color.r, current_color.g, current_color.b);
        vTaskDelay(pdMS_TO_TICKS(500));
        turn_led_off(strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void task_a_start(TaskHandle_t *task_handle) {
    xTaskCreate(task_a, "TaskA", 2048, NULL, 1, task_handle);
}