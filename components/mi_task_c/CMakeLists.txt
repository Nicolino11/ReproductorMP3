idf_component_register(
    SRCS "mi_task_c.c"
    INCLUDE_DIRS "include"
    REQUIRES mi_queue esp_timer 
)
