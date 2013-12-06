#include 	"includs.h"

#define CALL_CARD_MAX 20
#define CALL_CARD_LIVE 10											//�����п���������10s
static Call_Card call_card_buf[CALL_CARD_MAX];						//�����п����ݴ���(10s�ڿ���������20�ſ�)
Call_Card * call_card_list = null;									//�����п�����ͷ
Call_Card * call_card_idle_list = null;								//�洢����������ͷ
uint8 call_com[CALL_LEN]={0x88,0x99,0x99,0x99,0xff,0x00,0x00,0x00,0x00};	//���з��͸����ĺ�������




/*******************************************************************************
** ��������:   arry_commp
** ��������:  �Ƚ���������
** ��ڲ���:  a b ΪҪ�Ƚϵ����飬lenΪҪ�Ƚϵĳ���
** ���ڲ���:   
** ��    ע:  ��
*******************************************************************************/
MyBoolT arry_comp(uint8 *a,uint8 *b,uint8 len)//�Ѳ�ok
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
** �������� ��call_card_init()
** �������� ���������г�ʼ��
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
void call_card_init(void)//�Ѳ�ok
{
	uint8 i;
	call_card_idle_list = &call_card_buf[0];
	for(i=0;i<CALL_CARD_MAX;i++){							//��ʼ���ݴ��������ӳ�����
		call_card_buf[i].id = 0;
		call_card_buf[i].live = 0;
		if(0 == i) call_card_buf[i].pre = null;
		else call_card_buf[i].pre = &call_card_buf[(i-1)];
		if((CALL_CARD_MAX-1) == i) call_card_buf[i].next= null;
		else call_card_buf[i].next = &call_card_buf[(i+1)];
	}
}

/**************************************************************************************
** �������� ��malloc_call_card()
** �������� �����䲢��ʼ��һ�������п��ռ�
** ��ڲ��� ����
** ���ڲ��� �������Ƿ�ɹ�
**************************************************************************************/
MyBoolT malloc_call_card(uint16 id)//�Ѳ�ok
{
	Call_Card *call_card_temp;
	if(call_card_idle_list != null){
		call_card_temp = call_card_list;
		call_card_list = call_card_idle_list;				//�ӿ�������ͷȡ�ռ�
		call_card_idle_list = call_card_idle_list->next;
		call_card_idle_list->pre = null;
		call_card_list->next = call_card_temp;				//��ȡ�����Ŀռ���뵽����������ͷ
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
** �������� ��free_call_card()
** �������� ���ͷ�һ�������п��ռ�
** ��ڲ��� ��Ҫ�ͷſ��Ľṹ
** ���ڲ��� ��
**************************************************************************************/
void free_call_card(Call_Card *cur_card)//�Ѳ�ok
{
	cur_card->id = 0;
	cur_card->live = 0;
	if(cur_card->pre == null){								//�ͷŵĿռ�Ϊ����������ͷ
		call_card_list = cur_card->next;
		if(call_card_list != null)							
			call_card_list->pre = null;
	}else{													//�ͷŵ��������л�β��
		cur_card->pre->next = cur_card->next;
		if(cur_card->next != null){
			cur_card->next->pre = cur_card->pre;
		}
	}
	cur_card->next = call_card_idle_list;					//���ͷų�����Ԫ�����²��뵽��������ͷ��
	call_card_idle_list->pre = cur_card;
	call_card_idle_list = cur_card;
	call_card_idle_list->pre = null;
}

/**************************************************************************************
** �������� ��call_card_list_poll()
** �������� �����´����п������п�������ʱ�䣬���ͷ�������ʱ��Ŀ�
** ��ڲ��� ����
** ���ڲ��� ��
**************************************************************************************/
static uint32 call_card_time =0;							//��¼�´α��������������ʱ��s
#define POLL_TIME 2											//�������ö�ü������һ��,��λs
void call_card_list_poll(void)//�Ѳ�ok
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
** �������� ��search_call_card()
** �������� �������ض��Ŀ��Ƿ��ڴ����п�������
** ��ڲ��� ��id�����Ŀ���,
					opt��Ϊ1��ʾ���ѵ������ӳ�����ʱ��,Ϊ0���ӳ�
** ���ڲ��� ���Ƿ�ɹ���������
**************************************************************************************/
MyBoolT search_call_card(uint16 id,uint8 opt)//�Ѳ�ok
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
** �������� ��detach_call_card()
** �������� ���Ӵ����������з��뿨��IDΪid�Ŀռ�
** ���ڲ��� ��Ҫ���뿨���Ƿ��ڴ���������
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

uint8 comp_ok = 0;														//���յ�����������ͷ��ȷ��־
extern uint8 rState;
extern uint8 buff_rx[];
#define RANGING_END   0
#define	CARDID_MAX			8009			//�������ֵ
/**************************************************************************************
** �������� ��is_com_process()
** �������� �����ݴӿ����յ������ݣ��ж��Ƿ�Ϊ�������������Ӧ����
** ���ڲ��� ��Ҫ���뿨���Ƿ��ڴ���������
**************************************************************************************/
MyBoolT is_com_process(uint16 id)
{	
	/*��ʶ�������������*/
	if((rState == RANGING_END)&&arry_comp(call_com,buff_rx,5)){

		comp_ok = 1;
		switch(buff_rx[CALL_LEN-4]){								
			case 0x00:													//�����������
			case 0xDD:													//����������
				break;													//����������Ҫ������������,��δ�ѵ�����Ҫ�����ظ�,���������ظ���������
			case 0x11:													//��������
				call_com[(CALL_LEN-1)] = 0x11;							//�������ظ�ȷ������
				return TRUE;
			case 0xFF:													//������վ����
				call_com[(CALL_LEN-1)] = 0xFF;							//�������ظ�Ӧ��
				return TRUE;											
			case 0x21:													//����ȷ������,����Ҫ�����ظ�����,Ҳ����Ҫ���������п���
				return FALSE;
			default:
				break;			
		}																	
	}
	
	/*�������������ڵĿ�,���ѵ�,��������*/
	if(((id&0x1fff)<CARDID_MAX)&&(TRUE == search_call_card(id,0))){
		call_com[(CALL_LEN-1)] = 0x21;									//��������
		return TRUE;
	}
	
	return FALSE;
}





