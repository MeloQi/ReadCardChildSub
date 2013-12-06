
#include "includs.h" 

void IRQ_Timer1(void);
/**
 * timer1_init:
 *
 * 1ms��ʱ����ʼ��
 *
 * Returns: none.
*/
void timer1_init(void)
{
	T1TC   = 0;			
	T1PR   = 0;			
	T1MCR  = 0x03;		
	T1MR0  = Fpclk/1000;		
	T1TCR  = 0x00;						//��ֹ��ʱ

	VICVectCntl4 = 0x20 | 0x05;			
	VICVectAddr4 = (uint32)IRQ_Timer1;	
	T1IR = 0x01;
	VICIntEnable |= 1 << 0x05;
	
}
 