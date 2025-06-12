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

 //INCLUDES
#include "esp_err.h"
#include "driver/gpio.h"

// 24V STATUS
#define GPIO_CH1           CONFIG_CH1_STATUS
#define GPIO_FAULT_1       CONFIG_FAULT_1_STATUS
#define GPIO_FAULT_2       CONFIG_FAULT_2_STATUS
#define GPIO_CH2           CONFIG_CH2_STATUS

// 24V CONTROL
#define GPIO_RETRY         CONFIG_RETRY_GPIO
#define GPIO_SHDN          CONFIG_SHDN_GPIO
#define GPIO_DISABLE_1     CONFIG_DISABLE_1_GPIO
#define GPIO_DISABLE_2     CONFIG_DISABLE_2_GPIO

// 24V SENSE
#define GPIO_I_SENSE_24V   CONFIG_I_SENSE_24V_GPIO
#define GPIO_24V_SEN       CONFIG_24V_SEN_GPIO

esp_err_t init_24V(void);