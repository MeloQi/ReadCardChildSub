#include "includs.h"

/*******************************宏定义和部分变量定义*************************************/
#define DAT 1<<3
#define SCK 1<<2
#define BEE 1<<25	//P1.25

#define NANO_RCV_DSP 29

extern Card card;


/*
extern card		comm_card;								
extern card		locat_card;
extern uint8 	fenzhan_type;
*/
uint16 	fasongxianshi = 0xff,jieshouxianshi = 0xff;  //RS485接收发送灯显示控制
uint8 	ledcount;				//数码管刷新计数，用来控制下一次刷新的位选
uint8 	lednum[4];
uint8 	lednum1[4];
uint8 	nano_rcv_dsp = 0xff;
uint16 	display_count;
const uint8 LedShow[15]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xbf,0xC1,0x86,0x88,0xFF}; //数码管0-9、U、E、R的编码

/****************************************************************************
* 函数名称：void led_init(void)
* 函数功能：led初始化
* 入口参数：无
* 出口参数：无
* 修改时间：
* 备    注：
****************************************************************************/
void led_init(void)
{
	lednum[0] = 10;//上电数码管显示----
	lednum[1] = 10;
	lednum[2] = 10;
	lednum[3] = 10;	
	
	lednum1[0] = 10;
	lednum1[1] = 10;
	lednum1[2] = 10;
	lednum1[3] = 10;
	
	PINSEL0 &= 0xffffff0f;//设置P0.2口P0.3口为GPIO口
	IO0DIR |= (DAT | SCK);//设置P0.2口P0.3口为输出
	
	PINSEL1 &= 0xf3ffffff;
	IO0DIR |= (1<<NANO_RCV_DSP);
	IO0SET |= (1<<NANO_RCV_DSP);
	
	IO1DIR |= BEE;
	IO1SET |= BEE;

	IO0CLR |= SCK;//P0.2口，空闲时时钟线为低电平
	IO0CLR |= DAT;//P0.3口，空闲时数据线为低电平
}

/****************************************************************************
* 函数名称：void led_display(void)
* 函数功能：led显示
* 入口参数：无
* 出口参数：无
* 修改时间：
* 备    注：动态显示模式。前后刷新间隔时间必须要大于一定的值，
*           否则显示效果将打折扣。4位数码管扫描，建议刷新间隔时间4毫秒。
****************************************************************************/
static void led_display(void)
{
	uint8 temp,temp1;//中间变量
	
	temp1 = ~(1 << ledcount);//位选
	for(temp = 0;temp < 8;temp++)
	{
		if(temp1 & 0x80)
		{
			IO0SET |= DAT;
		}
		else
		{
			IO0CLR |= DAT; 
		} 
					
		IO0CLR |= SCK; 
				
		delayus(1);
					
		temp1 <<= 1;
					 
		IO0SET |= SCK;
	}
			
	temp1 = LedShow[lednum[ledcount]];//段选
	
//	if(ledcount == 2)				//小数点
//	{
//	  temp1 &= 0x7f;
//	}
	
	for(temp = 0;temp < 8;temp++)
	{
		if(temp1 & 0x80)
		{
			IO0SET |= DAT; 
		}
		else
		{
			IO0CLR |= DAT; 
		}
					
		IO0CLR |= SCK;
					
		delayus(1); 
					
		temp1 <<= 1;
					 
		IO0SET |= SCK;
	}
	
	if(ledcount < 3)
	{ 
		ledcount++;	 
	}
	else
	{
		ledcount = 0;
	}	
}

/************************************************************************************
                               更新LED卡号
*************************************************************************************/
void update_LED(void)
{	
	uint16 id;
	id = (card.id&0x1fff);
	lednum[0] = id% 10;
	lednum[1] = id % 100 /10;
	lednum[2] = id % 1000 /100;
	lednum[3] = id / 1000;
	display_count = 0;				//数码管显示计数清零
	nano_rcv_dsp = 0;				//无线接收等显示计数清零
}

/************************************************************************************
                           数码管和灯显示 
*************************************************************************************/
void display(void)
{
	if(lednum[0] == lednum1[0] && lednum[1] == lednum1[1] && \
	lednum[2] == lednum1[2] && lednum[3] == lednum1[3]){
	
		display_count++;
		//如果同一张卡显示了4秒，并且这期间没有再次被读到，则显示----
		//每次读到卡时display_count被清零
		if(display_count > 800){
			display_count = 801;
					
			lednum[0] = 10;
			lednum[1] = 10;
			lednum[2] = 10;
			lednum[3] = 10;
				
		}
	}else{ 
	
		display_count = 0;
	}
	//lednum1用来保存这一次显示值，以便下一次比较前后是不是同一张卡
	lednum1[0] = lednum[0];
	lednum1[1] = lednum[1];
	lednum1[2] = lednum[2];
	lednum1[3] = lednum[3];
	
	//485接受灯亮灭控制，中断接受时把计数清零
	if(jieshouxianshi < 5){
	
		jieshouxianshi++;
		IO1CLR |= 1 << JIESHOU;
	}else{
		jieshouxianshi = 0xff;
		IO1SET |= 1 << JIESHOU;
	}
	//485发送灯亮灭控制，主程序中发送时把计数清零
	if(fasongxianshi < 5){
		fasongxianshi++;
		IO1CLR |= 1 << FASONG;
	}else{
		fasongxianshi = 0xff;
		IO1SET |= 1 << FASONG;
	}
	
	//2420读卡灯亮灭控制、蜂鸣器响灭控制
	if(nano_rcv_dsp < 5){
		nano_rcv_dsp++;
		IO0CLR |= 1 << NANO_RCV_DSP;
		IO1CLR |= BEE;
	}else{
		nano_rcv_dsp = 0xff;
		IO0SET |= 1 << NANO_RCV_DSP;
		IO1SET |= BEE;	
	}

	led_display();
}

/************************************************************************************
                           上电或复位时显示4秒版本号 
*************************************************************************************/
void Ver_LED(void)
{
	lednum[0] = VerL;
	lednum[1] = 11;
	lednum[2] = VerH;
	lednum[3] = 11;
}

/************************************************************************************
                        闪烁显示版本号
*************************************************************************************/
void Scroll_Ver_LED(void)
{
	uint8 i=0;
	uint8 k=2;
	uint8 j;
	while(k--){
		if(i){
			lednum[0] = 14;
			lednum[1] = 14;
			lednum[2] = 14;
			lednum[3] = 14;
		}else{
			lednum[0] = VerL;
			lednum[1] = 11;
			lednum[2] = VerH;
			lednum[3] = 11;
		}
		
		for(j=0;j<50;j++){
			if(i){
				jieshouxianshi=fasongxianshi=nano_rcv_dsp=0;
			}else{
				jieshouxianshi=fasongxianshi=nano_rcv_dsp=0xff;
			}
			
			display();
			delayms(5);
		}
		i = (~i);
	}

}

