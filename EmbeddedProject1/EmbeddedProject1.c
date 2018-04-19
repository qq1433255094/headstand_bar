/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "tim3.h"
#include "tim4.h"
#include "usart2.h"
#include "spi1.h"
#include <stdio.h>
#include <stdlib.h>
#include "button.h"
#include "iwdg.h"
#include "oled.h"
#include "pwm.h"
#include "can2.h"
#include "AS5600.h"
#include "math.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId LEDThread1Handle, LEDThread2Handle, LEDThread3Handle;

/* Private function prototypes -----------------------------------------------*/
static void LED_Thread1(void const *argument);
static void LED_Thread2(void const *argument);
static void LED_Thread3(void const *argument);
void setSystemClock(void);

extern DEF_SPEED;
extern DEF_POSITION;
extern DEF_ACCLE;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	extern struct angle_as5600 Get_ang;
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	
	setSystemClock();
	SystemCoreClockUpdate();
	HAL_Init();  
	
	iwdg_init();
	
	usart2_init();
	button_init();
	tim3_init();
	tim4_init();
	AS5600_init();
	OLED_Configuration();
	
	OLED_Printf(0, 0, "abc");

	
#pragma region GPIOD init
	__GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
#pragma endregion

	

	/* Thread 1 definition */
	osThreadDef(LED1, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  
	 /*  Thread 2 definition */
	osThreadDef(LED2, LED_Thread2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	 /*  Thread 3 definition */
	osThreadDef(LED3, LED_Thread3, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	/* Start thread 1 */
	LEDThread1Handle = osThreadCreate(osThread(LED1), NULL);
  
	/* Start thread 2 */
	LEDThread2Handle = osThreadCreate(osThread(LED2), NULL);

	/* Start thread 3 */
	LEDThread3Handle = osThreadCreate(osThread(LED3), NULL);
	
	
  
	/* Start scheduler */
	osKernelStart();

	  /* We should never get here as control is now taken by the scheduler */
	for (;;)
		;
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	osSystickHandler();
}

/**
  * @brief  Toggle LED1
  * @param  thread not used
  * @retval None
  */
static void LED_Thread1(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);

		osDelay(2000);
		
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		//osThreadSuspend(LEDThread2Handle);
		
		osDelay(2000);
		
		
		
		//osThreadResume(LEDThread2Handle);
	}
}

/**
  * @brief  Toggle LED2 thread
  * @param  argument not used
  * @retval None
  */
static void LED_Thread2(void const *argument)
{
	uint32_t count;
	(void) argument;
	extern struct angle_as5600 Get_ang;
	
	for (;;)
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		reload_iwdg();
		osDelay(200);
		//OLED_Printf(0, 0, "angle=%d", Get_ang.angle);
		//OLED_Printf(0, 2, "speed=%d", Get_ang.angle_w);
	}
}

/**
  * @brief  Toggle LED3
  * @param  thread not used
  * @retval None
  */
