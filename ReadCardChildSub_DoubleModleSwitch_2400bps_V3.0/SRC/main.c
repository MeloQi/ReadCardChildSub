/****************************************Copyright (c)**************************************************
**                              
**                   
**                                
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			main.c
** Last modified Date:  2013-08-27
** Last Version:		1.0
** Descriptions:		主程序
**
**------------------------------------------------------------------------------------------------------
** Created by:			QI
** Created date:		2013-08-27
** Version:				1.0
** Descriptions:		主程序
**
*******************************************************************************************************
*/

#include "includs.h"

extern Que 			main_que;
extern Que 			rs485_pipe;
extern RangRs_Que 	rangrs_que;
extern uint8 		main_arr[];	
extern uint8 		rs485_arr[];
extern MyBoolT NtrxInit(void);	
extern void	InitApplication(void);
extern void comm_process(void);
uint32 	int_state;									//中断状态
void timer1_init(void);

//#define CLEAN_FLASH
/************************************************************************************
                                     初始化                                 
*************************************************************************************/
static void init(void)
{
	flash_init();									//flash初始化
#ifdef CLEAN_FLASH
	clean_userinfo_flash();							//清除flash中记录	
#endif					
	startup_process();								//冷热启动处理
	RESET_TYPE = 0;									//复位错误类型清零
#ifdef DISTCMP_ENABLE
	init_old_dist();
#endif
	subinfo_init();
	getsubinfo();
	InitQue(&main_que,MQUESIZE,main_arr);			//主队列初始化
	InitQue(&rs485_pipe,RS485_QUESIZE,rs485_arr);	//RS485管道初始化
	InitQueue(&rangrs_que);							//NANO管道初始化
	call_card_init();								//卡呼叫暂存区初始化
	hwclock_init();									//nano time相关
	//NtrxInit();										//nano trx相关
	//InitApplication();								//nano 用户相关
	//timer1_init();
	rs485_init();	
	led_init();
	rtc_init();
	Ver_LED();										//显示版本号
	delayms(100);
	//watchdog_init();
}


#define DEBUG  

extern Sub sub;
extern uint8 dog_flag;
extern uint16 dog_cnt;
uint8 interlock;									//环环相扣
uint8 is_boot = 1;									//启动后第一次循环
/************************************************************************************
                                        主 函 数                                           
*************************************************************************************/
int main (void)
{ 
	init();
	
	for(;;){
		dog_flag=1;									//喂狗标志
		interlock = 0xA0;
		if(interlock == 0xA0){
			comm_process();							//上位机命令处理线程
			interlock++;
		}else{			
			dog_cnt = 0;
			RESET_TYPE = 1;							//记录复位原因
			(*(void(*)())0)();						//复位
		}
		
		if(interlock == 0xA1){
			card_process();							//卡处理线程
			interlock++;
		}else{
			dog_cnt = 0;
			RESET_TYPE = 2;
			(*(void(*)())0)();
		}
		
		if(interlock == 0xA2){
			self_check_err();						//错误自检
			interlock++;
		}else{
			dog_cnt = 0;
			RESET_TYPE = 8;
			(*(void(*)())0)();
		}
		
#ifdef DEBUG
		if(sub.id == 0x0f){
			test_startup();
		}
#endif
		if(is_boot)									//首次执行循环标志
			is_boot = 0;							
//		watchdog_process();							//看门狗处理线程
	}
	
	return(0);
}
/******************************************************************************
                             End Of File
******************************************************************************/
