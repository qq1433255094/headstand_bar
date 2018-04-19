#include "can1.h"

CAN_HandleTypeDef hcan1;

HAL_StatusTypeDef can1_init(void)
{
	HAL_StatusTypeDef state;
	
	__HAL_RCC_CAN1_CLK_ENABLE();
	
	return state;
}
