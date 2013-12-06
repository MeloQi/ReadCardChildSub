#include "includs.h"
#include "ntrxdil.h"
#include "call_card.h"
#include "nnspi.h"

#define UP_HRD_LEN		5							//����������Ϣͷ����
#define SUBID_LEN		1							//��վID����
#define COMM_LEN		1							//�����
#define UP_LEN_LEN		2							//��Ϣͷ����λ����
#define UP_HRD_TOTALLEN	(UP_HRD_LEN+SUBID_LEN+COMM_LEN+UP_LEN_LEN)
#define COMP_TIME_LEN	6							//��ʱ����ʱ���ֶγ���
#define BACK_DATA_MAX   120							//����������ݰ�����PKG_MAXCARDNUM*3 + BACK_DATA_MAX = 21*3 + 120 = 183

static uint8 	comm_data[210];						//RS485������ռ����ݻ���
static uint8 	receive_data_cont = 0;				//485���յ����ֽ���
MyBoolT 		time_correct_flag = FALSE;			//Уʱ��־
MyBoolT 		wireless_on = FALSE;				//�Ƿ��ѿ������߱�־
uint8			wireless_module;					//Ŀǰ�����õ�ģ��

extern Que 		main_que;
extern Que 		rs485_pipe;
extern uint16 	mque_pkg_num;
extern uint8 	sec_write,min_write,hour_write,dom_write,month_write;
extern uint16 	year_read,year_write;
extern uint8 	sec_read,min_read,hour_read,dom_read,month_read;
extern Sub 		sub;
extern uint32 	int_state;
extern MyAddrT broadcast;
extern MyAddrT cardmac;
void	InitApplication(void);
MyBoolT NtrxInit(void);
const uint8 GetCRCLop[]={
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

const uint8 GetCRCHip[]={
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 
0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 
0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 
0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 
0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 
0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 
0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 
0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 
0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40};


/*******CRC����8λ********/
static uint8 GetCRCLo(uint8 iIndex)
{
	return GetCRCLop[(iIndex)];
}

//*******CRC����8λ********/
static uint8 GetCRCHi(uint8 iIndex)
{
	return GetCRCHip[(iIndex)];
}

/**************************************************************************
                                CRC����У������
*****************************************************************************/
static uint16 CRC_16(uint16 lenth)  
{
   uint8 CRC16Hi ;
   uint8 CRC16Lo ;
   uint16 i;
   uint8 iIndex;
   uint8 to;
   CRC16Hi = 0xFF;
   CRC16Lo = 0xFF;   
   
  for(i=0;i<lenth;i++){
   
      iIndex = CRC16Lo^(comm_data[i]);
      to=GetCRCLo(iIndex);
      CRC16Lo = CRC16Hi^to;
      CRC16Hi = GetCRCHi(iIndex);
   }
  return ((CRC16Hi<<8)|CRC16Lo);
}  

/**************************************************************************
                                CRC����У������
*****************************************************************************/
static uint16 CRC_16_readbuffer(uint8 *data,uint16 lenth)  
{
	uint8 CRC16Hi ;
	uint8 CRC16Lo ;
	uint16 i;
	uint8 iIndex;
	uint8 to;
	CRC16Hi = 0xFF;
	CRC16Lo = 0xFF;   
   
	for(i=0;i<lenth;i++)
	{ 
		iIndex = CRC16Lo^(*data);
		to = GetCRCLo(iIndex);
		CRC16Lo = CRC16Hi^to;
		CRC16Hi = GetCRCHi(iIndex);
		data++;
	}

	return((CRC16Hi << 8) | CRC16Lo);
}


/**************************************************************************************
** �������� ��check_crc()
** �������� ���Խ��յ�������У��
** ��ڲ��� ����
** ���ڲ��� ���������� ��ȷ����TRUE�����󷵻�FALSE
**************************************************************************************/
static MyBoolT check_crc()
{
	uint16 crc;
	
	uint8 len = UP_HRD_LEN+SUBID_LEN+COMM_LEN;		//��ҪУ�����ݵĳ���
	if(comm_data[UP_HRD_LEN+SUBID_LEN] == 0x17)
		len += COMP_TIME_LEN;						//��ʱ����У�鳤��Ҫ����ʱ��
	else if(comm_data[UP_HRD_LEN+SUBID_LEN] == 0x21)
		len += 2;									//˫��ͨѶ
	else if(comm_data[UP_HRD_LEN+SUBID_LEN] == 0x20)//�����ʲ���
		len += UP_LEN_LEN + comm_data[7] + (((uint16)comm_data[8])<<8);
	else{}
	
	crc = ((uint16)(comm_data[len+1])<<8) + (uint16)comm_data[len];
	if((crc == CRC_16_readbuffer(comm_data,len)) && (comm_data[UP_HRD_LEN] == sub.id))
		return TRUE;
	else
		return FALSE;
}




/**************************************************************************************
** �������� ��fill_frame_header_upstream()
** �������� ����������λ��������Ϣͷ��5��0xee
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void fill_frame_header_upstream(void)
{
	uint8 count;
	
	for(count = 0;count < UP_HRD_LEN;count++){
		comm_data[count] = 0xee;
	}
}

/**************************************************************************************
** �������� ��fill_frame_subid_upstream()
** �������� �����̽ͷ��
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void fill_frame_subid_upstream(void)
{
	if(sub.type == LOCAT_STATION){					//���̽ͷ��
		comm_data[UP_HRD_LEN] = sub.id | 0x80;		//��ȷ��λ̽ͷ�����λ��1
	}												//��ͨ��վ���øı�,comm_data������
}

/**************************************************************************************
** �������� ��do_14comm_process()
** �������� ������0x14������
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_14comm_process(void)
{	
	uint8 	ppl_num;												//����
	uint16	comm_data_cnt;
	uint16 	temp16;
	uint8 	temp8;
	uint16 	i;
	
	fill_frame_header_upstream();									//�����Ϣͷ��5��0xee
	fill_frame_subid_upstream();									//���̽ͷ��

	/*̽ͷ����*/
	if(RESET_TYPE){	
		read_rtc();
		comm_data_cnt = UP_HRD_LEN+SUBID_LEN+ COMM_LEN;	 			//comm_data�����������Ϣͷβ	
		comm_data[comm_data_cnt] = (TIME_FIELDLEN+PPL_FIELDLEN+3); 	//����Ϊʱ���ֶ�+�����ֶ�
		comm_data[++comm_data_cnt] = 0x00;
		comm_data[++comm_data_cnt] = dom_read;					 	//ʱ��		
		comm_data[++comm_data_cnt] = hour_read;
		comm_data[++comm_data_cnt] = min_read;
		comm_data[++comm_data_cnt] = sec_read; 						//�����λΪ1��ʾ̽ͷ����
		comm_data[++comm_data_cnt] = 0x01;							//����λ
		comm_data[++comm_data_cnt] = (uint8)(8001);					//����(��ʾ��������)
		comm_data[++comm_data_cnt] = (uint8)(8001>>8);			
		comm_data[++comm_data_cnt] = RESET_TYPE;					//��������
		comm_data_cnt=(UP_HRD_TOTALLEN+TIME_FIELDLEN+PPL_FIELDLEN+3);
	    goto	do_14comm_pro_end;
	}
	
	/*��������ȡ����,�������ؿհ�*/
	if(mque_pkg_num > 0){											//���������������ݰ�
																	//�������ʱ�䡢����������Ϣ
		comm_data_cnt = UP_HRD_TOTALLEN;							//comm_data�����������Ϣͷβ	
		while(mque_pkg_num && (comm_data_cnt < BACK_DATA_MAX)){
			temp16 = (main_que.front+TIME_FIELDLEN)%MQUESIZE;		//�ҵ�����λ
			ppl_num = main_que.elems[temp16];						//�������
			for(i=0;i<(ppl_num*CARDINFO_FIELDLEN \
				+TIME_FIELDLEN+PPL_FIELDLEN);i++){					//�Ӷ���ȡһ������
				OutQue(&main_que,&temp8);
				comm_data[comm_data_cnt] = temp8;
				comm_data_cnt++;									//comm_data������Ч���ݼ���
			}
			mque_pkg_num--;											//ȡ��һ�����ݣ����ݰ�������һ
		}
		temp16 = comm_data_cnt - \
				 UP_HRD_TOTALLEN;									//�ܳ���(��������Ϣͷ)
		comm_data[UP_HRD_LEN+SUBID_LEN+COMM_LEN] \
		 = 	(uint8)temp16;
		comm_data[UP_HRD_LEN+SUBID_LEN+COMM_LEN+1] \
		 = 	(uint8)(temp16>>8);

		if(mque_pkg_num > 0)										//�������л������ݰ�������λ��1
			comm_data[UP_HRD_TOTALLEN+2] |= 0x80;
	}				
	else{															//���������������ݰ�
		comm_data[UP_HRD_LEN+SUBID_LEN\
		+ COMM_LEN] = (TIME_FIELDLEN+PPL_FIELDLEN);					//����Ϊʱ���ֶ�+�����ֶ�
		comm_data[UP_HRD_LEN+SUBID_LEN\
		+ COMM_LEN +1] = 0x00;										//���ȵ��ֽ�
		read_rtc();
		comm_data[UP_HRD_TOTALLEN] 	 = dom_read;					//�졢ʱ���֡���
		comm_data[UP_HRD_TOTALLEN+1] = hour_read;
		comm_data[UP_HRD_TOTALLEN+2] = min_read;
		comm_data[UP_HRD_TOTALLEN+3] = sec_read;
		comm_data[UP_HRD_TOTALLEN+TIME_FIELDLEN] =0x00;				//����
		comm_data_cnt=(UP_HRD_TOTALLEN+TIME_FIELDLEN+PPL_FIELDLEN);
	}

do_14comm_pro_end:
	temp16 = CRC_16(comm_data_cnt);
	comm_data[comm_data_cnt] = (uint8)(temp16 & 0xff);
	comm_data_cnt++;
	comm_data[comm_data_cnt] = (uint8)((temp16 >> 8) & 0xff);
	comm_data_cnt++;
	rs485_send(comm_data,comm_data_cnt);							//����			
}


