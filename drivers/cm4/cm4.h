  
#ifndef __MAIN_CM4_H__
#define __MAIN_CM4_H__

#include "stm32f4xx_hal.h"

extern CAN_HandleTypeDef hcan1;
extern TIM_HandleTypeDef htim3;

// Add these 2 pointers, then user only need update this file when use other can/timer 
CAN_HandleTypeDef * CanPtr   = &hcan1;
TIM_HandleTypeDef * TimerPtr = &htim3;

#endif 

