#ifndef __USART3_H__
#define __USART3_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include <string.h>
#include <stdarg.h>

void usart3_init(void);
void usart3_send(const char *b);

	 
#ifdef __cplusplus
  }
#endif
	 
#endif
