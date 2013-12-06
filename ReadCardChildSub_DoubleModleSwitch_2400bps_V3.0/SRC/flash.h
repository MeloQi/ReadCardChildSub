#ifndef __FLASH_H__ 
#define __FLASH_H__


#define UINIT_RAM_ADDR  0x40000000										//δ��ʼ��RAM��ַ
#define RESET_TYPE 	 (*((volatile uint32 *)(UINIT_RAM_ADDR+8)))			//��¼��λ����

typedef	enum	{
	COMM_ERR = 1,														//485����ִ�д���
	CARD_ERR = 2,														//������ִ�д���
	WRIELESS_ERR = 4,													//�����Լ����
	DETECT_ERR = 8														//�Լ촦��ִ�д���
}	Err_Type;


/*flash�洢�û���Ϣ�ĸ����ֶ�*/
typedef struct user_info_flash{
	uint32 is_fistpoweron;												//�ж��Ƿ�Ϊ��һ���ϵ�
	uint32 powerup_cnt;													//��λ��������
	uint32 poweron_cnt;													//�ϵ���������
	uint32 err_type;													//��λԭ��
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

