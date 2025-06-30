#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "mi_queue.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define CONFIG_BROKER_URL "mqtt://192.168.1.16"
#define TOPIC_NAME "/player/control"

static QueueHandle_t player_event_queue = NULL;

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

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


void mi_mqtt_init_with_queue(QueueHandle_t queue) {
    vTaskDelay(pdMS_TO_TICKS(60));
    player_event_queue = queue;
    mqtt_app_start();
}
