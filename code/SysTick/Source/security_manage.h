#ifndef SECURITY_H
#define SECURITY_H

#include "apm32f00x.h"


typedef enum
{
	eSAMPLE_1 = 0,
	eSAMPLE_2,
	eSAMPLE_3,
	eSAMPLE_MAX
}SAMPLE_NUM;


typedef enum
{
	ePOWER_SAMPLE = 0,
	eTEMPRATURE_SAMPLE,
	eSAMPLE_TYPE_MAX
}SAMPLE_TYPE;

extern void reqSecurityManage(void);
extern uint16_t g_avaliable_ad_value[eSAMPLE_TYPE_MAX];
#endif
