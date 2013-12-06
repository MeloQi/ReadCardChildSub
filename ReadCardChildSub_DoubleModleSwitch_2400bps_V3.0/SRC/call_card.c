#include 	"includs.h"

#define CALL_CARD_MAX 20
#define CALL_CARD_LIVE 10											//待呼叫卡生存周期10s
static Call_Card call_card_buf[CALL_CARD_MAX];						//待呼叫卡的暂存区(10s内可连续呼叫20张卡)
Call_Card * call_card_list = null;									//待呼叫卡链表头
Call_Card * call_card_idle_list = null;								//存储区空闲链表头
uint8 call_com[CALL_LEN]={0x88,0x99,0x99,0x99,0xff,0x00,0x00,0x00,0x00};	//呼叫发送给卡的呼叫命令




/*******************************************************************************
** 函数名称:   arry_commp
** 函数功能:  比较两个数组
** 入口参数:  a b 为要比较的数组，len为要比较的长度
** 出口参数:   
** 备    注:  无
*******************************************************************************/
MyBoolT arry_comp(uint8 *a,uint8 *b,uint8 len)//已测ok
{
	uint8 i;
	for(i=0;i<len;){
		if((*(a+i))!=(*(b+i)))
			break;
		i++;
	}
	if(i == len)
		return TRUE;
	else
		return FALSE;
}

/**************************************************************************************
** 函数名称 ：call_card_init()
** 函数功能 ：定卡呼叫初始化
** 入口参数 ：无
** 出口参数 ：
**************************************************************************************/
void call_card_init(void)//已测ok
{
	uint8 i;
	call_card_idle_list = &call_card_buf[0];
	for(i=0;i<CALL_CARD_MAX;i++){							//初始化暂存区并连接成链表
		call_card_buf[i].id = 0;
		call_card_buf[i].live = 0;
		if(0 == i) call_card_buf[i].pre = null;
		else call_card_buf[i].pre = &call_card_buf[(i-1)];
		if((CALL_CARD_MAX-1) == i) call_card_buf[i].next= null;
		else call_card_buf[i].next = &call_card_buf[(i+1)];
	}
}

/**************************************************************************************
** 函数名称 ：malloc_call_card()
** 函数功能 ：分配并初始化一个带呼叫卡空间
** 入口参数 ：无
** 出口参数 ：分配是否成功
**************************************************************************************/
MyBoolT malloc_call_card(uint16 id)//已测ok
{
	Call_Card *call_card_temp;
	if(call_card_idle_list != null){
		call_card_temp = call_card_list;
		call_card_list = call_card_idle_list;				//从空闲链表头取空间
		call_card_idle_list = call_card_idle_list->next;
		call_card_idle_list->pre = null;
		call_card_list->next = call_card_temp;				//将取下来的空间插入到待呼叫链表头
		if(call_card_temp != null){
			call_card_temp->pre = call_card_list;
		}
		call_card_list->pre = null;
		call_card_list->id = id;
		call_card_list->live = CALL_CARD_LIVE;
		return TRUE;
	}else{
		return FALSE;
	}
}

/**************************************************************************************
** 函数名称 ：free_call_card()
** 函数功能 ：释放一个待呼叫卡空间
** 入口参数 ：要释放卡的结构
** 出口参数 ：
**************************************************************************************/
void free_call_card(Call_Card *cur_card)//已测ok
{
	cur_card->id = 0;
	cur_card->live = 0;
	if(cur_card->pre == null){								//释放的空间为待呼叫链表头
		call_card_list = cur_card->next;
		if(call_card_list != null)							
			call_card_list->pre = null;
	}else{													//释放的在链表中或尾部
		cur_card->pre->next = cur_card->next;
		if(cur_card->next != null){
			cur_card->next->pre = cur_card->pre;
		}
	}
	cur_card->next = call_card_idle_list;					//将释放出来的元素重新插入到空闲链表头部
	call_card_idle_list->pre = cur_card;
	call_card_idle_list = cur_card;
	call_card_idle_list->pre = null;
}

