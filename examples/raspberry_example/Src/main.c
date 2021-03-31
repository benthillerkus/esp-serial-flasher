/* Flash multiple partitions example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_loader.h"
#include "example_common.h"
#include "raspberry_port.h"

#define TARGET_RST_Pin 2
#define TARGET_IO0_Pin 3

#define DEFAULT_BAUD_RATE 115200
#define HIGHER_BAUD_RATE  460800
#define SERIAL_DEVICE     "/dev/ttyS0"

#define BINARY_PATH_APP       "./app.bin"
#define BINARY_PATH_PART      "./partitions.bin"
#define BINARY_PATH_BOOT      "./bootloader_dio_80m.bin"

#define BOOTLOADER_ADDRESS  0x1000
#define PARTITION_ADDRESS   0x8000
#define APPLICATION_ADDRESS 0x10000

static int upload_file(const char *boot_path, const char *part_path, const char *app_path) {
    uint8_t *boot = NULL;
    uint8_t *part = NULL;
    uint8_t *app = NULL;

    FILE *boot_image = fopen(boot_path, "r");
    if (boot_image == NULL) {
        fprintf(stderr, "Error:Failed to open file %s\n", boot_path);
        return 1;
    }

    FILE *part_image = fopen(part_path, "r");
    if (part_image == NULL) {
        fprintf(stderr, "Error:Failed to open file %s\n", part_path);
        return 1;
    }

    FILE *app_image = fopen(app_path, "r");
    if (app_image == NULL) {
        fprintf(stderr, "Error:Failed to open file %s\n", app_path);
        return 1;
    }

    fseek(boot_image, 0L, SEEK_END);
    size_t boot_size = ftell(boot_image);
    rewind(boot_image);
    printf("File %s opened. Size: %u bytes\n", boot_path, boot_size);

    fseek(part_image, 0L, SEEK_END);
    rewind(part_image);
    size_t part_size = ftell(part_image);
    printf("File %s opened. Size: %u bytes\n", part_path, part_size);

    fseek(app_image, 0L, SEEK_END);
    size_t app_size = ftell(app_image);
    rewind(app_image);
    printf("File %s opened. Size: %u bytes\n", app_path, app_size);

    int success_status = 0;

    boot = (uint8_t *) malloc(boot_size);
    if (boot == NULL) {
        printf("Error: Failed allocate memory\n");
        success_status = 1;
        goto cleanup;
    }

    part = (uint8_t *) malloc(part_size);
    if (part == NULL) {
        printf("Error: Failed allocate memory\n");
        success_status = 1;
        goto cleanup;
    }

    app = (uint8_t *) malloc(app_size);
    if (app == NULL) {
        fprintf(stderr, "Error: Failed allocate memory\n");
        success_status = 1;
        goto cleanup;
    }

    // copy file content to buffer
    {
        size_t boot_bytes_read = fread(boot, 1, boot_size, boot_image);
        if (boot_bytes_read != boot_size) {
            fprintf(stderr, "Error occurred while reading file");
            success_status = 1;
            goto cleanup;
        }
    }

    {
        size_t part_bytes_read = fread(part, 1, part_size, part_image);
        if (part_bytes_read != part_size) {
            fprintf(stderr, "Error occurred while reading file");
            success_status = 1;
            goto cleanup;
        }
    }

    {
        size_t app_bytes_read = fread(app, 1, app_size, app_image);
        if (app_bytes_read != app_size) {
            fprintf(stderr, "Error occurred while reading file");
            success_status = 1;
            goto cleanup;
        }
    }

    if (flash_binary(boot, boot_size, BOOTLOADER_ADDRESS) != ESP_LOADER_SUCCESS) return 1;
    if (flash_binary(part, part_size, PARTITION_ADDRESS) != ESP_LOADER_SUCCESS) return 1;
    if (flash_binary(app, app_size, APPLICATION_ADDRESS) != ESP_LOADER_SUCCESS) return 1;

    cleanup:
    fclose(boot_image);
    free(boot);
    fclose(part_image);
    free(part);
    fclose(app_image);
    free(app);
    return success_status;
}

void flush(void) {
    fflush(stderr);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    atexit(flush);
    bool useDefaults = true;

    if (argc <= 1) {
        printf("No arguments supplied, using defaults:\n");
        printf("Boot: %s\n", BINARY_PATH_BOOT);
        printf("Part: %s\n", BINARY_PATH_PART);
        printf("App: %s\n\n", BINARY_PATH_APP);
        useDefaults = false;
    } else if (argc < 4) {
        fprintf(stderr, "Not enough arguments supplied!\n");
        return 1;
    } else if (argc == 4) {
        printf("Using the following files:\n");
        printf("Boot: %s\n", argv[1]);
        printf("Part: %s\n", argv[2]);
        printf("App: %s\n\n", argv[3]);
    } else {
        fprintf(stderr, "Too many arguments supplied!\n");
        return 1;
    }

    const loader_raspberry_config_t config = {
            .device = SERIAL_DEVICE,
            .baudrate = DEFAULT_BAUD_RATE,
            .reset_trigger_pin = TARGET_RST_Pin,
            .gpio0_trigger_pin = TARGET_IO0_Pin,
    };

    if (loader_port_raspberry_init(&config) != ESP_LOADER_SUCCESS) return 1;

    if (connect_to_target(HIGHER_BAUD_RATE) != ESP_LOADER_SUCCESS) return 1;

    return upload_file(
            useDefaults ? argv[1] : BINARY_PATH_BOOT,
            useDefaults ? argv[2] : BINARY_PATH_PART,
            useDefaults ? argv[3] : BINARY_PATH_APP);

    // loader_port_reset_target(); // The ESP32-Cam is to be resetted by button press
}

#ifdef __cplusplus
}
#endif
