/****************************************Copyright (c)**************************************************
**                              
**                   
**                                 
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			flash.c
** Last modified Date:  2013-08-27
** Last Version:		1.0
** Descriptions:		flash存储用户信息
**
**------------------------------------------------------------------------------------------------------
** Created by:			QI 
** Created date:		2013-08-27
** Version:				1.0
** Descriptions:		flash存储用户信息
**
*******************************************************************************************************
*/
#include  "includs.h"

#define IAP_ENTER_ADR   0x7FFFFFF1  // IAP入口地址定义

/* 定义IAP命令字 */
                                    //   命令           参数
#define     IAP_SELECTOR        50  // 选择扇区     【起始扇区号、结束扇区号】
#define     IAP_RAMTOFLASH      51  // 拷贝数据     【FLASH目标地址、RAM源地址、写入字节数、系统时钟频率】
#define     IAP_ERASESECTOR     52  // 擦除扇区     【起始扇区号、结束扇区号、系统时钟频率】
#define     IAP_BLANKCHK        53  // 查空扇区     【起始扇区号、结束扇区号】
#define     IAP_READPARTID      54  // 读器件ID     【无】
#define     IAP_BOOTCODEID      55  // 读Boot版本号 【无】
#define     IAP_COMPARE         56  // 比较命令     【Flash起始地址、RAM起始地址、需要比较的字节数】

/* 定义IAP返回状态字 */
#define     CMD_SUCCESS          0
#define     INVALID_COMMAND      1
#define     SRC_ADDR_ERROR       2 
#define     DST_ADDR_ERROR       3 
#define     SRC_ADDR_NOT_MAPPED  4
#define     DST_ADDR_NOT_MAPPED  5
#define     COUNT_ERROR          6
#define     INVALID_SECTOR       7
#define     SECTOR_NOT_BLANK     8
#define     SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION 9
#define     COMPARE_ERROR        10
#define     BUSY                 11
 

/* 定义CCLK值大小，单位为KHz */
#define  IAP_FCCLK            11059

/* 定义函数指针 */
void (*IAP_Entry)(uint32 param_tab[], uint32 result_tab[]);

uint32  paramin[8];                         // IAP入口参数缓冲区
uint32  paramout[8];                        // IAP出口参数缓冲区
uint8  flashdata[512];
extern uint8 rssi_biaozhun;
/*
*********************************************************************************************************
** 函数名称：SelSector()
** 函数功能：IAP操作扇区选择，命令代码50。
** 入口参数：sec1        起始扇区
**           sec2        终止扇区
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  SelSector(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_SELECTOR;               // 设置命令字
    paramin[1] = sec1;                       // 设置参数
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序
   
    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：RamToFlash()
** 函数功能：复制RAM的数据到FLASH，命令代码51。
** 入口参数：dst        目标地址，即FLASH起始地址。以512字节为分界
**           src        源地址，即RAM地址。地址必须字对齐
**           no         复制字节个数，为512/1024/4096/8192
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  RamToFlash(uint32 dst, uint32 src, uint32 no)
{  
    paramin[0] = IAP_RAMTOFLASH;             // 设置命令字
    paramin[1] = dst;                        // 设置参数
    paramin[2] = src;
    paramin[3] = no;
    paramin[4] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序
    
    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：EraseSector()
** 函数功能：扇区擦除，命令代码52。
** 入口参数：sec1       起始扇区
**           sec2       终止扇区
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  EraseSector(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_ERASESECTOR;            // 设置命令字
    paramin[1] = sec1;                       // 设置参数
    paramin[2] = sec2;
    paramin[3] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序
   
    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：BlankCHK()
** 函数功能：扇区查空，命令代码53。
** 入口参数：sec1       起始扇区
**           sec2       终止扇区
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  BlankCHK(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_BLANKCHK;               // 设置命令字
    paramin[1] = sec1;                       // 设置参数
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序

    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：ReadParID()
** 函数功能：扇区查空，命令代码53。
** 入口参数：无
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  ReadParID(void)
{  
    paramin[0] = IAP_READPARTID;             // 设置命令字
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序

    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：BootCodeID()
** 函数功能：读取boot代码版本号，命令代码55。
** 入口参数：无
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  BootCodeID(void)
{  
    paramin[0] = IAP_BOOTCODEID;             // 设置命令字
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序

    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：Compare()
** 函数功能：校验数据，命令代码56。
** 入口参数：dst        目标地址，即RAM/FLASH起始地址。地址必须字对齐
**           src        源地址，即FLASH/RAM地址。地址必须字对齐
**           no         复制字节个数，必须能被4整除
** 出口参数：IAP操作状态码
**           IAP返回值(paramout缓冲区)
*******************************************************************************************************
*/
uint32  Compare(uint32 dst, uint32 src, uint32 no)
{  
    paramin[0] = IAP_COMPARE;                // 设置命令字
    paramin[1] = dst;                        // 设置参数
    paramin[2] = src;
    paramin[3] = no;
    (*IAP_Entry)(paramin, paramout);         // 调用IAP服务程序
 
    return(paramout[0]);                     // 返回状态码
}

