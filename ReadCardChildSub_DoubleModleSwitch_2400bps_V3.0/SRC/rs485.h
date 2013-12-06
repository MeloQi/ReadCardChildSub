#ifndef __RS485_H 
#define __RS485_H


//void __irq IRQ_UART0 (void);


void rs485_send(uint8 *p,uint16 length);
void rs485_init(void);


#endif



