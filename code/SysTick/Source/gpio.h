#ifndef GPIO_H
#define GPIO_H

#include "apm32f00x.h"

extern void GpioInit(void);
extern void actFuserRelayOff(void);
extern void actFuserRelayOn(void);
extern uint16_t actSleepQuery(void);
extern uint16_t actAcFlagQuery(void);
extern void actGetGpioValue(void);
extern uint16_t actFuserOnTimeQuery(void);
#endif
