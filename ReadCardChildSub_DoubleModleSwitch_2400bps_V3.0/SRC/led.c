#include "includs.h"

/*******************************�궨��Ͳ��ֱ�������*************************************/
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
uint16 	fasongxianshi = 0xff,jieshouxianshi = 0xff;  //RS485���շ��͵���ʾ����
uint8 	ledcount;				//�����ˢ�¼���������������һ��ˢ�µ�λѡ
uint8 	lednum[4];
uint8 	lednum1[4];
uint8 	nano_rcv_dsp = 0xff;
uint16 	display_count;
const uint8 LedShow[15]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xbf,0xC1,0x86,0x88,0xFF}; //�����0-9��U��E��R�ı���

/****************************************************************************
* �������ƣ�void led_init(void)
* �������ܣ�led��ʼ��
* ��ڲ�������
* ���ڲ�������
* �޸�ʱ�䣺
* ��    ע��
****************************************************************************/
void led_init(void)
{
	lednum[0] = 10;//�ϵ��������ʾ----
	lednum[1] = 10;
	lednum[2] = 10;
	lednum[3] = 10;	
	
	lednum1[0] = 10;
	lednum1[1] = 10;
	lednum1[2] = 10;
	lednum1[3] = 10;
	
	PINSEL0 &= 0xffffff0f;//����P0.2��P0.3��ΪGPIO��
	IO0DIR |= (DAT | SCK);//����P0.2��P0.3��Ϊ���
	
	PINSEL1 &= 0xf3ffffff;
	IO0DIR |= (1<<NANO_RCV_DSP);
	IO0SET |= (1<<NANO_RCV_DSP);
	
	IO1DIR |= BEE;
	IO1SET |= BEE;

	IO0CLR |= SCK;//P0.2�ڣ�����ʱʱ����Ϊ�͵�ƽ
	IO0CLR |= DAT;//P0.3�ڣ�����ʱ������Ϊ�͵�ƽ
}

/****************************************************************************
* �������ƣ�void led_display(void)
* �������ܣ�led��ʾ
* ��ڲ�������
* ���ڲ�������
* �޸�ʱ�䣺
* ��    ע����̬��ʾģʽ��ǰ��ˢ�¼��ʱ�����Ҫ����һ����ֵ��
*           ������ʾЧ�������ۿۡ�4λ�����ɨ�裬����ˢ�¼��ʱ��4���롣
****************************************************************************/
static void led_display(void)
{
	uint8 temp,temp1;//�м����
	
	temp1 = ~(1 << ledcount);//λѡ
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
			
	temp1 = LedShow[lednum[ledcount]];//��ѡ
	
//	if(ledcount == 2)				//С����
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
                               ����LED����
*************************************************************************************/
void update_LED(void)
{	
	uint16 id;
	id = (card.id&0x1fff);
	lednum[0] = id% 10;
	lednum[1] = id % 100 /10;
	lednum[2] = id % 1000 /100;
	lednum[3] = id / 1000;
	display_count = 0;				//�������ʾ��������
	nano_rcv_dsp = 0;				//���߽��յ���ʾ��������
}

/************************************************************************************
                           ����ܺ͵���ʾ 
*************************************************************************************/
void display(void)
{
	if(lednum[0] == lednum1[0] && lednum[1] == lednum1[1] && \
	lednum[2] == lednum1[2] && lednum[3] == lednum1[3]){
	
		display_count++;
		//���ͬһ�ſ���ʾ��4�룬�������ڼ�û���ٴα�����������ʾ----
		//ÿ�ζ�����ʱdisplay_count������
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
	//lednum1����������һ����ʾֵ���Ա���һ�αȽ�ǰ���ǲ���ͬһ�ſ�
	lednum1[0] = lednum[0];
	lednum1[1] = lednum[1];
	lednum1[2] = lednum[2];
	lednum1[3] = lednum[3];
	
	//485���ܵ�������ƣ��жϽ���ʱ�Ѽ�������
	if(jieshouxianshi < 5){
	
		jieshouxianshi++;
		IO1CLR |= 1 << JIESHOU;
	}else{
		jieshouxianshi = 0xff;
		IO1SET |= 1 << JIESHOU;
	}
	//485���͵�������ƣ��������з���ʱ�Ѽ�������
	if(fasongxianshi < 5){
		fasongxianshi++;
		IO1CLR |= 1 << FASONG;
	}else{
		fasongxianshi = 0xff;
		IO1SET |= 1 << FASONG;
	}
	
	//2420������������ơ��������������
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
                           �ϵ��λʱ��ʾ4��汾�� 
*************************************************************************************/
void Ver_LED(void)
{
	lednum[0] = VerL;
	lednum[1] = 11;
	lednum[2] = VerH;
	lednum[3] = 11;
}

/************************************************************************************
                        ��˸��ʾ�汾��
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

