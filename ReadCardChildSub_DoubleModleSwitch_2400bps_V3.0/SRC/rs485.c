#include "includs.h"

#define	RS485_BPS 	2400				//RS485波特率
#define RTEN 		8					//P0.8控制485发送接收方向

extern uint32 rs485self_check_cnt;

extern Que 		rs485_pipe;
extern uint16 	fasongxianshi,jieshouxianshi;
extern Sub sub;

/**************************************************************************************
** 函数名称 ：IRQ_UART0()
** 函数功能 ：485接收中断，接收数据到rs485_pipe[]
** 入口参数 ：无
** 出口参数 ：
**************************************************************************************/
void __irq IRQ_UART0 (void)
{
	uint8 i;
	uint8 data;
	
	rs485self_check_cnt = 0;			//rs485自检计数清零
	jieshouxianshi = 0;					//rs485接收灯显示
	
	i = U0IIR & 0x0e;
	if(i == 0x04){					
		i = U0LSR;		
		do{
			data = U0RBR;
			InQue(&rs485_pipe,data);	
		}while (U0LSR & 0x01);
	}else if(i == 0x0c){				//字符超时中断
		i = U0LSR;	
		do{
			data = U0RBR;
			InQue(&rs485_pipe,data);
		}while (U0LSR & 0x01);
	}else if(i == 0x06){				//接受数据出现错误处理
		i = U0LSR; 
		//receive_data_cont = 0;
		U0FCR |= 0x03;
	}
	VICVectAddr = 0x00;					// 中断处理结束	
}
 
 /**************************************************************************************
 ** 函数名称 ：rs485_send()
 ** 函数功能 ：485发送
 ** 入口参数 ：p为要发送的数组,length 为长度
 ** 出口参数 ：
 **************************************************************************************/
void rs485_send(uint8 *p,uint16 length)
{
	uint16 count;
	
    if(sub.type == LOCAT_CHILD_STATION)			//子分站(辅助定位分站)不发送485数据
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
	IO0SET |= 1 << RTEN;				//使能接收				
}

/**************************************************************************************
 ** 函数名称 ：rs485_init()
 ** 函数功能 ：485初始化
 ** 入口参数 ：
 ** 出口参数 ：
 **************************************************************************************/
void rs485_init(void)
{
	uint16 bak;

	PINSEL0 |= 0x00000005;
	IO0DIR |= 1 << RTEN;
	IO1DIR |= (1 << FASONG) | (1 << JIESHOU);
	
	IO1SET |= 1 << JIESHOU;	
	IO1SET |= 1 << FASONG;	
	
	//设置波特率4800	
	bak = (Fpclk>>4)/RS485_BPS;
	U0LCR = 0x80;
	U0DLM = (uint8)(bak>>8);
	U0DLL = (uint8)(bak&0xff);	

	U0LCR = 0x1b;						//8位数据、1位停止、偶校验
	
	U0IER = 0x05;						//中断使能
	IO0SET |= 1 << RTEN;				//使能接收	
	U0FCR = 0x87;						//8字节触发

	//使能UART0中断 
	VICVectCntl2 = 0x20 | 0x06;			// UART0分配到IRQ slot2，
	VICVectAddr2 = (uint32)IRQ_UART0;	// 设置UART0向量地址
	VICIntEnable |= 1 << 0x06;			// 使能UART0中断	
	
}
