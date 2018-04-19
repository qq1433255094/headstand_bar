#ifndef __CAN2_H__
#define __CAN2_H__

#ifdef __cplusplus
extern "C" {
#endif
	 
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "system_stm32f4xx.h"  

#define CAN2_FREQ 1000000
#define CAN2_Prescaler (SystemCoreClock / 4 / CAN2_FREQ)

HAL_StatusTypeDef can2_init(void);
HAL_StatusTypeDef can2_send(uint8_t *msg, uint8_t len, uint8_t id);

#ifdef __cplusplus
}
#endif

#endif