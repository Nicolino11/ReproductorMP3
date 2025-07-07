#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "mi_led.h"
#include "mi_delay.h"
#include "mi_touch.h"
#include "mi_wifi_ap.h"
#include "mi_web_server.h"
#include "mi_queue.h"
#include "mi_audio.h"
#include "mi_mqtt.h"
#include "mi_fs.h"
#include "mi_ntp_time.h"
#include "mi_config.h"

#define WIFI_SSID "Antelckcfe-2.4GHz"
#define WIFI_PASS "xxx"

//--> Variables globales compartidas
SemaphoreHandle_t mutex_color;

void main_task(void *pvParameters) {
    mi_fs_init();
    mi_config_init();

    Logger logger = read_logger_from_json();
    Config *config = mi_config_get();
    QueueHandle_t queue = mi_queue_init(10);

    init_wifi_apsta("Placa_WIFI", "Password123", config->sta_ssid, config->sta_password);
    mi_ntp_time_init(); // Sincroniza la hora con NTP
    
    mi_audio_init_with_queue(queue, logger); // Pasa referencia si es posible
    mi_touch_init_with_queue(queue);
    mi_mqtt_init_with_queue(queue, logger);
    mi_web_server_init_with_queue(queue);

    vTaskDelete(NULL); // Termina la tarea si no es cíclica
}

void app_main(void)
{
    // Solo crea la tarea principal con stack grande
    xTaskCreate(main_task, "main_task", 8192, NULL, 5, NULL);
}