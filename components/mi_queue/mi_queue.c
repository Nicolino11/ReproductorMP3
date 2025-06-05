#include "include/mi_queue.h"

QueueHandle_t mi_queue_init(int length) {
    return xQueueCreate(length, sizeof(mi_evento_t));
}

BaseType_t mi_queue_send(QueueHandle_t queue, mi_evento_t *evento, TickType_t ticks_to_wait) {
    return xQueueSend(queue, evento, ticks_to_wait);
}

BaseType_t mi_queue_receive(QueueHandle_t queue, mi_evento_t *evento, TickType_t ticks_to_wait) {
    return xQueueReceive(queue, evento, ticks_to_wait);
}
