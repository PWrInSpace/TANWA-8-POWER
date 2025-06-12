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

//INCLUDES
#include "driver/gpio.h"
#include "esp_err.h"
//24V CTRL SOL
#define GPIO_SOL_EN        CONFIG_SOL_EN_GPIO

esp_err_t init_24V_SOL(void);