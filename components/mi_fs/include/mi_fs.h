#pragma once


typedef struct {
    int pointer;
    char events[20][64]; // array de hasta 20 strings de 31 caracteres + null
} Logger;


typedef struct {
    char mqtt_url[64]; // URL del broker MQTT
    char sta_ssid[32]; // SSID de la red WiFi
    char sta_password[32]; // Contraseña de la red WiFi
    char play_list[7][32]; // Lista de reproducción con 7 canciones 
} Config;

void mi_fs_init();
void save_json(const char *path, const char *json_string);
char *read_json(const char *path);

Logger read_logger_from_json();
void save_logger_to_json(const Logger *logger);
void store_logger_event(Logger *logger, const char *event);

void save_config_to_json(const Config *config);
Config read_config_from_json();
