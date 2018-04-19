/*
Demo 程序适用范围： Si4431F13, Si4432F20, Si4431F27
RF 模块的参数设置为： FSK，  433.5MHz， 1.2KBPS， +/-10PPM, 频偏：30KHz， 调制带宽：61.2KHz 
使能AFC，使能CRC， PH + FIFO 模式, 头码："swwx", 同步字： 0x2d 0xd4
每秒钟发射的固定测试数据为： 
0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x6d, 
0x6d是发生数据的 CheckSum, 0x6d = (0x41 +0x42 +0x43 +0x44 +0x45 +0x46 +0x47 + 0x48 +0x49)
控制MCU : 8 bit MCU :STC89C52, 11.0592MHZ
*/
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common


#include "si4432.h"
#include <math.h>

#define  SI4432_PWRSTATE_READY		          01		// 模块 Ready 状态定义
#define  SI4432_PWRSTATE_TX				      0x09		// 模块 发射状态定义
#define  SI4432_PWRSTATE_RX				      05		// 模块 接收状态定义
#define  SI4432_PACKET_SENT_INTERRUPT	      04		// 模块 发射完成中断
#define  SI4432_Rx_packet_received_interrupt  0x02      // 模块 收到数据包中断

#define  TX1_RX0	spi_rw(0x0e|0x80, 0x01)		// 发射状态的天线开关定义
#define  TX0_RX1	spi_rw(0x0e|0x80, 0x02)		// 接收状态的天线开关定义
#define  TX0_RX0	spi_rw(0x0e|0x80, 0x00)         // 非发射，接收状态的天线开关定义

//========================================SI4432与MCU连接========================================
#define nIRQ (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_4))
#define SDO (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_0))

#define  SDN_0 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET)
#define nSEL_0 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET)
#define  SCK_0 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET)
#define  SDI_0 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET)
#define  SDO_0 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET)

#define  SDN_1 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_SET)
#define nSEL_1 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET)
#define  SCK_1 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET)
#define  SDI_1 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET)
#define  SDO_1 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET)

//========================================按键端口===============================================
int KEY   = 3^6;
//========================================LED显示端口============================================
int LED   = 3^4;

unsigned char ItStatus1, ItStatus2;
unsigned char rf_timeout;
unsigned char rx_buf[15];

// 检测到按键后发射的固定内容的测试信号，第10个数据是前9个数据的校验和
const unsigned char tx_test_data[10] = {0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x6d};  
//===============================================================================================
typedef struct 
{
	
	unsigned char reach_1s		;//		: 1;
	unsigned char rf_reach_timeout	;
	unsigned char is_tx			;
		
}	FlagType;

FlagType   Flag;


static void si4432_delay_ms(unsigned int t)
{
	int i;
	for( i=0;i<t;i++)
	{
		int a=42000;
		while(a--);
	}
}

static void si4432_delay_us(unsigned int t)
{
	int i;
	for( i=0;i<t;i++)
	{
	//	int a=40;
		int a=40;
		while(a--);
	}
}
//===============================================SPI读写函数=======================================

unsigned char spi_byte(unsigned char TRdata)
{
	unsigned char i;
	
	for (i = 0; i < 8; i++)				// 控制SCK 和 SDI，发射一个字节的命令，同事读取1个字节的数据
	{									// 没有包括nSEL的控制
		if (TRdata & 0x80)
			SDI_1;
		else
			SDI_0;
			
		TRdata <<= 1;
		SCK_1;
		
		if (SDO)
			TRdata |= 0x01;
		else
			TRdata &= 0xfe;
			
		SCK_0;
	}
	
	return (TRdata);
}
//-------------------------------------------对寄存器读写操作===========================================
unsigned char spi_rw(unsigned char addr, unsigned char TRdata)
{
	unsigned char i;
	
	SCK_0;
	nSEL_0;
	
	for (i = 0; i < 8; i++) 
	{
		if (addr & 0x80)
			SDI_1;
		else
			SDI_0;
		addr <<= 1;
		SCK_1;
 		si4432_delay_us(1);
		SCK_0;
	}
//======================================================================================================	
	for (i = 0; i < 8; i++) 
	{
		if (TRdata & 0x80)
			SDI_1;
		else
			SDI_0;
		TRdata <<= 1;
		SCK_1;
		if (SDO)
			TRdata |= 0x01;
		else
			TRdata &= 0xfe;
		SCK_0;
	}
	nSEL_1;
	SCK_1;
	return (TRdata);
}
//=========================================接收数据================================================
  
