#pragma once
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

#include "esp_err.h"
//12V CONTROL
#define GPIO_SEL_12V_IN    CONFIG_SEL_12V_IN_GPIO


//12V SENSE
#define GPIO_I_SENSE_12V   CONFIG_I_SENSE_12V_GPIO
#define GPIO_12V_SEN       CONFIG_12V_SEN_GPIO

esp_err_t init_12V(void);