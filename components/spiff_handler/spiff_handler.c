#include "spiff_handler.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "Spiff_handler";
static SemaphoreHandle_t xSemaphore = NULL;
static SemaphoreHandle_t spiff_mutex = NULL;

esp_err_t spiff_handler_init(void)
{
    // Configuración SPIFFS...
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format SPIFFS filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    ESP_LOGI(TAG, "SPIFFS initialized");

    // Crear el semáforo
    xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return ESP_FAIL;
    }
    spiff_mutex = xSemaphoreCreateMutex();
    if (spiff_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create SPIFF mutex");
    }

    return ESP_OK;
}

esp_err_t spiff_handler_get_info(size_t *total, size_t *used, size_t *free)
{
    esp_err_t ret = esp_spiffs_info(NULL, total, used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    *free = *total - *used;

    ESP_LOGI(TAG, "SPIFFS Partition Info:");
    ESP_LOGI(TAG, "Total space: %d bytes", *total);
    ESP_LOGI(TAG, "Used space: %d bytes", *used);
    ESP_LOGI(TAG, "Free space: %d bytes", *free);

    return ESP_OK;
}
esp_err_t spiff_handler_check_space(size_t file_size)
{
    size_t total, used, free;
    spiff_handler_get_info(&total, &used, &free);
    if (file_size > free)
    {
        ESP_LOGE(TAG, "Insufficient space in SPIFFS. File size (%d bytes) exceeds available space (%d bytes)", file_size, free);
        return ESP_FAIL;
    }
    return ESP_OK;
}
esp_err_t spiff_handler_write_file(const char *filename, const char *data, size_t size)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, size, f);
    if (written != size)
    {
        ESP_LOGE(TAG, "Error writing data to file");
        fclose(f);
        return ESP_FAIL;
    }

    fclose(f);
    return ESP_OK;
}

esp_err_t spiff_handler_insert_file(const char *filepath, const uint8_t *data, size_t size)
{
    FILE *f = fopen(filepath, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing", filepath);
        return ESP_FAIL;
    }

    size_t bytes_written = fwrite(data, 1, size, f);
    fclose(f);

    if (bytes_written != size)
    {
        ESP_LOGE(TAG, "Failed to write entire file %s (%s)", filepath, esp_err_to_name(ESP_FAIL));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "File %s inserted successfully", filepath);
    return ESP_OK;
}

esp_err_t spiff_handler_read_filenames(char ***out_filenames, size_t *num_files)
{
    DIR *dir;
    struct dirent *ent;
    char **filenames = NULL;
    size_t count = 0;

    // Abrir el directorio raíz de SPIFFS
    dir = opendir("/spiffs");
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "Error opening SPIFFS directory");
        return ESP_FAIL;
    }

    // Contar la cantidad de archivos
    while ((ent = readdir(dir)) != NULL)
    {
        // Ignorar las entradas especiales "." y ".."
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        count++;
    }

    // Reservar memoria para almacenar los nombres de archivo
    filenames = (char **)malloc(count * sizeof(char *));
    if (filenames == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for filenames");
        closedir(dir);
        return ESP_FAIL;
    }

    // Reiniciar el puntero del directorio
    rewinddir(dir);

    // Leer los nombres de archivo y almacenarlos en el array
    size_t i = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        filenames[i] = strdup(ent->d_name); // Duplicar el nombre para manejarlo independientemente
        i++;
    }

    closedir(dir);

    *out_filenames = filenames;
    *num_files = count;
    return ESP_OK;
}

esp_err_t spiff_handler_delete_file(const char *filename)
{
    int ret = unlink(filename);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to delete file %s (%s)", filename, strerror(errno));
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "File %s deleted successfully", filename);
        return ESP_OK;
    }
}

