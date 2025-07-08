#include "mi_web_server.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mi_wifi_ap.h"
#include "mi_delay.h"
#include "mi_audio.h"
#include "mi_fs.h"
#include "mi_config.h"
#include <string.h>
#include <stdlib.h>
#include "spiff_handler.h"
#include "nvs_handler.h"

// Declaraciones anticipadas de handlers
esp_err_t previous_track_handler(httpd_req_t *req);
esp_err_t next_track_handler(httpd_req_t *req);
esp_err_t stop_song_handler(httpd_req_t *req);
esp_err_t volume_up_handler(httpd_req_t *req);
esp_err_t volume_down_handler(httpd_req_t *req);
esp_err_t play_pause_handler(httpd_req_t *req);
esp_err_t delete_last_song_handler(httpd_req_t *req);
esp_err_t get_mqtt_config_handler(httpd_req_t *req);
esp_err_t mqtt_config_post_handler(httpd_req_t *req);
esp_err_t get_wifi_credentials_handler(httpd_req_t *req);
esp_err_t list_songs_handler(httpd_req_t *req);
esp_err_t add_song_handler(httpd_req_t *req);
esp_err_t remove_song_handler(httpd_req_t *req);
esp_err_t set_sta_config_handler(httpd_req_t *req);

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
static const char *TAG = "WEB_SERVER";

static QueueHandle_t web_event_queue = NULL;

// Configuración de las rutas
static const httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
};

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
    .user_ctx = NULL};

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

httpd_uri_t remove_song_uri = {
    .uri = "/removeSong",
    .method = HTTP_POST,
    .handler = remove_song_handler,
    .user_ctx = NULL};

httpd_uri_t list_songs_uri = {
    .uri = "/listSongs",
    .method = HTTP_GET,
    .handler = list_songs_handler,
    .user_ctx = NULL};

httpd_uri_t add_song_uri = {
    .uri = "/addSong",
    .method = HTTP_POST,
    .handler = add_song_handler,
    .user_ctx = NULL};

// Setear los parámetros de WiFi STA
httpd_uri_t set_sta_config_uri = {
    .uri = "/setStaConfig",
    .method = HTTP_POST,
    .handler = set_sta_config_handler,
    .user_ctx = NULL};

// Manejador para la ruta "/"
esp_err_t index_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Solicitud recibida para URI: %s", req->uri);
    
    // Establecer el tipo de contenido
    httpd_resp_set_type(req, "text/html");
    
    // Enviar la respuesta HTML
    extern const char resp[] asm("_binary_index_html_start");
    httpd_resp_send(req, resp, strlen(resp));
    
    return ESP_OK;
}

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
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &prevSong);
        httpd_register_uri_handler(server, &nextSong);
        httpd_register_uri_handler(server, &VolUp);
        httpd_register_uri_handler(server, &volDown);
        httpd_register_uri_handler(server, &PlayPause);
        httpd_register_uri_handler(server, &stopSong);
        httpd_register_uri_handler(server, &delete_last_song_uri);
        httpd_register_uri_handler(server, &get_mqtt_handler);
        httpd_register_uri_handler(server, &mqtt_config);
        httpd_register_uri_handler(server, &remove_song_uri);
        httpd_register_uri_handler(server, &list_songs_uri);
        httpd_register_uri_handler(server, &add_song_uri);
        httpd_register_uri_handler(server, &set_sta_config_uri);

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
    mi_evento_t evento = {0};
    evento.tipo = EVENT_VOL_UP;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t volume_down_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Volume Down  handler activated");
    httpd_resp_sendstr(req, "Volume down command received");
    mi_evento_t evento = {0};
    evento.tipo = EVENT_VOL_DOWN;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t play_pause_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Play Pause handler activated");
    httpd_resp_sendstr(req, "Play/Pause command received");
    mi_evento_t evento = {0};
    evento.tipo = EVENT_PLAY_PAUSE;
    evento.value = 0;
    mi_queue_send(web_event_queue, &evento, 0);
    return ESP_OK;
}

esp_err_t stop_song_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "Stop command received");
    mi_evento_t evento = {0};
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

