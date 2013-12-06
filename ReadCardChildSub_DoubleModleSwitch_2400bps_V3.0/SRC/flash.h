#ifndef __FLASH_H__ 
#define __FLASH_H__


#define UINIT_RAM_ADDR  0x40000000										//未初始化RAM地址
#define RESET_TYPE 	 (*((volatile uint32 *)(UINIT_RAM_ADDR+8)))			//记录复位类型

typedef	enum	{
	COMM_ERR = 1,														//485命令执行错误
	CARD_ERR = 2,														//卡处理执行错误
	WRIELESS_ERR = 4,													//无线自检错误
	DETECT_ERR = 8														//自检处理执行错误
}	Err_Type;


/*flash存储用户信息的各个字段*/
typedef struct user_info_flash{
	uint32 is_fistpoweron;												//判断是否为第一次上电
	uint32 powerup_cnt;													//复位启动次数
	uint32 poweron_cnt;													//上电启动次数
	uint32 err_type;													//复位原因
} User_Info_Flash;

uint32  SelSector(uint8 sec1, uint8 sec2);
uint32  RamToFlash(uint32 dst, uint32 src, uint32 no);
uint32  EraseSector(uint8 sec1, uint8 sec2);
uint32  BlankCHK(uint8 sec1, uint8 sec2);
uint32  ReadParID(void);
uint32  BootCodeID(void);
uint32  Compare(uint32 dst, uint32 src, uint32 no);
void  	flash_init(void);
void 	startup_process(void);
User_Info_Flash * read_userinfo_flash(void);
void 	clean_userinfo_flash(void);
void test_startup(void);



#endif

