#include    "ntrxranging.h"
#include "config.h"
#include "ntrxtypes.h"
#include    "ntrxdil.h"
#include    "hwclock.h"
#include    "ntrxutil.h"
#include    "nnspi.h"
#include    "includs.h"
#include    "call_card.h"
#include    "sub.h"

#define DIMADDRESS  6

unsigned char test;
extern Sub sub;
MyBoolT TxARQorgVal;
uint8 child_sub_res[6];
/*
 * static buffer for receiving and processing only one message
 * at a time. If you need more buffers in receive direction
 * you have to modify the access to the receive buffer by
 * a different method (like a buffer pool and queues).
 */
MyByte8T buff_rx[128];
extern MyByte8T 	lednum[];
extern MyWord16T 	display_count;
extern MyByte8T 	nano_flag;
extern MyByte8T 	nano_rcv_dsp;
extern uint32 int_state;
unsigned char test1 = 0x00;


extern RangRs_Que 	rangrs_que;

/* timeout for ranging in [ms] */
#define RANGING_TIMEOUT 60000

#define RANGING_END   0
/* ready means : nothing in process */
#define RANGING_READY   0
/* start means : try to send first rangingpacket */
#define RANGING_START   1
/* answer1 means : waiting for first ranging answer */
#define RANGING_ANSWER1 2
/* answer2 means : waiting for second ranging answer */
#define RANGING_ANSWER2 3
/* successfull means : all data is collected, ready to calculate now */
#define RANGING_FAST_START 4
#define RANGING_FAST_ANSWER1 5
#define RANGING_SUCCESSFULL 6

//static MyInt16T rcwd = 0;
 MyInt16T rcwd = 0;

extern void Callback_Ack(MyByte8T arqCount);


/* Interrupt flags in the nanoNET TRX chip */
#define NTRX_TX_START (0x01 << NA_TxCmdStart_B)
#define NTRX_TX_BUFF0 (0x01 << NA_TxBufferCmd_LSB)
#define NTRX_TX_BUFF1 (0x01 << NA_TxBufferCmd_MSB)
#define NTRX_TX_ARQ   (0x01 << NA_TxArq_B)
#define TxEND         (0x01 << NA_TxEnd_B)
#define RxEND         (0x01 << NA_RxEnd_B)
#define RxOVERFLOW    (0x01 << NA_RxOverflow_B)

#define TX_IRQ_MASK TxEND
#define RX_IRQ_MASK RxEND
#define NTRX_TX_HEADER (NA_RamTxDstAddr_O + 8)

/* delay time for recalibration */
MyDword32T calDelay = CONFIG_NTRX_RECAL_DELAY;
MyDword32T tiRecal;
/* local variables used by the ntrx driver */
MyByte8T txIrq;
MyByte8T rxIrq;
MyBoolT  ntrxTxPoll;
MyByte8T ntrxArqCount;

MyByte8T bbIrq;
MyByte8T bbtimerStatus;
MyByte8T lState,rState,rDest;



void NTRXTxEnd (void);

void NTRXSetCalInterval (MyDword32T ci)
{
	calDelay = ci;
}

/**
 * NTRXStart:
 *
 * NTRXStart() enables the tx and rx interrupts of the nanoNET TRX chip
 * and resets interrupt variables. This function should only be executed
 * once the nanoNET TRX chip is initialized.
 *
 * Returns: TRUE if everything O.K. else FALSE
 *
 */
