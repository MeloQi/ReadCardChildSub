#include "includs.h"

#define DOG_TIME	200						//ι��ʱ��200*5ms = 1s

extern uint16 	dog_cnt;
extern uint32 	int_state;

/**************************************************************************************
** �������� ��watchdog_process()
** �������� ����ʱι��
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
void watchdog_process(void)
{
	if(dog_cnt > DOG_TIME){
		dog_cnt = 0;
		ENTER_CRITICAL();					//ι��
		watchdog_weigou();
		EXIT_CRITICAL();
	}
}

