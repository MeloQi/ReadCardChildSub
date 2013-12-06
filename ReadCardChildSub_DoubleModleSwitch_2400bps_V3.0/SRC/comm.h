#ifndef __COMM_H 
#define __COMM_H

#define time_after_eq(a,b)  (((int32)(a) - (int32)(b)) >= 0)		//�����޷���32λ���ݱȽϣ������������

//**�˿ڶ���
#define FASONG 				24		//������ʾ��P0.24
#define JIESHOU 			22		//������ʾ��P1.22


//**����������ֶγ���
#define NANOHDR_LEN			5		//nano��ͷ	����				
#define	CARID_LEN			2		//�����ֶγ���
#define	RANGRS_LEN			2		//�����ֶγ���
#define	RANGRS_OTHER_LEN	2		//�����ֶγ���

#define TIME_FIELDLEN 		4		//ʱ���ֶγ���
#define PPL_FIELDLEN 		1		//�����ֶγ���
#define CARDINFO_FIELDLEN 	3		//����Ϣ�ֶγ���


//**����������
typedef struct card_info			//��������Ϣ
{	uint8	com;					//����
 	uint16 	id;						//ID
	int16	dist;					//����
	uint8	state;					//״̬
} Card;


//**ͨ�ö�������
#define MQUESIZE 		5120		//�����д�С5K
#define RS485_QUESIZE 	30			//485���ջ���ܵ���С

typedef struct queue				//ͨ�ö�������
{
	uint8  *elems;
	uint16 front,rear;
	uint16 length;
} Que;


//**��������������ͼ�����
#define RANGRS_QUESIZE		10		//�����������д�С
typedef 	uint8	RangRsPkgT[CARID_LEN+RANGRS_LEN+RANGRS_OTHER_LEN]; //�����������
typedef struct RangRs_queue		//�����������
{
	RangRsPkgT elems[RANGRS_QUESIZE];	//����Ԫ��Ϊ����	
	uint8 front,rear;
} RangRs_Que;

void delayus(unsigned int t);
void delayms(unsigned int t);

//**ͨ�ö���
uint8 InitQue(Que *q,uint16 len,uint8 *arr);
uint8 InQue(Que *q,uint8 e);
uint8 OutQue(Que *q,uint8 *rs);
uint16 QueLen(Que *q);

//**���������
uint8 InitQueue(RangRs_Que *q);
uint8 InQueue(RangRs_Que *q,RangRsPkgT e);
uint8 OutQueue(RangRs_Que *q,RangRsPkgT rs);
uint8 Length(RangRs_Que *q);

#endif