MyBoolT NTRXRestart (void)
{
    MyByte8T rxIrq = 0xFF;
    MyByte8T txIrq = TX_IRQ_MASK;

    /*
     * clear any pending interrupts in the nanoNET TRX chip
     */
    NTRXSetRegister (NA_TxIntsReset, rxIrq);
    NTRXSetRegister (NA_RxIntsReset, rxIrq);

    /*
     * allow some events in the receiver to trigger an interrupt
     */
    if (NTRXGetRxCRC2mode() == TRUE)
    {rxIrq =  RxEND;}
    else
    {rxIrq = (RxEND | RxOVERFLOW);}

    /*
     * select specific events in the nanoNET TRX chip to generate interrupts
     */
    NTRXSetRegister (NA_RxIntsEn, rxIrq);
    NTRXSetRegister (NA_TxIntsEn, txIrq);
    txIrqStatus = 0;
    rxIrqStatus = 0;
    txIrq = 0;
    rxIrq = 0;
    bbIrq = 0;

    /*
     * allow the the nanoNET TRX chip to drive the interrupt line
     */
    NTRXSetRegister (NA_RxIrqEnable, TRUE);
    //NTRXSetRegister (NA_TxIrqEnable, TRUE);
	NTRXSetRegister (NA_TxIrqEnable, FALSE);

    /*
     * start the receiver of the TRX chip
     */
    NTRXSetRegister (NA_RxCmdStart, TRUE);

    ntrxRun      = TRUE;
    ntrxState    = TxIDLE;
    ntrxTxPoll   = FALSE;
    /*
     * select message type date for transmitted messages
     */
    NTRXSetRegister (NA_TxPacketType, frmTypeData);
    /*
     * enable retransmissions
     */
    NTRXSetRegister (NA_TxArq, TRUE);
    return TRUE;
}

/**
 * NTRXStart:
 *
 * NTRXStart() enables all interrupts of the nanoNET TRX chip
 *
 * Returns: TRUE if everything O.K. else FALSE
 *
 */
MyBoolT NTRXStart (void)
{
    return NTRXRestart ();
}



/**
 * NTRXTxData:
 *
 * @payload: -input- data to be transmitted to destination
 * @len:     -input- length of payload to be written into transmit buffer
 *
 * NTRXTxData() writes len bytes of payload to transmit buffer of nanoNET TRX chip
 *
 * Returns: none
 *
 */
void NTRXTxData (MyPtrT payload, MyWord16T len)
{
    NTRXSetIndexReg (3);
    NTRXWriteSPI (NA_RamTxBuffer_O & 0xff, (MyByte8T *)payload, (MyByte8T)(len & 0xff));
}

/**
 * NTRXTxHeader:
 *
 * @addr: -input- data to be transmitted to destination
 * @len:  -input- length of payload to be written into transmit buffer
 * @rsvd: -input- reserved for future use
 * @seq_n: -input- sequence bit for detecting dublicated messages
 * @flush: -input- flush bit for syncronizing with peer
 *
 * NTRXTxData() writes len bytes of payload to transmit buffer of nanoNET TRX chip
 *
 * Returns: none
 *
 */
void NTRXTxHeader (MyByte8T *addr, MyWord16T len, MyByte8T rsvd, MyByte8T seq_n, MyByte8T flush)
{
    MyByte8T txLen[8];

    NTRXSetIndexReg (0);

    /* copy the destination address to temp buffer */
    NTRXWriteSPI (NA_RamTxDstAddr_O, addr, 6);


    /* merge the three bits into the temp buffer */
    txLen[0] = (MyByte8T)len;
    txLen[1] = (len & 0x1F00) >> 8;
    txLen[1] |= (rsvd == 1)  ? 0x20 : 0;
    txLen[1] |= (seq_n == 1) ? 0x40 : 0;
    txLen[1] |= (flush == 1) ? 0x80 : 0;

    NTRXWriteSPI (NA_RamTxLength_O, txLen, 2);
}


/**
 * NTRXTxStart:
 *
 * NTRXTxStart() marks the transmit buffer as valid and starts the transmission
 *
 * Returns: none
 *
 */
void NTRXTxStart (void)
{
    MyByte8T value;

    /* mark buffers as valid and start transmission */
    value = (NTRX_TX_BUFF0 | NTRX_TX_BUFF1 | NTRX_TX_START);
    NTRXWriteSingleSPI (NA_TxBufferCmd_O, value);
#   ifdef CONFIG_TRAFFIC_LED
    TRIGGER_LED0();
#   endif /* CONFIG_TRAFFIC_LED */
}

/**
 * NTRXSendMessage:
 *
 * @payload: -input- data to be transmitted to destination
 * @len:     -input- length of payload to be written into transmit buffer
 *
 * NTRXSendMessage() writes len bytes of payload to transmit buffer of
 * nanonet TRX chip and set MAC header and start transmission
 *
 * Returns: none
 *
 */
