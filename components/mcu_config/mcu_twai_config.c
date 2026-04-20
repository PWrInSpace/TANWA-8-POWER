// Copyright 2023 PWr in Space, Krzysztof Gliwiński

#include "mcu_twai_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define TAG "MCU_TWAI"

mcu_twai_config_t mcu_twai_config = {
    .tx_gpio_num = CONFIG_CAN_RX_GPIO,
    .rx_gpio_num = CONFIG_CAN_TX_GPIO,
    .mode = TWAI_MODE_NORMAL,
    .g_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = CONFIG_CAN_TX_GPIO,
        .rx_io = CONFIG_CAN_RX_GPIO,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 100,
        .rx_queue_len = 100,
        .alerts_enabled = TWAI_ALERT_BUS_OFF | 
                                          TWAI_ALERT_BUS_RECOVERED | 
                                          TWAI_ALERT_ERR_PASS | 
                                          TWAI_ALERT_BUS_ERROR |
                                          TWAI_ALERT_RX_DATA,
        .clkout_divider = 0,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    },
    .t_config = TWAI_TIMING_CONFIG_500KBITS(),
    .f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(),
};
esp_err_t mcu_twai_init() {
    esp_err_t err;
    err = twai_driver_install(&(mcu_twai_config.g_config), 
                              &(mcu_twai_config.t_config),
                              &(mcu_twai_config.f_config));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "TWAI driver install error");
      return err;
    }
    
    return ESP_OK;
}

esp_err_t mcu_twai_deinit() {
    esp_err_t err;
    err = twai_driver_uninstall();
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "TWAI driver uninstall error");
      return err;
    }
    
    return ESP_OK;
}

