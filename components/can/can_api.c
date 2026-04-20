#include "can_api.h"

#include <string.h>

#include "driver/twai.h"

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mcu_gpio_config.h"

#define CAN_TASK_STACK_SIZE CONFIG_CAN_TASK_STACK_SIZE
#define CAN_TASK_PRIORITY 8
#define CAN_TASK_CORE_ID 0

#define TWAI_MAX_MESSAGE_LENGTH 8

#define TAG "CAN_API"

static struct {
    can_command_t *commands;
    size_t num_commands;
    TaskHandle_t task_handle;
} gb;

esp_err_t can_start(void) {
    esp_err_t err;

    // Start the TWAI driver
    //vTaskDelay(pdMS_TO_TICKS(1000)); // Short delay to ensure proper startup
    err = twai_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "TWAI driver started");

    //can_send_message(CAN_SENSOR_DAT| OpenDrain: 1| Pullup: 0| Pulldown: 0| Intr:0 
    return ESP_OK;
}

esp_err_t can_stop(void) {
    esp_err_t err;

    // Stop the TWAI driver
    err = twai_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop TWAI driver: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t can_send_message(uint32_t id, uint8_t *data, uint8_t length) {
    esp_err_t err;
    twai_message_t message;

    if(length > TWAI_MAX_MESSAGE_LENGTH) {
        ESP_LOGE(TAG, "Data length exceeds maximum allowed length");
        return ESP_ERR_INVALID_ARG;
    }

    // Prepare the message
    message.identifier = id;
    message.extd = 1;
    message.data_length_code = length;
    memcpy(message.data, data, length);

    //ESP_LOGI(TAG, "Data: %02X %02X %02X %02X %02X %02X %02X %02X", message.data[0], message.data[1], message.data[2], message.data[3], message.data[4], message.data[5], message.data[6], message.data[7]);

    // Send the message
    err = twai_transmit(&message, pdMS_TO_TICKS(50));
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

esp_err_t can_register_commands(can_command_t *commands, size_t num_commands) {
    if (commands == NULL) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    gb.commands = commands;
    gb.num_commands = num_commands;

    return ESP_OK;
}

esp_err_t can_task_init(void) {

    if (gb.commands == NULL || gb.num_commands == 0) {
        ESP_LOGE(TAG, "No CAN commands registered");
        return ESP_ERR_INVALID_STATE;
    }

    if(xTaskCreatePinnedToCore(can_task, "CAN Task", CAN_TASK_STACK_SIZE, NULL, 9, &gb.task_handle, CAN_TASK_CORE_ID) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CAN task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void handle_can_alerts(uint32_t alerts) {
    if (alerts == 0) return;

    twai_status_info_t status;
    twai_get_status_info(&status);

    static uint32_t last_log_time = 0;
    uint32_t now = xTaskGetTickCount();
    bool can_log_periodic = (now - last_log_time) >= pdMS_TO_TICKS(2500);

    // --- 1. BUS OFF ---
if (alerts & TWAI_ALERT_BUS_OFF) {
    ESP_LOGE("CAN_DIAG", "Bus-Off! TEC:%lu, REC:%lu. Czekam na stabilizację...", 
             status.tx_error_counter, status.rx_error_counter);
    
    twai_clear_transmit_queue();
    
    // 100ms odpoczynku zanim w ogóle zaczniemy recovery
    vTaskDelay(pdMS_TO_TICKS(100)); 
    twai_initiate_recovery();
}
    
    // --- 2. BUS RECOVERED
    if (alerts & TWAI_ALERT_BUS_RECOVERED) {
        vTaskDelay(pdMS_TO_TICKS(200)); 
        twai_clear_transmit_queue(); 
        
        esp_err_t err = twai_start();
        if (err == ESP_OK) {
            twai_status_info_t post_status;
            twai_get_status_info(&post_status);
            
            ESP_LOGI("CAN_DIAG", "========================================");
            ESP_LOGI("CAN_DIAG", "Alert: BUS RECOVERED! Driver zrestartowany.");
            ESP_LOGI("CAN_DIAG", "Nowa sesja -> Stan: RUNNING, TEC: %lu, REC: %lu", 
                     post_status.tx_error_counter, post_status.rx_error_counter);
            ESP_LOGI("CAN_DIAG", "========================================");
        } else {
            ESP_LOGE("CAN_DIAG", "Błąd restartu po recovery: %s", esp_err_to_name(err));
        }
        last_log_time = now; 
    }

    // --- 3. ERROR PASSIVE (Ostrzeżenie przed Bus-Off) ---
    if (alerts & TWAI_ALERT_ERR_PASS) {
        ESP_LOGW("CAN_DIAG", "Alert: ERROR PASSIVE! Sprawdź fizyczne połączenie. TEC: %lu", status.tx_error_counter);
        if (status.msgs_to_tx > 15) twai_clear_transmit_queue();
    }

    // --- 4. BŁĘDY TRANSMISJI ---
    if (alerts & (TWAI_ALERT_TX_FAILED | TWAI_ALERT_BUS_ERROR)) {
        if (can_log_periodic) {
            ESP_LOGW("CAN_DIAG", "Błędy magistrali (Brak ACK/BitError). TEC: %lu, Oczekujące TX: %lu", 
                     status.tx_error_counter, status.msgs_to_tx);
            last_log_time = now;
        }
    }
}

#define CAN_HEARTBEAT_CHECK_ID 0x7FF 
void can_task(void *arg) {
    twai_message_t message;
    uint32_t alerts;
    twai_status_info_t status;
    
    TickType_t last_msg_time = xTaskGetTickCount();
    const TickType_t timeout = pdMS_TO_TICKS(5000);
    ESP_LOGI(TAG, "CAN task started, waiting for messages...");

    while (1) {
        if (twai_read_alerts(&alerts, 0) == ESP_OK) {
            handle_can_alerts(alerts);
        }

        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(100));

        if (err == ESP_OK) {
            last_msg_time = xTaskGetTickCount();
            
            for (size_t i = 0; i < gb.num_commands; i++) {
                if (gb.commands[i].message_id == message.identifier) {
                    gb.commands[i].handler(message.data, message.data_length_code);
                    break;
                }
            }
        } else {
            twai_get_status_info(&status);
            if (status.state == TWAI_STATE_RUNNING) {
                TickType_t now = xTaskGetTickCount();
                if ((now - last_msg_time) >= timeout) {
                    
                    uint8_t dummy = 0;
                    if (can_send_message(CAN_HEARTBEAT_CHECK_ID, &dummy, 0) != ESP_OK) {
                        twai_clear_transmit_queue();
                    }
                    
                    last_msg_time = now;
                }
            }
        }
    }
}

esp_err_t can_deinit(void) {
    esp_err_t err;

    // Stop the TWAI driver
    err = can_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop CAN driver: %s", esp_err_to_name(err));
        return err;
    }

    // Delete the CAN task
    if (gb.task_handle != NULL) {
        vTaskDelete(gb.task_handle);
        gb.task_handle = NULL;
    }

    // Clear the command list
    gb.commands = NULL;
    gb.num_commands = 0;

    return ESP_OK;
}