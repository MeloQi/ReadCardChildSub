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
** 函数名称 ：self_check_err()
** 函数功能 ：自检
** 入口参数 ：无
** 出口参数 ：
**************************************************************************************/
void self_check_err(void)
{
	//uint32 err_type;
	
	if(time_correct_flag&&((Nano_self_check_cnt >= 1000)||is_boot)){					//5s未收到无线信号Nano自检
		Nano_self_check_cnt = 0;
		if(FALSE == NTRXCheckVerRev()){													//无线自检
			NanoReset();
			if(wireless_module == MODULE_LEFT){wireless_module = MODULE_RIGHT;}else{wireless_module = MODULE_LEFT;}
			if(NtrxInit()){ 															//nano trx初始化
				wireless_on = TRUE;
				InitApplication();														//nano 初始化
			}else{
				Nano_rcved = 0;									
				//time_correct_flag = FALSE;
				wireless_on = FALSE;
				RESET_TYPE = 4;
				//dog_cnt = 0;
				//(*(void(*)())0)();														//重启
			}
			
		}else{RESET_TYPE &= (~4);}														//清除错误								
	}//end if
	if(Nano_rcved){RESET_TYPE &= (~4);}													//若收到无线信号,清除错误

	
	if(rs485self_check_cnt >= 12000){													//1分钟未收到485信号，RS485自检
		rs485self_check_cnt = 0;
		rs485_init();																	//重新初始化
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
				display_count = 0;														//数码管显示计数清零	
				break;
			default:
				break;
		}//end switch
	}
}





