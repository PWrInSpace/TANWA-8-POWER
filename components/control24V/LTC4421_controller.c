/**
 * @file LTC4421_controller.c
 * @author Mateusz Kłosiński
 * @brief 
 * @version 0.1
 * @date 2025-06-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "LTC4421_controller.h"


esp_err_t init_24V(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_num_t output_pins[] = {
        GPIO_RETRY,
        GPIO_SHDN,
        GPIO_DISABLE_1,
        GPIO_DISABLE_2
    };

    for (int i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        io_conf.pin_bit_mask = 1ULL << output_pins[i];
        gpio_config(&io_conf);
        gpio_set_level(output_pins[i], 0);  // Optional: initialize LOW
    }

    // Configure input pins for status
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;  // Enable pull-up for input stability
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_num_t input_pins[] = {
        GPIO_CH1,
        GPIO_FAULT_1,
        GPIO_FAULT_2,
        GPIO_CH2
    };

    for (int i = 0; i < sizeof(input_pins)/sizeof(input_pins[0]); i++) {
        io_conf.pin_bit_mask = 1ULL << input_pins[i];
        gpio_config(&io_conf);
    }

    return ESP_OK;
}