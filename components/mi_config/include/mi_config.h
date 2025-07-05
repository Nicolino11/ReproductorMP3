#pragma once
#include "mi_fs.h"
#include "esp_log.h"
#include <string.h>

//Eventos para cambiar la configuración
void mi_config_delete_last_song(void);
void mi_config_add_song(const char *song);
const char* mi_config_get_mqtt_url(void);
void mi_config_set_mqtt_url(const char *url);
void mi_config_set_sta(const char *ssid, const char *password);

void mi_config_init(void);
Config* mi_config_get(void);

typedef void (*mi_config_on_mqtt_url_changed_cb)(char *new_url);
typedef void (*mi_config_on_sta_changed_cb)(char *new_ssid, char *new_password);
typedef void (*mi_config_on_song_added_cb)(char *song);
typedef void (*mi_config_on_song_deleted_cb)(int index, char *song);

void mi_config_on_mqtt_url_changed(mi_config_on_mqtt_url_changed_cb cb);
void mi_config_on_sta_changed(mi_config_on_sta_changed_cb cb);
void mi_config_on_song_added(mi_config_on_song_added_cb cb);
void mi_config_on_song_deleted(mi_config_on_song_deleted_cb cb);