esp_err_t spiff_handler_save_file_list(const char *file_list)
{
    const char *filepath = "/spiffs/canciones.txt";

    FILE *f = fopen(filepath, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing", filepath);
        return ESP_FAIL;
    }

    size_t bytes_written = fwrite(file_list, 1, strlen(file_list), f);
    fclose(f);

    if (bytes_written != strlen(file_list))
    {
        ESP_LOGE(TAG, "Failed to write entire file %s (%s)", filepath, esp_err_to_name(ESP_FAIL));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "File list saved successfully to %s", filepath);
    return ESP_OK;
}

esp_err_t spiff_handler_delete_first_file(const char *file_list)
{
    // Verificar que la lista no esté vacía
    if (file_list == NULL || strlen(file_list) == 0)
    {
        ESP_LOGE(TAG, "File list is empty or invalid");
        return ESP_FAIL;
    }

    // Encontrar la posición del primer delimitador (", ")
    const char *first_delim = strstr(file_list, ", ");
    if (first_delim == NULL)
    {
        // Si no hay delimitador, significa que hay un solo archivo en la lista
        ESP_LOGI(TAG, "Only one file in the list, cannot delete the first file");
        return ESP_FAIL;
    }

    // Calcular la longitud del nombre del primer archivo
    size_t first_file_len = first_delim - file_list;

    // Crear un nuevo buffer para almacenar la lista de archivos sin el primer archivo
    char *new_file_list = (char *)malloc(strlen(file_list) - first_file_len);
    if (new_file_list == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for new file list");
        return ESP_FAIL;
    }

    // Copiar la parte de la lista después del primer archivo al nuevo buffer
    strcpy(new_file_list, file_list + first_file_len + 2); // +2 para saltar el ", "

    // Llamar a la función para guardar la nueva lista de archivos en el archivo canciones.txt
    esp_err_t ret = spiff_handler_save_file_list(new_file_list);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save updated file list after deleting the first file");
        free(new_file_list);
        return ret;
    }

    ESP_LOGI(TAG, "First file deleted from the list");
    free(new_file_list);
    return ESP_OK;
}

esp_err_t spiff_handler_delete_all_files(const char *file_list)

{
    // Verificar que la lista no esté vacía
    if (file_list == NULL || strlen(file_list) == 0)
    {
        ESP_LOGE(TAG, "File list is empty or invalid");
        return ESP_FAIL;
    }

    // Crear una copia de la lista de archivos para evitar modificar la original
    char *file_list_copy = strdup(file_list);
    if (file_list_copy == NULL)
    {
        ESP_LOGE(TAG, "Failed to duplicate file list");
        return ESP_FAIL;
    }

    // Dividir la lista de archivos usando strtok_r
    char *token;
    char *saveptr;
    token = strtok_r(file_list_copy, ", ", &saveptr);

    esp_err_t ret = ESP_OK;
    while (token != NULL)
    {
        // Eliminar cada archivo de la lista
        ret = spiff_handler_delete_file(token);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to delete file %s from SPIFFS", token);
            free(file_list_copy);
            return ret;
        }

        // Obtener el siguiente token
        token = strtok_r(NULL, ", ", &saveptr);
    }

    // Liberar la memoria de la copia de la lista de archivos
    free(file_list_copy);

    // Guardar una lista vacía en el archivo canciones.txt
    ret = spiff_handler_save_file_list("");
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save empty file list to canciones.txt");
        return ret;
    }

    ESP_LOGI(TAG, "All files deleted successfully");
    return ESP_OK;
}

