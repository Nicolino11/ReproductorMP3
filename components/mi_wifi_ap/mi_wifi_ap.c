#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>
#include <inttypes.h>
#include "mi_config.h"

static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
        case WIFI_EVENT_AP_START:
            ESP_LOGI("WIFI_EVENT", "Access Point started");
            break;
        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI("WIFI_EVENT", "Station connected to Access Point");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI("WIFI_EVENT", "Station disconnected from Access Point");
            break;
        case WIFI_EVENT_AP_STOP:
            ESP_LOGI("WIFI_EVENT", "Access Point stopped");
            break;
        case WIFI_EVENT_STA_START:
            ESP_LOGI("WIFI_EVENT", "Station started");
            break;
        case WIFI_EVENT_STA_STOP:
            ESP_LOGI("WIFI_EVENT", "Station stopped");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI("WIFI_EVENT", "Station connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI("WIFI_EVENT", "Station disconnected from AP");
            break;
        default:
            ESP_LOGI("WIFI_EVENT", "Unhandled event ID: %" PRId32, event_id);
            break;
    }
}

static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        ESP_LOGI("IP_EVENT", "IP assigned");
    }
}

void reconnect_wifi_ap(char *ssid, char *password){
    // Solo reconecta la STA, no toca el AP ni reinicializa el stack
    wifi_config_t sta_config = {0};
    strncpy((char *)sta_config.sta.ssid, ssid, sizeof(sta_config.sta.ssid));
    strncpy((char *)sta_config.sta.password, password, sizeof(sta_config.sta.password));
    sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_disconnect()); // Fuerza la reconexión
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI("WIFI", "STA reconnected: %s", ssid);
}

void on_sta_changed(char *new_ssid, char *new_password) {
    ESP_LOGI("WIFI", "STA config changed! New SSID: %s", new_ssid);
    reconnect_wifi_ap(new_ssid, new_password);
}

void init_wifi_apsta(const char *ap_ssid, const char *ap_password, const char *sta_ssid, const char *sta_password) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {0};
    // AP config
    strncpy((char *)wifi_config.ap.ssid, ap_ssid, sizeof(wifi_config.ap.ssid));
    strncpy((char *)wifi_config.ap.password, ap_password, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(ap_ssid);
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = strlen(ap_password) == 0 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    // STA config
    wifi_config_t sta_config = {0};
    strncpy((char *)sta_config.sta.ssid, sta_ssid, sizeof(sta_config.sta.ssid));
    strncpy((char *)sta_config.sta.password, sta_password, sizeof(sta_config.sta.password));
    sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_cb, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, ip_event_cb, NULL);

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI("WIFI", "APSTA mode started. AP SSID: %s, STA SSID: %s", ap_ssid, sta_ssid);
    mi_config_on_sta_changed(on_sta_changed); // Register the callback for STA changes
}

