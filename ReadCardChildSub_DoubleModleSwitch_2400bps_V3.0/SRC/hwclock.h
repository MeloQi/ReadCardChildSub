#ifndef	_HWCLOCK_H
#define	_HWCLOCK_H

#include "includs.h"
#include "config.h"
#include "ntrxtypes.h"



#define NKEYS 4
void set_systime(uint32 s,uint16 ms);
uint32 get_systime_s(void);
uint16 get_systime_ms(void);
extern	void		hwclock_init(void);
extern  void 		hwclock_stop (void);
extern  void 		StartTimer2 (void);
extern	MyDword32T	hwclock(void);
extern	void		hwdelay(MyDword32T t);
void	hwclockRestart(MyDword32T start);
void 	SetWdtCounter (MyDword32T value);
void __irq IRQ_Timer0 (void);
void timer0_init(void);
#endif	/* _HWCLOCK_H */