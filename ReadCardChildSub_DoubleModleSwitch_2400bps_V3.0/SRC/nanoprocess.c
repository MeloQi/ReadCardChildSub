#include "includs.h"
		
		
//#define DISTCMP_ENABLE						//�Ƿ���о������

#define DIS_DIF_CAL 		0 				//��ǰ��������ʷ���뷧ֵ

#define	FIND_CARD 			0xffff  		//����������ǰ���п�
#define PKG_MAXCARDNUM 		20				//���������
#define	CARDID_MAX			8009			//�������ֵ
#define PKG_TIME 			600 			//���ʱ��= (PKG_TIME*5/1000)s
#define CARDOUT_TIME		4000			//����ʱ��=(CARDOUT_TIME*5/1000)s

#define OLD_DIST_SIZE		4005			//�洢��ʷ��������Ĵ�С
#define CARDINOUT_SIZE 		1005			//������״̬�����С

Card 		  card;							//���ڴ洢��ǰ����Ϣ 
static uint16 curpkg_ppl_addr;				//��ǰ������λ��ַ
static uint16 curpkg_start_addr = 0;		//��ǰ����ʼλ��
static uint8  is_fistcard = TRUE;			//��һ�ν��ܵ�����־
static uint16 curpkg_cardcnt = 0;			//ǰ����������(ZI���ѳ�ʼ��Ϊ0)

#ifdef DISTCMP_ENABLE
static uint8  old_dist[OLD_DIST_SIZE];		//�洢��ʷ����
#endif

static uint8  curcard_insta[CARDINOUT_SIZE];//��ǰ������״̬
static uint8  precard_insta[CARDINOUT_SIZE];//��ʷ������״̬
RangRsPkgT rangrs_one_pkg;					//�ݴ�һ����������


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
** �������� ��init_old_dist()
** �������� ����ʼ����ʷ�����
** ��ڲ��� ����
** ���ڲ��� ����
**************************************************************************************/
void init_old_dist()
{
	uint16 i;
	for(i=0;i<OLD_DIST_SIZE;i++)
		old_dist[i]=0xFF;
}
#endif

/**************************************************************************************
** �������� ��parse_nanopkg()
** �������� ��������ǰ����Ϣ
** ��ڲ��� ����
** ���ڲ��� �������ɹ�TRUE��ʧ��FALSE
** ����֡��ʽ ��	0  ����
**						1  ������ֽ�
**						2  ������ֽ�
**                                   3  ��״̬
**						4  ���ŵ��ֽ�
**						5  ���Ÿ��ֽ�
**************************************************************************************/
static MyBoolT parse_nanopkg(void)
{
//	if((card.sub = rangrs_one_pkg[0]) != sub.id)	//�ж��Ƿ񷢸�����
//		return FALSE;
	card.com = rangrs_one_pkg[0];					//��ʶ��������������
	card.id = (uint16)rangrs_one_pkg[4] + (((uint16)rangrs_one_pkg[5])<<8);
	if((card.id&0x1fff) > CARDID_MAX)
		return FALSE;

	switch(card.com){
		case 0x21:									//����ȷ��,
			detach_call_card(card.id);				//�򽫴˿��Ӵ�������������
			//.............�Ƿ���͸�PC����
			return FALSE;
		case 0xFF:									//����������վ
			return FALSE;
		case 0x11:									//����
		case 0x00:									//��ͨ����
			;							
	}
	
	card.dist = (uint16)rangrs_one_pkg[1] + (((uint16)rangrs_one_pkg[2])<<8);
	card.state = rangrs_one_pkg[3];
	card.id = ((card.id&0x1fff)|((card.state&0xE0)<<8));
	card.id = ((card.id) | (0x4000));
	return TRUE;
}

