#include "includs.h"
		
		
//#define DISTCMP_ENABLE						//是否进行距离过滤

#define DIS_DIF_CAL 		0 				//当前距离与历史距离阀值

#define	FIND_CARD 			0xffff  		//用于搜索当前包中卡
#define PKG_MAXCARDNUM 		20				//包中最大卡数
#define	CARDID_MAX			8009			//卡号最大值
#define PKG_TIME 			600 			//打包时间= (PKG_TIME*5/1000)s
#define CARDOUT_TIME		4000			//卡出时间=(CARDOUT_TIME*5/1000)s

#define OLD_DIST_SIZE		4005			//存储历史距离数组的大小
#define CARDINOUT_SIZE 		1005			//卡进出状态数组大小

Card 		  card;							//用于存储当前卡信息 
static uint16 curpkg_ppl_addr;				//当前包人数位地址
static uint16 curpkg_start_addr = 0;		//当前包开始位置
static uint8  is_fistcard = TRUE;			//第一次接受到卡标志
static uint16 curpkg_cardcnt = 0;			//前包卡数计数(ZI段已初始化为0)

#ifdef DISTCMP_ENABLE
static uint8  old_dist[OLD_DIST_SIZE];		//存储历史距离
#endif

static uint8  curcard_insta[CARDINOUT_SIZE];//当前卡进出状态
static uint8  precard_insta[CARDINOUT_SIZE];//历史卡进出状态
RangRsPkgT rangrs_one_pkg;					//暂存一个测距包数据


extern Que 			main_que;
extern RangRs_Que 	rangrs_que;
extern uint16 		mque_pkg_num;
extern uint8 		sec_read,min_read,hour_read,dom_read,month_read;
extern Sub 			sub;
extern uint16 card_out_timer;				
extern uint16 package_timer;				
extern MyDword32T calDelay;
extern MyInt16T rcwd;
extern MyDword32T tiRecal;
extern MyDword32T	jiffies;
extern MyBoolT 	wireless_on;
extern void NTRXAllCalibration (void);

#ifdef DISTCMP_ENABLE
/**************************************************************************************
** 函数名称 ：init_old_dist()
** 函数功能 ：初始化历史距离表
** 入口参数 ：无
** 出口参数 ：无
**************************************************************************************/
void init_old_dist()
{
	uint16 i;
	for(i=0;i<OLD_DIST_SIZE;i++)
		old_dist[i]=0xFF;
}
#endif

/**************************************************************************************
** 函数名称 ：parse_nanopkg()
** 函数功能 ：解析当前卡信息
** 入口参数 ：无
** 出口参数 ：解析成功TRUE，失败FALSE
** 数据帧格式 ：	0  命令
**						1  距离低字节
**						2  距离高字节
**                                   3  卡状态
**						4  卡号低字节
**						5  卡号高字节
**************************************************************************************/
static MyBoolT parse_nanopkg(void)
{
//	if((card.sub = rangrs_one_pkg[0]) != sub.id)	//判断是否发给本机
//		return FALSE;
	card.com = rangrs_one_pkg[0];					//标识卡发送来的命令
	card.id = (uint16)rangrs_one_pkg[4] + (((uint16)rangrs_one_pkg[5])<<8);
	if((card.id&0x1fff) > CARDID_MAX)
		return FALSE;

	switch(card.com){
		case 0x21:									//呼叫确认,
			detach_call_card(card.id);				//则将此卡从待呼叫链表脱离
			//.............是否回送给PC待定
			return FALSE;
		case 0xFF:									//搜索附近分站
			return FALSE;
		case 0x11:									//求助
		case 0x00:									//普通测距包
			;							
	}
	
	card.dist = (uint16)rangrs_one_pkg[1] + (((uint16)rangrs_one_pkg[2])<<8);
	card.state = rangrs_one_pkg[3];
	card.id = ((card.id&0x1fff)|((card.state&0xE0)<<8));
	card.id = ((card.id) | (0x4000));
	return TRUE;
}

/**************************************************************************************
** 函数名称 ：card_into_que()
** 函数功能 ：卡信息入队
** 入口参数 ：无
** 出口参数 ：成功TRUE，失败FALSE
**************************************************************************************/
static void card_into_que(void)
{
	InQue(&main_que,(uint8)card.id);				//卡号
	InQue(&main_que,(uint8)(card.id>>8));	
	InQue(&main_que,(uint8)card.dist);				//距离
	//InQue(&main_que,(uint8)(card.dist>>8));
	//InQue(&main_que,0x08);							//扩展位
	curpkg_cardcnt++;								//卡数计数(当前包)
}

