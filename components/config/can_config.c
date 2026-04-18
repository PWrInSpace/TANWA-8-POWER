#include "can_config.h"
#include "can_api.h"
#include "can_commands.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/twai.h"
#include "BoardData.h"
#include "string.h"

#define TAG "CAN_CONFIG"

esp_err_t new_command_handler(uint8_t *data, uint8_t length) {
    // Example: just print received data
    printf("New command received with length %d\n", length);
    for (int i = 0; i < length; ++i) {
        printf("Byte %d: %02X\n", i, data[i]);
    }

    // Return success
    return ESP_OK;
}

esp_err_t power_status_command_handler(uint8_t *data, uint8_t length) {
    xSemaphoreTake(BoardDataSemaphore, portMAX_DELAY);
    uint16_t v24V     = (uint16_t)(BoardData.Voltage24V_in * 1000.0f + 0.5f);
    uint16_t v24VSOL  = (uint16_t)(BoardData.Voltage24VSOL_in * 1000.0f + 0.5f);
    uint16_t i5V      = (uint16_t)(BoardData.Current5V_in * 1000.0f + 0.5f);
    uint16_t i24VSOL  = (uint16_t)(BoardData.Current24VSOL_in * 1000.0f + 0.5f);
xSemaphoreGive(BoardDataSemaphore);
    memcpy(data, &v24V, 2);
    memcpy(data + 2, &v24VSOL, 2);
    memcpy(data + 4, &i5V, 2);
    memcpy(data + 6, &i24VSOL, 2);

    ESP_LOGI(TAG, "CAN SENT POWER STATUS: 24V=%.2fV, 24V-SOL=%.2fV, 5V Current=%.2fmA, 24V-SOL Current=%.2fmA", 
             BoardData.Voltage24V_in, BoardData.Voltage24VSOL_in, BoardData.Current5V_in * 1000.0f, BoardData.Current24VSOL_in * 1000.0f);
    return ESP_OK;
}

can_command_t can_commands[] = {
    // Example command registration
    {CAN_TEMPLATE_MESSAGE_ID, new_command_handler},
    {CAN_SEND_POWER_STATUS_MESSAGE_ID, power_status_command_handler},
    // Add your CAN commands here
};

esp_err_t can_config_init(void) {
    esp_err_t err;

    // Register CAN commands
    err = can_register_commands(can_commands, sizeof(can_commands) / sizeof(can_commands[0]));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN command registration failed");
        return err;
    }

    // Initialize CAN driver
    err = can_task_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN driver initialization failed");
        return err;
    }

    // Start CAN driver
    err = can_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN driver start failed");
        return err;
    }

    return ESP_OK;
}