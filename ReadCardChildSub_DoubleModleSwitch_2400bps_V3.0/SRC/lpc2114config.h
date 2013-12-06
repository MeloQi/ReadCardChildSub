#ifndef _LPC2114CONFIG_H
#define _LPC2114CONFIG_H

//This segment should not be modified
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef	null
#define	null	((void *)0)
#endif	/* null */

typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	�޷���8λ���ͱ���  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		�з���8λ���ͱ���  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	�޷���16λ���ͱ��� */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		�з���16λ���ͱ��� */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	�޷���32λ���ͱ��� */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		�з���32λ���ͱ��� */
typedef float          fp32;                    /* single precision floating point variable (32bits) �����ȸ�������32λ���ȣ� */
typedef double         fp64;

                    
                    /* double precision floating point variable (64bits) ˫���ȸ�������64λ���ȣ� */

/********************************/
/*Application Program Configurations*/
/*	   Ӧ�ó������� 			*/
/********************************/
#define   VerH	0x01		// ����汾�Ÿ��ֽ�
#define   VerL	0x00		// ����汾�ŵ��ֽ�
					
					//***���жϿ��غ�****
#define ENTER_CRITICAL()	 \
							do {\
								int_state=VICIntEnable; \
								VICIntEnClr=0xFFFFFFFF; \
							} while(0)
#define EXIT_CRITICAL()	 \
							do{\
								VICIntEnable=int_state; \
								VICVectAddr=0x00;\
							} while(0)
					
					//***nano�жϿ��غ�****
#define NANO_ENTER_CRITICAL()	 \
									do {\
										VICIntEnClr |= ((1 << 0x11)|(1 << 0x10)); \
									} while(0)
#define NANO_EXIT_CRITICAL()	 \
									do{\
										VICIntEnable |= ((1 << 0x11)|(1 << 0x10)); \
									} while(0)
					
					//***RS485�жϿ��غ�****
#define RS485_ENTER_CRITICAL()	 \
									do {\
										VICIntEnClr |= (1 << 0x06); \
									} while(0)
#define RS485_EXIT_CRITICAL()	 \
									do{\
										VICIntEnable |= (1 << 0x06); \
									} while(0)
					
						
					
					
/********************************/
/*Configuration of the example */
/*	   �����ӵ����� 			*/
/********************************/
/* System configuration .Fosc��Fcclk��Fcco��Fpclk must be defined */
/* ϵͳ����, Fosc��Fcclk��Fcco��Fpclk���붨��*/
#define Fosc            11059200                    //Crystal frequence,10MHz~25MHz��should be the same as actual status. 
												//Ӧ����ʵ��һ������Ƶ��,10MHz~25MHz��Ӧ����ʵ��һ��
#define Fcclk           (Fosc * 5)                  //System frequence,should be (1~32)multiples of Fosc,and should be equal or less  than 60MHz. 
												//ϵͳƵ�ʣ�����ΪFosc��������(1~32)����<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO frequence,should be 2��4��8��16 multiples of Fcclk, ranged from 156MHz to 320MHz. 
												//CCOƵ�ʣ�����ΪFcclk��2��4��8��16������ΧΪ156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 2             //VPB clock frequence , must be 1��2��4 multiples of (Fcclk / 4).
												//VPBʱ��Ƶ�ʣ�ֻ��Ϊ(Fcclk / 4)��1��2��4��




#endif

