/****************************************Copyright (c)**************************************************
**                              
**                   
**                                
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			main.c
** Last modified Date:  2013-08-27
** Last Version:		1.0
** Descriptions:		������
**
**------------------------------------------------------------------------------------------------------
** Created by:			QI
** Created date:		2013-08-27
** Version:				1.0
** Descriptions:		������
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
uint32 	int_state;									//�ж�״̬
void timer1_init(void);

//#define CLEAN_FLASH
/************************************************************************************
                                     ��ʼ��                                 
*************************************************************************************/
static void init(void)
{
	flash_init();									//flash��ʼ��
#ifdef CLEAN_FLASH
	clean_userinfo_flash();							//���flash�м�¼	
#endif					
	startup_process();								//������������
	RESET_TYPE = 0;									//��λ������������
#ifdef DISTCMP_ENABLE
	init_old_dist();
#endif
	subinfo_init();
	getsubinfo();
	InitQue(&main_que,MQUESIZE,main_arr);			//�����г�ʼ��
	InitQue(&rs485_pipe,RS485_QUESIZE,rs485_arr);	//RS485�ܵ���ʼ��
	InitQueue(&rangrs_que);							//NANO�ܵ���ʼ��
	call_card_init();								//�������ݴ�����ʼ��
	hwclock_init();									//nano time���
	//NtrxInit();										//nano trx���
	//InitApplication();								//nano �û����
	//timer1_init();
	rs485_init();	
	led_init();
	rtc_init();
	Ver_LED();										//��ʾ�汾��
	delayms(100);
	//watchdog_init();
}


#define DEBUG  

extern Sub sub;
extern uint8 dog_flag;
extern uint16 dog_cnt;
uint8 interlock;									//�������
uint8 is_boot = 1;									//�������һ��ѭ��
/************************************************************************************
                                        �� �� ��                                           
*************************************************************************************/
int main (void)
{ 
	init();
	
	for(;;){
		dog_flag=1;									//ι����־
		interlock = 0xA0;
		if(interlock == 0xA0){
			comm_process();							//��λ��������߳�
			interlock++;
		}else{			
			dog_cnt = 0;
			RESET_TYPE = 1;							//��¼��λԭ��
			(*(void(*)())0)();						//��λ
		}
		
		if(interlock == 0xA1){
			card_process();							//�������߳�
			interlock++;
		}else{
			dog_cnt = 0;
			RESET_TYPE = 2;
			(*(void(*)())0)();
		}
		
		if(interlock == 0xA2){
			self_check_err();						//�����Լ�
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
		if(is_boot)									//�״�ִ��ѭ����־
			is_boot = 0;							
//		watchdog_process();							//���Ź������߳�
	}
	
	return(0);
}
/******************************************************************************
                             End Of File
******************************************************************************/
