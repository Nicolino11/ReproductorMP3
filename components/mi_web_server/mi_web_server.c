#include "mi_web_server.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mi_wifi_ap.h"
#include "mi_delay.h"
#include "mi_audio.h"
#include <string.h>
#include <stdlib.h>
#include "spiff_handler.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
static const char *TAG = "WEB_SERVER";
// static int cont = 0;

static QueueHandle_t web_event_queue = NULL;

const httpd_uri_t prevSong = {
    .uri = "/prevSong",
    .method = HTTP_POST,
    .handler = previous_track_handler,
    .user_ctx = NULL};

const httpd_uri_t nextSong = {
    .uri = "/nextSong",
    .method = HTTP_POST,
    .handler = next_track_handler,
    .user_ctx = NULL};

const httpd_uri_t stopSong = {
    .uri = "/stopSong",
    .method = HTTP_POST,
    .handler = stop_song_handler,
    .user_ctx = NULL}

const httpd_uri_t VolUp = {
    .uri = "/VolUp",
    .method = HTTP_POST,
    .handler = volume_up_handler,
    .user_ctx = NULL};

const httpd_uri_t volDown = {
    .uri = "/VolDown",
    .method = HTTP_POST,
    .handler = volume_down_handler,
    .user_ctx = NULL};

const httpd_uri_t PlayPause = {
    .uri = "/PlayPause",
    .method = HTTP_POST,
    .handler = play_pause_handler,
    .user_ctx = NULL};

httpd_uri_t delete_last_song_uri = {
    .uri = "/deleteLastSong",
    .method = HTTP_POST,
    .handler = delete_last_song_handler,
    .user_ctx = NULL};

httpd_uri_t get_mqtt_handler = {
    .uri = "/getMQTTCredentials",
    .method = HTTP_GET,
    .handler = get_mqtt_config_handler,
    .user_ctx = NULL};

const httpd_uri_t mqtt_config = {
    .uri = "/mqttConfig",
    .method = HTTP_POST,
    .handler = mqtt_config_post_handler,
    .user_ctx = NULL};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 20;

    // cont = load_counter_from_nvs();

    // Iniciar el servidor httpd
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Registrar los manejadores de URI
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_uri_t wifi_credentials_handler = {
            .uri = "/getWifiCredentials",
            .method = HTTP_GET,
            .handler = get_wifi_credentials_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &wifi_credentials_handler);

        // Otros URI handlers
        httpd_register_uri_handler(server, &prevSong);
        httpd_register_uri_handler(server, &nextSong);
        httpd_register_uri_handler(server, &VolUp);
        httpd_register_uri_handler(server, &volDown);
        httpd_register_uri_handler(server, &PlayPause);
        httpd_register_uri_handler(server, &stopSong);

        // httpd_register_uri_handler(server, &delete_last_song_uri);

#if CONFIG_EXAMPLE_BASIC_AUTH
        httpd_register_basic_auth(server);
#endif
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    if (server != NULL)
    {
        httpd_stop(server);
        ESP_LOGI(TAG, "Servidor web detenido");
    }
}

void disconnect_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

void connect_handler(void *arg, esp_event_base_t event_base,
                     int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

esp_err_t previous_track_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Prev/Pause up handler activated");
    httpd_resp_sendstr(req, "Previous track command received");
    mi_evento_t evento = {0};
    evento.tipo = EVENT_PREV_TRACK;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);

    return ESP_OK;
}

esp_err_t next_track_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Next Song handler activated");
    httpd_resp_sendstr(req, "Next track command received");
    mi_evento_t evento = {0};
    evento.tipo = EVENT_NEXT_TRACK;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);

    return ESP_OK;
}

esp_err_t volume_up_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Volume up handler activated");
    httpd_resp_sendstr(req, "Volume up command received");
    evento.tipo = EVENT_VOL_UP;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t volume_down_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Volume Down  handler activated");
    httpd_resp_sendstr(req, "Volume down command received");
    evento.tipo = EVENT_VOL_DOWN;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t play_pause_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Play Pause handler activated");
    httpd_resp_sendstr(req, "Play/Pause command received");
    evento.tipo = EVENT_PLAY_PAUSE;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t stop_song_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "Stop command received");
    evento.tipo = EVENT_STOP;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t delete_last_song_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, " Request to delete the last song");

    esp_err_t result = spiff_handler_remove_last_song();
    if (result == ESP_OK)
    {
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_sendstr(req, "Last song deleted successfully");
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete the last song");
    }

    return result;
}

