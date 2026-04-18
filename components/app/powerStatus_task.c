#include "powerStatus_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "mcu_adc_config.h"
#include "BoardData.h"
#include <stdio.h>
#include "esp_timer.h"

#define TAG "POWER_TASK"

// Definicje pinów statusowych (zostawiam, żeby nie było błędów kompilacji)
#define GPIO_CH1         15
#define GPIO_CH2         16
#define GPIO_FAULT_1     17
#define GPIO_FAULT_2     18

#include <stdio.h>

// Kody ANSI dla formatowania terminala
#define ANSI_COLOR_CYAN    "\e[0;36m"
#define ANSI_COLOR_GREEN   "\e[0;32m"
#define ANSI_COLOR_YELLOW  "\e[0;33m"
#define ANSI_COLOR_RED     "\e[0;31m"
#define ANSI_COLOR_RESET   "\e[0m"
#define ANSI_BOLD          "\e[1m"
#define ANSI_CLEAR_SCREEN  "\e[2J\e[H"
void display_power_table(const BoardData_t *data) {
    if (data == NULL) return;

        // Czyścimy ekran i wracamy na górę
    printf(ANSI_CLEAR_SCREEN);

    // Nagłówek z aktualizacją na samej górze (ładniej wygląda)
    printf(ANSI_COLOR_CYAN " [ System Uptime: %8lld ms ]" ANSI_COLOR_RESET "\n", (long long)(esp_timer_get_time() / 1000));
    printf(ANSI_BOLD ANSI_COLOR_CYAN "       === TANWA-8-POWER SYSTEM MONITOR ===\n" ANSI_COLOR_RESET);
    
    // Tabela
    printf("----------------------------------------------------\n");
    printf("| %-18s | %-12s | %-10s |\n", "Parametr", "Wartość", "Status");
    printf("----------------------------------------------------\n");

    // Sekcja Napięć - %8.2f gwarantuje, że kropka zawsze będzie w tym samym miejscu
    printf("| %-18s | " ANSI_COLOR_YELLOW "%8.2f V" ANSI_COLOR_RESET " | %-10s |\n", 
           "24V-SYS", data->Voltage24V_in, "OK");
    printf("| %-18s | " ANSI_COLOR_YELLOW "%8.2f V" ANSI_COLOR_RESET " | %-10s |\n", 
           "24V-SOL", data->Voltage24VSOL_in, "OK");
   // --- Solenoid Current (24V-SOL) ---
    float sol_curr_ma = data->Current24VSOL_in * 1000.0f;
    char* sol_status;
    char* sol_color;

    if (sol_curr_ma > 5000.0f) {
        sol_status = "MALFUNCTION";
        sol_color = ANSI_COLOR_RED;
    } else if (sol_curr_ma > 10.0f) { // Jeśli prąd > 10mA, uznajemy że jest obciążenie
        sol_status = "LOAD_OK";
        sol_color = ANSI_COLOR_GREEN;
    } else {
        sol_status = "NO LOAD";
        sol_color = ANSI_COLOR_RESET; // Biały/domyślny dla braku obciążenia
    }

    printf("| %-18s | %8.1f mA | %s%-12s" ANSI_COLOR_RESET " |\n", 
           "24V-SOL Current", 
           sol_curr_ma, 
           sol_color, 
           sol_status);

    // --- Board Current (5V) ---
    float board_curr_ma = data->Current5V_in * 1000.0f;
    bool board_fail = (board_curr_ma > 250.0f);

    printf("| %-18s | %8.1f mA | %s%-12s" ANSI_COLOR_RESET " |\n", 
           "Board Current", 
           board_curr_ma, 
           board_fail ? ANSI_COLOR_RED : ANSI_COLOR_GREEN, 
           board_fail ? "MALFUNCTION" : "OK");
    printf("----------------------------------------------------\n");

    // Sekcja Cyfrowa
    const char* f1_txt = data->fault_1_status ? "DETECTED" : "BRAK";
    const char* f1_sta = data->fault_1_status ? "MALFUNCTION" : "OK";
    const char* f1_col = data->fault_1_status ? ANSI_COLOR_RED : ANSI_COLOR_GREEN;

    const char* f2_txt = data->fault_2_status ? "DETECTED" : "BRAK";
    const char* f2_sta = data->fault_2_status ? "MALFUNCTION" : "OK";
    const char* f2_col = data->fault_2_status ? ANSI_COLOR_RED : ANSI_COLOR_GREEN;

    printf("| %-18s | %-12s | %s%-10s" ANSI_COLOR_RESET " |\n", "24V-SYS Error", f1_txt, f1_col, f1_sta);
    printf("| %-18s | %-12s | %s%-10s" ANSI_COLOR_RESET " |\n", "24V-SOL Error", f2_txt, f2_col, f2_sta);

    printf("----------------------------------------------------\n");
    
    // Status wyjść pod tabelą
    printf(" Wyjscia:  CH1: [%s]    CH2: [%s]\n", 
           data->ch1_status ? ANSI_COLOR_GREEN " ON " ANSI_COLOR_RESET : ANSI_COLOR_RED " OFF ",
           data->ch2_status ? ANSI_COLOR_GREEN " ON " ANSI_COLOR_RESET : ANSI_COLOR_RED " OFF ");
    printf("----------------------------------------------------\n");
}

