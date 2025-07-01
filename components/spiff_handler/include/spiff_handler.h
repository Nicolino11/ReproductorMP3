// spiff_handler.h
#ifndef SPIFF_HANDLER_H_
#define SPIFF_HANDLER_H_

#include "esp_err.h"

esp_err_t spiff_handler_init(void);
esp_err_t spiff_handler_get_info(size_t *total, size_t *used, size_t *free);
esp_err_t spiff_handler_delete_file(const char *filename);
esp_err_t spiff_handler_list_files(char **out_file_list);
esp_err_t spiff_handler_delete_all_files(const char *file_list);
esp_err_t spiff_handler_add_song_to_list(const char *song_name);
esp_err_t spiff_handler_check_space(size_t file_size);
esp_err_t spiff_handler_write_file(const char *filename, const char *data, size_t size);
esp_err_t spiff_handler_remove_last_song();


#endif /* SPIFF_HANDLER_H_ */

