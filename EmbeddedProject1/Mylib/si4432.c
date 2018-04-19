/*
Demo �������÷�Χ�� Si4431F13, Si4432F20, Si4431F27
RF ģ��Ĳ�������Ϊ�� FSK��  433.5MHz�� 1.2KBPS�� +/-10PPM, Ƶƫ��30KHz�� ���ƴ���61.2KHz 
ʹ��AFC��ʹ��CRC�� PH + FIFO ģʽ, ͷ�룺"swwx", ͬ���֣� 0x2d 0xd4
ÿ���ӷ���Ĺ̶���������Ϊ�� 
0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x6d, 
0x6d�Ƿ������ݵ� CheckSum, 0x6d = (0x41 +0x42 +0x43 +0x44 +0x45 +0x46 +0x47 + 0x48 +0x49)
����MCU : 8 bit MCU :STC89C52, 11.0592MHZ
*/
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common


#include "si4432.h"
#include <math.h>

#define  SI4432_PWRSTATE_READY		          01		// ģ�� Ready ״̬����
#define  SI4432_PWRSTATE_TX				      0x09		// ģ�� ����״̬����
#define  SI4432_PWRSTATE_RX				      05		// ģ�� ����״̬����
#define  SI4432_PACKET_SENT_INTERRUPT	      04		// ģ�� ��������ж�
#define  SI4432_Rx_packet_received_interrupt  0x02      // ģ�� �յ����ݰ��ж�

#define  TX1_RX0	spi_rw(0x0e|0x80, 0x01)		// ����״̬�����߿��ض���
#define  TX0_RX1	spi_rw(0x0e|0x80, 0x02)		// ����״̬�����߿��ض���
#define  TX0_RX0	spi_rw(0x0e|0x80, 0x00)         // �Ƿ��䣬����״̬�����߿��ض���

//========================================SI4432��MCU����========================================
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

//========================================�����˿�===============================================
int KEY   = 3^6;
//========================================LED��ʾ�˿�============================================
int LED   = 3^4;

unsigned char ItStatus1, ItStatus2;
unsigned char rf_timeout;
unsigned char rx_buf[15];

// ��⵽��������Ĺ̶����ݵĲ����źţ���10��������ǰ9�����ݵ�У���
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
//===============================================SPI��д����=======================================

