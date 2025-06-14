#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "stdbool.h"

typedef struct {
    bool ch1_status;
    bool fault_1_status;
    bool fault_2_status;
    bool ch2_status;
    float Voltage24V_in;
    float Current24V_in;
    float Voltage12V_in;
    float Current12V_in;
    float CurrentBoard;
} BoardData_t;

extern BoardData_t BoardData;
extern SemaphoreHandle_t BoardDataSemaphore;