#include "includs.h"
#include "config.h"
#include "ntrxtypes.h"
#include "hwclock.h"
#include "ntrxutil.h"

#ifndef __OPTIMIZE__
#define __OPTIMIZE__ 0
#endif

#define	MILLISECONDS_PER_TICK	5 		// 5ms  ʱ��
#define	STATE_HIGH	5
#define	STATE_LOW	-5

extern MyDword32T calDelay;
extern MyInt16T rcwd;
extern MyDword32T tiRecal;

uint16 card_out_timer = 0;				//����ʱ���ʱ
uint16 package_timer = 0;				//���ʱ���ʱ
uint16 dog_cnt = 0;						//ι��ʱ���ʱ

MyBoolT	key_flags[NKEYS] = {
	FALSE, FALSE, FALSE, FALSE
};

MyDword32T	jiffies = 0;
extern  MyDword32T tiRecal;
extern uint8 dog_flag;

extern uint32 Nano_self_check_cnt;
extern uint32 rs485self_check_cnt;


/**
 * ���ü���ȡϵͳʱ�亯��
 *
 * 
 *
 * 
 */
static uint16 tick_ms = 0;				//ϵͳʱ��ĵ�λms						 
static uint32 sys_time = 0;				//ϵͳʱ�䵥λs
void set_systime(uint32 s,uint16 ms)
{
	sys_time = s;
	tick_ms = ms;
}

uint32 get_systime_s(void)
{
	return sys_time;
}

uint16 get_systime_ms(void)
{
	return tick_ms;
}
/**
 * IRQ_Timer0:
 *
 * 5ms��ʱ���ж�
 *
 * Returns: none.
 */
void __irq IRQ_Timer0 (void)
{
	package_timer++;					//���ʱ��
	card_out_timer++;					//������ʱ��
	dog_cnt++;							//ι��ʱ��
	Nano_self_check_cnt++;				//nano�����Լ�ʱ��
	rs485self_check_cnt++;				//rs485�Լ�ʱ��
//	if(package_timer > 3200){
//		package_timer = 3201;
//	}
//	if(card_out_timer > 3200){
//		card_out_timer = 3201;
//	}

	tick_ms += MILLISECONDS_PER_TICK;	//ϵͳʱ��
	if(tick_ms >= 1000){
		sys_time++;
		tick_ms = 0;
	}
	
	jiffies += MILLISECONDS_PER_TICK;	

	if((dog_cnt > 200)&&dog_flag){
		watchdog_weigou();				//ι��1s
		dog_flag = 0;
	}	
	display();					
	T0IR = 0x01;				
	VICVectAddr = 0x00;
}

/**
 * timer0_init:
 *
 * 5ms��ʱ����ʼ��
 *
 * Returns: none.
 */
void timer0_init(void)
{
	T0TC   = 0;			
	T0PR   = 0;			
	T0MCR  = 0x03;		
	T0MR0  = Fpclk/200;		
	T0TCR  = 0x01;		

	VICVectCntl1 = 0x20 | 0x04;			
	VICVectAddr1 = (uint32)IRQ_Timer0;	
	T0IR = 0x01;
	VICIntEnable |= 1 << 0x04;
}

/**
 * hwdelay:
 *
 * Delays the CPU @t microseconds.
 *
 * Returns: none.
 */
void	hwdelay(MyDword32T t)
{
  unsigned long lc;
  unsigned char timercount;

  for(lc = 0;lc < t;lc++)
  {
    for(timercount = 0;timercount < 5;timercount++);
  }
}


void hwclockRestart (MyDword32T start)
{
  
  timer0_init();
  jiffies = start;
}

/**
 * hwclock_init:
 *
 * clock tick initialization function.
 *
 * Returns: none.
 */
void hwclock_init(void)
{
	hwclockRestart (0);
}


/**
 * hwclock:
 *
 * clock tick function.
 *
 * Returns: The elapsed time since program start in milliseconds.
 */
MyDword32T	hwclock(void)
{
	return	jiffies;
}

