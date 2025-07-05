#pragma once
#include "mi_queue.h"
#include "mi_fs.h"

void mi_mqtt_init_with_queue(QueueHandle_t queue, Logger logger);
void mi_mqtt_send_all_logger_events();
void send_logger_event_to_mqtt(const char *event);