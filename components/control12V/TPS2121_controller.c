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

// 12V Logic

//INCLUDES
#include "TPS2121_controller.h"
#include "driver/gpio.h"


esp_err_t init_12V(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << GPIO_SEL_12V_IN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
    return ESP_OK;
}
