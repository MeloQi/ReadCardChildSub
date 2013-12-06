#ifndef __CALL_CARD_H__ 
#define __CALL_CARD_H__

//**双向通讯暂存卡结构
typedef struct communication_card		  		
{
	uint16 id;							//呼叫卡ID	
	int8 live;							//生存周期，用于超时判断，单位秒
	struct communication_card * pre;		//用于连接成链表
	struct communication_card * next;
} Call_Card;

#define CALL_LEN 9						//呼叫卡数据长度

MyBoolT detach_call_card(uint16 id);
void call_card_init(void);
MyBoolT malloc_call_card(uint16 id);
void free_call_card(Call_Card *cur_card);
void call_card_list_poll(void);
MyBoolT is_com_process(uint16 id);
MyBoolT search_call_card(uint16 id,uint8 opt);



#endif