/**************************************************************************************
** �������� ��do_15comm_process()
** �������� ������0x15��ȷ������
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_15comm_process(void)
{
	uint16 	crc;
	uint8 len = UP_HRD_LEN+SUBID_LEN+COMM_LEN;
	
	fill_frame_header_upstream();									//���������Ϣͷ
	crc = CRC_16(len);												//У��
	comm_data[len+1] = (uint8)((crc >> 8) & 0xff);		
	comm_data[len] = (uint8)(crc & 0xff);
	rs485_send(comm_data,(len+2));									//����
}



/**************************************************************************************
** �������� ��do_17comm_process()
** �������� ������0x17�Ŷ�ʱ����
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_17comm_process(void)
{
	uint16 	crc;
	uint8 time_addr = UP_HRD_LEN+SUBID_LEN+COMM_LEN;
	
	year_write = 2000 + comm_data[time_addr];	 
	sub.father_id = comm_data[time_addr];
	month_write = comm_data[time_addr+1];	
	dom_write = comm_data[time_addr+2];
	hour_write = comm_data[time_addr+3];		
	min_write = comm_data[time_addr+4];		
	sec_write = comm_data[time_addr+5]; 
	write_rtc();
	
	if(wireless_on == FALSE){
		
		ENTER_CRITICAL();
		wireless_module = MODULE_LEFT;								//Ĭ�����������ģ��
		if(NtrxInit()){ 											//nano trx��ʼ��
			wireless_on = TRUE;
			InitApplication();										//nano ��ʼ��
		}else{
			NanoReset();
			wireless_module = MODULE_RIGHT;							//�������ģ�����,�л����Ҳ�
			if(NtrxInit()){
				wireless_on = TRUE;
				InitApplication();
			}else{
				NanoReset();
				wireless_module = MODULE_ERR;
				RESET_TYPE = 4;										//��¼��������
				wireless_on = FALSE;
			}
		}
		EXIT_CRITICAL();						
	}

	fill_frame_header_upstream();									//���������Ϣͷ
	
	comm_data[time_addr] = VerL;									//�汾�ŵ�λ
	comm_data[time_addr+1] = VerH;									//�汾�Ÿ�λ

	crc = CRC_16((time_addr+2));
	comm_data[time_addr+3] = (uint8)((crc >> 8) & 0xff);
	comm_data[time_addr+2] = (uint8)(crc & 0xff);

	rs485_send(comm_data,(time_addr+4));							//����
	time_correct_flag = TRUE;										//��ʱ��־��λ

}



/**************************************************************************************
** �������� ��do_18req_time()
** �������� ��0x18�������ʱ
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_18req_time(void)
{
	uint16 	crc;
	
	fill_frame_header_upstream();					//���������Ϣͷ
	comm_data[UP_HRD_LEN] = sub.id;					
	comm_data[UP_HRD_LEN+1] = 0x18;

	crc = CRC_16(UP_HRD_LEN+2);						//���У��
	comm_data[UP_HRD_LEN+3] = (uint8)((crc >> 8) & 0xff);		
	comm_data[UP_HRD_LEN+2] = (uint8)(crc & 0xff);

	rs485_send(comm_data,(UP_HRD_LEN+4));			//����
	
}

/**************************************************************************************
** �������� ��do_19comm_process()
** �������� ��0x19������
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_19comm_process(void)
{
	watchdog_init();								//����
	ENTER_CRITICAL();
	watchdog_fuwei();
	ENTER_CRITICAL();
}

 /**************************************************************************************
 ** �������� ��do_20comm_process()
 ** �������� ��0x20,�����ʲ���
 ** ��ڲ��� ����
 ** ���ڲ��� ��
 **************************************************************************************/
 static void do_20comm_process(void)
 {
 	uint16 	crc;
 	uint16 len = UP_HRD_TOTALLEN + comm_data[7] + (((uint16)comm_data[8])<<8);
	
	crc = CRC_16(len);								//���У��
	comm_data[len+1] = (uint8)((crc >> 8) & 0xff);		
	comm_data[len] = (uint8)(crc & 0xff);
	rs485_send(comm_data,(len+2));					//����
 }



