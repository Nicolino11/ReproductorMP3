#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mi_task_b.h"
#include "esp_log.h"
#include "mi_queue.h"
#include "string.h"
#include "esp_err.h"
#include <ctype.h>


#define EX_UART_NUM UART_NUM_0
#define BUF_SIZE (1024)
#define LINE_BUF_SIZE 256
#define MAX_EVENTS 10
#define N_COLORS (sizeof(colors)/sizeof(colors[0]))

static const char *TAG = "UART_PAT";

typedef struct {
    const char* name;
    int r, g, b;
} color_map_t;

static const color_map_t colors[] = {
    {"RED", 255, 0, 0},
    {"GREEN", 0, 255, 0},
    {"BLUE", 0, 0, 255},
    {"YELLOW", 255, 255, 0},
    {"ORANGE", 255, 128, 0},
    {"VIOLET", 128, 0, 255},
    {"WHITE", 255, 255, 255},
    {"BLACK", 0, 0, 0},
};

// Buscar color en el mapa de colores
static int find_color(const char* name, int* r, int* g, int* b) {
    for (size_t i = 0; i < N_COLORS; ++i) {
        if (strcasecmp(name, colors[i].name) == 0) {
            *r = colors[i].r;
            *g = colors[i].g;
            *b = colors[i].b;
            return 1;
        }
    }
    return 0;
}

// Se hace un split de la línea recibida por UART y se parsea cada evento
// Ejemplo de línea: "YELLOW 10, RED 5, BLUE 20"
static int parse_line(char* line, mi_evento_t* arr, int max_arr) {
    int count = 0;
    char* token = strtok(line, ",");
    
    while (token && count < max_arr) {
        char color[16];
        int time;

        //e.g. "YELLOW 10"
        if (sscanf(token, "%15s %d", color, &time) == 2) {
            int r, g, b;
            if (find_color(color, &r, &g, &b)) {
                arr[count].r = r;
                arr[count].g = g;
                arr[count].b = b;
                arr[count].tiempo_ms = time;
                count++;
            }
        }
        token = strtok(NULL, ",");
    }
    if (count == 0) {
        ESP_LOGW(TAG, "No hay eventos válidos en la línea: %s", line);
    } else {
        ESP_LOGI(TAG, "Se analizaron %d eventos de la línea", count);
    }
    return count;
}

// Comunicacion con UART ver:
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/uart/uart_echo/main/uart_echo_example_main.c
static void uart_event_task(void *arg)
{
    QueueHandle_t queue = (QueueHandle_t)arg;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    #if CONFIG_UART_ISR_IN_IRAM
        intr_alloc_flags = ESP_INTR_FLAG_IRAM;
    #endif

    ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    int line_idx = 0;
    char line_buf[LINE_BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(EX_UART_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (len) {
            data[len] = '\0';
            line_buf[line_idx] = '\0';
            ESP_LOGI(TAG, "%s%s", line_buf, (char *) data);
        }
        for (int i = 0; i < len; ++i) {
            if (data[i] == '\n' || data[i] == '\r') {
                line_buf[line_idx] = '\0';
                ESP_LOGI(TAG, "Procesar línea: %s", line_buf);
                mi_evento_t arr[MAX_EVENTS];
                int n = parse_line(line_buf, arr, MAX_EVENTS);
                for (int i = 0; i < n; ++i) {
                    ESP_LOGI(TAG, "Color: R=%d G=%d B=%d Time=%d", arr[i].r, arr[i].g, arr[i].b, arr[i].tiempo_ms);
                    mi_queue_send(queue, &arr[i], portMAX_DELAY);
                }
                line_idx = 0;
            } else if (line_idx < LINE_BUF_SIZE - 1) {
                line_buf[line_idx++] = data[i];
            }
        }
    }
}

TaskHandle_t mi_task_b_start(QueueHandle_t queue) {
    TaskHandle_t handle = NULL;
    xTaskCreate(uart_event_task, "uart_event_task", 3072, (void*)queue, 12, &handle);
    return handle;
}
