#ifndef __TIM4_H_
#define __TIM4_H_

#ifdef __cplusplus
 extern "C" {
#endif
	 
#define LED_1HZ  100
#define LED_5HZ  20
#define LED_10HZ 10
	 
#define DEF_SPEED int SERVO_SPEED
#define DEF_POSITION int SERVO_POSITION
#define DEF_ACCLE int SERVO_ACCLE


void tim4_init(void);

#ifdef __cplusplus
}
#endif

#endif