esp_err_t spiff_handler_list_files(char **out_file_list)
{
    DIR *dir;
    struct dirent *ent;
    char *file_list = NULL;
    size_t list_size = 0;

    if (xSemaphore != NULL)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            dir = opendir("/spiffs");
            if (dir == NULL)
            {
                ESP_LOGE(TAG, "Error opening SPIFFS directory");
                xSemaphoreGive(xSemaphore);
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Files in SPIFFS:");

            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                {
                    continue;
                }

                size_t name_len = strlen(ent->d_name);
                size_t delimiter_len = (list_size > 0) ? 2 : 1;

                char *temp_list = (char *)realloc(file_list, list_size + name_len + delimiter_len + 1);
                if (temp_list == NULL)
                {
                    ESP_LOGE(TAG, "Failed to reallocate memory for file list");
                    closedir(dir);
                    free(file_list);
                    xSemaphoreGive(xSemaphore);
                    return ESP_FAIL;
                }
                file_list = temp_list;

                if (list_size > 0)
                {
                    strncat(file_list, ", ", list_size + 2);
                }
                else
                {
                    file_list[0] = '\0';
                }
                strncat(file_list, ent->d_name, name_len + 1);
                list_size += name_len + delimiter_len;
            }

            ESP_LOGI(TAG, "Lista de archivos: %s", file_list);

            closedir(dir);

            *out_file_list = file_list;
            xSemaphoreGive(xSemaphore);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "Semaforo listado de archivos liberado");
    return ESP_FAIL;
}

esp_err_t spiff_handler_add_song_to_list(const char *song_name)
{
    const char *filepath = "/spiffs/canciones.txt";

    if (xSemaphore != NULL)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            FILE *f = fopen(filepath, "a"); // Abrir el archivo en modo de adición
            if (f == NULL)
            {
                ESP_LOGE(TAG, "Failed to open file %s for writing", filepath);
                xSemaphoreGive(xSemaphore);
                return ESP_FAIL;
            }

            // Escribir el nombre de la canción seguido de un salto de línea
            size_t bytes_written = fprintf(f, "%s\n", song_name);
            fclose(f);

            xSemaphoreGive(xSemaphore);

            if (bytes_written < strlen(song_name) + 1) // +1 para el salto de línea
            {
                ESP_LOGE(TAG, "Failed to write entire song name to %s (%s)", filepath, esp_err_to_name(ESP_FAIL));
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Song %s added successfully to %s", song_name, filepath);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "semaforo agregar cancion liberado ");
    return ESP_FAIL;
}

esp_err_t spiff_handler_remove_last_song()
{
    if (spiff_mutex == NULL)
    {
        ESP_LOGE(TAG, "SPIFF mutex not initialized");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(spiff_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to take SPIFF mutex");
        return ESP_FAIL;
    }

    FILE *f = fopen("/spiffs/canciones.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open canciones.txt for reading");
        xSemaphoreGive(spiff_mutex);
        return ESP_FAIL;
    }

    char line[128];
    char last_song[128] = {0};
    size_t line_count = 0;

    while (fgets(line, sizeof(line), f))
    {
        strcpy(last_song, line);
        line_count++;
    }

    fclose(f);

    if (line_count <= 1)
    {
        ESP_LOGW(TAG, "Cannot remove the last song. At least one song must remain.");
        xSemaphoreGive(spiff_mutex);
        return ESP_OK;
    }

    f = fopen("/spiffs/canciones.txt", "r");
    FILE *temp_f = fopen("/spiffs/canciones_temp.txt", "w");
    if (f == NULL || temp_f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open files for updating canciones.txt");
        if (f)
            fclose(f);
        if (temp_f)
            fclose(temp_f);
        xSemaphoreGive(spiff_mutex);
        return ESP_FAIL;
    }

    size_t current_line = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (current_line < line_count - 1)
        {
            fputs(line, temp_f);
        }
        current_line++;
    }

    fclose(f);
    fclose(temp_f);

    remove("/spiffs/canciones.txt");
    rename("/spiffs/canciones_temp.txt", "/spiffs/canciones.txt");

    last_song[strcspn(last_song, "\n")] = 0; // Remove newline character
    if (remove(last_song) != 0)
    {
        ESP_LOGE(TAG, "Failed to remove song file: %s", last_song);
        xSemaphoreGive(spiff_mutex);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Successfully removed the last song: %s", last_song);
    esp_err_t result = spiff_handler_delete_file(last_song);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete the file.");
    }
    else
    {
        ESP_LOGI(TAG, "File deleted successfully.");
    }

    xSemaphoreGive(spiff_mutex);
    return ESP_OK;
}