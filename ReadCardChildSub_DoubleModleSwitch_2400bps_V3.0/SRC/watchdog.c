/****************************************Copyright (c)**************************************************
**                               
**                                      
**                                 
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			main.c
** Last modified Date:  2004-09-16
** Last Version:		1.0
** Descriptions:		The main() function example template
**
**------------------------------------------------------------------------------------------------------
** Created by:			
** Created date:		2004-09-16
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:		2005-02-16
** Version:
** Descriptions:
**
*******************************************************************************************************
*/
#include "includs.h"

extern uint32 	int_state;
uint8 dog_flag;
void watchdog_weigou(void)	// ����ι������ 
{
	WDFEED = 0xAA;			    
	WDFEED = 0x55;		    
}

void watchdog_fuwei(void)		// ���и�λ���� 
{
	WDFEED = 0x00;
	WDFEED = 0xff;
} 

void watchdog_init(void)
{
	WDTC = (Fpclk/2);				// ����WDTC��ι����װֵ2��

	WDMOD = 0x03;			   	 	// ���ò�����WDT
	ENTER_CRITICAL();
	watchdog_weigou();
	EXIT_CRITICAL();
	
} 