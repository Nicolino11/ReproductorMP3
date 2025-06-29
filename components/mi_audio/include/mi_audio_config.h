#pragma once
#include <stdbool.h>
#include <stdint.h>


#include "sdkconfig.h"

/* Example configurations */
#define EXAMPLE_RECV_BUF_SIZE   (2400)
#define EXAMPLE_SAMPLE_RATE     (8000)
#define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    60
#define EXAMPLE_PA_CTRL_IO      (GPIO_NUM_10)


/* I2C port and GPIOs */
#define I2C_NUM         (0)
#define I2C_SCL_IO      (GPIO_NUM_7)
#define I2C_SDA_IO      (GPIO_NUM_8)


/* I2S port and GPIOs */
#define I2S_NUM         (1)
#define I2S_MCK_IO      (GPIO_NUM_35)
#define I2S_BCK_IO      (GPIO_NUM_18)
#define I2S_WS_IO       (GPIO_NUM_17)
#define I2S_DO_IO       (GPIO_NUM_12)
#define I2S_DI_IO       (GPIO_NUM_46)