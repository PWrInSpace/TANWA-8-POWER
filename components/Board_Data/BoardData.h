#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "stdbool.h"
#include "mcu_adc_config.h"
#include "esp_err.h"

typedef struct {
    // Statusy cyfrowe
    bool ch1_status;
    bool fault_1_status;
    bool fault_2_status;
    bool ch2_status;

    // Pomiary napięć (Voltage Sense)
    float Voltage24V_in;
    float Voltage24VSOL_in;

    // Pomiary prądów (Isense)
    float Current5V_in;
    float Current24VSOL_in;

    // Opcjonalne: Całkowity pobór prądu przez płytkę
    float CurrentBoard;
} BoardData_t;

extern BoardData_t BoardData;
extern SemaphoreHandle_t BoardDataSemaphore;

esp_err_t tanwa_read_i_sense(float *i_sense);