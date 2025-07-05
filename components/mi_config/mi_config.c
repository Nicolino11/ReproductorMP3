#include "mi_config.h"


static const char *TAG = "mi_config";
static Config global_config;

void mi_config_init(void) {
    global_config = read_config_from_json();
}

Config* mi_config_get(void) {
    return &global_config;
}

// Definición de tipos de callback para eventos de configuración
typedef void (*mi_config_on_mqtt_url_changed_cb)(char *new_url);
typedef void (*mi_config_on_sta_changed_cb)(char *new_ssid, char *new_password);
typedef void (*mi_config_on_song_added_cb)(char *song);
typedef void (*mi_config_on_song_deleted_cb)(int index, char *song);

// Variables estáticas para los callbacks
static mi_config_on_mqtt_url_changed_cb mqtt_url_cb = NULL;
static mi_config_on_sta_changed_cb sta_cb = NULL;
static mi_config_on_song_added_cb song_added_cb = NULL;
static mi_config_on_song_deleted_cb song_deleted_cb = NULL;

// Funciones para registrar los callbacks
void mi_config_on_mqtt_url_changed(mi_config_on_mqtt_url_changed_cb cb) { mqtt_url_cb = cb; }
void mi_config_on_sta_changed(mi_config_on_sta_changed_cb cb) { sta_cb = cb; }
void mi_config_on_song_added(mi_config_on_song_added_cb cb) { song_added_cb = cb; }
void mi_config_on_song_deleted(mi_config_on_song_deleted_cb cb) { song_deleted_cb = cb; }

// Modificaciones en setters para disparar los callbacks
void mi_config_set_mqtt_url(const char *url) {
    strncpy(global_config.mqtt_url, url, sizeof(global_config.mqtt_url) - 1);
    global_config.mqtt_url[sizeof(global_config.mqtt_url) - 1] = '\0';
    save_config_to_json(&global_config);
    if (mqtt_url_cb) mqtt_url_cb(global_config.mqtt_url);
}

// Setea los parámetros del STA (SSID y password) y guarda la config
void mi_config_set_sta(const char *ssid, const char *password) {
    strncpy(global_config.sta_ssid, ssid, sizeof(global_config.sta_ssid) - 1);
    global_config.sta_ssid[sizeof(global_config.sta_ssid) - 1] = '\0';
    strncpy(global_config.sta_password, password, sizeof(global_config.sta_password) - 1);
    global_config.sta_password[sizeof(global_config.sta_password) - 1] = '\0';
    save_config_to_json(&global_config);
    if (sta_cb) sta_cb(global_config.sta_ssid, global_config.sta_password);
}

// Agrega una canción a la lista de reproducción y guarda la config
void mi_config_add_song(const char *song) {
    for (int i = 0; i < 7; i++) {
        if (strlen(global_config.play_list[i]) == 0) {
            strncpy(global_config.play_list[i], song, sizeof(global_config.play_list[i]) - 1);
            global_config.play_list[i][sizeof(global_config.play_list[i]) - 1] = '\0';
            ESP_LOGI(TAG, "Added song '%s' at index %d", song, i);
            save_config_to_json(&global_config);
            if (song_added_cb) song_added_cb(song);
            return;
        }
    }
    ESP_LOGW(TAG, "No space to add new song");
}

void mi_config_delete_last_song(void) {
    int last = -1;
    for (int i = 0; i < 7; i++) {
        if (strlen(global_config.play_list[i]) > 0) {
            last = i;
        }
    }
    if (last >= 0) {
        char deleted_song[32];
        strncpy(deleted_song, global_config.play_list[last], sizeof(deleted_song) - 1);
        deleted_song[sizeof(deleted_song) - 1] = '\0';
        global_config.play_list[last][0] = '\0';
        ESP_LOGI(TAG, "Deleted last song at index %d", last);
        save_config_to_json(&global_config);
        if (song_deleted_cb) song_deleted_cb(last, deleted_song);
    } else {
        ESP_LOGW(TAG, "No song to delete");
    }
}

// Obtiene la URL del broker MQTT de la config
const char* mi_config_get_mqtt_url(void) {
    return global_config.mqtt_url;
}
