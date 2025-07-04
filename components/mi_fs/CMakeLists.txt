idf_component_register(SRCS "mi_fs.c"
                    INCLUDE_DIRS "include"
                    INCLUDE_DIRS "."
                    REQUIRES littlefs json)


littlefs_create_partition_image(storage ./flash_data FLASH_IN_PROJECT)