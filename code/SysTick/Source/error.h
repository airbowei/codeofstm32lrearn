#ifndef ERROR_H
#define ERROR_H

#include "apm32f00x.h"

extern void actErrorDetection(void);
extern void actEraseErrorInfo(void);
extern void actReadErrorInfo(void);
/********************************************************************************/
/*�궨��                                                                        */
/********************************************************************************/
/*************************������********************************/

#define    AC_V_HIGH_ERROR_CODE              185   //AC��ѹ���ߴ�����
#define    AC_V_LOW_ERROR_CODE               180   //AC��ѹ���ʹ�����
#define    OUT_TEMP_ERROR_CODE               183   //�����쳣������
#define    OUT_HEAT_TIME_ERROR_CODE          181   //����ʱ���쳣������
#define    EXCESSIVE_TEMP_ERROR_ERROR_CODE   184   //�²���������
#define    SLEEP_HEAT_ERROR_CODE             182   //˯�����м�����Ϊ������
#endif
