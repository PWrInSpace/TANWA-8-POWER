#include "can_task.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "mcu_twai_config.h" 

#include "can_commands.h"

#define CAN_TASK_STACK_SIZE 4096
#define CAN_TASK_PRIORITY 9
#define CAN_TASK_CORE 1
#define CAN_TASK_DEFAULT_FREQ 100

#define TAG "TWAI Listen Only"

#ifdef master_conf

#endif

static TaskHandle_t can_task_handle = NULL;

TickType_t Max_time = 500;

/**
 * @brief function that inserts float into uint8_t array
 * 
 * @param value 
 * @param bytes 
 * @param start_idx 
 * @return * void 
 */
void insert_float_into_uint8_array(float *value, uint8_t *bytes, uint8_t start_idx)
{
    if (start_idx > 4)
    {
        ESP_LOGE(TAG, "Invalid start index");
        return;
    }
    memcpy(bytes + start_idx, value, sizeof(float));
}


/**
 * @brief functiin that formats and sends FLC status with temp fro TMP1075
 * 
 */

/**void send_status(void)
{

    twai_message_t tx_msg;
    tx_msg.identifier = CAN_FLC_TX_STATUS;

    float temperature_celsius[TMP1075_QUANTITY] = {0};
    int16_t temp_parsed[TMP1075_QUANTITY] = {0};
    uint8_t data[8] = {0};

    uint16_t status = get_status();
    memcpy(tx_msg.data, &status, sizeof(status));

    for (int i = 0; i < TMP1075_QUANTITY; i++)
    {
        if (tmp1075_get_temp_celsius(&TANWA_hardware.tmp1075[i], &temperature_celsius[i]) != ESP_OK)
        {
            ESP_LOGE(TAG, "TEMP READ CELSIUS FAIL");
            return;
        }
    }

    for (int i = 0; i < TMP1075_QUANTITY; i++)
    {
        temp_parsed[i] = temperature_celsius[i];
    }

    // Insert the int16_t value into the data array starting at index x

    int i = 0;
    while (i < TMP1075_QUANTITY)
    {
        for (int j = sizeof(status); j < 4; j += 2)
        {
            if (i >= TMP1075_QUANTITY)
            break;
            insert_int16_t_into_uint8_array(&temp_parsed[i], data, j);
            i++;
        }

        if (i >= TMP1075_QUANTITY)
            break;
        
    }

    tx_msg.data_length_code = 8; // Full length


    memcpy(tx_msg.data, data, tx_msg.data_length_code);

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(100)) != ESP_OK)
    {
        ESP_LOGE(TAG, "TRANSMIT TEMP CELSIUS FAIL");
        return;
    }
}

**/
/**
 * @brief function that formats and sends the thermocouple data
 * 
 */
void send_data(void)

{

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(10)) != ESP_OK)
    {
        ESP_LOGE(TAG, "TRANSMIT ThermoCouple FAIL");
        return;
    }
}

/**
 * @brief function that formats and sends the pressure data
 * 
 */

void update_state()
{

    twai_message_t tx_msg;
    tx_msg.identifier = CAN_FLC_TX_BEACON;
    tx_msg.data_length_code = 8;

        if (twai_transmit(&tx_msg, 0) != ESP_OK)
        {
        ESP_LOGE(TAG, "&&&&&&&&&&&&&&&TRANSMIT Beacon FAIL&&&&&&&&&&&&&&&&&&&");
        return;
        }
    
}

/**
 * @brief function that decodes the message
 * 
 * @param rx_msg // inputed message from CAN
 */
void can_decode_message(twai_message_t rx_msg)
{
    switch (rx_msg.identifier)
    {
    case CAN_PWR_RX_GET_STATUS:
    {
        ESP_LOGI(TAG, "FLC RX SEND_STATUS");
        send_status();
    }
    break;

    case CAN_PWR_RX_GET_DATA:
    {
        ESP_LOGI(TAG, "FLC RX GET_THERMOCOUPLES");
        send_data();
    }
    break;

    break;

    case CAN_PWR_RX_SOFT_RESET:
    {
        can_flc_commands_t cmd = CAN_FLC_RX_SOFT_RESET;
        if (xQueueSend(CMDS_queue, &cmd, 0) != pdTRUE)
        {
            ESP_LOGE(TAG, "TRANSMIT  CAN_FLC_TX_SOFT_RESET TO COMMANDS QUEUE FAIL");
        }
    }
    break;

    case CAN_PWR_RX_NOTHING:
    {
        __asm__ __volatile__("nop");
    }
    break;

    default:
        break;
    }
}

/**
 * @brief task function to handle can transcieving
 * 
 * @param pvParameters 
 */
void can_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t update_time = pdMS_TO_TICKS(500);
    while (true)
        
    while (true)
    {
             twai_message_t rx_msg;
        if (twai_receive(&rx_msg, pdMS_TO_TICKS(100)) == ESP_OK)
        {
            can_decode_message(rx_msg);
        }

        TickType_t current_tick_count = xTaskGetTickCount();

         if ((current_tick_count - xLastWakeTime) >= update_time)
        {
            monitor_twai_status();
            ESP_LOGI(TAG,"&&&&&&&&&&&&&&&Update_state&&&&&&&&&&&&&&&");
            update_state();
            xLastWakeTime = current_tick_count;
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

/**
 * @brief function that starts the can_task() task
 * 
 */
void run_can_task(void)
{
    if (twai_start() != ESP_OK) {
      ESP_LOGE(TAG, "TWAI start error");
    } else {
    xTaskCreatePinnedToCore(can_task, "can_task", CAN_TASK_STACK_SIZE, NULL, CAN_TASK_PRIORITY,
                            &can_task_handle, CAN_TASK_CORE);
    //xTaskCreatePinnedToCore(send_beacon_task, "send_beacon_task", 1024, NULL, CAN_TASK_PRIORITY-1,
    //                        &send_beacon_task_handle, CAN_TASK_CORE-1);
    }

}