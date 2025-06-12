/**
 * @file 24V_SOL_controller.h
 * @author Mateusz Kłosiński
 * @brief 
 * @version 0.1
 * @date 2025-06-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "24V_SOL_controller.h" 


esp_err_t init_24V_SOL(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << GPIO_SOL_EN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
    gpio_set_level(GPIO_SOL_EN, 1);  // or 1 depending on default state
    return ESP_OK;
}
