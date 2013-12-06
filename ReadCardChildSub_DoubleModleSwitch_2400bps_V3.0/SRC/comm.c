#include 	"includs.h"

Que 		main_que;					//主队列
Que 		rs485_pipe;					//485队列
RangRs_Que 	rangrs_que;					//卡接收暂存队列
uint16 		mque_pkg_num = 0;			//主队列数据包的个数	

uint8 		main_arr[MQUESIZE];			//主队列数组
uint8 		rs485_arr[RS485_QUESIZE];	//485接收缓冲管道

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
**函数功能 ：通用队列操作函数
************************************************************************************
**************************************************************************************/
//初始化队列
uint8 InitQue(Que *q,uint16 len,uint8 *arr)
{
	q->front=q->rear=0;
	q->length = len;
	q->elems = arr;
	return TRUE;
}


//入队
uint8 InQue(Que *q,uint8 e)
{
	if((q->rear+1)%q->length==q->front)
		return FALSE;	//满
	q->elems[q->rear]=e;
	q->rear=(q->rear+1)%q->length;
	return TRUE;
}

//出队
uint8 OutQue(Que *q,uint8 *rs)
{	
	if(q->front==q->rear)
		return FALSE;	//空
	*rs = q->elems[q->front];
	q->front=(q->front+1)%q->length;
	return TRUE;
}

//队列中的元素个数
uint16 QueLen(Que *q)
{
	return (q->rear + q->length - q->front)%q->length;
}


/**************************************************************************************
************************************************************************************
**函数功能 ：卡接收暂存队列处理相关函数
*************************************************************************************
************************************************************************************/

//初始化
uint8 InitQueue(RangRs_Que *q)
{
	q->front=q->rear=0;
	return TRUE;
}


//入队
uint8 InQueue(RangRs_Que *q,RangRsPkgT e)
{
	uint8 	i;
	uint8	len=CARID_LEN+RANGRS_OTHER_LEN+RANGRS_LEN;
	if((q->rear+1)%RANGRS_QUESIZE==q->front){
		return FALSE;	//满
	}
	for(i = 0;i < len;i++){
		q->elems[q->rear][i]=e[i];
	}
	q->rear=(q->rear+1)%RANGRS_QUESIZE;
	return TRUE;
}

//出队
uint8 OutQueue(RangRs_Que *q,RangRsPkgT rs)
{
	uint8 	i;
	uint8	len=CARID_LEN+RANGRS_OTHER_LEN+RANGRS_LEN;
	if(q->front==q->rear){
		return FALSE;	//空
	}
	for(i = 0;i < len;i++){
		rs[i] = q->elems[q->front][i];
	}
	//NANO_ENTER_CRITICAL();
	q->front=(q->front+1)%RANGRS_QUESIZE;
	//NANO_EXIT_CRITICAL();
	return TRUE;
}

//队列中的元素个数
uint8 Length(RangRs_Que *q)
{
	return (q->rear + RANGRS_QUESIZE - q->front)%RANGRS_QUESIZE;
}

