#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "system_stm32f4xx.h"  

#include "i2s2.h"

I2S_HandleTypeDef hi2s2;

HAL_StatusTypeDef i2s2_init()
{
	/*
    (#) Declare a I2S_HandleTypeDef handle structure.
    (#) Initialize the I2S low level resources by implement the HAL_I2S_MspInit() API:
        (##) Enable the SPIx interface clock.                      
        (##) I2S pins configuration:
            (+++) Enable the clock for the I2S GPIOs.
            (+++) Configure these I2S pins as alternate function pull-up.
  
   (#) Program the Mode, Standard, Data Format, MCLK Output, Audio frequency and Polarity
       using HAL_I2S_Init() function.
			 */
	
	GPIO_InitTypeDef GPIO_Init;
	HAL_StatusTypeDef state;
	
	__SPI2_CLK_ENABLE(); //(##) Enable the SPIx interface clock. 
												
  __GPIOB_CLK_ENABLE();//(##) I2S pins configuration:
	__GPIOC_CLK_ENABLE();
	
  GPIO_Init.Pin   = GPIO_PIN_10;
  GPIO_Init.Mode  = GPIO_MODE_AF_PP;
  GPIO_Init.Pull  = GPIO_PULLUP;
  GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Alternate =GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);		

	 GPIO_Init.Pin   = GPIO_PIN_3;
  GPIO_Init.Mode  = GPIO_MODE_AF_PP;
  GPIO_Init.Pull  = GPIO_PULLUP;
  GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Alternate =GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);	
//												| GPIO_PIN_3
	hi2s2.Instance	= SPI2;
	hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
	hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s2.Init.DataFormat = I2S_DATAFORMAT_32B;
	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
	hi2s2.Init.CPOL = I2S_CPOL_HIGH;
	hi2s2.Init.ClockSource=I2S_CLOCK_PLL;
	state=HAL_I2S_Init(&hi2s2);
	__HAL_I2S_ENABLE(&hi2s2);
	return state;
}