/**************************************************************************************
** �������� ��do_21comm_process()
** �������� ��0x21��˫��ͨѶ
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void do_21comm_process(void)
{
	uint8 	addr;
	uint16  command;
	
	uint16 	crc;
	/*�������*/
	addr = UP_HRD_LEN+SUBID_LEN+COMM_LEN;
	command = ((((uint16)comm_data[addr])<<8) + comm_data[addr+1]);
	if(!(search_call_card(command,1))){
		malloc_call_card(command);					//����
	}

	/*����ȷ������*/
	fill_frame_header_upstream();					//���������Ϣͷ
	comm_data[UP_HRD_LEN] = sub.id;	
	crc = CRC_16(UP_HRD_LEN+2);						//���У��
	comm_data[UP_HRD_LEN+3] = (uint8)((crc >> 8) & 0xff);		
	comm_data[UP_HRD_LEN+2] = (uint8)(crc & 0xff);
	rs485_send(comm_data,(UP_HRD_LEN+4));			//����
												
}


/*
uint32 hold_time = 0;
#define NANO_RCV_DSP 29
*************************************************************************************
** �������� ��IRQ_Timer1()
** �������� ����1ms��ʱ���ж���ִ��˫��ͨѶ
** ��ڲ��� ����
** ���ڲ��� ��
*************************************************************************************
void __irq IRQ_Timer1 (void)
{
	hold_time++;
	if(hold_time < 10000){			//����10s
		IO0CLR |= 1 << NANO_RCV_DSP;
		NTRXSendMessage(comm_dest,help,2);
		
	}else{
		IO0SET |= 1 << NANO_RCV_DSP;			
		hold_time = 0;
		T1TCR = 0;
		T1TC = 0;
		//�л��ŵ�
		//.....
	}
	
						
	T1IR = 0x01;				
	VICVectAddr = 0x00;
}
*/