/* Función de callback para manejar la solicitud HTTP GET /getMQTTCredentials */
esp_err_t get_mqtt_config_handler(httpd_req_t *req)
{
    char url_encoded_mqtt_url[128];
    uint16_t mqtt_port;

    // Obtener los datos de la configuración MQTT desde el NVS
    esp_err_t err = nvs_handler_get_mqtt_config(url_encoded_mqtt_url, sizeof(url_encoded_mqtt_url), &mqtt_port);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve MQTT config from NVS");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error retrieving MQTT config");
        return ESP_FAIL;
    }

    // Decodificar el URL de MQTT
    char mqtt_url[128];
    url_decode(mqtt_url, url_encoded_mqtt_url);

    ESP_LOGI(TAG, "Received MQTT config data: mqttUrl=%s&mqttPort=%d", url_encoded_mqtt_url, mqtt_port);
    ESP_LOGI(TAG, "MQTT URL: %s, MQTT Port: %d", mqtt_url, mqtt_port);

    // Construir la respuesta con la configuración MQTT
    char response_buffer[256];
    snprintf(response_buffer, sizeof(response_buffer),
             "MQTT configuration: URL: %s, Port: %d",
             mqtt_url, mqtt_port);

    // Enviar la respuesta HTML
    httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t list_files_handler(httpd_req_t *req)
{
    char *list;

    esp_err_t ret = spiff_handler_list_files(&list);
    if (ret == ESP_OK)
    {
        printf("Files in SPIFFS: %s\n", list);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to list files in SPIFFS");
    }
    httpd_resp_send(req, list, strlen(list));
    free(list); // Liberar la memoria asignada por la función
    return ESP_OK;
}

esp_err_t mqtt_config_post_handler(httpd_req_t *req)
{
    char buf[1024];
    int ret = httpd_req_recv(req, buf, sizeof(buf)); // Recibir datos de la solicitud HTTP
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req); // Enviar respuesta 408 si hay timeout
        }
        return ESP_FAIL;
    }

    buf[ret] = '\0'; // Asegurar que el buffer esté terminado con null byte
    ESP_LOGI(TAG, "Received MQTT config data: %s", buf);

    char mqtt_url_encoded[128];
    char mqtt_url[128];
    char mqtt_port_str[6]; // Para almacenar el puerto como cadena

    // Encontrar mqttUrl en los datos recibidos
    char *mqtt_url_start = strstr(buf, "mqttUrl=");
    if (!mqtt_url_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: missing 'mqttUrl'");
        return ESP_FAIL;
    }
    mqtt_url_start += strlen("mqttUrl=");

    char *mqtt_url_end = strchr(mqtt_url_start, '&');
    if (!mqtt_url_end)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: missing separator or 'mqttPort'");
        return ESP_FAIL;
    }
    strncpy(mqtt_url_encoded, mqtt_url_start, mqtt_url_end - mqtt_url_start);
    mqtt_url_encoded[mqtt_url_end - mqtt_url_start] = '\0';

    // Decodificar el URL de MQTT
    url_decode(mqtt_url, mqtt_url_encoded);

    // Encontrar mqttPort en los datos recibidos
    char *mqtt_port_start = strstr(mqtt_url_end, "mqttPort=");
    if (!mqtt_port_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: missing 'mqttPort'");
        return ESP_FAIL;
    }
    mqtt_port_start += strlen("mqttPort=");

    char *mqtt_port_end = strchr(mqtt_port_start, '&');
    if (!mqtt_port_end)
    {
        mqtt_port_end = mqtt_port_start + strlen(mqtt_port_start);
    }
    strncpy(mqtt_port_str, mqtt_port_start, mqtt_port_end - mqtt_port_start);
    mqtt_port_str[mqtt_port_end - mqtt_port_start] = '\0';

    long mqtt_port = strtol(mqtt_port_str, NULL, 10);
    if (mqtt_port <= 0 || mqtt_port > 65535)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: 'mqttPort' is not a valid port number");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "MQTT URL: %s, MQTT Port: %ld", mqtt_url, mqtt_port);

    // Guardar configuración MQTT en NVS
    // esp_err_t err = nvs_handler_set_mqtt_config(mqtt_url, mqtt_port);
    // if (err != ESP_OK)
    // {
    //     httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save MQTT config");
    //     return ESP_FAIL;
    // }

    // Enviar una respuesta de éxito al cliente
    httpd_resp_sendstr(req, "MQTT config received successfully");

    return ESP_OK;
}

void mi_web_server_init_with_queue(QueHandle_t queue)
{
    web_event_queue = queue;
    start_webserver();
}

esp_err_t get_wifi_credentials_handler(void) {}
