#ifndef __CM4_CANFESTIVAL_H__
#define __CM4_CANFESTIVAL_H__


#include "can_driver.h"
#include "timerscfg.h"
#include "timer.h"
#include "declaration.h"


void initTimer(void);
void clearTimer(void);

unsigned char canSend(CAN_PORT notused, Message *m);
unsigned char canInit(CO_Data * d, uint32_t bitrate);
void canClose(void);

// void disable_it(void);
// void enable_it(void);


#endif /* __CM4_CANFESTIVAL_H__ */
