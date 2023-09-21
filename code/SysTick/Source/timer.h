#ifndef TIMER_H
#define TIMER_H

#include "apm32f00x.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_misc.h"
#include "apm32f00x_eint.h"
#include "apm32f00x_usart.h"
#include "apm32f00x_rcm.h"

void TMR4Init(void);
void TMR4Isr(void);
extern void FuserHeaterTimeCount(void);
extern void TimeInit(void);
extern void Aculate_Real_Time(void);
#endif

