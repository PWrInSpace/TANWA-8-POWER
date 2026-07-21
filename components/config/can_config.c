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
    uint8_t tx_buffer[8] = {0}; // Lokalny bezpieczny bufor

    xSemaphoreTake(BoardDataSemaphore, portMAX_DELAY);
    uint16_t v24V     = (uint16_t)(BoardData.Voltage24V_in * 1000.0f + 0.5f);
    uint16_t v24VSOL  = (uint16_t)(BoardData.Voltage24VSOL_in * 1000.0f + 0.5f);
    uint16_t i5V      = (uint16_t)(BoardData.Current5V_in * 1000.0f + 0.5f);
    uint16_t i24VSOL  = (uint16_t)(BoardData.Current24VSOL_in * 1000.0f + 0.5f);
    xSemaphoreGive(BoardDataSemaphore);

    memcpy(&tx_buffer[0], &v24V, 2);
    memcpy(&tx_buffer[2], &v24VSOL, 2);
    memcpy(&tx_buffer[4], &i5V, 2);
    memcpy(&tx_buffer[6], &i24VSOL, 2);

    // Wysyłamy tx_buffer, nie data!
    can_send_message(CAN_POWER_DATA_ID, tx_buffer, 8);

    ESP_LOGI(TAG, "Raw data sent: %02X %02X %02X %02X %02X %02X %02X %02X",
             tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3], 
             tx_buffer[4], tx_buffer[5], tx_buffer[6], tx_buffer[7]);
             
    return ESP_OK;
}

esp_err_t power_status_handler(uint8_t *data, uint8_t length) {

    ESP_LOGI(TAG, "Power status command received, preparing response...");
    uint8_t data_send[8] = {0};
    data_send[0] = BoardData.ch1_status ? 1 : 0;
    data_send[1] = BoardData.fault_1_status ? 1 : 0;
    data_send[2] = BoardData.fault_2_status ? 1 : 0;
    can_send_message(CAN_POWER_STATUS_ID, data_send, 3);
    return ESP_OK;
}

can_command_t can_commands[] = {
    // Example command registration
    {CAN_TEMPLATE_MESSAGE_ID, new_command_handler},
    {CAN_POWER_GET_DATA_ID, power_status_command_handler},
    {CAN_POWER_GET_STATUS_ID, power_status_handler},
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