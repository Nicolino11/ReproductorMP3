#include "esp_netif.h"


/**
 * @brief Inicializa el dispositivo en modo Access Point (AP).
 *
 * Esta función configura la interfaz de red WiFi del ESP32 para que funcione como un punto de acceso (Access Point),
 * permitiendo que otros dispositivos se conecten a él. Se debe proporcionar un SSID y una contraseña para definir
 * el nombre y la clave de la red WiFi que se creará.
 *
 * @param ssid Nombre (SSID) de la red WiFi que será visible para los dispositivos que se quieran conectar.
 * @param password Contraseña que los dispositivos deberán ingresar para conectarse al Access Point.
 */
void init_wifi_ap(char *ssid, char *password);

/**
 * @brief Conecta el dispositivo a una red WiFi como cliente (Station).
 *
 * Esta función configura el ESP32 para que se conecte a una red WiFi existente utilizando el SSID y la contraseña
 * proporcionados. 
 *
 * @param ssid Nombre (SSID) de la red WiFi a la que se desea conectar.
 * @param password Contraseña de la red WiFi a la que se desea conectar.
 */
void connect_wifi_ap(char *ssid, char *password);


/**
 * @brief Callback que se ejecuta cuando cambian las credenciales del modo Station (STA).
 *
 * Esta función es llamada cuando se actualizan las credenciales de la red WiFi en modo Station.
 * Se utiliza para reconectar el dispositivo a la nueva red con las nuevas credenciales.
 *
 * @param new_ssid Nuevo SSID de la red WiFi.
 * @param new_password Nueva contraseña de la red WiFi.
 */
void on_sta_changed(char *new_ssid, char *new_password);


/**
 * @brief Inicializa la conexión WiFi en modo Station (STA) con las credenciales proporcionadas.
 *
 * Esta función configura el ESP32 para conectarse a una red WiFi como cliente (Station) utilizando
 * el SSID y la contraseña especificados. Se debe llamar a esta función antes de iniciar el dispositivo
 * en modo Access Point o cuando se desee cambiar las credenciales de conexión.
 *
 * @param ssid Nombre (SSID) de la red WiFi a la que se desea conectar.
 * @param password Contraseña de la red WiFi a la que se desea conectar.
 */
void init_connect_wifi_ap(char *ssid, char *password);

void init_wifi_apsta(const char *ap_ssid, const char *ap_password, const char *sta_ssid, const char *sta_password);