void convert_from_voltage_divider(float voltage_in, float *voltage_out , float multiplier) {
    *voltage_out = voltage_in * multiplier;
}

void power_mng_task(void *pvParameters)
{

    if (BoardDataSemaphore == NULL) {
        BoardDataSemaphore = xSemaphoreCreateMutex();
        xSemaphoreGive(BoardDataSemaphore);
    }

    // Inicjalizacja ADC
    if (mcu_adc_init() != ESP_OK) {
        ESP_LOGE(TAG, "MCU ADC initialization failed!");
        vTaskDelete(NULL);
        return;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_CH1) | (1ULL << GPIO_FAULT_1) | (1ULL << GPIO_FAULT_2) | (1ULL << GPIO_CH2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // Zazwyczaj dla Fault/Status lepiej dać Pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Power Management Task started - Monitoring 5V Isense (GPIO4)");

   while (1) {
    if (xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        BoardData.ch1_status     = (gpio_get_level(GPIO_CH1) != 0);
        BoardData.ch2_status     = (gpio_get_level(GPIO_CH2) != 0);
        BoardData.fault_1_status = (gpio_get_level(GPIO_FAULT_1) != 0);
        BoardData.fault_2_status = (gpio_get_level(GPIO_FAULT_2) != 0);
float i_sense_5V = 0.0f, i_sense_24V_SOL = 0.0f, voltage_24V = 0.0f, voltage_24V_SOL = 0.0f;
        _mcu_adc_read_voltage(I_SENSE_5V_INDEX,     &i_sense_5V);
        _mcu_adc_read_voltage(V_SENSE_24V_INDEX,    &voltage_24V);
        _mcu_adc_read_voltage(V_SENSE_24VSOL_INDEX, &voltage_24V_SOL);
        _mcu_adc_read_voltage(I_SENSE_24VSOL_INDEX, &i_sense_24V_SOL);
        convert_from_voltage_divider(voltage_24V, &BoardData.Voltage24V_in, 10.68f);
        convert_from_voltage_divider(i_sense_5V, &BoardData.Current5V_in, 1.0f);
        convert_from_voltage_divider(voltage_24V_SOL, &BoardData.Voltage24VSOL_in, 10.68f);
        convert_from_voltage_divider(i_sense_24V_SOL, &BoardData.Current24VSOL_in, 1.0f);
        
        display_power_table(&BoardData);
    
        xSemaphoreGive(BoardDataSemaphore);
    } else {
        ESP_LOGW(TAG, "Problem z dostępem do danych (Semaphore Timeout)");
    }

    vTaskDelay(pdMS_TO_TICKS(500)); 
}
}

esp_err_t power_mng_task_init(void) {
    BaseType_t result = xTaskCreate(power_mng_task, "power_mng_task", 4096, NULL, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Power Management Task");
        return ESP_FAIL;
    }
    return ESP_OK;
}