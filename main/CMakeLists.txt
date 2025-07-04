idf_component_register(SRCS "main.c"
                      INCLUDE_DIRS "."
                      REQUIRES driver led_strip mi_web_server mi_wifi_ap mi_delay mi_led mi_touch mi_queue mi_audio mi_mqtt mi_fs)