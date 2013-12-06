#include "includs.h"

Sub sub;								//分站信息
#define DAT 1<<3
#define SCK 1<<2
#define BEE 1<<25	//P1.25
#define RTEN 		8
#define NANO_RCV_DSP 29

void subinfo_init(void)
{
	PINSEL2 = 0x04;						//设置P1.25-16为GPIO口
}

void getsubinfo(void)
{
	uint32 keynum;
	uint8 num;
	
	keynum = IO1PIN;
	keynum >>= 16;
	keynum = ~keynum;
	keynum &= 0x1f;
	num = (uint8)keynum;
	
	if((num > MAX_SUB_NUM)||(0 == num)){				//超出分站号范围,所有灯都
		PINSEL0 &= 0xffffff0f;							//设置P0.2口P0.3口为GPIO口
		IO0DIR |= (DAT | SCK);							//设置P0.2口P0.3口为输出
		
		PINSEL1 &= 0xf3ffffff;
		IO0DIR |= (1<<NANO_RCV_DSP);
		IO0CLR |= (1<<NANO_RCV_DSP);
		
		IO1DIR |= BEE;
		IO1CLR |= BEE;

		PINSEL0 |= 0x00000005;
		IO0DIR |= 1 << RTEN;
		IO0SET |= 1 << RTEN;							//使能接收
		IO1DIR |= (1 << FASONG) | (1 << JIESHOU);
	
		IO1CLR |= 1 << JIESHOU; 
		IO1CLR |= 1 << FASONG;	
		
		while(1){
			Scroll_Ver_LED();
		}
	}
	else{
		sub.id	 = num;
		sub.type = LOCAT_CHILD_STATION;
	}

/*
	if(num&0x10){
		sub.id = num&0x0f;
		sub.type = LOCAT_STATION;
	}else{
		sub.id   = num;
		sub.type = COMM_STATION;
	}
*/
}


