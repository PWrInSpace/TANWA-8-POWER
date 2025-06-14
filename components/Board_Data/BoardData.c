#include "BoardData.h"

BoardData_t BoardData = {
    .ch1_status = false,
    .fault_1_status = false,
    .fault_2_status = false,
    .ch2_status = false,
    .Voltage24V_in = 0.0f,
    .Current24V_in = 0.0f,
    .Voltage12V_in = 0.0f,
    .Current12V_in = 0.0f,
    .CurrentBoard = 0.0f
};