// Función de callback pa1ra manejar la solicitud HTTP GET /getMQTTCredentials
esp_err_t get_mqtt_config_handler(httpd_req_t *req)
{
    char mqtt_url_param[128] = {0};
    // Obtener el parámetro 'mqtt_url' de la query string si existe
    esp_err_t param_err = httpd_req_get_url_query_str(req, mqtt_url_param, sizeof(mqtt_url_param));
    if (param_err == ESP_OK && strlen(mqtt_url_param) > 0) {
        // Buscar el valor del parámetro mqtt_url
        char value[128] = {0};
        if (httpd_query_key_value(mqtt_url_param, "mqtt_url", value, sizeof(value)) == ESP_OK) {
            // Aquí podrías hacer algo con el parámetro recibido si lo necesitas
            // Pero según la consigna, solo se debe devolver la URL almacenada
        }
    }
    // Obtener la URL almacenada
    const char* stored_url = mi_config_get_mqtt_url();
    if (!stored_url) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No MQTT URL configured");
        return ESP_FAIL;
    }
    char response_buffer[256];
    snprintf(response_buffer, sizeof(response_buffer), "MQTT configuration: URL: %s", stored_url);
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
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    buf[ret] = '\0';
    ESP_LOGI(TAG, "Received MQTT config data: %s", buf);

    // Buscar mqttUrl en los datos recibidos
    char *mqtt_url_start = strstr(buf, "mqttUrl=");
    if (!mqtt_url_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: missing 'mqttUrl'");
        return ESP_FAIL;
    }
    mqtt_url_start += strlen("mqttUrl=");

    // Extraer la URL completa hasta el final o hasta el siguiente &
    char *mqtt_url_end = strchr(mqtt_url_start, '&');
    if (!mqtt_url_end)
    {
        mqtt_url_end = mqtt_url_start + strlen(mqtt_url_start);
    }

    char mqtt_url[128] = {0};
    int url_len = mqtt_url_end - mqtt_url_start;
    if (url_len >= sizeof(mqtt_url))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request: mqttUrl too long");
        return ESP_FAIL;
    }

    strncpy(mqtt_url, mqtt_url_start, url_len);
    mqtt_url[url_len] = '\0';

    // URL decode si es necesario
    char decoded_url[128];
    url_decode(decoded_url, mqtt_url);

    ESP_LOGI(TAG, "Setting MQTT URL: %s", decoded_url);

    // Guardar la URL usando mi_config
    mi_config_set_mqtt_url(decoded_url);

    // Enviar respuesta de éxito
    httpd_resp_sendstr(req, "MQTT URL saved successfully");

    return ESP_OK;
}

// Setear los parámetros de WiFi STA
esp_err_t set_sta_config_handler(httpd_req_t *req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Extraer los parámetros ssid y password
    char ssid[64] = {0};
    char password[64] = {0};
    char *ssid_start = strstr(buf, "ssid=");
    char *password_start = strstr(buf, "password=");
    if (!ssid_start || !password_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing ssid or password param");
        return ESP_FAIL;
    }
    ssid_start += strlen("ssid=");
    char *ssid_end = strchr(ssid_start, '&');
    int ssid_len = ssid_end ? (ssid_end - ssid_start) : strlen(ssid_start);
    if (ssid_len >= sizeof(ssid)) ssid_len = sizeof(ssid) - 1;
    strncpy(ssid, ssid_start, ssid_len);
    ssid[ssid_len] = '\0';

    password_start += strlen("password=");
    char *password_end = strchr(password_start, '&');
    int password_len = password_end ? (password_end - password_start) : strlen(password_start);
    if (password_len >= sizeof(password)) password_len = sizeof(password) - 1;
    strncpy(password, password_start, password_len);
    password[password_len] = '\0';

    // Guardar la configuración usando mi_config
    mi_config_set_sta(ssid, password);
    httpd_resp_sendstr(req, "WiFi STA config saved successfully");
    return ESP_OK;
}

void mi_web_server_init_with_queue(QueueHandle_t queue)
{
    web_event_queue = queue;
    start_webserver();
}

esp_err_t get_wifi_credentials_handler(httpd_req_t *req) { return ESP_OK; }

// Listar canciones disponibles (devuelve JSON)
esp_err_t list_songs_handler(httpd_req_t *req)
{
    // Respuesta JSON básica con canciones disponibles
    const char *json_response = "{\"available_songs\":["
        "{\"name\":\"doom\""
        "{\"name\":\"dance\""
        "{\"name\":\"mission\""
        "{\"name\":\"tetris\""
        "{\"name\":\"pacman\""
        "{\"name\":\"undertale\""
        "],\"playlist\":[],\"total_size_kb\":0,\"max_size_kb\":600}";
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, strlen(json_response));
    return ESP_OK;
}

// Agregar una canción a la lista de reproducción
esp_err_t add_song_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    // Espera formato: song=nombre
    char *song_start = strstr(buf, "song=");
    if (!song_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing song param");
        return ESP_FAIL;
    }
    song_start += strlen("song=");
    char song_name[64] = {0};
    strncpy(song_name, song_start, sizeof(song_name) - 1);
    song_name[sizeof(song_name) - 1] = '\0';
    
    ESP_LOGI(TAG, "Adding song: %s", song_name);
    
    // Por ahora, solo devolvemos éxito
    httpd_resp_sendstr(req, "Song added");
    return ESP_OK;
}

// Quitar una canción de la lista de reproducción
esp_err_t remove_song_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    // Espera formato: song=nombre
    char *song_start = strstr(buf, "song=");
    if (!song_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing song param");
        return ESP_FAIL;
    }
    song_start += strlen("song=");
    char song_name[64] = {0};
    strncpy(song_name, song_start, sizeof(song_name) - 1);
    song_name[sizeof(song_name) - 1] = '\0';
    
    ESP_LOGI(TAG, "Removing song: %s", song_name);
    
    // Por ahora, solo devolvemos éxito
    httpd_resp_sendstr(req, "Song removed");
    return ESP_OK;
}



void mi_web_server_init(void) {
}
