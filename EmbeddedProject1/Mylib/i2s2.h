#ifndef __I2S2_H__
#define __I2S2_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "system_stm32f4xx.h"  

HAL_StatusTypeDef i2s2_init(void);

#ifdef __cplusplus
}
#endif

#endif


