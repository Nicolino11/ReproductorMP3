#ifndef NVS_HANDLER_H
#define NVS_HANDLER_H

#include "esp_err.h"
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64

esp_err_t nvs_handler_init(void);
esp_err_t nvs_handler_set_wifi_credentials(const char *ssid, const char *password);
esp_err_t nvs_handler_get_wifi_credentials(char *ssid, size_t ssid_max_len, char *password, size_t password_max_len);
esp_err_t nvs_handler_get_mqtt_config(char *mqtt_url, size_t url_max_len, uint16_t *mqtt_port);
esp_err_t nvs_handler_set_mqtt_config(const char *mqtt_url, uint16_t mqtt_port);
void save_counter_to_nvs(int counter);
int load_counter_from_nvs();
void url_decode(char *dst, const char *src);
#endif /* NVS_HANDLER_H */