#ifdef DISTCMP_ENABLE
/**************************************************************************************
** 函数名称 ：check_dist()
** 函数功能 ：用于判断是否在当前包中更新当前卡
** 入口参数 ：无
** 出口参数 ：TRUE表示更新，FALSE表示不用更新
************************************************************************************/
static MyBoolT check_dist()
{	
	uint8 olddist,curdist,cal,temp8;
	uint16 id,temp16;
	id = (card.id&0x1fff);
	temp16 = id/2;									//在历史位置表中找到卡对应的索引
	temp8  = old_dist[temp16];						
	temp16 = id%2;									//判断是高4位还是低4位	
	if(temp16 == 1)
		olddist = (temp8 >> 4)&0x0f;				//取出历史距离
	else
		olddist = temp8&0x0f;
	olddist = (olddist<<1);							//历史距离乘2，也就是用1~5位共4位来保存历史距离
	curdist = ((uint8)card.dist)&0x1e;
	if(olddist > curdist)
		cal = olddist - curdist;
	else
		cal = curdist -	olddist;
	if(cal >= DIS_DIF_CAL){							//当前与历史差值距离大于阀值
		if(temp16 == 1)								//更新历史距离
			old_dist[(id/2)] = (old_dist[(id/2)]&0x0f)|((curdist<<3)&0xf0);		//当前距离的1~5位保存到历史距离表
		else
			old_dist[(id/2)] = (old_dist[(id/2)]&0xf0)|((curdist>>1)&0x0f);
		return TRUE;
	}else{
		return FALSE;
	}
}
#else
static MyBoolT check_dist()
{	
	return TRUE;
}
#endif

/**************************************************************************************
** 函数名称 ：update_cardin()
** 函数功能 ：更新卡进状态
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
static void update_cardin(void)
{
	uint16 temp16;
	uint8  card_bit;								//卡在卡进状态标志数组中对应的位
	uint16 card_byte;								//卡在卡进状态标志数组中对应的字节

	temp16	  = (card.id&0x1FFF) & 0x0007;			//下面用于计算并置位当前卡进状态位	
	card_bit  = (uint8)temp16;
	card_byte = (card.id&0x1FFF) >> 3;
	curcard_insta[card_byte] |= (1 <<card_bit); 	//当前卡进状态位置位，用于卡出比较

}

/**************************************************************************************
** 函数名称 ：update_curpkg_rtc()
** 函数功能 ：更新时间字段
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
static void update_curpkg_rtc(void)
{
	uint16 update_rtc;
	
	read_rtc();
	update_rtc = curpkg_start_addr;
	main_que.elems[update_rtc] = dom_read;
	update_rtc = (update_rtc + 1)%MQUESIZE;
	main_que.elems[update_rtc] = hour_read;
	update_rtc = (update_rtc + 1)%MQUESIZE;
	main_que.elems[update_rtc] = min_read;
	update_rtc = (update_rtc + 1)%MQUESIZE;
	main_que.elems[update_rtc] = sec_read;
}

/**************************************************************************************
** 函数名称 ：fill_pkg_header()
** 函数功能 ：填充包头
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
static void fill_pkg_header(void)
{
	read_rtc();
	InQue(&main_que,dom_read);						//时间
	InQue(&main_que,hour_read);
	InQue(&main_que,min_read);
	InQue(&main_que,sec_read);
	InQue(&main_que,0x00);							//留出人数位
	curpkg_ppl_addr = (main_que.rear+MQUESIZE-1)%MQUESIZE;
	
}

/**************************************************************************************
** 函数名称 ：do_package()
** 函数功能 ：打包和当前卡入下一包
** 入口参数 ：opt 为	1 执行卡入队
							0 不执行卡入队
** 出口参数 ：无
************************************************************************************/
static void do_package(uint8 opt)
{
	package_timer = 0;
	main_que.elems[curpkg_ppl_addr] = curpkg_cardcnt;//填入人数到人数字段
	curpkg_cardcnt = 0;								 //清零人数计数
	
	curpkg_start_addr = main_que.rear;				 //更新当前包开始地址(时间字段:天位)
	fill_pkg_header();								 //填充下一数据包头
	if(opt==1){
		card_into_que();							 //将本卡信息入队
	}
	mque_pkg_num++;									 //更新主队列数据包个数
	
}

