#include "powerStatus_task.h"
#include "TPS2121_controller.h"
#include "LTC4421_controller.h"
#include "24V_SOL_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "mcu_adc_config.h"

#define TAG "POWER_TASK"

void power_mng_task(void *pvParameters)
{
    // Initialize ADC
    if (!adc_init()) {
        ESP_LOGE(TAG, "ADC initialization failed. Exiting task.");
        vTaskDelete(NULL);
        return;
    }

    // Get ADC channels for GPIO_I_SENSE_24V and GPIO_24V_SEN
    adc_unit_t unit_i_sense, unit_24v_sen,unit_12v_sen;
    adc_channel_t channel_i_sense = gpio_to_adc_channel(GPIO_I_SENSE_24V, &unit_i_sense);
    adc_channel_t channel_24v_sen = gpio_to_adc_channel(GPIO_24V_SEN, &unit_24v_sen);
    adc_channel_t channel_12v_sen = gpio_to_adc_channel(GPIO_12V_SEN, &unit_12v_sen);

    // Validate ADC pins
    if (channel_i_sense < 0 || channel_24v_sen < 0) {
        ESP_LOGE(TAG, "Invalid ADC pin configuration. Exiting task.");
        adc_cali_delete_scheme_line_fitting(adc_cali_handle);
        adc_oneshot_del_unit(adc_handle);
        vTaskDelete(NULL);
        return;
    }

    // Configure GPIO pins for status reads (if not already configured)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_CH1) | (1ULL << GPIO_FAULT_1) | (1ULL << GPIO_FAULT_2) | (1ULL << GPIO_CH2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    while (1) {
        // Read 24V STATUS
        bool ch1_status = gpio_get_level(GPIO_CH1);
        bool fault_1_status = gpio_get_level(GPIO_FAULT_1);
        bool fault_2_status = gpio_get_level(GPIO_FAULT_2);
        bool ch2_status = gpio_get_level(GPIO_CH2);

        // Log GPIO status
        ESP_LOGI(TAG, "CH1: %d, FAULT_1: %d, FAULT_2: %d, CH2: %d",
                 ch1_status, fault_1_status, fault_2_status, ch2_status);

        // Read ADC voltages
        float voltage_i_sense = 0.0f, voltage_24v_sen = 0.0f , voltage_12v_sen = 0.0f;
        if (_mcu_adc_read_voltage(channel_i_sense, &voltage_i_sense)) {
            ESP_LOGI(TAG, "I_SENSE_24V (GPIO %d): %.3f V", GPIO_I_SENSE_24V, voltage_i_sense);
        } else {
            ESP_LOGE(TAG, "Failed to read I_SENSE_24V voltage");
        }

        if (_mcu_adc_read_voltage(channel_24v_sen, &voltage_24v_sen)) {
            ESP_LOGI(TAG, "24V_SEN (GPIO %d): %.3f V", GPIO_24V_SEN, voltage_24v_sen);
        } else {
            ESP_LOGE(TAG, "Failed to read 24V_SEN voltage");
        }

        if (_mcu_adc_read_voltage(channel_12v_sen, &voltage_12v_sen)) {
            ESP_LOGI(TAG, "12V_SEN (GPIO %d): %.3f V", GPIO_12V_SEN, voltage_12v_sen);
        } else {
            ESP_LOGE(TAG, "Failed to read 12V_SEN voltage");
        }

        // Placeholder for READ 12V STATUS
        // Add code here for 12V ADC readings if needed

        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 1 second
    }

    // Cleanup (unreachable due to infinite loop, but included for completeness)
    adc_cali_delete_scheme_line_fitting(adc_cali_handle);
    adc_oneshot_del_unit(adc_handle);
}