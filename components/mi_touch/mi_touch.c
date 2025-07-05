#include "mi_touch.h"
#include "mi_delay.h"
#include "esp_log.h"
#include "driver/touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "mi_queue.h"

#define TOUCH_THRESHOLD 20000 //--> Umbral para el valor en crudo que detecta si fue presionado
#define TOUCH_BUTTON_NUM 6 //--> Cantidad de botones

// Definición de índices para los botones del touchpad
#define BTN_VOL_UP      0
#define BTN_PLAY_PAUSE  1
#define BTN_VOL_DOWN    2
#define BTN_NEXT_TRACK  3
#define BTN_PREV_TRACK  4
#define BTN_STOP        5

static const touch_pad_t buttons[TOUCH_BUTTON_NUM] = {
    TOUCH_PAD_NUM1,   // TP5 - VOL_UP
    TOUCH_PAD_NUM2,   // TP2 - PLAY/PAUSE
    TOUCH_PAD_NUM3,   // TP6 - VOL_DOWN
    TOUCH_PAD_NUM5,   // TP4 - RECORD
    TOUCH_PAD_NUM6,   // TP1 - PHOTO
    TOUCH_PAD_NUM11,  // TP3 - NETWORK
};

static QueueHandle_t touch_event_queue = NULL;

void touch_buttons_init(void){
    //--> Iniciamos el touchpad
    touch_pad_init();

    //--> Configuramos todos los botones
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_pad_config(buttons[i]);
    }
    //--> Iniciamos el touchpad para medidas manuales con touch_pad_read_raw_data
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_SW);
    
}

int touch_buttons_get_pressed(void){
    static uint32_t last_values[TOUCH_BUTTON_NUM] = {0};
    uint32_t values[TOUCH_BUTTON_NUM];
    static int last_state = -1;
    static bool released = true;
    // Leer todos los valores primero
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_pad_sw_start();
        vTaskDelay(pdMS_TO_TICKS(20));
        touch_pad_read_raw_data(buttons[i], &values[i]);
    }
    // Loguear todos los valores juntos
    char logbuf[200];
    int offset = 0;
    offset += snprintf(logbuf + offset, sizeof(logbuf) - offset, "[MI_TOUCH] Valores touch: ");
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        offset += snprintf(logbuf + offset, sizeof(logbuf) - offset, "%d(G%d)=%lu ", i, buttons[i], values[i]);
    }
    //ESP_LOGI("MI_TOUCH", "%s", logbuf);
    // Detectar presión por diferencia
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        int32_t diff = (int32_t)values[i] - (int32_t)last_values[i];
        if (diff > TOUCH_THRESHOLD) {
            if (released || last_state != i) {
                released = false;
                last_state = i;
                // Actualizar los valores previos antes de retornar
                for (int j = 0; j < TOUCH_BUTTON_NUM; j++) last_values[j] = values[j];
                return i;
            }
        }
    }
    released = true;
    last_state = -1;
    // Actualizar los valores previos
    for (int j = 0; j < TOUCH_BUTTON_NUM; j++) last_values[j] = values[j];
    return -1;
}

static void touch_event_task(void *arg){   
    //--> Iniciamos el touchpad
    touch_buttons_init();
    int STATE;
    while (1){
        vTaskDelay(pdMS_TO_TICKS(30));
        STATE = touch_buttons_get_pressed();
        mi_evento_t evento = {0};
        switch (STATE){
            case -1: //No se presiona nada
                break;
            case BTN_VOL_UP:
                ESP_LOGI("MI_TOUCH", "[TOUCH] VOL_UP");
                if (touch_event_queue) {
                    evento.tipo = EVENT_VOL_UP;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            case BTN_PLAY_PAUSE:
                ESP_LOGI("MI_TOUCH", "[TOUCH] PLAY/PAUSE");
                if (touch_event_queue) {
                    evento.tipo = EVENT_PLAY_PAUSE;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            case BTN_VOL_DOWN:
                ESP_LOGI("MI_TOUCH", "[TOUCH] VOL_DOWN");
                if (touch_event_queue) {
                    evento.tipo = EVENT_VOL_DOWN;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            case BTN_NEXT_TRACK:
                ESP_LOGI("MI_TOUCH", "[TOUCH] NEXT_TRACK");
                if (touch_event_queue) {
                    evento.tipo = EVENT_NEXT_TRACK;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            case BTN_PREV_TRACK:
                ESP_LOGI("MI_TOUCH", "[TOUCH] PREV_TRACK");
                if (touch_event_queue) {
                    evento.tipo = EVENT_PREV_TRACK;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            case BTN_STOP:
                ESP_LOGI("MI_TOUCH", "[TOUCH] STOP");
                if (touch_event_queue) {
                    evento.tipo = EVENT_STOP;
                    evento.value = 0;
                    mi_queue_send(touch_event_queue, &evento, 0);
                }
                break;
            default:
                ESP_LOGW("MI_TOUCH", "[TOUCH] Evento desconocido: %d", STATE);
                break;
        }
    }
}

void mi_touch_init_with_queue(QueueHandle_t queue) {
    touch_event_queue = queue;
    xTaskCreate(touch_event_task, "touch_event_task", 2048, NULL, 5, NULL);
}