/**************************************************************************************
** �������� ��parse_comm()
** �������� ����λ���������
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static void parse_comm(void)
{
	if(TRUE == check_crc()){
		if((time_correct_flag == TRUE)){
			switch(comm_data[6]){
			case 0x14:
				if(sub.type == LOCAT_STATION)
					do_14comm_process();
				break;
			case 0x15:
				if(sub.type == LOCAT_STATION)
					do_15comm_process();
				break;
			case 0x17:
				do_17comm_process();
				break;
			case 0x19:
				do_19comm_process();
				break;
			case 0x20:
				if(sub.type == LOCAT_STATION)
					do_20comm_process();
				break;
			case 0x21:
				if(sub.type == LOCAT_STATION)
					do_21comm_process();
				break;
			//case 0x22:
			//	do_22comm_process();
			//	break;
			default:
				break;
			}		
		}else{
			switch(comm_data[6]){
			case 0x17:
				do_17comm_process();
				break;
			case 0x19:
				do_19comm_process();
				break;
			case 0x20:
				if(sub.type == LOCAT_STATION)
					do_20comm_process();
				break;
			default:
				if(sub.type == LOCAT_STATION)
					do_18req_time();
				break;
			}
			
		}
	}
}



/**************************************************************************************
** �������� ��get_comm()
** �������� ����485���չܵ��ж�������֡��comm_data[]
** ��ڲ��� ����
** ���ڲ��� ���ɹ�����һ֡���ݷ���TRUE�����򷵻�FALSE
**************************************************************************************/
static MyBoolT get_comm(void)

