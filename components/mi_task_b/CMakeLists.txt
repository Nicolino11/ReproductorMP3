idf_component_register(SRCS "mi_task_b.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_uart mi_queue
)
