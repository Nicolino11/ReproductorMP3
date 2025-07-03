#include "nvs_handler.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "string.h"
#include <ctype.h>
#include <stdint.h>

static const char *TAG = "NVS_HANDLER";

#define NVS_NAMESPACE "wifi_config"

esp_err_t nvs_handler_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    return ESP_OK;
}

esp_err_t nvs_handler_set_wifi_credentials(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    // Validar longitud de los datos antes de guardarlos
    size_t ssid_len = strlen(ssid);
    size_t password_len = strlen(password);
    if (ssid_len >= WIFI_SSID_MAX_LENGTH || password_len >= WIFI_PASSWORD_MAX_LENGTH)
    {
        ESP_LOGE(TAG, "Invalid SSID or password length");
        nvs_close(nvs_handle);
        return ESP_ERR_INVALID_SIZE;
    }

    // Guardar SSID y password en NVS
    err = nvs_set_str(nvs_handle, "ssid", ssid);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error setting SSID in NVS (%s)", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error setting password in NVS (%s)", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Confirmar cambios en NVS
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error committing data to NVS (%s)", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "WiFi credentials saved to NVS");
    return ESP_OK;
}
esp_err_t nvs_handler_get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len)
{
    esp_err_t ret;
    nvs_handle_t nvs_handle;

    // Abrir el espacio de almacenamiento NVS para las credenciales WiFi
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle for WiFi credentials", esp_err_to_name(ret));
        return ret;
    }

    // Leer el SSID del NVS
    size_t required_size = ssid_len;
    ret = nvs_get_str(nvs_handle, "ssid", ssid, &required_size);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error retrieving SSID from NVS (%s)", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    // Leer la contraseña del NVS
    required_size = password_len;
    ret = nvs_get_str(nvs_handle, "password", password, &required_size);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error retrieving password from NVS (%s)", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "WiFi credentials retrieved from NVS: SSID=%s, Password=%s", ssid, password);
    return ESP_OK;
}

esp_err_t nvs_handler_set_mqtt_config(const char *mqtt_url, uint16_t mqtt_port)
{
    esp_err_t err;
    nvs_handle_t nvs_handle;

    // Abrir el espacio de almacenamiento NVS para la configuración MQTT
    err = nvs_open("mqtt_config", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for MQTT config");
        return err;
    }

    // Guardar la URL MQTT
    err = nvs_set_str(nvs_handle, "mqtt_url", mqtt_url);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting MQTT URL in NVS");
        nvs_close(nvs_handle);
        return err;
    }

    // Guardar el puerto MQTT
    err = nvs_set_u16(nvs_handle, "mqtt_port", mqtt_port);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting MQTT port in NVS");
        nvs_close(nvs_handle);
        return err;
    }

    // Commit para escribir los cambios en el almacenamiento NVS
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing MQTT config to NVS");
        nvs_close(nvs_handle);
        return err;
    }

    // Cerrar el handle NVS
    nvs_close(nvs_handle);
    return ESP_OK;
}
esp_err_t nvs_handler_get_mqtt_config(char *mqtt_url, size_t url_max_len, uint16_t *mqtt_port)
{
    esp_err_t err;
    nvs_handle_t nvs_handle;

    // Abrir el espacio de almacenamiento NVS para la configuración MQTT
    err = nvs_open("mqtt_config", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for MQTT config");
        return err;
    }

    // Leer la URL MQTT del NVS
    size_t required_size = url_max_len;
    err = nvs_get_str(nvs_handle, "mqtt_url", mqtt_url, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading MQTT URL from NVS");
        nvs_close(nvs_handle);
        return err;
    }

    // Leer el puerto MQTT del NVS
    err = nvs_get_u16(nvs_handle, "mqtt_port", mqtt_port);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading MQTT port from NVS");
        nvs_close(nvs_handle);
        return err;
    }

    // Cerrar el handle NVS
    nvs_close(nvs_handle);
    return ESP_OK;
}

void url_decode(char *dst, const char *src)
{
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}
void save_counter_to_nvs(int32_t counter)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err == ESP_OK)
    {
        err = nvs_set_i32(my_handle, "file_counter", counter);
        if (err == ESP_OK)
        {
            nvs_commit(my_handle);
        }
        nvs_close(my_handle);
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) saving counter to NVS!", esp_err_to_name(err));
    }
}
int32_t load_counter_from_nvs()
{
    nvs_handle_t my_handle;
    int32_t counter = 0; // Valor predeterminado

    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err == ESP_OK)
    {
        err = nvs_get_i32(my_handle, "file_counter", &counter);
        nvs_close(my_handle);
    }

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "Counter not found in NVS, using default value 0");
        counter = 0;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) reading counter from NVS!", esp_err_to_name(err));
    }

    return counter;
}