static void LED_Thread3(void const *argument)
{
	(void) argument;
	extern button_type button_state;
	extern struct angle_as5600 Get_ang;
	extern short angle_set;
	char str[20] = { 1, 2, 3, 4, 0 };
	int angle = 0;
	int inc = 0;
	int speed=0;
	int rev_data = 0;
	char inc_enable = 0;
	char rev_dis = 0;
	char TheOne = 0;
	
	for (;;)
	{
//		uint8_t addr = 0x80 | 0x00 | 0x0f;
//		Si4332_CS_LOW();
//		SPI1_WriteRead(addr);
//		addr = 0;
//		data = SPI1_WriteRead(addr);
//		Si4332_CS_HIGH();
		Get_angle();
		Get_angle_w();
		
		speed=Get_ang.angle_w;
		angle = 2048 - Get_ang.angle;
		if (inc_enable)
			inc += angle;
		else
			inc = 0;
		usart2_ptintf("data=", angle, 0);
		
		
		
		//SERVO_SPEED = (2048 - Get_ang.angle) / 120;
		//SERVO_SPEED = (2048 - Get_ang.angle) / 60;
		//SERVO_ACCLE = -11*(sin((2048 - Get_ang.angle) / 13.378 / 180 * 3.1415) / cos((2048 - Get_ang.angle) / 13.378 / 180 * 3.1415));

//		if (rev_dis == 0)
//		{
//			SERVO_POSITION+=10;
//			rev_data+=10;
//			if (rev_data > 6400)
//				rev_dis = 1;
//		}
//		else if(rev_dis == 1)
//		{
//			SERVO_POSITION-=10;
//			rev_data-=10;
//			if (rev_data < 0)
//				rev_dis = 0;
//		}
		
		
		angle_set = SERVO_SPEED *1.2+SERVO_POSITION*0.08;
//		
		if (angle_set > 90)
			angle_set = 90;
		else if (angle_set < -90)
			angle_set = -90;
		
		if (abs(angle) < 400)
		{
			SERVO_SPEED = angle * -0.2 + inc * -0.035;//+ speed * -1;
			TheOne = 1;
		}
		else
		{
			angle_set = 0;
			inc_enable = 0;
			if (TheOne)
				SERVO_SPEED = angle / 60 - speed * 0.7;// - speed * 0.8;
			else
				SERVO_SPEED = angle / 80;
		}
		
		
		
		if (abs(angle) < 100)
			inc_enable = 1;
		
			
		
//		if (abs(data) < 50)
//			SERVO_SPEED = -(data) / 3 ;// *Get_ang.angle_w/10;
//		else if (abs(data) < 100)
//			SERVO_SPEED = -(data) / 2 ;
//		else if (abs(data) < 140)
//			SERVO_SPEED = -(data) / 1.5 ;
//		else if (abs(data) < 180)
//			SERVO_SPEED = -(data) / 1.5 ;
//		else if (abs(data) < 400)
//			SERVO_SPEED = -(data) / 1.5 ;
//		else
//			
//			SERVO_SPEED =0;
		
		if (abs(SERVO_POSITION) > 5000)
			SERVO_SPEED = 0;
		
//		can2_send((uint8_t *)str, 2, 11);
//		can2_send((uint8_t *)str, 2, 12);
//		can2_send((uint8_t *)str, 2, 13);
//		can2_send((uint8_t *)str, 2, 14);
		//_printf("data=%d\n", data);
		//data++;
		
		
		osDelay(5);
	}
}

void setSystemClock(void)
{
/******************************************************************************/
/*            PLL (clocked by HSE) used as System clock source                */
/*            System clock source = HSE / PLL_M *  PLL_N / PLL_P              */
/******************************************************************************/
	
	#ifndef HSE_STARTUP_TIMEOUT
		uint32_t HSE_STARTUP_TIMEOUT = 1000;
	#endif // !HSE_STARTUP_TIMEOUT

	__IO uint32_t StartUpCounter = 0, HSEStatus = 0;
	uint16_t	PLL_M = 8,
				PLL_N = 336,
				PLL_P = 2,
				PLL_Q = 7,
				PLLI2S_N = 302,
				PLLI2S_R = 2;
	
	/* Enable HSE */
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);
 
    /* Wait till HSE is ready and if Time out is reached exit */
	do
	{
		HSEStatus = RCC->CR & RCC_CR_HSERDY;
		StartUpCounter++;
	} while (HSEStatus == 0);// && (StartUpCounter != HSE_STARTUP_TIMEOUT));

	if ((RCC->CR & RCC_CR_HSERDY) != RESET)
	{
		HSEStatus = (uint32_t)0x01;
	}
	else
	{
		HSEStatus = (uint32_t)0x00;
	}

	if (HSEStatus == (uint32_t)0x01)
	{
	  /* Select regulator voltage output Scale 1 mode, System frequency up to 168 MHz */
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;

		    /* HCLK = SYSCLK / 1*/
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
      
		/* PCLK2 = HCLK / 2*/
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
    
		/* PCLK1 = HCLK / 4*/
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

		    /* Configure the main PLL */
		RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) - 1) << 16) |
		               (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
		
		RCC->PLLI2SCFGR = (PLLI2S_R << 28) + (PLLI2S_N << 6);

    /* Enable the main PLL and PLLI2S*/
		RCC->CR |= RCC_CR_PLLON | RCC_CR_PLLI2SON;

		    /* Wait till the main PLL is ready */
		while ((RCC->CR & RCC_CR_PLLRDY) == 0)
		{
		}
   
	    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
		FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;

		    /* Select the main PLL as system clock source */
		RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		RCC->CFGR |= RCC_CFGR_SW_PLL;

		    /* Wait till the main PLL is used as system clock source */
		while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
			;
		{
		}
	}
	else
	{ /* If HSE fails to start-up, the application will have wrong clock
	       configuration. User can add here some code to deal with this error */
	}

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
	while (1)
	{
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
