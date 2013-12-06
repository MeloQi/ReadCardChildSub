#include "includs.h"
#include "nnspi.h"

		uint32 		Nano_self_check_cnt;
		uint32 		rs485self_check_cnt;
extern 	uint16 		dog_cnt;
extern 	uint8 		lednum[];
extern 	uint8 		is_boot;
extern 	uint16 		display_count;
extern 	MyBoolT		time_correct_flag;
extern 	MyBoolT 	wireless_on;
extern 	uint8 		wireless_module;
extern	uint8		Nano_rcved;

MyBoolT NTRXCheckVerRev (void);
MyBoolT NtrxInit(void);
void InitApplication(void);
void InitSPI(void);
void NanoReset(void);


/**************************************************************************************
** �������� ��self_check_err()
** �������� ���Լ�
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
void self_check_err(void)
{
	//uint32 err_type;
	
	if(time_correct_flag&&((Nano_self_check_cnt >= 1000)||is_boot)){					//5sδ�յ������ź�Nano�Լ�
		Nano_self_check_cnt = 0;
		if(FALSE == NTRXCheckVerRev()){													//�����Լ�
			NanoReset();
			if(wireless_module == MODULE_LEFT){wireless_module = MODULE_RIGHT;}else{wireless_module = MODULE_LEFT;}
			if(NtrxInit()){ 															//nano trx��ʼ��
				wireless_on = TRUE;
				InitApplication();														//nano ��ʼ��
			}else{
				Nano_rcved = 0;									
				//time_correct_flag = FALSE;
				wireless_on = FALSE;
				RESET_TYPE = 4;
				//dog_cnt = 0;
				//(*(void(*)())0)();														//����
			}
			
		}else{RESET_TYPE &= (~4);}														//�������								
	}//end if
	if(Nano_rcved){RESET_TYPE &= (~4);}													//���յ������ź�,�������

	
	if(rs485self_check_cnt >= 12000){													//1����δ�յ�485�źţ�RS485�Լ�
		rs485self_check_cnt = 0;
		rs485_init();																	//���³�ʼ��
	}//end if

	if(RESET_TYPE){
		//err_type = RESET_TYPE;
		switch(RESET_TYPE){
			case 1:
				break;
			case 2:
				break;
			case 4:
				lednum[0] = RESET_TYPE;
				lednum[1] = 13;
				lednum[2] = 13;
				lednum[3] = 12;
				display_count = 0;														//�������ʾ��������	
				break;
			default:
				break;
		}//end switch
	}
}





