#ifndef __CALL_CARD_H__ 
#define __CALL_CARD_H__

//**˫��ͨѶ�ݴ濨�ṹ
typedef struct communication_card		  		
{
	uint16 id;							//���п�ID	
	int8 live;							//�������ڣ����ڳ�ʱ�жϣ���λ��
	struct communication_card * pre;		//�������ӳ�����
	struct communication_card * next;
} Call_Card;

#define CALL_LEN 9						//���п����ݳ���

MyBoolT detach_call_card(uint16 id);
void call_card_init(void);
MyBoolT malloc_call_card(uint16 id);
void free_call_card(Call_Card *cur_card);
void call_card_list_poll(void);
MyBoolT is_com_process(uint16 id);
MyBoolT search_call_card(uint16 id,uint8 opt);



#endif