void rx_data(void)
{	

	Flag.is_tx = 0;
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	//进入 Ready 模式
	si4432_delay_ms(5);		
	TX0_RX1;									// 设置天线开关
	spi_rw(0x08|0x80, 0x03);  					//清发射，接收缓冲区
	spi_rw(0x08|0x80, 0x00);  					//清发射，接收缓冲区
		
	spi_rw(0x07|0x80,SI4432_PWRSTATE_RX );  	// RF 模块进入接收模式
	
	spi_rw(0x05|0x80, SI4432_Rx_packet_received_interrupt);  // RF模块收到整包数据后，产生中断
		
	ItStatus1 = spi_rw(0x03,0x00);				//清掉现有的中断标志
	ItStatus2 = spi_rw(0x04,0x00);				//清掉现有的中断标志
		
}	
//=========================================发送数据================================================
void tx_data(void)
{
	unsigned char i;
	
	Flag.is_tx = 1;
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	// rf 模块进入Ready 模式

	TX1_RX0;		//设置天线开关的方向
	
	si4432_delay_ms(5);		// 延时 5ms, 让系统稳定
	
	spi_rw(0x08|0x80, 0x03);  // 
	spi_rw(0x08|0x80, 0x00);  // 清发射，接收缓冲区
			
	spi_rw(0x34|0x80, 40);    // 发射40个Nibble 的前导码
	spi_rw(0x3e|0x80, 10);    // 总共发射10个字节的数据
  	for (i = 0; i<10; i++)
	{
		spi_rw(0x7f|0x80, tx_test_data[i]); 	// 将要发射的数据载入缓冲区
	}
	spi_rw(0x05|0x80, SI4432_PACKET_SENT_INTERRUPT);	// 整包数据发射完后，产生中断
	ItStatus1 = spi_rw(0x03,0x00);		// 清RF模块中断	
	ItStatus2 = spi_rw(0x04,0x00);
	spi_rw(0x07|0x80, SI4432_PWRSTATE_TX);  // 进入发射模式
//	si4432_delay_ms(5);	

	rf_timeout = 0;
	Flag.rf_reach_timeout = 0;

	while(nIRQ)		// 等待中断
	{
		
		if(Flag.rf_reach_timeout)
		{
			
			SDN_1;		//如果0.5秒还没有中断,则RF模块工作不正常，重新复位并初始化模块
			si4432_delay_ms(10);
			SDN_0;
			si4432_delay_ms(200);
			
			SI4432_init();
			break;		// 则强制跳出
		}
			
	}
	

   // rx_data();		//rf 发射完成，进入接收模式
}	
//==========================================================================================================