void NTRXSendMessage (MyByte8T *addr, MyPtrT payload, MyWord16T len)
{
    MyByte8T FragC, SeqN, LCh;

    FragC=(lState&0x01)?1:0;        /* fragmentation bit - not used in TRX driver demo */
    SeqN=(lState&0x02)?1:0;         /* sequence N bit - not used in TRX driver demo */
    LCh=(lState&0x04)?1:0;          /* logical channel bit - not used in TRX driver demo */
    /* write user data to transmit buffer in ntrx chip */
    NTRXTxData (payload, len);

    /* write header data to ntrx chip */
    NTRXTxHeader (addr, len, FragC, SeqN, LCh);

    ntrxState     = TxSEND;
    NTRXTxStart ();

    if (ntrxState != TxIDLE)
    {
        while (ntrxState != TxWAIT)
            nanoInterrupt ();
        NTRXTxEnd ();
    }

}


/**
 * NTRXGetTxARQCnt:
 *
 * NTRXGetTxARQCnt() provides the number of transmission tries needed by the
 * nanoNET TRX chip for determining a successful or unsuccessful transmission
 *
 * Returns: number of transmissions tries needed for last message
 *
 */
MyByte8T NTRXGetTxARQCnt (void)
{
    return ntrxArqCount;
}

/**
 * NTRXTxEnd:
 *
 * NTRXTxEnd() finish transmission and reset internal state.
 *
 * Returns: none
 *
 */
void NTRXTxEnd (void)
{
    MyByte8T maxArqCount;
    /*
     * get number of transmissions needed to last message
     */
    NTRXGetRegister (NA_TxArqCnt, &ntrxArqCount);
    maxArqCount = NTRXGetTxARQmax();

    if ((ntrxArqCount > maxArqCount) && (ntrxState == TxWAIT))
    {
	rcwd++;
	txIrq &= ~(TxEND);
	ntrxState = TxIDLE;
	if (ntrxCal != 0)
	{
#	   ifdef CONFIG_NTRX_AUTO_RECALIB
       tiRecal = hwclock() + calDelay;
#      endif
       NTRXAllCalibration ();
       rcwd = 0;
#      ifdef CONFIG_RECALIB_LED
       TRIGGER_LED3();
#      endif /*  CONFIG_RECALIB_LED */
	}
	lState = RANGING_READY;

#   	ifdef CONFIG_TRAFFIC_LED
    	TRIGGER_LED2();
#   	endif /* CONFIG_TRAFFIC_LED */

    }else{
	txIrq &= ~(TxEND);
	ntrxState = TxIDLE;
	if (lState != RANGING_READY)
	{
		RangingCallback_Ack(ntrxArqCount);
		lState = RANGING_ANSWER1;
	}

    }
}

MyByte8T Nano_rcved;
extern MyByte8T call_com[];
extern MyByte8T comp_ok;
extern MyDword32T Nano_self_check_cnt;								//NANO自检计数
/**
 * NTRXRxReceive:
 *
 * NTRXRxReceive() read out the payload of a received message and
 * calls the upper layer/application
 *
 * Returns: none
 *
 */
