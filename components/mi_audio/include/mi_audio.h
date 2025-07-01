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

void mi_audio_next_track(void);

void mi_audio_prev_track(void);

void mi_audio_play_pause(void);

void mi_audio_volume_up(void);

void mi_audio_volume_down(void);

void mi_audio_set_volume(uint8_t volume);