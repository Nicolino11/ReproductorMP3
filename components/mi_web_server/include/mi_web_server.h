#ifndef MI_WEB_SERVER_H
#define MI_WEB_SERVER_H

#include "esp_http_server.h"
#include "led_strip.h"
#include "esp_event.h"
#include "mi_audio.h"
#include "mi_wifi_ap.h"
#include "mi_delay.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

httpd_handle_t start_webserver(void);

void stop_webserver(httpd_handle_t server);

void disconnect_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);

void connect_handler(void *arg, esp_event_base_t event_base,
                     int32_t event_id, void *event_data);

void hello_handler(httpd_req_t *req);

esp_err_t previous_track_handler(httpd_req_t *req);
esp_err_t next_track_handler(httpd_req_t *req);
esp_err_t volume_up_handler(httpd_req_t *req);
esp_err_t volume_down_handler(httpd_req_t *req);
esp_err_t play_pause_handler(httpd_req_t *req);
esp_err_t delete_last_song_handler(httpd_req_t *req);
esp_err_t get_mqtt_config_handler(httpd_req_t *req);
esp_err_t mqtt_config_post_handler(httpd_req_t *req);

esp_err_t get_wifi_credentials_handler(httpd_req_t *req);

void mi_web_server_init_with_queue(QueueHandle_t queue);

#endif // MI_WEB_SERVER_H