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

#define WIFI_SSID "Antelckcfe-2.4GHz"
#define WIFI_PASS "xxx"

//--> Variables globales compartidas
SemaphoreHandle_t mutex_color;

void app_main(void)
{   
    mi_fs_init();

    connect_wifi_ap(WIFI_SSID, WIFI_PASS);

    Logger logger = read_logger_from_json();
    Config config = read_config_from_json();

    QueueHandle_t queue = mi_queue_init(10);
    mi_audio_init_with_queue(queue, logger);
    mi_touch_init_with_queue(queue);
    mi_mqtt_init_with_queue(queue);
    mi_web_server_init_with_queue(queue);

    // change some config
    // config.broker_url = "mqtt://newbroker.com";
    // save_config_to_json(&config);

    //Add a event to logger
    //connect_wifi_ap(WIFI_SSID, WIFI_PASS);
    //QueueHandle_t queue = mi_queue_init(10);
    //mi_audio_init_with_queue(queue);
    //mi_touch_init_with_queue(queue);
    //mi_mqtt_init_with_queue(queue);

}