void NTRXRxReceive (void)
{
    MyByte8T       status;
    MyByte8T       *rxPayload;
    MyByte8T       len = 0;
    MyByte8T       value;
    MyByte8T       source[DIMADDRESS];
    MyByte8T       dest[DIMADDRESS];
    MyByte8T       reg[2];
	MyWord16T      id;

    NTRXReadSPI (NA_RxCrc2Stat_O, &status, 1);

    /* check if data is valid */
    if ((status & (1 << NA_RxCrc2Stat_B)) == (1 << NA_RxCrc2Stat_B))
    {
        rxPayload = buff_rx;
        NTRXSetIndexReg (0);
        /* read destination address */
        NTRXReadSPI (NA_RamRxDstAddr_O, dest, sizeof(MyAddressT));
        /* read source address */
        NTRXReadSPI (NA_RamRxSrcAddr_O, source, sizeof(MyAddressT));
        /* read length plus additionl bits */
        NTRXReadSPI (NA_RamRxLength_O, reg, 2);

        rState = reg[1]>>5;

		len  = (reg[0]  | (reg[1] & 0x1F) << 8);
        if (len > 128)
        {
            len = 0;
        }
        else
        {
            NTRXSetIndexReg (2);
            /* read user data*/
            NTRXReadSPI ((NA_RamRxBuffer_O & 0xFF), rxPayload, len);
        }
    }
    value = (MyByte8T)(ntrxShadowReg[NA_RxCmdStart_O]
                            | (1 << NA_RxCmdStart_B)
                            | (1 << NA_RxBufferCmd_LSB)
                            | (1 << NA_RxBufferCmd_MSB));
    /* restart receiver */
    NTRXWriteSingleSPI (NA_RxCmdStart_O, value);
    rxIrq = 0;

    if (len > 0)
    {
    	id = (source[DIMADDRESS-1]<<8) + source[DIMADDRESS-2];			//解析出卡号,source地址的低两字节为卡号
		
    	Nano_self_check_cnt = 0;										//nano自检计数清零
    	Nano_rcved = 1;
    	if(lState == RANGING_READY && rState == RANGING_END)
    	{
    		if(sub.type == LOCAT_CHILD_STATION)							//子站(定位辅助分站)不接收此类型数据
				goto end1;
    		if(len == CALL_LEN){

				if(is_com_process(id)){									//向下发送命令
					NTRXSendMessage (source, call_com, CALL_LEN);
				}

				/*接收到数据拷贝到nano_pipe,由主线程处理*/
				if(comp_ok){
					comp_ok = 0;
					rxPayload[len] = source[DIMADDRESS-2];				//source地址的低两字节为卡号
					rxPayload[(len+1)] = source[DIMADDRESS-1];
					InQueue(&rangrs_que,(rxPayload+5));						//将数据拷贝到nano管道
					//memcpy_p(lednum,rxPayload,len);
					display_count = 0;									//更新显示
					nano_rcv_dsp = 0;
				}
			}
    	}
    	
end1: 	if(lState == RANGING_READY && rState == RANGING_FAST_START)		//测距过程中接收基站只执行此if语句
		{
                  if(test1 < 0x01)
                  {
                  test1 = 0x01;
                  }
			rDest = source[0];
			
			/* received ranging data to RangingCallback_Rx 
			 * (without protocol header stuff) 
			 */
			RangingCallback_Rx(rxPayload, len);
		
			lState = RANGING_FAST_ANSWER1;

			/*
			if(is_com_process(id)){					//向下发送命令
				NTRXSendMessage (source, call_com, CALL_LEN);
				goto rang_end;
			}
			*/
			
			/* send ranging packet */
			RangingMode(source);
			lState = RANGING_ANSWER2;
			/* send ranging packet with TxEnd information 
			* from RangingMode before */
			//RangingMode(source);
//rang_end:
			lState = RANGING_READY;
			if(sub.type == LOCAT_CHILD_STATION){						//子分站(辅助定位分站)将卡号通知给主程序
				child_sub_res[4] = source[DIMADDRESS-2];				//source地址的低两字节为卡号
				child_sub_res[5] = source[DIMADDRESS-1];
				InQueue(&rangrs_que,child_sub_res);
				display_count = 0;										//更新显示
				nano_rcv_dsp = 0;
			}
		}else if(lState == RANGING_ANSWER1 && rDest == source[0] && rState == RANGING_ANSWER1)
		{
                  if(test1 < 0x02)
                  {
                  test1 = 0x02;
                  }
			/* received ranging data to RangingCallback_Rx 
			* (without protocol header stuff) 
			*/
			RangingCallback_Rx(rxPayload, len);
			lState = RANGING_ANSWER2;
		}
		else if(lState == RANGING_ANSWER2 && rDest == source[0] && rState == RANGING_ANSWER2)
		{
                  if(test1 < 0x03)
                  {
                  test1 = 0x03;
                  }
			/* received ranging data to RangingCallback_Rx 
			*(without protocol header stuff) 
			*/
			RangingCallback_Rx(rxPayload, len);
			lState = RANGING_SUCCESSFULL;
		}
    }
}


/**
 * NTRXUpdate:
 *
 * NTRXUpdate() operates the complete receive part of the driver. It serves
 * receiver interrupt flags and picks up the received frame.
 *
 * Returns: none
 *
 */
void NTRXUpdate (void)
{
	
    nanoInterrupt ();

    if (ntrxState == TxWAIT)
    {
        NTRXTxEnd ();
    }

    // check if nanoNET TRX chip has received valid data 
    if (ntrxState == TxIDLE)
    {
        if ((rxIrq & RxEND) == RxEND )
        {
                
			NTRXRxReceive ();

        }
    }
//******
    if (calDelay != 0)
    {/*
        if (tiRecal < hwclock())
        {
            tiRecal = hwclock() + calDelay;
            NTRXAllCalibration ();
            rcwd = 0;
        }
        */
        
        /*
        if ((rcwd > 3) || (tiRecal < hwclock()))
        {
            tiRecal = hwclock() + calDelay;
            NTRXAllCalibration ();
            rcwd = 0;
        }
        */
        
    }

}

