idf_component_register(SRCS "mi_mqtt.c"
                    INCLUDE_DIRS "include"
                    INCLUDE_DIRS "."
                    REQUIRES mi_delay mi_queue mqtt)