#pragma once
#include "esp_err.h"

void power_mng_task(void * pvParameters);
esp_err_t power_mng_task_init(void);

