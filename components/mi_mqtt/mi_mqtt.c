#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "mi_queue.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mi_config.h"

#define CONFIG_BROKER_URL "mqtt://192.168.4.2"
#define TOPIC_NAME "/player/control"
#define TOPIC_LOGGER "/player/logger"

static QueueHandle_t player_event_queue = NULL;
static esp_mqtt_client_handle_t client = NULL;
static Logger event_logger;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGI("MQTT","Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGI("MQTT", "Event dispatched from event loop base=%s, event_id=%ld", base, (long)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("MQTT","MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_NAME, 0);
        ESP_LOGI("MQTT","sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("MQTT","MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI("MQTT","MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI("MQTT","MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI("MQTT","MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI("MQTT","MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (player_event_queue && event->data && event->data_len > 0) {
            mi_evento_t evento = {0};
            if (strncmp(event->data, "EVENT_NEXT_TRACK", event->data_len) == 0) {
                evento.tipo = EVENT_NEXT_TRACK;
            } else if (strncmp(event->data, "EVENT_PREV_TRACK", event->data_len) == 0) {
                evento.tipo = EVENT_PREV_TRACK;
            } else if (strncmp(event->data, "EVENT_VOL_UP", event->data_len) == 0) {
                evento.tipo = EVENT_VOL_UP;
            } else if (strncmp(event->data, "EVENT_VOL_DOWN", event->data_len) == 0) {
                evento.tipo = EVENT_VOL_DOWN;
            } else if (strncmp(event->data, "EVENT_PLAY_PAUSE", event->data_len) == 0) {
                evento.tipo = EVENT_PLAY_PAUSE;
            } else if (strncmp(event->data, "EVENT_STOP", event->data_len) == 0) {
                evento.tipo = EVENT_STOP;
            } else {
                ESP_LOGW("MQTT", "Evento desconocido recibido por MQTT: %.*s", event->data_len, event->data);
                break;
            }
            evento.value = 0;
            mi_queue_send(player_event_queue, &evento, 0);
            ESP_LOGI("MQTT", "Evento MQTT pusheado a la queue: tipo=%d", evento.tipo);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI("MQTT","MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI("MQTT","Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI("MQTT","Other event id:%d", event->event_id);
        break;
    }
}

static void send_logger_event_to_mqtt(const char *topic, const char *event) {
    if (client && event) {
        int msg_id = esp_mqtt_client_publish(client, topic, event, 0, 1, 0);
        ESP_LOGI("MQTT", "Logger event sent to MQTT: %s, msg_id=%d", event, msg_id);
    } else {
        ESP_LOGW("MQTT", "No MQTT client or event to send");
    }
}   

static void mi_mqtt_send_all_logger_events() {
    int idx = event_logger.pointer;
    int start = idx;
    int first = 1;
    do {
        idx = (idx - 1 + 20) % 20;
        if (strlen(event_logger.events[idx]) > 0) {
            send_logger_event_to_mqtt(TOPIC_LOGGER, event_logger.events[idx]);
        } else if (!first) {
            break;
        }
        first = 0;
    } while (idx != start);
}

static void on_mqtt_url_changed(char *new_url) {
    ESP_LOGI("MQTT", "MQTT URL changed! New URL: %s", new_url);
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = NULL;
    }
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = new_url
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    mi_mqtt_send_all_logger_events();
}

static void mqtt_app_start(char* broker_url)
{
    ESP_LOGI("MQTT", "MQTT URL: %s", broker_url);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_url
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    mi_mqtt_send_all_logger_events();
}

void mi_mqtt_init_with_queue(QueueHandle_t queue, Logger logger, char* broker_url) {
    vTaskDelay(pdMS_TO_TICKS(60));
    player_event_queue = queue;
    event_logger = logger;
    mi_config_on_mqtt_url_changed(on_mqtt_url_changed);
    mqtt_app_start(broker_url);
}
