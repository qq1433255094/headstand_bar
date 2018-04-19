#include "can2.h"

CAN_HandleTypeDef hcan2;

HAL_StatusTypeDef can2_init(void)
{
	HAL_StatusTypeDef state;
	GPIO_InitTypeDef gpio_init;
	CAN_FilterConfTypeDef can2_filter;
	
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	gpio_init.Pin = GPIO_PIN_12 | GPIO_PIN_13;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	gpio_init.Alternate = GPIO_AF9_CAN2;
	HAL_GPIO_Init(GPIOB, &gpio_init);
	
	hcan2.Instance = CAN2;
	hcan2.Init.TTCM = DISABLE;
	hcan2.Init.ABOM = DISABLE;
	hcan2.Init.AWUM = DISABLE;
	hcan2.Init.NART = DISABLE;
	hcan2.Init.RFLM = DISABLE;
	hcan2.Init.TXFP = ENABLE;
	hcan2.Init.Mode = CAN_MODE_NORMAL;
	hcan2.Init.SJW = CAN_SJW_1TQ;
	hcan2.Init.BS1 = CAN_BS1_9TQ;
	hcan2.Init.BS2 = CAN_BS2_4TQ;
	hcan2.Init.Prescaler = CAN2_Prescaler / (1 + 9 + 4);
	
	//HAL_CAN_DeInit(&hcan2);
	state = HAL_CAN_Init(&hcan2);
	
	can2_filter.FilterNumber = 14;
	can2_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	can2_filter.FilterScale = CAN_FILTERSCALE_32BIT;
	can2_filter.FilterIdHigh = 0x0000;
	can2_filter.FilterIdLow = 0x0000;
	can2_filter.FilterMaskIdHigh = 0x0000;
	can2_filter.FilterMaskIdLow = 0x0000;
	can2_filter.FilterFIFOAssignment = 0;
	can2_filter.FilterActivation = ENABLE;
	
	HAL_CAN_ConfigFilter(&hcan2, &can2_filter);
	
	HAL_CAN_Receive_IT(&hcan2, CAN_FIFO0);
	return state;
}

HAL_StatusTypeDef can2_send(uint8_t *msg, uint8_t len, uint8_t id)
{	
	uint8_t i = 0;
	HAL_StatusTypeDef state;
	
	hcan2.pTxMsg->StdId = (id << 3);
	hcan2.pTxMsg->ExtId = 0;
	hcan2.pTxMsg->IDE = CAN_ID_STD;
	hcan2.pTxMsg->RTR = CAN_RTR_DATA;
	hcan2.pTxMsg->DLC = len;
	
	for (i = 0; i < len; i++)
	{
		hcan2.pTxMsg->Data[i] = msg[i];
	}
	
	state = HAL_CAN_Transmit(&hcan2, 50);
	return state;
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	//hcan->pRxMsg;
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
}
