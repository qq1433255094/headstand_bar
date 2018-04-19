/** 
  ******************************************************************************
  * @file    
  * @author  
  * @date    
  * @brief   
  ******************************************************************************
**/




#include "AS5600.h"
#include "oled.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private extern ---------------------------------------------------------*/



void AS5600_init(void)
{
	GPIO_InitTypeDef gpio;
	
	__GPIOC_CLK_ENABLE();
	
	gpio.Pin = GPIO_PIN_2 | GPIO_PIN_1;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FAST;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &gpio);
	
	SDA_OUT();
	I2C_SCL_HIGH;
	I2C_SDA_HIGH;
}

void I2C_start(void)
{
	SDA_OUT();  
	I2C_SDA_HIGH;	  	  
	I2C_SCL_HIGH;
	oled_delay_us(4);
 	I2C_SDA_LOW;                         //START:when CLK is high,DATA change form high to low 
	oled_delay_us(4);
	I2C_SCL_LOW;                         //钳住I2C总线，准备发送或接收数据 
}

void I2C_Stop(void)
{
	SDA_OUT();                          //sda线输出
	I2C_SCL_LOW;
	I2C_SDA_LOW;                        //STOP:when CLK is high DATA change form low to high
	oled_delay_us(4);
	I2C_SCL_HIGH; 
	I2C_SDA_HIGH;                       //发送I2C总线结束信号
	oled_delay_us(4);	
}

uint8_t I2C_Waitack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	I2C_SDA_HIGH;oled_delay_us(1);	   
//	I2C_SCL_HIGH;oled_delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>50)
		{
			I2C_Stop();
			return 1;
		}
	}
	I2C_SCL_HIGH;oled_delay_us(5);	 
	I2C_SCL_LOW;	   
	return 0;  
}


void I2C_Ack(void)
{
	I2C_SCL_LOW;
	SDA_OUT();
	I2C_SDA_LOW;
	oled_delay_us(2);
	I2C_SCL_HIGH;
	oled_delay_us(2);
	I2C_SCL_LOW;
}

void I2C_Nack(void)
{
	I2C_SCL_LOW;
	SDA_OUT();
	I2C_SDA_HIGH;
	oled_delay_us(2);
	I2C_SCL_HIGH;
	oled_delay_us(2);
	I2C_SCL_LOW;
}

void I2C_Sendbyte(uint8_t d)
{
	uint8_t t;   
	SDA_OUT(); 	    
    I2C_SCL_LOW;                            //拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, ((d & 0x80) >> 7));
        d<<=1; 	  
		oled_delay_us(2);  
		I2C_SCL_HIGH;
		oled_delay_us(2); 
		I2C_SCL_LOW;	
		oled_delay_us(2);
    }	 
}

uint8_t I2C_Readbyte(uint8_t ack)
{
	unsigned char i,receive=0;
	SDA_IN();
    for(i=0;i<8;i++ )
	{
        I2C_SCL_LOW; 
        oled_delay_us(2);
		I2C_SCL_HIGH;
        receive<<=1;
		
      	if(READ_SDA)
	      	receive++;   
		oled_delay_us(1); 
    }					 
    if (!ack)
        I2C_Nack();//发送nACK
    else
        I2C_Ack(); //发送ACK   
    return receive;
}


uint8_t I2C_Read_byte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
{
	uint8_t count = 0;
	I2C_start();
	I2C_Sendbyte(dev<<1);	
	I2C_Waitack();
	I2C_Sendbyte(reg); 
	I2C_Waitack();
	I2C_start();
	I2C_Sendbyte((dev<<1)+1); 
	I2C_Waitack();
	
	for(count=0;count<length;count++)
	{
			if(count!=length-1)data[count]=I2C_Readbyte(1);  //带ACK的读数据
		 	else  data[count]=I2C_Readbyte(0);            	 //最后一个字节NACK
	}
	I2C_Stop();
	return count;
}

uint8_t I2C_Write_byte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
{
	uint8_t count = 0;
	I2C_start();
	I2C_Sendbyte(dev<<1);	   //发送写命令
	I2C_Waitack();
	I2C_Sendbyte(reg);   //发送地址
	I2C_Waitack();	
	for(count=0;count<length;count++)
	{
		I2C_Sendbyte(data[count]); 
		I2C_Waitack(); 
	}
	I2C_Stop();//产生一个停止条件

    return 1; //status == 0;  
}

short CharToShort(uint8_t *data)
{
	return (short)((data[0] << 8) + data[1]);
}

struct angle_as5600 Get_ang={0,0,0};
int angle_c=0;
uint8_t datc[2]={0};
int angle_w[5]={0};
int angle_a[5]={0};
short offsit = 426-25;
short angle_set = 0;

void Get_angle(void)
{
	I2C_Read_byte(0x36,0x0c,2,&datc[0]);
	angle_c = (int)CharToShort(&datc[0]) + offsit + angle_set;
	if (angle_c > 4095)
		angle_c -= 4096;
	else if (angle_c < 0)
	{
		angle_c += 4096;
	}
	Get_ang.angle=angle_c;
}

void Get_angle_w(void)  
{
	//angle_w[2]=angle_w[1];
	angle_w[1]=angle_w[0];
	angle_w[0]=Get_ang.angle;
	if(abs(angle_w[0] - angle_w[1]) > 2000 && angle_w[0]>angle_w[1])
	{
		angle_w[1]+=4096;
	}else if(abs(angle_w[0] - angle_w[1]) > 2000 && angle_w[1]>angle_w[0])
	{
		angle_w[0]+=4096;
	}
	
	//angle_w[4] = Get_ang.angle_w;
	angle_w[3]=angle_w[0]-angle_w[1];	
	//angle_w[4]=angle_w[1]-angle_w[2];
	//Get_ang.angle_w = (angle_w[3] + angle_w[4]) /2;
	Get_ang.angle_w = angle_w[3];
}

void Get_angle_a(void)
{
		angle_a[2]=angle_a[1];
		angle_a[1]=angle_a[0];
		angle_a[0]=Get_ang.angle_w;
		angle_a[3]=angle_a[0]-angle_a[1];	
		angle_a[4]=angle_a[1]-angle_a[2];	
		Get_ang.angle_a=(angle_a[3]+angle_a[4])/2;
}

double angle_Valuechange(anglechange value) //value取值：angle_To角度转换 angle_w_To角速度转换 angle_a_To加速度转换
{
	double val;
	if(value == angle_To)
	{
		val =(double)Get_ang.angle/13.378;
		 
	}else if(value== angle_w_To)
	{
		val = Get_ang.angle_w * 200 / 13.378;
	}
	else if(value == angle_a_To)
	{
		val = Get_ang.angle_a * 200 / 13.378;
	}
	return val;
}