unsigned char spi_byte(unsigned char TRdata)
{
	unsigned char i;
	
	for (i = 0; i < 8; i++)				// ����SCK �� SDI������һ���ֽڵ����ͬ�¶�ȡ1���ֽڵ�����
	{									// û�а���nSEL�Ŀ���
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
//-------------------------------------------�ԼĴ�����д����===========================================
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
//=========================================��������================================================
  
void rx_data(void)
{	

	Flag.is_tx = 0;
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	//���� Ready ģʽ
	si4432_delay_ms(5);		
	TX0_RX1;									// �������߿���
	spi_rw(0x08|0x80, 0x03);  					//�巢�䣬���ջ�����
	spi_rw(0x08|0x80, 0x00);  					//�巢�䣬���ջ�����
		
	spi_rw(0x07|0x80,SI4432_PWRSTATE_RX );  	// RF ģ��������ģʽ
	
	spi_rw(0x05|0x80, SI4432_Rx_packet_received_interrupt);  // RFģ���յ��������ݺ󣬲����ж�
		
	ItStatus1 = spi_rw(0x03,0x00);				//������е��жϱ�־
	ItStatus2 = spi_rw(0x04,0x00);				//������е��жϱ�־
		
}	
//=========================================��������================================================
void tx_data(void)
{
	unsigned char i;
	
	Flag.is_tx = 1;
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	// rf ģ�����Ready ģʽ

	TX1_RX0;		//�������߿��صķ���
	
	si4432_delay_ms(5);		// ��ʱ 5ms, ��ϵͳ�ȶ�
	
	spi_rw(0x08|0x80, 0x03);  // 
	spi_rw(0x08|0x80, 0x00);  // �巢�䣬���ջ�����
			
	spi_rw(0x34|0x80, 40);    // ����40��Nibble ��ǰ����
	spi_rw(0x3e|0x80, 10);    // �ܹ�����10���ֽڵ�����
  	for (i = 0; i<10; i++)
	{
		spi_rw(0x7f|0x80, tx_test_data[i]); 	// ��Ҫ������������뻺����
	}
	spi_rw(0x05|0x80, SI4432_PACKET_SENT_INTERRUPT);	// �������ݷ�����󣬲����ж�
	ItStatus1 = spi_rw(0x03,0x00);		// ��RFģ���ж�	
	ItStatus2 = spi_rw(0x04,0x00);
	spi_rw(0x07|0x80, SI4432_PWRSTATE_TX);  // ���뷢��ģʽ
//	si4432_delay_ms(5);	

	rf_timeout = 0;
	Flag.rf_reach_timeout = 0;

	while(nIRQ)		// �ȴ��ж�
	{
		
		if(Flag.rf_reach_timeout)
		{
			
			SDN_1;		//���0.5�뻹û���ж�,��RFģ�鹤�������������¸�λ����ʼ��ģ��
			si4432_delay_ms(10);
			SDN_0;
			si4432_delay_ms(200);
			
			SI4432_init();
			break;		// ��ǿ������
		}
			
	}
	

   // rx_data();		//rf ������ɣ��������ģʽ
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
	
	ItStatus1 = spi_rw(0x03,0x00);				// ��RFģ���ж�	
	ItStatus2 = spi_rw(0x04,0x00);
	
	spi_rw(0x06|0x80, 0x00);  					//  �رղ���Ҫ���ж�
	
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);   // ���� Ready ģʽ
	 
	spi_rw(0x09|0x80, 0x7f);  					//  ���ص���= 12P

	spi_rw(0x0a|0x80, 0x05);					// �رյ�Ƶ���
	spi_rw(0x0b|0x80, 0xea); 					// GPIO 0 ������ͨ�����
	spi_rw(0x0c|0x80, 0xea); 					// GPIO 1 ������ͨ�����	
	spi_rw(0x0d|0x80, 0xf4);  					// GPIO 2 ����յ�������
	
	spi_rw(0x70|0x80, 0x2c);  					// ��������ø���Silabs ��Excel
	spi_rw(0x1d|0x80, 0x40);  					// ʹ�� afc
		
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
	spi_rw(0x30|0x80, 0x8c);   // ʹ��PH+ FIFOģʽ����λ��ǰ�棬ʹ��CRCУ��
	 				
	spi_rw(0x32|0x80, 0xff);  // byte0, 1,2,3 ��Ϊͷ��
	 
	spi_rw(0x33|0x80, 0x42);//  byte 0,1,2,3 ��ͷ�룬ͬ����3,2 ��ͬ����
		
	spi_rw(0x34|0x80, 16);  // ����16��Nibble��Preamble
	spi_rw(0x35|0x80, 0x20);  // ��Ҫ���4��nibble��Preamble
	spi_rw(0x36|0x80, 0x2d);  // ͬ����Ϊ 0x2dd4
	spi_rw(0x37|0x80, 0xd4);
	spi_rw(0x38|0x80, 0x00);
	spi_rw(0x39|0x80, 0x00);
	spi_rw(0x3a|0x80, 's');   // �����ͷ��Ϊ�� ��swwx"
	spi_rw(0x3b|0x80, 'w');
	spi_rw(0x3c|0x80, 'w');
	spi_rw(0x3d|0x80, 'x');
	spi_rw(0x3e|0x80, 10);    // �ܹ�����10���ֽڵ�����
	spi_rw(0x3f|0x80, 's');   // ��ҪУ���ͷ��Ϊ����swwx"
	spi_rw(0x40|0x80, 'w');
	spi_rw(0x41|0x80, 'w');
	spi_rw(0x42|0x80, 'x');
	spi_rw(0x43|0x80, 0xff);  // ͷ��1,2,3,4 ������λ����ҪУ��
	spi_rw(0x44|0x80, 0xff);  // 
	spi_rw(0x45|0x80, 0xff);  // 
	spi_rw(0x46|0x80, 0xff);  // 
	spi_rw(0x6d|0x80, 0x07);  // ����Ϊ����ʷ���

	spi_rw(0x79|0x80, 0x0);  // ����Ҫ��Ƶ
	spi_rw(0x7a|0x80, 0x0);  // ����Ҫ��Ƶ
	
	spi_rw(0x71|0x80, 0x22); // ���䲻��Ҫ CLK��FiFo �� FSKģʽ
			
	spi_rw(0x72|0x80, 0x30);  // ƵƫΪ 30KHz	

	spi_rw(0x73|0x80, 0x0);  // û��Ƶ��ƫ��
	spi_rw(0x74|0x80, 0x0);  // û��Ƶ��ƫ��
		
	spi_rw(0x75|0x80, 0x53);  // Ƶ������ 434
	spi_rw(0x76|0x80, 0x57);  // 
	spi_rw(0x77|0x80, 0x80);	
}
//=================================================������==========================================
void xxxx()
{ 
	

	unsigned char  i, chksum;
 	SDN_1;
 	si4432_delay_ms(10);	// RF ģ���ϵ縴λ
 	SDN_0;
 	si4432_delay_ms(200);	// ��ʱ200ms RF ģ����빤��״̬
 	SI4432_init();  // RF ģ���ʼ�� 	
 	TX0_RX0;	// ���߿��ز��ڷ��䣬����״̬
	rx_data();	
	Flag.reach_1s = 0;	
	while(1)
	{		
		if(KEY==0)
		{
		//	Flag.reach_1s = 0;
			tx_data();		// ÿ���һ��ʱ�䣬����һ�����ݣ������� Acknowledge �ź�
			LED=0;  si4432_delay_ms(200);  LED=1	; si4432_delay_ms(200);
			rx_data();		//rf ������ɣ��������ģʽ
		}


		if(!Flag.is_tx)
	
		{
	
			if(!nIRQ)
			{ 	
				ItStatus1 = spi_rw(0x03,0x00);	// ���ж�											//read the Interrupt Status1 register
				ItStatus2 = spi_rw(0x04,0x00);	// ���ж�
				
				SCK_0;
				nSEL_0;
				spi_byte(0x7f);		// �ӻ�����������
				for(i = 0;i<10;i++)	
				{
					rx_buf[i] = spi_byte(0x00);
				}
				nSEL_1;
				
				spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	// ���ݽ�����ɣ��˳�����ģʽ��
									
				chksum = 0;
				for(i=0;i<9;i++)		// ����Checksum
        			chksum += rx_buf[i];          	 		
        				
        			
     				if(( chksum == rx_buf[9] )&&( rx_buf[0] == 0x41 ))
     					
     				{
     					
     					LED=0;  si4432_delay_ms(200);  LED=1	; si4432_delay_ms(200);  // �����յ���
						tx_data();
						rx_data(); 
        			} 
        			else
        			{
        				rx_data();     // �յ������ݲ��ԣ������¿�ʼ����
        			}	
        		 
        		}	
		}	
	
	}	
}


