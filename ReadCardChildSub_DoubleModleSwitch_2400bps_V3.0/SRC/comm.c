#include 	"includs.h"

Que 		main_que;					//������
Que 		rs485_pipe;					//485����
RangRs_Que 	rangrs_que;					//�������ݴ����
uint16 		mque_pkg_num = 0;			//���������ݰ��ĸ���	

uint8 		main_arr[MQUESIZE];			//����������
uint8 		rs485_arr[RS485_QUESIZE];	//485���ջ���ܵ�

void delayus(uint32 t)
{
	uint32 i; 
    uint32 j; 
    j=Fcclk/6700000 * t; 
    for(i = 0; i < j; i++); 
}

void delayms(uint32 t)
{
	uint32 i; 
    uint32 j; 
    j=Fcclk/6700 * t; 
    for(i = 0; i < j; i++); 
}

/**************************************************************************************
************************************************************************************
**�������� ��ͨ�ö��в�������
************************************************************************************
**************************************************************************************/
//��ʼ������
uint8 InitQue(Que *q,uint16 len,uint8 *arr)
{
	q->front=q->rear=0;
	q->length = len;
	q->elems = arr;
	return TRUE;
}


//���
uint8 InQue(Que *q,uint8 e)
{
	if((q->rear+1)%q->length==q->front)
		return FALSE;	//��
	q->elems[q->rear]=e;
	q->rear=(q->rear+1)%q->length;
	return TRUE;
}

//����
uint8 OutQue(Que *q,uint8 *rs)
{	
	if(q->front==q->rear)
		return FALSE;	//��
	*rs = q->elems[q->front];
	q->front=(q->front+1)%q->length;
	return TRUE;
}

//�����е�Ԫ�ظ���
uint16 QueLen(Que *q)
{
	return (q->rear + q->length - q->front)%q->length;
}


/**************************************************************************************
************************************************************************************
**�������� ���������ݴ���д�����غ���
*************************************************************************************
************************************************************************************/

//��ʼ��
uint8 InitQueue(RangRs_Que *q)
{
	q->front=q->rear=0;
	return TRUE;
}


//���
uint8 InQueue(RangRs_Que *q,RangRsPkgT e)
{
	uint8 	i;
	uint8	len=CARID_LEN+RANGRS_OTHER_LEN+RANGRS_LEN;
	if((q->rear+1)%RANGRS_QUESIZE==q->front){
		return FALSE;	//��
	}
	for(i = 0;i < len;i++){
		q->elems[q->rear][i]=e[i];
	}
	q->rear=(q->rear+1)%RANGRS_QUESIZE;
	return TRUE;
}

//����
uint8 OutQueue(RangRs_Que *q,RangRsPkgT rs)
{
	uint8 	i;
	uint8	len=CARID_LEN+RANGRS_OTHER_LEN+RANGRS_LEN;
	if(q->front==q->rear){
		return FALSE;	//��
	}
	for(i = 0;i < len;i++){
		rs[i] = q->elems[q->front][i];
	}
	//NANO_ENTER_CRITICAL();
	q->front=(q->front+1)%RANGRS_QUESIZE;
	//NANO_EXIT_CRITICAL();
	return TRUE;
}

//�����е�Ԫ�ظ���
uint8 Length(RangRs_Que *q)
{
	return (q->rear + RANGRS_QUESIZE - q->front)%RANGRS_QUESIZE;
}

