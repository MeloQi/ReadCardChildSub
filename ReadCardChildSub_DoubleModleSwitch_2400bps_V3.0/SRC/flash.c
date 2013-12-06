/****************************************Copyright (c)**************************************************
**                              
**                   
**                                 
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			flash.c
** Last modified Date:  2013-08-27
** Last Version:		1.0
** Descriptions:		flash�洢�û���Ϣ
**
**------------------------------------------------------------------------------------------------------
** Created by:			QI 
** Created date:		2013-08-27
** Version:				1.0
** Descriptions:		flash�洢�û���Ϣ
**
*******************************************************************************************************
*/
#include  "includs.h"

#define IAP_ENTER_ADR   0x7FFFFFF1  // IAP��ڵ�ַ����

/* ����IAP������ */
                                    //   ����           ����
#define     IAP_SELECTOR        50  // ѡ������     ����ʼ�����š����������š�
#define     IAP_RAMTOFLASH      51  // ��������     ��FLASHĿ���ַ��RAMԴ��ַ��д���ֽ�����ϵͳʱ��Ƶ�ʡ�
#define     IAP_ERASESECTOR     52  // ��������     ����ʼ�����š����������š�ϵͳʱ��Ƶ�ʡ�
#define     IAP_BLANKCHK        53  // �������     ����ʼ�����š����������š�
#define     IAP_READPARTID      54  // ������ID     ���ޡ�
#define     IAP_BOOTCODEID      55  // ��Boot�汾�� ���ޡ�
#define     IAP_COMPARE         56  // �Ƚ�����     ��Flash��ʼ��ַ��RAM��ʼ��ַ����Ҫ�Ƚϵ��ֽ�����

/* ����IAP����״̬�� */
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
 

/* ����CCLKֵ��С����λΪKHz */
#define  IAP_FCCLK            11059

/* ���庯��ָ�� */
void (*IAP_Entry)(uint32 param_tab[], uint32 result_tab[]);

uint32  paramin[8];                         // IAP��ڲ���������
uint32  paramout[8];                        // IAP���ڲ���������
uint8  flashdata[512];
extern uint8 rssi_biaozhun;
/*
*********************************************************************************************************
** �������ƣ�SelSector()
** �������ܣ�IAP��������ѡ���������50��
** ��ڲ�����sec1        ��ʼ����
**           sec2        ��ֹ����
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  SelSector(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_SELECTOR;               // ����������
    paramin[1] = sec1;                       // ���ò���
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������
   
    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�RamToFlash()
** �������ܣ�����RAM�����ݵ�FLASH���������51��
** ��ڲ�����dst        Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�
**           src        Դ��ַ����RAM��ַ����ַ�����ֶ���
**           no         �����ֽڸ�����Ϊ512/1024/4096/8192
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  RamToFlash(uint32 dst, uint32 src, uint32 no)
{  
    paramin[0] = IAP_RAMTOFLASH;             // ����������
    paramin[1] = dst;                        // ���ò���
    paramin[2] = src;
    paramin[3] = no;
    paramin[4] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������
    
    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�EraseSector()
** �������ܣ������������������52��
** ��ڲ�����sec1       ��ʼ����
**           sec2       ��ֹ����
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  EraseSector(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_ERASESECTOR;            // ����������
    paramin[1] = sec1;                       // ���ò���
    paramin[2] = sec2;
    paramin[3] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������
   
    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�BlankCHK()
** �������ܣ�������գ��������53��
** ��ڲ�����sec1       ��ʼ����
**           sec2       ��ֹ����
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  BlankCHK(uint8 sec1, uint8 sec2)
{  
    paramin[0] = IAP_BLANKCHK;               // ����������
    paramin[1] = sec1;                       // ���ò���
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������

    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�ReadParID()
** �������ܣ�������գ��������53��
** ��ڲ�������
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  ReadParID(void)
{  
    paramin[0] = IAP_READPARTID;             // ����������
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������

    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�BootCodeID()
** �������ܣ���ȡboot����汾�ţ��������55��
** ��ڲ�������
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  BootCodeID(void)
{  
    paramin[0] = IAP_BOOTCODEID;             // ����������
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������

    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�Compare()
** �������ܣ�У�����ݣ��������56��
** ��ڲ�����dst        Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���
**           src        Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���
**           no         �����ֽڸ����������ܱ�4����
** ���ڲ�����IAP����״̬��
**           IAP����ֵ(paramout������)
*******************************************************************************************************
*/
uint32  Compare(uint32 dst, uint32 src, uint32 no)
{  
    paramin[0] = IAP_COMPARE;                // ����������
    paramin[1] = dst;                        // ���ò���
    paramin[2] = src;
    paramin[3] = no;
    (*IAP_Entry)(paramin, paramout);         // ����IAP�������
 
    return(paramout[0]);                     // ����״̬��
}

