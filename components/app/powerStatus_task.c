#include "powerStatus_task.h"
#include "TPS2121_controller.h"
#include "LTC4421_controller.h"
#include "24V_SOL_controller.h"


void power_mng_task(void * pvParameters)
{
    while (1) {
        // Read 24V STATUS
        bool ch1_status = gpio_get_level(GPIO_CH1);
        bool fault_1_status = gpio_get_level(GPIO_FAULT_1);
        bool fault_2_status = gpio_get_level(GPIO_FAULT_2);
        bool ch2_status = gpio_get_level(GPIO_CH2);

        //READ 12V STATUS
    }
}


