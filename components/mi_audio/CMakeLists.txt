idf_component_register(
    SRCS "i2s_es8311_example.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_i2s esp_driver_gpio
    EMBED_FILES
        "butterfly.pcm"
        "dance.pcm"
        "alive.pcm"
)