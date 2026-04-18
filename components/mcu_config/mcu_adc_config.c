#include "mcu_adc_config.h"
#include "esp_log.h"

#define TAG "MCU_ADC"

mcu_adc_config_t mcu_adc_config = {
  .adc_cal = {
    [V_SENSE_24V_INDEX]    = 0.99587f, 
    [I_SENSE_5V_INDEX]     = 1.0f,
    [I_SENSE_24VSOL_INDEX] = 1.0f,
    [V_SENSE_24VSOL_INDEX] = 1.0f
  },
  .adc_chan = {
    [V_SENSE_24V_INDEX]    = ADC_CHANNEL_7, // GPIO 18
    [I_SENSE_5V_INDEX]     = ADC_CHANNEL_3, // GPIO 4
    [I_SENSE_24VSOL_INDEX] = ADC_CHANNEL_2, 
    [V_SENSE_24VSOL_INDEX] = ADC_CHANNEL_3
  },
  .adc_unit_map = {
    [V_SENSE_24V_INDEX]    = ADC_UNIT_2,    // GPIO 18 jest na ADC2
    [I_SENSE_5V_INDEX]     = ADC_UNIT_1,    // Reszta na ADC1
    [I_SENSE_24VSOL_INDEX] = ADC_UNIT_2,
    [V_SENSE_24VSOL_INDEX] = ADC_UNIT_2
  },
  .adc_chan_num = MAX_CHANNEL_INDEX,
  .oneshot_chan_cfg = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_12,
  },
  .unit1_handle = NULL,
  .unit2_handle = NULL,
  .unit1_cali_handle = NULL,
  .unit2_cali_handle = NULL,
  .unit1_calibrated = false,
  .unit2_calibrated = false
};


// Helper function to setup calibration
static bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated) {
        // Curve Fitting scheme is the best for ESP32-S3
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) { calibrated = true; }
    }

    *out_handle = handle;
    return calibrated;
}

esp_err_t mcu_adc_init() {
  // 1. Inicjalizacja Jednostki 1
  adc_oneshot_unit_init_cfg_t init_cfg1 = { .unit_id = ADC_UNIT_1 };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg1, &mcu_adc_config.unit1_handle));

  // 2. Inicjalizacja Jednostki 2 (dla GPIO 18)
  adc_oneshot_unit_init_cfg_t init_cfg2 = { .unit_id = ADC_UNIT_2 };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg2, &mcu_adc_config.unit2_handle));


  mcu_adc_config.unit1_calibrated = example_adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_12, &mcu_adc_config.unit1_cali_handle);
    mcu_adc_config.unit2_calibrated = example_adc_calibration_init(ADC_UNIT_2, ADC_ATTEN_DB_12, &mcu_adc_config.unit2_cali_handle);


  // 3. Konfiguracja kanałów
  for (uint8_t i = 0; i < mcu_adc_config.adc_chan_num; i++) {
    // Wybieramy właściwy uchwyt na podstawie mapy
    adc_oneshot_unit_handle_t current_handle = 
        (mcu_adc_config.adc_unit_map[i] == ADC_UNIT_1) ? mcu_adc_config.unit1_handle : mcu_adc_config.unit2_handle;

    ESP_ERROR_CHECK(adc_oneshot_config_channel(current_handle, 
                                               mcu_adc_config.adc_chan[i], 
                                               &mcu_adc_config.oneshot_chan_cfg));
  }
  return ESP_OK;
}

bool _mcu_adc_read_raw(uint8_t channel_index, uint16_t* adc_raw) {
  if (channel_index >= MAX_CHANNEL_INDEX) return false;

  int vRaw;
  // Wybieramy właściwy uchwyt przy odczycie
  adc_oneshot_unit_handle_t current_handle = 
      (mcu_adc_config.adc_unit_map[channel_index] == ADC_UNIT_1) ? mcu_adc_config.unit1_handle : mcu_adc_config.unit2_handle;

  if (adc_oneshot_read(current_handle, mcu_adc_config.adc_chan[channel_index], &vRaw) != ESP_OK) {
    return false;
  }
  *adc_raw = (uint16_t)vRaw;
  return true;
}

bool _mcu_adc_read_voltage(uint8_t channel_index, float* adc_voltage) {
    uint16_t vRaw;
    if (!_mcu_adc_read_raw(channel_index, &vRaw)) return false;

    int voltage_mv = 0;
    adc_unit_t unit = mcu_adc_config.adc_unit_map[channel_index];
    
    // Check if we have calibration for this unit
    if (unit == ADC_UNIT_1 && mcu_adc_config.unit1_calibrated) {
        adc_cali_raw_to_voltage(mcu_adc_config.unit1_cali_handle, vRaw, &voltage_mv);
    } else if (unit == ADC_UNIT_2 && mcu_adc_config.unit2_calibrated) {
        adc_cali_raw_to_voltage(mcu_adc_config.unit2_cali_handle, vRaw, &voltage_mv);
    } else {
        voltage_mv = (vRaw * 3100) / 4095; 
    }
    *adc_voltage = ((float)voltage_mv / 1000.0f) * mcu_adc_config.adc_cal[channel_index];
    
    return true;
}

// Funkcja _mcu_adc_read_voltage pozostaje bez zmian (korzysta z poprawionego _mcu_adc_read_raw)