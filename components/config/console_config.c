///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//

#include "esp_log.h"
#include "esp_system.h"

#include "console.h"
#include "console_config.h"

#define TAG "CONSOLE_CONFIG"


// example function to reset the device
int reset_device(int argc, char **argv) {
    ESP_LOGI(TAG, "Resetting device...");
    esp_restart();
    return 0;
}
static int read_temperature(int argc, char **argv) {
    float temp[2] = {50, 100};
    int raw[2] = {1000,2000};
    uint8_t ret = 0;
/**
    ret = tmp1075_get_temp_raw(&(TANWA_hardware.tmp1075[0]), &raw[0]);
    if (ret != TMP1075_OK) {
        ESP_LOGE(TAG, "TMP1075 #1 read raw failed - status: %d", ret);
        return -1;
    }
    ret = tmp1075_get_temp_celsius(&(TANWA_hardware.tmp1075[0]), &temp[0]);
    if (ret != TMP1075_OK) {
        ESP_LOGE(TAG, "TMP1075 #1 read temp failed - status: %d", ret);
        return -1;
    }
    ret = tmp1075_get_temp_raw(&(TANWA_hardware.tmp1075[1]), &raw[1]);
    if (ret != TMP1075_OK) {
        ESP_LOGE(TAG, "TMP1075 #2 read raw failed - status: %d", ret);
        return -1;
    }
    ret = tmp1075_get_temp_celsius(&(TANWA_hardware.tmp1075[1]), &temp[1]);
    if (ret != TMP1075_OK) {
        ESP_LOGE(TAG, "TMP1075 #2 read temp failed - status: %d", ret);
        return -1;
    }
        */
    
    CONSOLE_WRITE("TMP1075 Temperature:");
    CONSOLE_WRITE("#1 => raw: %d, temp: %f", raw[0], temp[0]);
    CONSOLE_WRITE("#2 => raw: %d, temp: %f", raw[1], temp[1]);

    return 0;
}

static int read_pwr_data(int argc, char **arg)
{
    CONSOLE_WRITE("PWR DATA :");
    CONSOLE_WRITE("##########  24V  ############");
    CONSOLE_WRITE("V_in => %f, I_in: %f", 55, 55);
    CONSOLE_WRITE("##########  12V  ############");
    CONSOLE_WRITE("V_in => %f, I_in: %f", 55, 55);
    return 0;
}

 // Place for the console configuration

static esp_console_cmd_t cmd[] = {
    // system commands
    {"reset-dev", "restart device", NULL, reset_device, NULL},
    {"temp-read", "read temperature", NULL, read_temperature, NULL},
    {"pwr-data", "read pwr data", NULL, read_pwr_data, NULL},
};

esp_err_t console_config_init() {
    esp_err_t ret;
    ret = console_init();
    ret = console_register_commands(cmd, sizeof(cmd) / sizeof(cmd[0]));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(ret));
        return ret;
    }
    return ret;
}