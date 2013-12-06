#include "includs.h"

#define	RS485_BPS 	2400				//RS485������
#define RTEN 		8					//P0.8����485���ͽ��շ���

extern uint32 rs485self_check_cnt;

extern Que 		rs485_pipe;
extern uint16 	fasongxianshi,jieshouxianshi;
extern Sub sub;

/**************************************************************************************
** �������� ��IRQ_UART0()
** �������� ��485�����жϣ��������ݵ�rs485_pipe[]
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
void __irq IRQ_UART0 (void)
{
	uint8 i;
	uint8 data;
	
	rs485self_check_cnt = 0;			//rs485�Լ��������
	jieshouxianshi = 0;					//rs485���յ���ʾ
	
	i = U0IIR & 0x0e;
	if(i == 0x04){					
		i = U0LSR;		
		do{
			data = U0RBR;
			InQue(&rs485_pipe,data);	
		}while (U0LSR & 0x01);
	}else if(i == 0x0c){				//�ַ���ʱ�ж�
		i = U0LSR;	
		do{
			data = U0RBR;
			InQue(&rs485_pipe,data);
		}while (U0LSR & 0x01);
	}else if(i == 0x06){				//�������ݳ��ִ�����
		i = U0LSR; 
		//receive_data_cont = 0;
		U0FCR |= 0x03;
	}
	VICVectAddr = 0x00;					// �жϴ������	
}
 
 /**************************************************************************************
 ** �������� ��rs485_send()
 ** �������� ��485����
 ** ��ڲ��� ��pΪҪ���͵�����,length Ϊ����
 ** ���ڲ��� ��
 **************************************************************************************/
void rs485_send(uint8 *p,uint16 length)
{
	uint16 count;
	
    if(sub.type == LOCAT_CHILD_STATION)			//�ӷ�վ(������λ��վ)������485����
		return;
	
    U0IER = 0x00;                
	IO0CLR |= 1 << RTEN; 
	if(length>0x400)
		goto rs485_send_end;
	for(count = 0;count < length;count++){
		U0THR = p[count];
		while ((U0LSR & 0x40) == 0);
		fasongxianshi = 0;
	}
	/*
	U0THR = 0x0e;
	while ((U0LSR & 0x40) == 0);
	fasongxianshi = 0;
	U0THR = 0xfe;
	while ((U0LSR & 0x40) == 0);
	fasongxianshi = 0;
	U0THR = 0xfe;
	while ((U0LSR & 0x40) == 0);
	fasongxianshi = 0;
	U0THR = 0xfe;
	while ((U0LSR & 0x40) == 0);
	fasongxianshi = 0;
	*/
rs485_send_end:	
	U0IER = 0x05;
	IO0SET |= 1 << RTEN;				//ʹ�ܽ���				
}

/**************************************************************************************
 ** �������� ��rs485_init()
 ** �������� ��485��ʼ��
 ** ��ڲ��� ��
 ** ���ڲ��� ��
 **************************************************************************************/
void rs485_init(void)
{
	uint16 bak;

	PINSEL0 |= 0x00000005;
	IO0DIR |= 1 << RTEN;
	IO1DIR |= (1 << FASONG) | (1 << JIESHOU);
	
	IO1SET |= 1 << JIESHOU;	
	IO1SET |= 1 << FASONG;	
	
	//���ò�����4800	
	bak = (Fpclk>>4)/RS485_BPS;
	U0LCR = 0x80;
	U0DLM = (uint8)(bak>>8);
	U0DLL = (uint8)(bak&0xff);	

	U0LCR = 0x1b;						//8λ���ݡ�1λֹͣ��żУ��
	
	U0IER = 0x05;						//�ж�ʹ��
	IO0SET |= 1 << RTEN;				//ʹ�ܽ���	
	U0FCR = 0x87;						//8�ֽڴ���

	//ʹ��UART0�ж� 
	VICVectCntl2 = 0x20 | 0x06;			// UART0���䵽IRQ slot2��
	VICVectAddr2 = (uint32)IRQ_UART0;	// ����UART0������ַ
	VICIntEnable |= 1 << 0x06;			// ʹ��UART0�ж�	
	
}
