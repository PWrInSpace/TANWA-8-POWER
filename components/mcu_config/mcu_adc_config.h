///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 28.01.2024 by Michał Kos
/// Updated: 2024 for TANWA-8-POWER
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains the configuration of the ADC peripheral for the MCU.
/// This can only be used for ADC1!
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_MCU_ADC_CONFIG_H_
#define PWRINSPACE_MCU_ADC_CONFIG_H_
#include <stdbool.h>
#include <stdint.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "soc/adc_channel.h"

#define READ_ERROR_RETURN_VAL 0xFFFF
#define VOLTAGE_READ_ERROR_RETURN_VAL -1.0f
#define MAX_ADC_CHANNELS 8

typedef enum {
  V_SENSE_24V_CHANNEL    = ADC_CHANNEL_7, // Zmień na właściwy kanał ADC1 (np. GPIO 1)
  I_SENSE_5V_CHANNEL     = ADC_CHANNEL_3, // GPIO 4 (Zgodnie z Twoim opisem)
  I_SENSE_24VSOL_CHANNEL = ADC_CHANNEL_2, // Przykładowy kanał na ADC1
  V_SENSE_24VSOL_CHANNEL = ADC_CHANNEL_3, // Przykładowy kanał na ADC1
} mcu_adc_chan_cfg_t;

typedef enum {
  V_SENSE_24V_INDEX = 0,
  I_SENSE_5V_INDEX,
  I_SENSE_24VSOL_INDEX,
  V_SENSE_24VSOL_INDEX,
  MAX_CHANNEL_INDEX
} mcu_adc_chan_index_cfg_t;
/*!
 * \brief Voltage measure struct
 */
typedef struct {
  float adc_cal[MAX_CHANNEL_INDEX];
  uint8_t adc_chan[MAX_CHANNEL_INDEX];
  adc_unit_t adc_unit_map[MAX_CHANNEL_INDEX]; // Mapowanie: który indeks to która jednostka
  uint8_t adc_chan_num;
  
  // Dwa uchwyty - po jednym dla każdej jednostki
  adc_oneshot_unit_handle_t unit1_handle;
  adc_oneshot_unit_handle_t unit2_handle;
  
  adc_cali_handle_t unit1_cali_handle;
  adc_cali_handle_t unit2_cali_handle;
  bool unit1_calibrated;
  bool unit2_calibrated;

  adc_oneshot_chan_cfg_t oneshot_chan_cfg;
} mcu_adc_config_t;
extern mcu_adc_config_t mcu_adc_config;

/*!
  \brief Init for a voltage measure.
*/
esp_err_t mcu_adc_init(void);

/*!
 * \brief Read raw value from ADC
 */
bool _mcu_adc_read_raw(uint8_t adc_chan, uint16_t* adc_raw);

/*!
 * \brief Read voltage from ADC
 */
bool _mcu_adc_read_voltage(uint8_t adc_chan, float* adc_voltage);

#endif /* PWRINSPACE_MCU_ADC_CONFIG_H_ */