#include "tim4.h"
#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim4;
TIM_OC_InitTypeDef tim4_oc;

void tim4_init(void)
{
  uint16_t prescalervalue = 0;
  uint32_t tmpvalue = 0;

  /* TIM4 clock enable */
  __HAL_RCC_TIM4_CLK_ENABLE();

  /* Enable the TIM4 global Interrupt */
  HAL_NVIC_SetPriority(TIM4_IRQn, 6, 0);  
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  
  /* -----------------------------------------------------------------------
  TIM4 Configuration: Output Compare Timing Mode:  
    To get TIM4 counter clock at 550 KHz, the prescaler is computed as follows:
    Prescaler = (TIM4CLK / TIM4 counter clock) - 1
    Prescaler = ((f(APB1) * 2) /550 KHz) - 1
  
    CC update rate = TIM4 counter clock / CCR_Val = 32.687 Hz
    ==> Toggling frequency = 16.343 Hz  
  ----------------------------------------------------------------------- */
  
  /* Compute the prescaler value */
  tmpvalue = HAL_RCC_GetPCLK1Freq();
  prescalervalue = (uint16_t) ((tmpvalue * 2) / 4000000) - 1;
  
  /* Time base configuration */
  htim4.Instance = TIM4;
  htim4.Init.Period = 1000;
  htim4.Init.Prescaler = prescalervalue;
  htim4.Init.ClockDivision = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    /* Initialization Error */
 //   Error_Handler();
  }
  
  /* Output Compare Timing Mode configuration: Channel1 */
  tim4_oc.OCMode = TIM_OCMODE_TIMING;
  tim4_oc.OCIdleState = TIM_OCIDLESTATE_SET;
  tim4_oc.Pulse = 16826;
  tim4_oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  tim4_oc.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  tim4_oc.OCFastMode = TIM_OCFAST_ENABLE;
  tim4_oc.OCNIdleState = TIM_OCNIDLESTATE_SET;
  
  /* Initialize the TIM4 Channel1 with the structure above */
  if(HAL_TIM_OC_ConfigChannel(&htim4, &tim4_oc, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
  //  Error_Handler();
  }

  /* Start the Output Compare */
  if(HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Start Error */
 //   Error_Handler();
  }
}

void TIM4_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim4);
}


uint8_t tim4_cnt=0,tim4_freq=LED_1HZ;
DEF_SPEED = 0;
DEF_POSITION = 0;
DEF_ACCLE = 0;
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (SERVO_SPEED > 240)
		SERVO_SPEED = 240;
	else if (SERVO_SPEED < -240)
		SERVO_SPEED = -240;
	
	if ((int)SERVO_SPEED < 0)
	{	
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
		htim->Instance->ARR = 60000 / (-SERVO_SPEED);
		GPIOD->ODR ^= GPIO_PIN_14;	
		SERVO_POSITION--;
		
	}
	else if((int)SERVO_SPEED > 0)
	{	
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		
		htim->Instance->ARR = 60000 / SERVO_SPEED;
		GPIOD->ODR ^= GPIO_PIN_14;
		SERVO_POSITION++;
	}
	
}
