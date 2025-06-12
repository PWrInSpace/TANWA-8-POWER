///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 10.06.2024 by Mateusz Kłosiński
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the CAN BUS task. This task is responsible for handling the
/// communication over the CAN BUS, it receives the data from the TWAI driver receive queue and
/// parses the commands.
///===-----------------------------------------------------------------------------------------===//
#ifndef PWRINSPACE_TANWA_CAN_TASK_H_
#define PWRINSPACE_TANWA_CAN_TASK_H_

#include <stdint.h>
#include "mcu_twai_config.h"
/**
 * @brief Function for starting the can bus task.
 */
void run_can_task(void);

/**
 * @brief Function for stopping the can bus task.
 */
void stop_can_task(void);

/**
 * @brief Task for receiving and parsing the CAN BUS messages.
 */
void can_task(void *pvParameters);

/**
 * @brief Function to send status via CAN
 */
void send_status(void);

/**
 * @brief Function that sends beacon do master
 * 
 */
void send_beacon_task(void *pvParameters);
#endif /* PWRINSPACE_TANWA_CAN_TASK_H_ */