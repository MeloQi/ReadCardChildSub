#ifndef NTRXRANGING_H
#define NTRXRANGING_H

#include "ntrxtypes.h"

double getDistance(void);

void RangingCallback_Rx(MyByte8T *payload, MyByte8T len);

void RangingCallback_Ack(MyByte8T arqCount);

void RangingMode(MyAddrT dest);

void memcpy_p(unsigned char *destdata,unsigned char *scrdata,unsigned char length);

#endif /* NTRXRANGING_H */