/*
*******************************************************************************************************
** 函数名称：flash_init()
** 函数功能：IAP函数调用，测试
*******************************************************************************************************

void flash_init1(void)
{  
    uint8 count1;
    uint16 count;
    uint8 data[8];
    
    for(count = 0;count < 512;count++)
    {
    	flashdata[count] = 0x00;
    }
    
	flashdata[1] = 0x11;
	flashdata[2] = 0x22;
	flashdata[3] = 0x33;
	flashdata[4] = 0x44;
  	flashdata[5] = 0x55;
  	flashdata[6] = 0x66;
  	flashdata[7] = 0x77;

    IAP_Entry = (void(*)())IAP_ENTER_ADR;           // 初始化函数指针IAP_Entry
   
    ReadParID();                                    // 读器件ID
    BootCodeID();                                   // 读Boot版本号

  	for(count1 = 0;count1 < 8;count1++)
  	{
  		data[count1] = (*((uint32 *)(0x0001a000 + count1)));
  	}

  	if(data[0] == 0xff && data[1] == 0xff && data[2] == 0xff && data[3] == 0xff && \
  		data[4] == 0xff && data[5] == 0xff && data[6] == 0xff && data[7] == 0xff)
  	{
	  		rssi_biaozhun = 0xd0;
  	}
  	else if(data[1] == 0x11 && data[2] == 0x22 && data[3] == 0x33 && data[4] == 0x44 && \
  		data[5] == 0x55 && data[6] == 0x66 && data[7] == 0x77)
  	{
  	  		rssi_biaozhun = data[0];
  	  					
  	}
  	else
  	{
	  		rssi_biaozhun = 0xd0;	
  	}
} 
*/

#define USER_FLASH_ADDR 0x0001C000										//flash存储用户数据地址13扇区
#define UINIT_RAM_ADDR  0x40000000										//未初始化RAM地址
#define UINIT_RAM_SIZE  0x10											//未初始化RAM大小(单位为字节)

#define IS_HOTSTART1 (*((volatile uint32 *)UINIT_RAM_ADDR))				//两个用于判断是否为热启动
#define IS_HOTSTART2 (*((volatile uint32 *)(UINIT_RAM_ADDR+4)))
#define RESET_TYPE 	 (*((volatile uint32 *)(UINIT_RAM_ADDR+8)))			//记录复位类型

/*
*******************************************************************************************************
** 函数名称：flash_init()
** 函数功能：初始化flash
** 入口参数：无
** 出口参数：无
**          
*******************************************************************************************************
*/
void flash_init(void)
{
	IAP_Entry = (void(*)())IAP_ENTER_ADR;           // 初始化函数指针IAP_Entry
	ReadParID();                                    // 读器件ID
    BootCodeID();                                   // 读Boot版本号
}

