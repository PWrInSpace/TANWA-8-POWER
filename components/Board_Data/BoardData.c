#include "BoardData.h"
#include "esp_log.h"

#define TAG "BOARD_DATA"

// Inicjalizacja struktury BoardData
BoardData_t BoardData = {
    .ch1_status = false,
    .ch2_status = false,
    .fault_1_status = false,
    .fault_2_status = false,
    .Voltage24V_in = 0.0f,
    .Voltage24VSOL_in = 0.0f,
    .Current5V_in = 0.0f,
    .Current24VSOL_in = 0.0f
};

SemaphoreHandle_t BoardDataSemaphore = NULL;

esp_err_t tanwa_read_i_sense(float *i_sense) {
    if (i_sense == NULL) {
        ESP_LOGE(TAG, "i_sense pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    // Przykład: zwracamy prąd 5V jako główny i_sense
    *i_sense = BoardData.Current5V_in;
    return ESP_OK;
}