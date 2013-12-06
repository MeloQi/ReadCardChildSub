#ifndef __SUB_H__
#define __SUB_H__


#define MAX_SUB_NUM				4			//最大分站号

//分站类型
#define COMM_STATION			0x00		//普通分站
#define LOCAT_STATION			0x01		//精确定位分站
#define LOCAT_CHILD_STATION		0x02		//精确定位子分站


typedef struct sub_info		//描述分站
{
	uint8 id;					//ID
	uint8 father_id;			//相连的传输分站ID
	uint8 type;					//类型
} Sub;

void subinfo_init(void);
void getsubinfo(void);

#endif