/*
*******************************************************************************************************
** 函数名称：powerup_process()
** 函数功能：冷热启动处理
** 入口参数：无
** 出口参数：无
**          
*******************************************************************************************************
*/
void startup_process(void)
{
	User_Info_Flash *readflash;
	User_Info_Flash *writeflash;

	readflash = (User_Info_Flash *)USER_FLASH_ADDR;						//flash地址
	writeflash = (User_Info_Flash *)flashdata;							//暂存flash数据数组

	/*更新User_Info_Flash 结构体的各个字段*/
	if(readflash->is_fistpoweron != 0x12801280){						//判断是否为首次上电
		IS_HOTSTART1 = 0x128A128A;
		IS_HOTSTART2 = 0x128B128B;
		RESET_TYPE = 0;
		writeflash->is_fistpoweron = 0x12801280;						//初始化各个字段
		writeflash->poweron_cnt = 1;
		writeflash->powerup_cnt = 0;
		writeflash->err_type = 0;
	}else{
		if((IS_HOTSTART1 == 0x128A128A)&&(IS_HOTSTART2 == 0x128B128B)){ //复位启动
			writeflash->is_fistpoweron = 0x12801280;
			writeflash->poweron_cnt = readflash->poweron_cnt;
			writeflash->powerup_cnt = (readflash->powerup_cnt+1);
			writeflash->err_type = RESET_TYPE;
		}else{
			IS_HOTSTART1 = 0x128A128A;
			IS_HOTSTART2 = 0x128B128B;
			RESET_TYPE = 0;															//上电启动
			writeflash->is_fistpoweron = 0x12801280;
			writeflash->poweron_cnt = (readflash->poweron_cnt + 1);
			writeflash->powerup_cnt = readflash->powerup_cnt;
			writeflash->err_type = readflash->err_type;
		}
	}

	/*将更新好的数据写入到flash*/
	SelSector(14,14);     												// 选择扇区14
	EraseSector(14,14);   												// 擦除扇区14
	BlankCHK(14,14) ;     												// 查空扇区14
	SelSector(14,14);													// 选择扇区14
	RamToFlash(USER_FLASH_ADDR,(uint32)flashdata, 512);
	
}


/*
*******************************************************************************************************
** 函数名称：read_userinfo_flash()
** 函数功能：获得冷热启动信息
** 入口参数：无
** 出口参数：无
**          
*******************************************************************************************************
*/
User_Info_Flash * read_userinfo_flash(void)
{
	User_Info_Flash *readflash;
	readflash = (User_Info_Flash *)USER_FLASH_ADDR;
	return readflash;
}

/*
*******************************************************************************************************
** 函数名称：read_userinfo_flash()
** 函数功能：清除冷热启动信息
** 入口参数：无
** 出口参数：无
**          
*******************************************************************************************************
*/
void clean_userinfo_flash(void)
{
	//User_Info_Flash *readflash;
	User_Info_Flash *writeflash = (User_Info_Flash *)flashdata;
	
	writeflash->poweron_cnt = 1;
	writeflash->powerup_cnt = 0;
	writeflash->is_fistpoweron = 0x12801280;
	writeflash->err_type = 0;
	
	/*将更新好的数据写入到flash*/
	SelSector(14,14);     												// 选择扇区14
	EraseSector(14,14);   												// 擦除扇区14
	BlankCHK(14,14) ;     												// 查空扇区14
	SelSector(14,14);													// 选择扇区14
	RamToFlash(USER_FLASH_ADDR,(uint32)flashdata, 512);
}


extern uint8 	lednum[];
extern uint16 	display_count; 
extern Sub sub;
/*
*******************************************************************************************************
** 函数名称：test_startup()
** 函数功能：将冷热启动记录显示到数码管上
** 入口参数：无
** 出口参数：无
**          
*******************************************************************************************************
*/
void test_startup(void)
{
	uint32 temp;
	User_Info_Flash *readflash = ((void *)0);
	readflash  = read_userinfo_flash();
	if(readflash == ((void *)0)){}else{
		if(sub.type == COMM_STATION)										//用拨码盘最高位来区分显示冷启动还是热启动						
		temp = readflash->powerup_cnt;
		if(sub.type == LOCAT_STATION)
			temp = readflash->poweron_cnt;
		
		lednum[0] = temp% 10;
		lednum[1] = temp % 100 /10;
		lednum[2] = temp % 1000 /100;
		lednum[3] = temp/ 1000;
		display_count = 0;													//数码管显示计数清零
	}	
}
									