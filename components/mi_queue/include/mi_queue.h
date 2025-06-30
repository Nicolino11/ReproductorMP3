#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
* @brief Evento que contiene los datos a enviar por la cola
*
*/

typedef enum {
    EVENT_NEXT_TRACK,
    EVENT_PREV_TRACK,
    EVENT_VOL_UP,
    EVENT_VOL_DOWN,
    EVENT_PLAY_PAUSE,
    EVENT_STOP
} mi_tipo_evento_t;

typedef struct {
    int r;
    int g;
    int b;
    int tiempo_ms;
    float brghtness;
} mi_evento_player;

typedef struct {
    mi_tipo_evento_t tipo;
    int value; // Puede usarse para volumen, 1=play/pause, etc. Si no aplica, poner 0
} mi_evento_t;

// Inicializa la cola y retorna el handle
QueueHandle_t mi_queue_init(int length);

// Envia un evento a la cola
BaseType_t mi_queue_send(QueueHandle_t queue, mi_evento_t *evento, TickType_t ticks_to_wait);

// Recibe un evento de la cola
BaseType_t mi_queue_receive(QueueHandle_t queue, mi_evento_t *evento, TickType_t ticks_to_wait);