/*
*******************************************************************************************************
** �������ƣ�flash_init()
** �������ܣ�IAP�������ã�����
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

    IAP_Entry = (void(*)())IAP_ENTER_ADR;           // ��ʼ������ָ��IAP_Entry
   
    ReadParID();                                    // ������ID
    BootCodeID();                                   // ��Boot�汾��

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

#define USER_FLASH_ADDR 0x0001C000										//flash�洢�û����ݵ�ַ13����
#define UINIT_RAM_ADDR  0x40000000										//δ��ʼ��RAM��ַ
#define UINIT_RAM_SIZE  0x10											//δ��ʼ��RAM��С(��λΪ�ֽ�)

#define IS_HOTSTART1 (*((volatile uint32 *)UINIT_RAM_ADDR))				//���������ж��Ƿ�Ϊ������
#define IS_HOTSTART2 (*((volatile uint32 *)(UINIT_RAM_ADDR+4)))
#define RESET_TYPE 	 (*((volatile uint32 *)(UINIT_RAM_ADDR+8)))			//��¼��λ����

/*
*******************************************************************************************************
** �������ƣ�flash_init()
** �������ܣ���ʼ��flash
** ��ڲ�������
** ���ڲ�������
**          
*******************************************************************************************************
*/
void flash_init(void)
{
	IAP_Entry = (void(*)())IAP_ENTER_ADR;           // ��ʼ������ָ��IAP_Entry
	ReadParID();                                    // ������ID
    BootCodeID();                                   // ��Boot�汾��
}

/*
*******************************************************************************************************
** �������ƣ�powerup_process()
** �������ܣ�������������
** ��ڲ�������
** ���ڲ�������
**          
*******************************************************************************************************
*/
void startup_process(void)
{
	User_Info_Flash *readflash;
	User_Info_Flash *writeflash;

	readflash = (User_Info_Flash *)USER_FLASH_ADDR;						//flash��ַ
	writeflash = (User_Info_Flash *)flashdata;							//�ݴ�flash��������

	/*����User_Info_Flash �ṹ��ĸ����ֶ�*/
	if(readflash->is_fistpoweron != 0x12801280){						//�ж��Ƿ�Ϊ�״��ϵ�
		IS_HOTSTART1 = 0x128A128A;
		IS_HOTSTART2 = 0x128B128B;
		RESET_TYPE = 0;
		writeflash->is_fistpoweron = 0x12801280;						//��ʼ�������ֶ�
		writeflash->poweron_cnt = 1;
		writeflash->powerup_cnt = 0;
		writeflash->err_type = 0;
	}else{
		if((IS_HOTSTART1 == 0x128A128A)&&(IS_HOTSTART2 == 0x128B128B)){ //��λ����
			writeflash->is_fistpoweron = 0x12801280;
			writeflash->poweron_cnt = readflash->poweron_cnt;
			writeflash->powerup_cnt = (readflash->powerup_cnt+1);
			writeflash->err_type = RESET_TYPE;
		}else{
			IS_HOTSTART1 = 0x128A128A;
			IS_HOTSTART2 = 0x128B128B;
			RESET_TYPE = 0;															//�ϵ�����
			writeflash->is_fistpoweron = 0x12801280;
			writeflash->poweron_cnt = (readflash->poweron_cnt + 1);
			writeflash->powerup_cnt = readflash->powerup_cnt;
			writeflash->err_type = readflash->err_type;
		}
	}

	/*�����ºõ�����д�뵽flash*/
	SelSector(14,14);     												// ѡ������14
	EraseSector(14,14);   												// ��������14
	BlankCHK(14,14) ;     												// �������14
	SelSector(14,14);													// ѡ������14
	RamToFlash(USER_FLASH_ADDR,(uint32)flashdata, 512);
	
}


/*
*******************************************************************************************************
** �������ƣ�read_userinfo_flash()
** �������ܣ��������������Ϣ
** ��ڲ�������
** ���ڲ�������
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
** �������ƣ�read_userinfo_flash()
** �������ܣ��������������Ϣ
** ��ڲ�������
** ���ڲ�������
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
	
	/*�����ºõ�����д�뵽flash*/
	SelSector(14,14);     												// ѡ������14
	EraseSector(14,14);   												// ��������14
	BlankCHK(14,14) ;     												// �������14
	SelSector(14,14);													// ѡ������14
	RamToFlash(USER_FLASH_ADDR,(uint32)flashdata, 512);
}


extern uint8 	lednum[];
extern uint16 	display_count; 
extern Sub sub;
/*
*******************************************************************************************************
** �������ƣ�test_startup()
** �������ܣ�������������¼��ʾ���������
** ��ڲ�������
** ���ڲ�������
**          
*******************************************************************************************************
*/
void test_startup(void)
{
	uint32 temp;
	User_Info_Flash *readflash = ((void *)0);
	readflash  = read_userinfo_flash();
	if(readflash == ((void *)0)){}else{
		if(sub.type == COMM_STATION)										//�ò��������λ��������ʾ����������������						
		temp = readflash->powerup_cnt;
		if(sub.type == LOCAT_STATION)
			temp = readflash->poweron_cnt;
		
		lednum[0] = temp% 10;
		lednum[1] = temp % 100 /10;
		lednum[2] = temp % 1000 /100;
		lednum[3] = temp/ 1000;
		display_count = 0;													//�������ʾ��������
	}	
}
									