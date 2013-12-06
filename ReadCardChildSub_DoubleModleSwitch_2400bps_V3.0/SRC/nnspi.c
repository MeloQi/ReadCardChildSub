#include "includs.h"
#include "config.h"
#include "ntrxtypes.h"
#include "nnspi.h"
#include "hwclock.h"

#define PONRESET2  			23				//复位引脚
#define SPISSN2 			21				//片选
#define PONRESET0			11				//复位引脚MODULE_RIGHT
#define SPISSN0				10				//p0.10片选MODULE_RIGHT


#define CS_ON_L   (IO0CLR |= (1<<SPISSN2)) 
#define CS_OFF_L  (IO0SET |= (1<<SPISSN2))
#define CS_ON_R   (IO0CLR |= (1<<SPISSN0)) 
#define CS_OFF_R  (IO0SET |= (1<<SPISSN0))

extern uint8 wireless_module;


/**
 * NanoReset:
 *
 * NanoReset() resets the nanoNET chip and adjusts the pin level.
 *
 * Returns: none
 */
void nanorest_init(void)
{
  	PINSEL1 &= 0xFFFF3FFF;					//P0.23复位引脚GPIO输出
  	IO0DIR  |= (1 << PONRESET2);

	PINSEL0 &= 0xFF3FFFFF;		 			//p0.11复位引脚GPIO输出MODULE_RIGHT
	IO0DIR  |= (1 << PONRESET0);
}

void NanoReset(void)
{
	if(MODULE_LEFT == wireless_module){
		IO0CLR |= 1 << PONRESET2;
		hwdelay(1000);  
		IO0SET |= 1 << PONRESET2;
	}else if(MODULE_RIGHT== wireless_module){
		IO0CLR |= 1 << PONRESET0;
		hwdelay(1000);  
		IO0SET |= 1 << PONRESET0;
	}else{}
}

/**
 * InitSPI:
 *
 * InitSPI() initializes the SCI2 module and adjusts the pin level
 *
 * Returns: none
 */
MyByte8T transmission(MyByte8T value)
{ 
	uint16 spi_timeout = 4400;
    if(MODULE_LEFT == wireless_module){
		S1PDR = value;
		
		do{
			spi_timeout--;
		}while ((0==(S1PSR&0x80))&&(spi_timeout > 0));
    	
    	return(S1PDR); 
	}else if(MODULE_RIGHT == wireless_module){
		S0PDR = value; 
		
    	do{
			spi_timeout--;
		}while ((0==(S0PSR&0x80))&&(spi_timeout > 0));
		
    	return(S0PDR); 
	}else{}
	return 0; 
}

void InitSPI(void)
{
	uint8 sta;
	if(MODULE_LEFT == wireless_module){		//左侧模块SPI初始化
		PINSEL1 |= 0x000000A8; 				//SPI1(LPC2114)
	  	PINSEL1 &= 0xFFFFFFAB;
	  	PINSEL1 &= 0xFFFFF3FF;				//p0.21片选GPIO输出
	  	IO0DIR  |= (1<<SPISSN2);	

	  	S1PCR  = 0x60;						//SPI1模式,初始化 ok 
	  	S1PCCR = 0x08; 						//spi时钟=fpclk/pccr,必须设置为大于等于8的偶数

	  	sta = S1PSR;						//读状态、数据;用于清除寄存器
	  	sta = S1PDR;
	}else if(MODULE_RIGHT == wireless_module){		//右侧模块SPI初始化
		PINSEL0 |= 0x00001500;		 				//SPI0
		PINSEL0 &= 0xFFFFD5FF;
		PINSEL0 &= 0xFFCFFFFF;		 				//p0.10片选
		IO0DIR  |= (1<<SPISSN0);

		S0PCR  = 0x60;				
	  	S0PCCR = 0x08; 				

	  	sta = S0PSR;				
	  	sta = S0PDR;
	}else{}
}

void SetupSPI (void)
{
	MyByte8T value = 0x42;
	NTRXWriteSPI (0x00, &value, 1);
}


void NTRXReadSPI(MyByte8T address, MyByte8T *buffer, MyByte8T len)
{
  if(MODULE_LEFT==wireless_module){CS_ON_L;}else if(MODULE_RIGHT == wireless_module){CS_ON_R;}else{}
  hwdelay(10);
  transmission(len & 0x7F);
  transmission(address); 
  while(len--)
  {
    *(buffer++) = transmission(0xff);
  }  
  hwdelay(10);  
  if(MODULE_LEFT==wireless_module){CS_OFF_L;}else if(MODULE_RIGHT == wireless_module){CS_OFF_R;}else{}
}

void NTRXWriteSPI(MyByte8T address, MyByte8T *buffer, MyByte8T len)
{
  if(MODULE_LEFT==wireless_module){CS_ON_L;}else if(MODULE_RIGHT == wireless_module){CS_ON_R;}else{}
  hwdelay(10);
  transmission(0x80 | (len & 0x7F));
  transmission(address); 
  while(len--)
  {
    transmission(*buffer);
    buffer++;
    address++;
  }  
  hwdelay(10);  
  if(MODULE_LEFT==wireless_module){CS_OFF_L;}else if(MODULE_RIGHT == wireless_module){CS_OFF_R;}else{}
}

void NTRXReadSingleSPI(MyByte8T address, MyByte8T *buffer)
{
  NTRXReadSPI(address, buffer, 1);
}

void NTRXWriteSingleSPI(MyByte8T address, MyByte8T buffer)
{
  NTRXWriteSPI(address, &buffer, 1);
}


