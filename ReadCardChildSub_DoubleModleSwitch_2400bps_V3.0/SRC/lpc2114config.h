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

typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         fp64;

                    
                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

/********************************/
/*Application Program Configurations*/
/*	   应用程序配置 			*/
/********************************/
#define   VerH	0x01		// 定义版本号高字节
#define   VerL	0x00		// 定义版本号低字节
					
					//***总中断开关宏****
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
					
					//***nano中断开关宏****
#define NANO_ENTER_CRITICAL()	 \
									do {\
										VICIntEnClr |= ((1 << 0x11)|(1 << 0x10)); \
									} while(0)
#define NANO_EXIT_CRITICAL()	 \
									do{\
										VICIntEnable |= ((1 << 0x11)|(1 << 0x10)); \
									} while(0)
					
					//***RS485中断开关宏****
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
/*	   本例子的配置 			*/
/********************************/
/* System configuration .Fosc、Fcclk、Fcco、Fpclk must be defined */
/* 系统设置, Fosc、Fcclk、Fcco、Fpclk必须定义*/
#define Fosc            11059200                    //Crystal frequence,10MHz~25MHz，should be the same as actual status. 
												//应当与实际一至晶振频率,10MHz~25MHz，应当与实际一至
#define Fcclk           (Fosc * 5)                  //System frequence,should be (1~32)multiples of Fosc,and should be equal or less  than 60MHz. 
												//系统频率，必须为Fosc的整数倍(1~32)，且<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO frequence,should be 2、4、8、16 multiples of Fcclk, ranged from 156MHz to 320MHz. 
												//CCO频率，必须为Fcclk的2、4、8、16倍，范围为156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 2             //VPB clock frequence , must be 1、2、4 multiples of (Fcclk / 4).
												//VPB时钟频率，只能为(Fcclk / 4)的1、2、4倍




#endif

