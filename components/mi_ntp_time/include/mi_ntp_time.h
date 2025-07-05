#pragma once
#include <stddef.h>
#include "esp_log.h"

void mi_ntp_time_init(void);
void mi_ntp_time_get_str(char *buffer, size_t len);