#ifndef __CAN1_H__
#define __CAN1_H__

#ifdef __cplusplus
extern "C" {
#endif
	 
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "system_stm32f4xx.h"  

HAL_StatusTypeDef can1_init(void);

#ifdef __cplusplus
}
#endif

#endif