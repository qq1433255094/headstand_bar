/** 
  ******************************************************************************
  * @file    
  * @author  
  * @date    
  * @brief   
  ******************************************************************************
**/
#ifndef __AS5600_H__
#define __AS5600_H__



#ifdef __cplusplus
extern "C" {
#endif
	 
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "system_stm32f4xx.h"  

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_SCL_HIGH  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)
#define I2C_SCL_LOW  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define I2C_SDA_HIGH  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
#define I2C_SDA_LOW   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
#define READ_SDA      HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2)
#define SDA_IN()  {GPIOC->MODER&=~(3<<(2*2));GPIOC->MODER|=0<<(2*2);}	//PB9输入模式
#define SDA_OUT() {GPIOC->MODER&=~(3<<(2*2));GPIOC->MODER|=1<<(2*2);} //PB9输出模式
#define abs(x)	(x<0?-x:x)
	
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private extern ---------------------------------------------------------*/
struct angle_as5600
{
	int angle;
	int angle_w;
	int angle_a;
};

typedef enum
{
	angle_To = 0,
	angle_w_To = 1,
	angle_a_To = 2
}anglechange;


uint8_t I2C_Readbyte(uint8_t ack);
void I2C_Sendbyte(uint8_t d);
void I2C_Nack(void);
void I2C_Ack(void);
uint8_t I2C_Waitack(void);
void I2C_Stop(void);
void I2C_start(void);
void AS5600_init(void);
uint8_t I2C_Read_byte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data);
uint8_t I2C_Write_byte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data);
void Get_angle(void);
void Get_angle_w(void);
void Get_angle_a(void);
double angle_Valuechange(anglechange value);


	
#ifdef __cplusplus
}
#endif
	
#endif


