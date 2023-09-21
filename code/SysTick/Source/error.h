#ifndef ERROR_H
#define ERROR_H

#include "apm32f00x.h"

extern void actErrorDetection(void);
extern void actEraseErrorInfo(void);
extern void actReadErrorInfo(void);
/********************************************************************************/
/*宏定义                                                                        */
/********************************************************************************/
/*************************错误码********************************/

#define    AC_V_HIGH_ERROR_CODE              185   //AC电压过高错误码
#define    AC_V_LOW_ERROR_CODE               180   //AC电压过低错误码
#define    OUT_TEMP_ERROR_CODE               183   //过温异常错误码
#define    OUT_HEAT_TIME_ERROR_CODE          181   //加热时长异常错误码
#define    EXCESSIVE_TEMP_ERROR_ERROR_CODE   184   //温差过大错误码
#define    SLEEP_HEAT_ERROR_CODE             182   //睡眠下有加热行为错误码
#endif
