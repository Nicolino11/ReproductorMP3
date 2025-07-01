#include "mi_audio.h"
#include "esp_log.h"

static const char *TAG = "MI_AUDIO";

void mi_audio_next_track(void) {
    ESP_LOGI(TAG, "Siguiente pista (next track)");
    // TODO: Implementar lógica para reproducir la siguiente pista
}

void mi_audio_prev_track(void) {
    ESP_LOGI(TAG, "Pista anterior (previous track)");
    // TODO: Implementar lógica para reproducir la pista anterior
}

void mi_audio_play_pause(void) {
    ESP_LOGI(TAG, "Play/Pause");
    // TODO: Implementar lógica para pausar o reanudar la reproducción
}

void mi_audio_volume_up(void) {
    ESP_LOGI(TAG, "Subir volumen (volume up)");
    // TODO: Implementar lógica para subir el volumen
}

void mi_audio_volume_down(void) {
    ESP_LOGI(TAG, "Bajar volumen (volume down)");
    // TODO: Implementar lógica para bajar el volumen
}

void mi_audio_set_volume(uint8_t volume) {
    ESP_LOGI(TAG, "Setear volumen a %d", volume);
    // TODO: Implementar lógica para setear el volumen a un valor específico
}