/**************************************************************************************
** �������� ��card_into_que()
** �������� ������Ϣ���
** ��ڲ��� ����
** ���ڲ��� ���ɹ�TRUE��ʧ��FALSE
**************************************************************************************/
static void card_into_que(void)
{
	InQue(&main_que,(uint8)card.id);				//����
	InQue(&main_que,(uint8)(card.id>>8));	
	InQue(&main_que,(uint8)card.dist);				//����
	//InQue(&main_que,(uint8)(card.dist>>8));
	//InQue(&main_que,0x08);							//��չλ
	curpkg_cardcnt++;								//��������(��ǰ��)
}

#ifdef DISTCMP_ENABLE
/**************************************************************************************
** �������� ��check_dist()
** �������� �������ж��Ƿ��ڵ�ǰ���и��µ�ǰ��
** ��ڲ��� ����
** ���ڲ��� ��TRUE��ʾ���£�FALSE��ʾ���ø���
************************************************************************************/
static MyBoolT check_dist()
{	
	uint8 olddist,curdist,cal,temp8;
	uint16 id,temp16;
	id = (card.id&0x1fff);
	temp16 = id/2;									//����ʷλ�ñ����ҵ�����Ӧ������
	temp8  = old_dist[temp16];						
	temp16 = id%2;									//�ж��Ǹ�4λ���ǵ�4λ	
	if(temp16 == 1)
		olddist = (temp8 >> 4)&0x0f;				//ȡ����ʷ����
	else
		olddist = temp8&0x0f;
	olddist = (olddist<<1);							//��ʷ�����2��Ҳ������1~5λ��4λ��������ʷ����
	curdist = ((uint8)card.dist)&0x1e;
	if(olddist > curdist)
		cal = olddist - curdist;
	else
		cal = curdist -	olddist;
	if(cal >= DIS_DIF_CAL){							//��ǰ����ʷ��ֵ������ڷ�ֵ
		if(temp16 == 1)								//������ʷ����
			old_dist[(id/2)] = (old_dist[(id/2)]&0x0f)|((curdist<<3)&0xf0);		//��ǰ�����1~5λ���浽��ʷ�����
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
** �������� ��update_cardin()
** �������� �����¿���״̬
** ��ڲ��� ����
** ���ڲ��� ����
************************************************************************************/
static void update_cardin(void)
{
	uint16 temp16;
	uint8  card_bit;								//���ڿ���״̬��־�����ж�Ӧ��λ
	uint16 card_byte;								//���ڿ���״̬��־�����ж�Ӧ���ֽ�

	temp16	  = (card.id&0x1FFF) & 0x0007;			//�������ڼ��㲢��λ��ǰ����״̬λ	
	card_bit  = (uint8)temp16;
	card_byte = (card.id&0x1FFF) >> 3;
	curcard_insta[card_byte] |= (1 <<card_bit); 	//��ǰ����״̬λ��λ�����ڿ����Ƚ�

}

/**************************************************************************************
** �������� ��update_curpkg_rtc()
** �������� ������ʱ���ֶ�
** ��ڲ��� ����
** ���ڲ��� ����
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
** �������� ��fill_pkg_header()
** �������� ������ͷ
** ��ڲ��� ����
** ���ڲ��� ����
************************************************************************************/
static void fill_pkg_header(void)
{
	read_rtc();
	InQue(&main_que,dom_read);						//ʱ��
	InQue(&main_que,hour_read);
	InQue(&main_que,min_read);
	InQue(&main_que,sec_read);
	InQue(&main_que,0x00);							//��������λ
	curpkg_ppl_addr = (main_que.rear+MQUESIZE-1)%MQUESIZE;
	
}

/**************************************************************************************
** �������� ��do_package()
** �������� ������͵�ǰ������һ��
** ��ڲ��� ��opt Ϊ	1 ִ�п����
							0 ��ִ�п����
** ���ڲ��� ����
************************************************************************************/
static void do_package(uint8 opt)
{
	package_timer = 0;
	main_que.elems[curpkg_ppl_addr] = curpkg_cardcnt;//���������������ֶ�
	curpkg_cardcnt = 0;								 //������������
	
	curpkg_start_addr = main_que.rear;				 //���µ�ǰ����ʼ��ַ(ʱ���ֶ�:��λ)
	fill_pkg_header();								 //�����һ���ݰ�ͷ
	if(opt==1){
		card_into_que();							 //��������Ϣ���
	}
	mque_pkg_num++;									 //�������������ݰ�����
	
}

/**************************************************************************************
** �������� ��searchcard_in_curpkg()
** �������� ��������ǰ�������µ�ǰ����ͬ��
** ��ڲ��� ����
** ���ڲ��� ��TRUE��ǰ������ͬ�����Ѹ��£�FALSE����ͬ��
************************************************************************************/
static MyBoolT searchcard_in_curpkg(void)
{	
	uint16 searchcard_addr;
	uint8 i;
	searchcard_addr = \
					(curpkg_start_addr\
					+ TIME_FIELDLEN \
					+ PPL_FIELDLEN)%MQUESIZE;						//����searchcard_addr����ǰ����һ�ſ�λ��
		
		for(i=0;i<curpkg_cardcnt;i++){ 								//������ǰ���еĿ�
			if(main_que.elems[searchcard_addr] == (uint8)card.id){
				searchcard_addr = (searchcard_addr + 1)%MQUESIZE;	//���Ÿ��ֽ�											
				if((main_que.elems[searchcard_addr] & 0x1f) == (((uint8)(card.id>>8)) & 0x1f)){ 	 

					main_que.elems[searchcard_addr] = (uint8)(card.id>>8)| 0x40;		
					searchcard_addr = (searchcard_addr + 1)%MQUESIZE;//������ֽ�
					main_que.elems[searchcard_addr] = (uint8)card.dist;
//					searchcard_addr = (searchcard_addr + 1)%MQUESIZE;//������ֽ�
//					main_que.elems[searchcard_addr] = (uint8)(card.dist>>8);
							
					searchcard_addr = FIND_CARD; 					//�ҵ�����־
					break;
				}else{												//δ�ҵ���ͬ��������search_cur_package����һ����
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
** �������� ��do_save_card()
** �������� ���洢�������в���
** ��ڲ��� ����
** ���ڲ��� ����
************************************************************************************/
static void do_save_card(void)
{
	if(is_fistcard == TRUE){						//��һ�ν��յ���
		fill_pkg_header();
		card_into_que();
		is_fistcard = FALSE;
	}else{
		if(package_timer < PKG_TIME){				//С�ڴ��ʱ��
			if(curpkg_cardcnt > PKG_MAXCARDNUM){	//��ǰ���������ڰ��������ֵ
				package_timer = 0;
				do_package(1);						//������������							
			}else{
				if(searchcard_in_curpkg() == FALSE){//����ӻ����
					card_into_que();					
				}else{}
			}
		}else{										//���ڴ��ʱ��
			package_timer = 0x00;	
			if(curpkg_cardcnt != 0){
				do_package(1);						//������������
			}else{
				update_curpkg_rtc();
				card_into_que();
			}
		}
	}
	
}

/**************************************************************************************
** �������� ��do_cardout()
** �������� ����������
** ��ڲ��� ����
** ���ڲ��� ����
************************************************************************************/
static void do_cardout(void)
{
	uint16 	card_id,card_byte;
	uint8	card_bit;

	uint8	precardin_flag,curcardin_flag;
	uint8 	fist_checkout = TRUE;										//�ҵ��ĵ�һ�ų�����ʶ
	uint16 	temp16;
											
	card_out_timer = 0;
	for(card_id = 1;card_id < CARDID_MAX;){
		temp16    = card_id& 0x0007;			
		card_bit  = (uint8)temp16;										//���ڿ���״̬��־�����ж�Ӧ��λ
		card_byte = card_id >> 3;										//���ڿ���״̬��־�����ж�Ӧ���ֽ�
																		//Ŀ���Ǽӿ�����ٶȣ��ڱ��ֽڵ���ʷ��
		if((precard_insta[card_byte] == 0)&&(curcard_insta[card_byte] == 0)){//��ǰ��û����λ��������������������һ�ֽ� 
			card_id = ((card_id >> 3) + 1)<<3;							//�������ŵ���һ�ֽڵĵ�0λ
			continue;
		}else{
			
			precardin_flag = precard_insta[card_byte] & (1 <<card_bit);	//��������ʷ����״̬
			curcardin_flag = curcard_insta[card_byte] & (1 <<card_bit);	//�����ĵ�ǰ����״̬

			if((precardin_flag != 0) && (curcardin_flag == 0)){			//����
				temp16 = QueLen(&main_que);
				if((MQUESIZE - temp16) \
					> (2*(TIME_FIELDLEN+PPL_FIELDLEN+CARDINFO_FIELDLEN))){
					card.id = card_id;
					card.dist = 0x0fff;
					if(fist_checkout == TRUE){							//��֮ǰ���ݴ����Ŀ���ǿ���״̬�������
						fist_checkout = FALSE;
						package_timer = PKG_TIME+1;						//���Ĵ������ֵ��Ŀ�ĳ�״̬�������
						if(curpkg_cardcnt != 0){
							do_package(0);								//����ǰ���ݴ��
						}
					}
					if(curpkg_cardcnt > PKG_MAXCARDNUM){				//��̫�࣬�ض�����һ�����ݰ�			
						do_package(0);									//����ǰ���ݴ��
					}
					//card.dist = 0;
					card_into_que();
					//old_dist[card_id] = 0xff;							//��������ʷdist��ǳ�255
				}else{break;}
			}								//
			
			if(curcardin_flag){											//���¿���ʷ������־
				precard_insta[card_byte] |= (1 <<card_bit);
			}else{
				precard_insta[card_byte] &= ~(1 <<card_bit);
			}
			curcard_insta[card_byte] &= ~(1 <<card_bit);				//����ǰ������־�����
																		//���´ν��յ�����ʱ,���������ط���λ
			++card_id;
		}													
	}
}

MyBoolT NTRXCheckVerRev (void);
/**************************************************************************************
** �������� ��nano_process()
** �������� ��nano�����̣߳���������
					1. �Խ��յ����Ĵ���
					2.nanoоƬУ׼
** ��ڲ��� ����
** ���ڲ��� ����
************************************************************************************/
void card_process(void)
{
	//******���տ�����***********
	uint8 	boolen;
	uint16 	main_que_uselen;
	boolen = OutQueue(&rangrs_que,rangrs_one_pkg);
	if(boolen == TRUE){
//	while((boolen = OutQueue(&rangrs_que,rangrs_one_pkg))){
		if(parse_nanopkg() == TRUE){
			update_LED();
			update_cardin();
			if(is_fistcard == TRUE){						//��һ�ν��յ���
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

	//******�������***********
	if(package_timer > PKG_TIME){
		package_timer = 0;
		if(curpkg_cardcnt != 0){
			do_package(0);
		}else if(is_fistcard == FALSE){
			update_curpkg_rtc();
		}
	}
	
	//******��������***********
	if(card_out_timer > CARDOUT_TIME){
		card_out_timer = 0;
		do_cardout();
	}
	
	//******�����п���ʱ����*******
	call_card_list_poll();
	
	//******nanoУ׼***********
	
	if(wireless_on){
		if ((calDelay != 0)){
		   if((time_after_eq((hwclock()),tiRecal))||(rcwd > 3)){		
		   	   tiRecal = (hwclock() + calDelay);				//200msУ׼1��
		   	   NANO_ENTER_CRITICAL();
			   if(NTRXCheckVerRev())
			   		NTRXAllCalibration ();
			   NANO_EXIT_CRITICAL();
			   rcwd = 0;
		   }
		   
		   /*
		   if ((rcwd > 3) || (tiRecal < hwclock())){		//Ӧ���������3��У׼1��
		   	   tiRecal = hwclock() + calDelay;				//3sУ׼1��
		   	   NANO_ENTER_CRITICAL();
			   NTRXAllCalibration ();
			   NANO_EXIT_CRITICAL();
			   rcwd = 0;
		   }
		   */
	    }
	}else{jiffies = 0;}
	

	//******nano�Լ�***********

	
}

