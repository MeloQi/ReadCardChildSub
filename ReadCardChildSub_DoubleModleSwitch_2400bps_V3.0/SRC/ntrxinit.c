#include "includs.h"
#include "config.h"
#include "ntrxtypes.h"
#include    "ntrxutil.h"
#include    "hwclock.h"
#include    "nnspi.h"

extern	uint8 	wireless_module;

void NTRXUpdate (void);

/**
 * irq_nano:
 *
 * nano�����ж�
 * 
 *
 * Returns: 
 */
void __irq irq_nano_left(void)
{
	NTRXUpdate();
	EXTINT = 0x08;	
	VICVectAddr=0x00; 
}

void __irq irq_nano_right(void)
{
	NTRXUpdate();
	EXTINT = 0x04;	
	VICVectAddr=0x00; 
}


/**
 * nanoint_init:
 *
 * nano�����жϳ�ʼ��
 * 
 *
 * Returns: 
 */
static void nanoint_init(void)
{
	if(wireless_module == MODULE_LEFT){
		PINSEL0 |= (3<<18); 				//ENIT3
			
		EXTMODE |= 0x08;					//���ش���
		EXTPOLAR &= (~0x08);				//�½��ش���
		VICVectCntl3 = 0x20 | 0x11;
		VICVectAddr3 = (uint32)irq_nano_left;	//�����жϷ�������ַ		
		EXTINT = 0x08;
		VICIntEnable |= (1 << 0x11);
	}else if(wireless_module == MODULE_RIGHT){
		PINSEL0 |= 0x80000000; 				//ENIT2 p0.15
		PINSEL0 &= 0xBFFFFFFF;

		EXTMODE |= 0x04;					//���ش���
		EXTPOLAR &= (~0x04);				//�½��ش���

		VICVectCntl5 = 0x20 | 0x10;
		VICVectAddr5 = (uint32)irq_nano_right;	//�����жϷ�������ַ		
		EXTINT = 0x04;
		VICIntEnable |= (1 << 0x10);
	}
	
}


/**
 * NtrxInit:
 *
 * NtrxInit() initializes the entire necessary hardware parts of
 * the nanoNET TRX chip. It also initializes all necessary software data.
 *
 * Returns: TRUE if everything O.K. else FALSE
 */
MyBoolT NtrxInit(void)
{
  settingVal.bw = NA_80MHz;
  //settingVal.bw = NA_22MHz;
  settingVal.sd = NA_1us;
  settingVal.sr = NA_1M_S;
  settingVal.fdma= FALSE;
  settingVal.fixnmap=NA_FIX_MODE;
  settingVal.rangingConst=122.492363;

  NTRXCheckTable();
  /* init SPI HW&SW */
  nanorest_init();
  InitSPI();

  NanoReset();
  hwdelay(500);		//�ȴ��㹻����ʱ�䣬ʹnano�����ȶ�
  /*
  * configure SPI output of chip MSB first / push pull
  */
  SetupSPI ();
  /* initialize shadow registers */
  NTRXInitShadowRegister ();
  hwdelay(100);
   //!! Some short delay seems necessary here??
  /* check connection and firmware version and revision */
  if(FALSE == NTRXCheckVerRev()){
  	return FALSE;
  }
  NTRXSetupTRxMode (settingVal.bw, settingVal.sd, settingVal.sr);
  nanoint_init();
  ntrxRun = FALSE;
  return TRUE;
}
