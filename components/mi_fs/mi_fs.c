
#include <mi_fs.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_littlefs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

static const char *TAG = "esp_littlefs";

const char *logger_path = "/littlefs/logger.json";

void mi_fs_init(){   

    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "storage",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

}


void save_json(const char *path, const char *json_string) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    ESP_LOGI(TAG, "Writing to file: %s", json_string);
    fprintf(f, "%s", json_string);
    fclose(f);
    ESP_LOGI(TAG, "File written: %s", path);
}

char *read_json(const char *path) {
    FILE *f = fopen(path, "r");


    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *buffer = malloc(len + 1);
    if (!buffer) {
        fclose(f);
        ESP_LOGE(TAG, "Memory allocation failed");
        return NULL;
    }

    fread(buffer, 1, len, f);
    buffer[len] = '\0';
    fclose(f);

    ESP_LOGI(TAG, "Read from file: %s", buffer);
    return buffer;
}

Logger read_logger_from_json() {
    Logger logger = {0};  // inicializa todo en 0/null

    char *json_str = read_json(logger_path);  // la función que vimos antes
    if (!json_str) {
        ESP_LOGW(TAG, "JSON file not found, creating default logger");
        save_logger_to_json(&logger);
        return logger;
    }

    cJSON *root = cJSON_Parse(json_str);
    free(json_str);

    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON");
        return logger;
    }

    cJSON *events = cJSON_GetObjectItemCaseSensitive(root, "events");
    if (cJSON_IsArray(events)) {
        int event_count = cJSON_GetArraySize(events);
        for (int i = 0; i < event_count && i < 20; i++) {
            cJSON *event = cJSON_GetArrayItem(events, i);
            if (cJSON_IsString(event) && event->valuestring) {
                strncpy(logger.events[i], event->valuestring, sizeof(logger.events[i]) - 1);
            }
        }
    }

    cJSON *pointer = cJSON_GetObjectItemCaseSensitive(root, "pointer");
    if (cJSON_IsNumber(pointer)) {
        logger.pointer = pointer->valueint;
    }

    cJSON_Delete(root);
    return logger;
}

void save_logger_to_json(const Logger *logger) {
    cJSON *root = cJSON_CreateObject();

    cJSON *events_array = cJSON_CreateArray();
    for (int i = 0; i < 20; i++) {
        if (strlen(logger->events[i]) > 0) {
            cJSON_AddItemToArray(events_array, cJSON_CreateString(logger->events[i]));
        }
    }

    cJSON_AddItemToObject(root, "events", events_array);
    cJSON_AddItemToObject(root, "pointer", cJSON_CreateNumber(logger->pointer));

    char *json_str = cJSON_PrintUnformatted(root);
    save_json(logger_path, json_str);  // ya definida arriba

    free(json_str);
    cJSON_Delete(root);
}

void store_logger_event(Logger *logger, const char *event) {

    ESP_LOGI(TAG, "Storing logger event: %s", event);
    
    if (logger->pointer >= 20) {
        logger->pointer = 0; // Reinicia el puntero si está lleno
    }

    strncpy(logger->events[logger->pointer], event, sizeof(logger->events[0]) - 1);
    logger->events[logger->pointer][sizeof(logger->events[0]) - 1] = '\0'; // Asegura que la cadena esté terminada en nulo
    logger->pointer++;

    

    save_logger_to_json(logger);
}

void save_config_to_json(const Config *config) {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "mqtt_url", config->mqtt_url);
    cJSON_AddStringToObject(root, "sta_ssid", config->sta_ssid);
    cJSON_AddStringToObject(root, "sta_password", config->sta_password);

    cJSON *playlist_array = cJSON_CreateArray();
    for (int i = 0; i < 10; i++) {
        if (strlen(config->play_list[i]) > 0) {
            cJSON_AddItemToArray(playlist_array, cJSON_CreateString(config->play_list[i]));
        }
    }
    cJSON_AddItemToObject(root, "play_list", playlist_array);

    char *json_str = cJSON_PrintUnformatted(root);
    save_json("/littlefs/config.json", json_str);  // ya definida arriba

    free(json_str);
    cJSON_Delete(root);
}

Config read_config_from_json() {
    Config config = {0};

    char *json_str = read_json("/littlefs/config.json");
    if (!json_str) {
        ESP_LOGW(TAG, "Config file not found, using default config");
        return config;
    }

    cJSON *root = cJSON_Parse(json_str);
    free(json_str);

    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON");
        return config;
    }

    cJSON *mqtt_url = cJSON_GetObjectItemCaseSensitive(root, "mqtt_url");
    if (cJSON_IsString(mqtt_url) && mqtt_url->valuestring) {
        strncpy(config.mqtt_url, mqtt_url->valuestring, sizeof(config.mqtt_url) - 1);
    }

    cJSON *sta_ssid = cJSON_GetObjectItemCaseSensitive(root, "sta_ssid");
    if (cJSON_IsString(sta_ssid) && sta_ssid->valuestring) {
        strncpy(config.sta_ssid, sta_ssid->valuestring, sizeof(config.sta_ssid) - 1);
    }

    cJSON *sta_password = cJSON_GetObjectItemCaseSensitive(root, "sta_password");
    if (cJSON_IsString(sta_password) && sta_password->valuestring) {
        strncpy(config.sta_password, sta_password->valuestring, sizeof(config.sta_password) - 1);
    }

    cJSON *play_list = cJSON_GetObjectItemCaseSensitive(root, "play_list");
    if (cJSON_IsArray(play_list)) {
        int count = cJSON_GetArraySize(play_list);
        for (int i = 0; i < count && i < 10; i++) {
            cJSON *item = cJSON_GetArrayItem(play_list, i);
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(config.play_list[i], item->valuestring, sizeof(config.play_list[i]) - 1);
            }
        }
    }

    cJSON_Delete(root);
    return config;
}
