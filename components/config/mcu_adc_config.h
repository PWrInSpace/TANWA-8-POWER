#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define tag1 "ADC INIT"


// Global ADC handle for oneshot mode
static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;

// Function to map GPIO to ADC channel and unit
static adc_channel_t gpio_to_adc_channel(int gpio, adc_unit_t *unit) {
    if (gpio >= 1 && gpio <= 10) {
        *unit = ADC_UNIT_1;
        return (adc_channel_t)(gpio - 1); // GPIO1 -> ADC1_CHANNEL_0, etc.
    } else if (gpio >= 11 && gpio <= 20) {
        *unit = ADC_UNIT_2;
        return (adc_channel_t)(gpio - 11); // GPIO11 -> ADC2_CHANNEL_0, etc.
    } else {
        ESP_LOGE(tag1, "GPIO %d is not a valid ADC pin", gpio);
        return -1;
    }
}

// Initialize ADC unit and calibration
static bool adc_init(void) {
    // Initialize ADC unit (try ADC1 first, fall back to ADC2 if needed)
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(tag1, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
        return false;
    }
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));

    // Configure ADC calibration
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12, // 0–3.3V range
        .bitwidth = ADC_BITWIDTH_12,
    };

    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(tag1, "Failed to initialize ADC calibration: %s", esp_err_to_name(ret));
        adc_oneshot_del_unit(adc_handle);
        return false;
    }

    return true;
}

// Read raw ADC value for a given channel
static bool _mcu_adc_read_raw(adc_channel_t channel, uint16_t *raw) {
    if (!adc_handle) {
        ESP_LOGE(tag1, "ADC not initialized");
        return false;
    }

    // Configure channel if not already configured
    static adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12, // 0–3.3V range
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_oneshot_config_channel(adc_handle, channel, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(tag1, "Failed to configure ADC channel %d: %s", channel, esp_err_to_name(ret));
        return false;
    }

    // Read raw value
    ret = adc_oneshot_read(adc_handle, channel, (int *)raw);
    if (ret != ESP_OK) {
        ESP_LOGE(tag1, "Failed to read ADC channel %d: %s", channel, esp_err_to_name(ret));
        return false;
    }

    return true;
}

// Read ADC voltag1e for a given channel (returns voltag1e in volts)
bool _mcu_adc_read_voltage(uint8_t channel, float *adc_voltag1e) {
    uint16_t vRaw;
    if (!_mcu_adc_read_raw(channel, &vRaw)) {
        return false;
    }

    // Convert raw value to voltag1e using calibration
    int voltag1e_mv;
    esp_err_t ret = adc_cali_raw_to_voltage(adc_cali_handle, vRaw, &voltag1e_mv);
    if (ret != ESP_OK) {
        ESP_LOGE(tag1, "Failed to convert raw ADC to voltag1e: %s", esp_err_to_name(ret));
        return false;
    }

    *adc_voltag1e = voltag1e_mv / 1000.0f; // Convert mV to V
    return true;
}