void SI4432_IO_init(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct;
  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();

  /* Configure GPIO pins: PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin   =   GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3  | GPIO_PIN_5;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 
	
	 GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_4;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 
}


void SI4432_init(void)
{
	SI4432_IO_init();
	
	ItStatus1 = spi_rw(0x03,0x00);				// 清RF模块中断	
	ItStatus2 = spi_rw(0x04,0x00);
	
	spi_rw(0x06|0x80, 0x00);  					//  关闭不需要的中断
	
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);   // 进入 Ready 模式
	 
	spi_rw(0x09|0x80, 0x7f);  					//  负载电容= 12P

	spi_rw(0x0a|0x80, 0x05);					// 关闭低频输出
	spi_rw(0x0b|0x80, 0xea); 					// GPIO 0 当做普通输出口
	spi_rw(0x0c|0x80, 0xea); 					// GPIO 1 当做普通输出口	
	spi_rw(0x0d|0x80, 0xf4);  					// GPIO 2 输出收到的数据
	
	spi_rw(0x70|0x80, 0x2c);  					// 下面的设置根据Silabs 的Excel
	spi_rw(0x1d|0x80, 0x40);  					// 使能 afc
		
//======================================1.2K bps setting start===============================================
	spi_rw(0x1c|0x80, 0x16);	
		
	spi_rw(0x20|0x80, 0x83);   	 
	spi_rw(0x21|0x80, 0xc0); 
	spi_rw(0x22|0x80, 0x13); 
	spi_rw(0x23|0x80, 0xa9); 
	spi_rw(0x24|0x80, 0x00); 
	spi_rw(0x25|0x80, 0x04); 
	
	spi_rw(0x2a|0x80, 0x14);
	spi_rw(0x6e|0x80, 0x09);
	spi_rw(0x6f|0x80, 0xd5);	
//=====================================1.2K bps setting end	===============================================		
	spi_rw(0x30|0x80, 0x8c);   // 使能PH+ FIFO模式，高位在前面，使能CRC校验
	 				
	spi_rw(0x32|0x80, 0xff);  // byte0, 1,2,3 作为头码
	 
	spi_rw(0x33|0x80, 0x42);//  byte 0,1,2,3 是头码，同步字3,2 是同步字
		
	spi_rw(0x34|0x80, 16);  // 发射16个Nibble的Preamble
	spi_rw(0x35|0x80, 0x20);  // 需要检测4个nibble的Preamble
	spi_rw(0x36|0x80, 0x2d);  // 同步字为 0x2dd4
	spi_rw(0x37|0x80, 0xd4);
	spi_rw(0x38|0x80, 0x00);
	spi_rw(0x39|0x80, 0x00);
	spi_rw(0x3a|0x80, 's');   // 发射的头码为： “swwx"
	spi_rw(0x3b|0x80, 'w');
	spi_rw(0x3c|0x80, 'w');
	spi_rw(0x3d|0x80, 'x');
	spi_rw(0x3e|0x80, 10);    // 总共发射10个字节的数据
	spi_rw(0x3f|0x80, 's');   // 需要校验的头码为：”swwx"
	spi_rw(0x40|0x80, 'w');
	spi_rw(0x41|0x80, 'w');
	spi_rw(0x42|0x80, 'x');
	spi_rw(0x43|0x80, 0xff);  // 头码1,2,3,4 的所有位都需要校验
	spi_rw(0x44|0x80, 0xff);  // 
	spi_rw(0x45|0x80, 0xff);  // 
	spi_rw(0x46|0x80, 0xff);  // 
	spi_rw(0x6d|0x80, 0x07);  // 设置为最大功率发射

	spi_rw(0x79|0x80, 0x0);  // 不需要跳频
	spi_rw(0x7a|0x80, 0x0);  // 不需要跳频
	
	spi_rw(0x71|0x80, 0x22); // 发射不需要 CLK，FiFo ， FSK模式
			
	spi_rw(0x72|0x80, 0x30);  // 频偏为 30KHz	

	spi_rw(0x73|0x80, 0x0);  // 没有频率偏差
	spi_rw(0x74|0x80, 0x0);  // 没有频率偏差
		
	spi_rw(0x75|0x80, 0x53);  // 频率设置 434
	spi_rw(0x76|0x80, 0x57);  // 
	spi_rw(0x77|0x80, 0x80);	
}
//=================================================主函数==========================================
void xxxx()
{ 
	

	unsigned char  i, chksum;
 	SDN_1;
 	si4432_delay_ms(10);	// RF 模块上电复位
 	SDN_0;
 	si4432_delay_ms(200);	// 延时200ms RF 模块进入工作状态
 	SI4432_init();  // RF 模块初始化 	
 	TX0_RX0;	// 天线开关不在发射，接收状态
	rx_data();	
	Flag.reach_1s = 0;	
	while(1)
	{		
		if(KEY==0)
		{
		//	Flag.reach_1s = 0;
			tx_data();		// 每间隔一段时间，发射一包数据，并接收 Acknowledge 信号
			LED=0;  si4432_delay_ms(200);  LED=1	; si4432_delay_ms(200);
			rx_data();		//rf 发射完成，进入接收模式
		}


		if(!Flag.is_tx)
	
		{
	
			if(!nIRQ)
			{ 	
				ItStatus1 = spi_rw(0x03,0x00);	// 清中断											//read the Interrupt Status1 register
				ItStatus2 = spi_rw(0x04,0x00);	// 清中断
				
				SCK_0;
				nSEL_0;
				spi_byte(0x7f);		// 从缓冲区读数据
				for(i = 0;i<10;i++)	
				{
					rx_buf[i] = spi_byte(0x00);
				}
				nSEL_1;
				
				spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	// 数据接收完成，退出接收模式，
									
				chksum = 0;
				for(i=0;i<9;i++)		// 计算Checksum
        			chksum += rx_buf[i];          	 		
        				
        			
     				if(( chksum == rx_buf[9] )&&( rx_buf[0] == 0x41 ))
     					
     				{
     					
     					LED=0;  si4432_delay_ms(200);  LED=1	; si4432_delay_ms(200);  // 数据收到了
						tx_data();
						rx_data(); 
        			} 
        			else
        			{
        				rx_data();     // 收到的数据不对，则重新开始接收
        			}	
        		 
        		}	
		}	
	
	}	
}


