#include "usart3.h"


USART_HandleTypeDef husart3;
uint8_t receive3[10],transmit3[10];

/**
  * @brief 初始化串口3
  * @param 无
  * @note 无
  * @retval 无
  */
void usart3_init(void)
{
	GPIO_InitTypeDef gpio_init;
	
	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	gpio_init.Alternate=GPIO_AF7_USART3;
	gpio_init.Pin=GPIO_PIN_10|GPIO_PIN_11;
	gpio_init.Mode=GPIO_MODE_AF_PP;
	gpio_init.Speed=GPIO_SPEED_FREQ_HIGH;
	gpio_init.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB,&gpio_init);
	
	husart3.Init.BaudRate=115200;
	husart3.Init.WordLength=USART_WORDLENGTH_8B;
	husart3.Init.StopBits=USART_STOPBITS_1;
	husart3.Init.Parity=USART_PARITY_NONE;
	husart3.Init.Mode=USART_MODE_TX_RX;
	husart3.Instance=USART3;

	HAL_USART_Init(&husart3);
	
}

/**
  * @brief 串口3发送字符串
  * @param *b：字符串数组指针
  * @note 无
  * @retval 无
  */
void usart3_send(const char *b)
{
	char len=strlen(b);
	HAL_USART_Transmit(&husart3,(uint8_t *)b,len,50);
}

/*
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};

FILE __stdout;

_sys_exit(int x)
{
    x = x;
}
_ttywrch(int ch)
{
    ch = ch;
}

int fputc(int ch, FILE *f)
{
		HAL_USART_Transmit(&husart3,(uint8_t *)(&ch),1,50);
    return ch;
}
*/
void USART3_IRQHandler(void)
{
	
}