/**************************************************************************************
** 函数名称 ：searchcard_in_curpkg()
** 函数功能 ：搜索当前包并更新当前包相同卡
** 入口参数 ：无
** 出口参数 ：TRUE当前包有相同卡并已更新，FALSE无相同卡
************************************************************************************/
static MyBoolT searchcard_in_curpkg(void)
{	
	uint16 searchcard_addr;
	uint8 i;
	searchcard_addr = \
					(curpkg_start_addr\
					+ TIME_FIELDLEN \
					+ PPL_FIELDLEN)%MQUESIZE;						//调整searchcard_addr到当前包第一张卡位置
		
		for(i=0;i<curpkg_cardcnt;i++){ 								//遍历当前包中的卡
			if(main_que.elems[searchcard_addr] == (uint8)card.id){
				searchcard_addr = (searchcard_addr + 1)%MQUESIZE;	//卡号高字节											
				if((main_que.elems[searchcard_addr] & 0x1f) == (((uint8)(card.id>>8)) & 0x1f)){ 	 

					main_que.elems[searchcard_addr] = (uint8)(card.id>>8)| 0x40;		
					searchcard_addr = (searchcard_addr + 1)%MQUESIZE;//距离低字节
					main_que.elems[searchcard_addr] = (uint8)card.dist;
//					searchcard_addr = (searchcard_addr + 1)%MQUESIZE;//距离高字节
//					main_que.elems[searchcard_addr] = (uint8)(card.dist>>8);
							
					searchcard_addr = FIND_CARD; 					//找到卡标志
					break;
				}else{												//未找到相同卡，调整search_cur_package到下一个卡
					searchcard_addr = (searchcard_addr + CARDINFO_FIELDLEN - 1)%MQUESIZE;
				}
			}else{																
				searchcard_addr = (searchcard_addr + CARDINFO_FIELDLEN)%MQUESIZE;
			}			
		}	
	
		if(searchcard_addr == FIND_CARD)
			return TRUE;
		else
			return FALSE;

}


