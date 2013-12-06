#ifndef __SUB_H__
#define __SUB_H__


#define MAX_SUB_NUM				4			//����վ��

//��վ����
#define COMM_STATION			0x00		//��ͨ��վ
#define LOCAT_STATION			0x01		//��ȷ��λ��վ
#define LOCAT_CHILD_STATION		0x02		//��ȷ��λ�ӷ�վ


typedef struct sub_info		//������վ
{
	uint8 id;					//ID
	uint8 father_id;			//�����Ĵ����վID
	uint8 type;					//����
} Sub;

void subinfo_init(void);
void getsubinfo(void);

#endif

