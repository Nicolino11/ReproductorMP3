## Reproductor MP3

**Integrantes del trabajo:** Nicolas Raposo, Martin Da Rosa y Rafael Durán.

## Uso de Touchpad

Para usar el touchpad simplemente es necesario conectarlo, los botones hacen lo siguiente:

VOL_UP -> Sube volumen<br />
VOL_DOWN -> Baja volumen<br />
PHOTO -> Canción anterior<br />
PLAY/PAUSE -> Pausa/Resumir<br />
RECORD -> Canción siguiente<br />

El botón NETWORK no hace nada<br />

## Levantar el servidor MQTT

Para levantar el servidor MQTT, se debe ejecutar el siguiente comando en la terminal:

Crear un archivo que tenga la configuracion del servidor MQTT `mosquitto.conf`:

```conf
listener 1883 0.0.0.0
allow_anonymous true
```

Luego ya que tenemos el archivo de configuracion, podemos levantar el servidor MQTT con Docker:

```bash
docker run -it --rm -p 1883:1883 -p 9001:9001 -v /path/to/mosquitto.conf:/mosquitto/config/mosquitto.conf eclipse-mosquitto
```

El topico que se va a utilizar para el laboratorio es `/player/control`.
Y en el payload se debe indicar el comando:

```
EVENT_NEXT_TRACK,
EVENT_PREV_TRACK,
EVENT_VOL_UP,
EVENT_VOL_DOWN,
EVENT_PLAY_PAUSE,
EVENT_STOP
```