/**************************************************************************************
** 函数名称 ：do_save_card()
** 函数功能 ：存储卡到队列操作
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
static void do_save_card(void)
{
	if(is_fistcard == TRUE){						//第一次接收到卡
		fill_pkg_header();
		card_into_que();
		is_fistcard = FALSE;
	}else{
		if(package_timer < PKG_TIME){				//小于打包时间
			if(curpkg_cardcnt > PKG_MAXCARDNUM){	//当前包卡数大于包卡数最大值
				package_timer = 0;
				do_package(1);						//大包并将卡入队							
			}else{
				if(searchcard_in_curpkg() == FALSE){//卡入队或更新
					card_into_que();					
				}else{}
			}
		}else{										//大于打包时间
			package_timer = 0x00;	
			if(curpkg_cardcnt != 0){
				do_package(1);						//打包并将卡入队
			}else{
				update_curpkg_rtc();
				card_into_que();
			}
		}
	}
	
}

/**************************************************************************************
** 函数名称 ：do_cardout()
** 函数功能 ：卡出处理
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
static void do_cardout(void)
{
	uint16 	card_id,card_byte;
	uint8	card_bit;

	uint8	precardin_flag,curcardin_flag;
	uint8 	fist_checkout = TRUE;										//找到的第一张出卡标识
	uint16 	temp16;
											
	card_out_timer = 0;
	for(card_id = 1;card_id < CARDID_MAX;){
		temp16    = card_id& 0x0007;			
		card_bit  = (uint8)temp16;										//卡在卡进状态标志数组中对应的位
		card_byte = card_id >> 3;										//卡在卡进状态标志数组中对应的字节
																		//目的是加快遍历速度，在本字节的历史和
		if((precard_insta[card_byte] == 0)&&(curcard_insta[card_byte] == 0)){//当前都没有置位的则跳过，继续遍历下一字节 
			card_id = ((card_id >> 3) + 1)<<3;							//调整卡号到下一字节的第0位
			continue;
		}else{
			
			precardin_flag = precard_insta[card_byte] & (1 <<card_bit);	//本卡的历史进出状态
			curcardin_flag = curcard_insta[card_byte] & (1 <<card_bit);	//本卡的当前进出状态

			if((precardin_flag != 0) && (curcardin_flag == 0)){			//卡出
				temp16 = QueLen(&main_que);
				if((MQUESIZE - temp16) \
					> (2*(TIME_FIELDLEN+PPL_FIELDLEN+CARDINFO_FIELDLEN))){
					card.id = card_id;
					card.dist = 0x0fff;
					if(fist_checkout == TRUE){							//将之前数据打包，目的是卡出状态单独打包
						fist_checkout = FALSE;
						package_timer = PKG_TIME+1;						//更改打包计数值，目的出状态单独打包
						if(curpkg_cardcnt != 0){
							do_package(0);								//将当前数据打包
						}
					}
					if(curpkg_cardcnt > PKG_MAXCARDNUM){				//卡太多，截断至下一个数据包			
						do_package(0);									//将当前数据打包
					}
					//card.dist = 0;
					card_into_que();
					//old_dist[card_id] = 0xff;							//将本卡历史dist标记成255
				}else{break;}
			}								//
			
			if(curcardin_flag){											//更新卡历史进出标志
				precard_insta[card_byte] |= (1 <<card_bit);
			}else{
				precard_insta[card_byte] &= ~(1 <<card_bit);
			}
			curcard_insta[card_byte] &= ~(1 <<card_bit);				//将当前卡进标志清除，
																		//在下次接收到本卡时,会在其他地方置位
			++card_id;
		}													
	}
}

MyBoolT NTRXCheckVerRev (void);
/**************************************************************************************
** 函数名称 ：nano_process()
** 函数功能 ：nano处理线程，两个功能
					1. 对接收到卡的处理
					2.nano芯片校准
** 入口参数 ：无
** 出口参数 ：无
************************************************************************************/
void card_process(void)
{
	//******接收卡处理***********
	uint8 	boolen;
	uint16 	main_que_uselen;
	boolen = OutQueue(&rangrs_que,rangrs_one_pkg);
	if(boolen == TRUE){
//	while((boolen = OutQueue(&rangrs_que,rangrs_one_pkg))){
		if(parse_nanopkg() == TRUE){
			update_LED();
			update_cardin();
			if(is_fistcard == TRUE){						//第一次接收到卡
				fill_pkg_header();
				card_into_que();
				is_fistcard = FALSE;
			}
			main_que_uselen = QueLen(&main_que);
			if((MQUESIZE - main_que_uselen) \
				> (3*(TIME_FIELDLEN+PPL_FIELDLEN+CARDINFO_FIELDLEN))){
				if(check_dist() == TRUE){
					do_save_card();
				}else{}
			}else{}
		}else{}
	}
	else{}

	//******打包处理***********
	if(package_timer > PKG_TIME){
		package_timer = 0;
		if(curpkg_cardcnt != 0){
			do_package(0);
		}else if(is_fistcard == FALSE){
			update_curpkg_rtc();
		}
	}
	
	//******卡出处理***********
	if(card_out_timer > CARDOUT_TIME){
		card_out_timer = 0;
		do_cardout();
	}
	
	//******待呼叫卡超时处理*******
	call_card_list_poll();
	
	//******nano校准***********
	
	if(wireless_on){
		if ((calDelay != 0)){
		   if((time_after_eq((hwclock()),tiRecal))||(rcwd > 3)){		
		   	   tiRecal = (hwclock() + calDelay);				//200ms校准1次
		   	   NANO_ENTER_CRITICAL();
			   if(NTRXCheckVerRev())
			   		NTRXAllCalibration ();
			   NANO_EXIT_CRITICAL();
			   rcwd = 0;
		   }
		   
		   /*
		   if ((rcwd > 3) || (tiRecal < hwclock())){		//应答次数错误3次校准1次
		   	   tiRecal = hwclock() + calDelay;				//3s校准1次
		   	   NANO_ENTER_CRITICAL();
			   NTRXAllCalibration ();
			   NANO_EXIT_CRITICAL();
			   rcwd = 0;
		   }
		   */
	    }
	}else{jiffies = 0;}
	

	//******nano自检***********

	
}