/**************************************************************************************
** 函数名称 ：call_card_list_poll()
** 函数功能 ：更新带呼叫卡链表中卡的生存时间，并释放无生存时间的卡
** 入口参数 ：无
** 出口参数 ：
**************************************************************************************/
static uint32 call_card_time =0;							//记录下次遍历待呼叫链表的时间s
#define POLL_TIME 2											//用于配置多久间隔遍历一次,单位s
void call_card_list_poll(void)//已测ok
{
	Call_Card *call_card_temp;
	Call_Card *call_card_temp1;

	if(get_systime_s() >= call_card_time){
		call_card_time = get_systime_s() + POLL_TIME;
		//NANO_ENTER_CRITICAL();
		for(call_card_temp = call_card_list; call_card_temp != null;){
			call_card_temp1 = call_card_temp->next;
			call_card_temp->live = (call_card_temp->live - POLL_TIME);
			if(call_card_temp->live <= 0){
				free_call_card(call_card_temp);
			}
			call_card_temp = call_card_temp1;
		}
		//NANO_EXIT_CRITICAL();
	}		
}

/**************************************************************************************
** 函数名称 ：search_call_card()
** 函数功能 ：搜索特定的卡是否在待呼叫卡链表中
** 入口参数 ：id搜索的卡号,
					opt若为1表示在搜到卡后延长生存时间,为0不延长
** 出口参数 ：是否成功搜索到卡
**************************************************************************************/
MyBoolT search_call_card(uint16 id,uint8 opt)//已测ok
{
	Call_Card *call_card_temp;
	for(call_card_temp = call_card_list; call_card_temp != null;){
		if((0xffff == call_card_temp->id)||(call_card_temp->id == id)){
			if(opt){
				call_card_temp->live = CALL_CARD_LIVE;
			}
			return TRUE;
		}
			
		call_card_temp = call_card_temp->next;
	}
	return FALSE;
}

/**************************************************************************************
** 函数名称 ：detach_call_card()
** 函数功能 ：从待呼叫链表中分离卡号ID为id的空间
** 出口参数 ：要分离卡号是否在待呼叫链表
**************************************************************************************/
MyBoolT detach_call_card(uint16 id)
{
	Call_Card *call_card_temp;
	Call_Card *call_card_temp1;
	for(call_card_temp = call_card_list; call_card_temp != null;){
		call_card_temp1 = call_card_temp->next;
		if((call_card_temp->id == id)){
			free_call_card(call_card_temp);
			return TRUE;	
		}
		call_card_temp = call_card_temp1;
	}
	return FALSE;
}

uint8 comp_ok = 0;														//接收到的数据域祯头正确标志
extern uint8 rState;
extern uint8 buff_rx[];
#define RANGING_END   0
#define	CARDID_MAX			8009			//卡号最大值
/**************************************************************************************
** 函数名称 ：is_com_process()
** 函数功能 ：根据从卡接收到的数据，判断是否为命令，若是则做相应处理
** 出口参数 ：要分离卡号是否在待呼叫链表
**************************************************************************************/
MyBoolT is_com_process(uint16 id)
{	
	/*标识卡发来的命令处理*/
	if((rState == RANGING_END)&&arry_comp(call_com,buff_rx,5)){

		comp_ok = 1;
		switch(buff_rx[CALL_LEN-4]){								
			case 0x00:													//测距结果包命令
			case 0xDD:													//方向性命令
				break;													//以上命令需要搜索待呼叫区,若未搜到则不需要立即回复,否则立即回复呼叫命令
			case 0x11:													//求助命令
				call_com[(CALL_LEN-1)] = 0x11;							//需立即回复确认命令
				return TRUE;
			case 0xFF:													//搜索分站命令
				call_com[(CALL_LEN-1)] = 0xFF;							//需立即回复应答
				return TRUE;											
			case 0x21:													//呼叫确认命令,不需要立即回复数据,也不需要搜索待呼叫卡区
				return FALSE;
			default:
				break;			
		}																	
	}
	
	/*搜索待呼叫区内的卡,若搜到,立即呼叫*/
	if(((id&0x1fff)<CARDID_MAX)&&(TRUE == search_call_card(id,0))){
		call_com[(CALL_LEN-1)] = 0x21;									//呼叫命令
		return TRUE;
	}
	
	return FALSE;
}