{
	uint8  data;
	uint16 comm_length = 0;
	
	while(OutQue(&rs485_pipe,&data) == TRUE){
		if(receive_data_cont > 150){ 				//���һ֡���ܵ����ݳ���150�������������ݲ��ٱ��浽����
			receive_data_cont = 150;
		}
		comm_data[receive_data_cont] = data;		//��������
		
		if(receive_data_cont == 5){ 				//�ҵ�����֡ͷ
			if(comm_data[0] == 0xff && comm_data[1] == 0xff && comm_data[2] == 0xff && \
			comm_data[3] == 0xff && comm_data[4] == 0xff && ((comm_data[5] == sub.id)|| (comm_data[5] == 0x00))){
				receive_data_cont++;
			}else{
				comm_data[0] = comm_data[1];
				comm_data[1] = comm_data[2];
				comm_data[2] = comm_data[3];
				comm_data[3] = comm_data[4];
				comm_data[4] = comm_data[5];
			}
		}else{
			receive_data_cont++;
		}
		
		if(receive_data_cont >=  7){				//��ͬ����������
			switch(comm_data[6]){
				case 0x14:
					comm_length = 9;
					break;
				case 0x15:
					comm_length = 9;
					break;
				case 0x17:
					comm_length = 15;
					break;
				case 0x19:
					comm_length = 9;
					break;
				case 0x20:
					if(receive_data_cont >= 9)
						comm_length = 11 + comm_data[7] + (((uint16)comm_data[8])<<8);
					break;
				case 0x21:
					comm_length = 11;
					break;
				default:
					receive_data_cont = 0;
					break;
			}
		}
													//һ������֡��������ж�
		if((receive_data_cont == comm_length)&&(receive_data_cont != 0)){
			return TRUE;
		}else{
			return FALSE;
		}
	}//end while
//	else{
//		return FALSE;
//	}
	return FALSE;
}

/**************************************************************************************
** �������� ��comm_process()
** �������� ����λ�������
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
void comm_process(void)
{	
	if(get_comm() == TRUE){							//��һ֡����
		parse_comm();								//�����������
		receive_data_cont = 0;						//RS485���ռ�������
	}
	
}

