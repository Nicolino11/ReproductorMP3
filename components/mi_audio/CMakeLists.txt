idf_component_register(
    SRCS "i2s_es8311_example.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2s esp_driver_gpio mi_queue mi_led mi_fs
    EMBED_FILES
        "butterfly.pcm"
        "dance.pcm"
        "doom.pcm"
        "mission.pcm"
        "pacman.pcm"
        "tetris.pcm"
        "undertale.pcm"
)