/**
 * NTRXRange:
 *
 * @dest: -input- destination address of ranging partner
 *
 * NTRXRange() This function does all necessary steps for a full
 *             ranging cycle and return the result to the caller.
 *
 * Returns: ranging result
 *
 */
MyDouble32T NTRXRange(MyAddrT dest)
{

	if (lState != RANGING_READY) return -1;

	lState = RANGING_START;
	rDest = dest[0];
	
	RangingMode(dest);

	bbIrq = 0;
	NTRXStartBasebandTimer(RANGING_TIMEOUT);
	
	do{
		
		nanoInterrupt ();

		/* check if nanoNET TRX chip has received valid data */
		if (ntrxState == TxIDLE)
		{
			if ((rxIrq & RxEND) == RxEND )
			{
				NTRXRxReceive ();
			}
		}
		
		if(bbIrq) 
		{
			lState = RANGING_READY;
			return -1;
		}
	}while(lState < RANGING_SUCCESSFULL);
	
	NTRXStopBasebandTimer();

	lState = RANGING_READY;

	return getDistance();
}


/**
 * nanoInterrupt:
 *
 * nanoInterrupt() is an interrupt service routine of the nanochip.
 * It updates the TX and RX status flags.
 *
 */
void nanoInterrupt (void)
{
        /* we have received an interrupt and neede to find out what caused it */
        NTRXReadSPI (NA_TxIntsRawStat_O, ntrxIrqStatus, 2);
        /* check if it was the transmitter */
        if (txIrqStatus != 0)
        {
            /* clear interrupts */
            NTRXSetRegister (NA_TxIntsReset, txIrqStatus);
            txIrq |= txIrqStatus & TX_IRQ_MASK;
            if ((txIrq & TxEND) == TxEND)
            {
                ntrxState = TxWAIT;
            }
        }
        /* check if it was the receiver */
        if (rxIrqStatus != 0)
        {
            /* clear interrupts */
            NTRXSetRegister (NA_RxIntsReset, rxIrqStatus);
            rxIrq |= rxIrqStatus & RX_IRQ_MASK;
            
            test = rxIrqStatus;
        }

	//BASEBAND
	NTRXReadSPI(NA_BbTimerIrqStatus_O,&bbtimerStatus,1);
	/* check if it was the basebandtimer */
	if ((bbtimerStatus&0x40) != 0)
	{
		NTRXStopBasebandTimer();
		bbIrq = 1;
		bbtimerStatus = 0;
	}	

}

/**
 * nanoInterrupt_1:
 *
 * nanoInterrupt() is an interrupt service routine of the nanochip.
 * It updates the TX and RX status flags.
 *
 */
void nanoInterrupt_1 (void)
{
        /* we have received an interrupt and neede to find out what caused it*/
        NTRXReadSPI (NA_TxIntsRawStat_O, ntrxIrqStatus, 2);
        //* check if it was the transmitter 
        if (txIrqStatus != 0)
        {
            //* clear interrupts 
            NTRXSetRegister (NA_TxIntsReset, txIrqStatus);
            txIrq |= txIrqStatus & TX_IRQ_MASK;
            if ((txIrq & TxEND) == TxEND)
            {
                ntrxState = TxWAIT;
            }
        }
    /* check if it was the receiver 
        if (rxIrqStatus != 0)
        {
            // clear interrupts 
            NTRXSetRegister (NA_RxIntsReset, rxIrqStatus);
            rxIrq |= rxIrqStatus & RX_IRQ_MASK;
            
            test = rxIrqStatus;
        }
	*/
	//BASEBAND
	NTRXReadSPI(NA_BbTimerIrqStatus_O,&bbtimerStatus,1);
	/* check if it was the basebandtimer */
	if ((bbtimerStatus&0x40) != 0)
	{
		NTRXStopBasebandTimer();
		bbIrq = 1;
		bbtimerStatus = 0;
	}	

}