#ifndef __COMM_H 
#define __COMM_H

#define time_after_eq(a,b)  (((int32)(a) - (int32)(b)) >= 0)		//两个无符号32位数据比较，解决回绕问题

//**端口定义
#define FASONG 				24		//发送显示灯P0.24
#define JIESHOU 			22		//发送显示灯P1.22


//**测距结果包各字段长度
#define NANOHDR_LEN			5		//nano包头	长度				
#define	CARID_LEN			2		//卡号字段长度
#define	RANGRS_LEN			2		//距离字段长度
#define	RANGRS_OTHER_LEN	2		//其他字段长度

#define TIME_FIELDLEN 		4		//时间字段长度
#define PPL_FIELDLEN 		1		//人数字段长度
#define CARDINFO_FIELDLEN 	3		//卡信息字段长度


//**卡类型描述
typedef struct card_info			//描述卡信息
{	uint8	com;					//命令
 	uint16 	id;						//ID
	int16	dist;					//距离
	uint8	state;					//状态
} Card;


//**通用队列描述
#define MQUESIZE 		5120		//主队列大小5K
#define RS485_QUESIZE 	30			//485接收缓冲管道大小

typedef struct queue				//通用队列描述
{
	uint8  *elems;
	uint16 front,rear;
	uint16 length;
} Que;


//**测距结果包数据类型及队列
#define RANGRS_QUESIZE		10		//测距结果缓存队列大小
typedef 	uint8	RangRsPkgT[CARID_LEN+RANGRS_LEN+RANGRS_OTHER_LEN]; //定义测距包类型
typedef struct RangRs_queue		//定义测距包队列
{
	RangRsPkgT elems[RANGRS_QUESIZE];	//队列元素为测距包	
	uint8 front,rear;
} RangRs_Que;

void delayus(unsigned int t);
void delayms(unsigned int t);

//**通用队列
uint8 InitQue(Que *q,uint16 len,uint8 *arr);
uint8 InQue(Que *q,uint8 e);
uint8 OutQue(Que *q,uint8 *rs);
uint16 QueLen(Que *q);

//**测距结果队列
uint8 InitQueue(RangRs_Que *q);
uint8 InQueue(RangRs_Que *q,RangRsPkgT e);
uint8 OutQueue(RangRs_Que *q,RangRsPkgT rs);
uint8 Length(RangRs_Que *q);

#endif