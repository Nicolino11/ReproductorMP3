#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "mi_queue.h"

/**
 * @brief Inicializa el driver de audio, codec y canales I2S.
 */
void mi_audio_init(void);

/**
 * @brief Inicializa el driver de audio y recibe la queue de eventos.
 */
void mi_audio_init_with_queue(QueueHandle_t queue);