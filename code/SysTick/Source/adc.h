#ifndef ADC_H
#define ADC_H

#include "apm32f00x.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_adc.h"



void ADCCalibration(void);
void ADC_GPIO_Init(ADC_CHANNEL_T channel);
void ADCInit(void);
void ADC_GPIOConfig(void);
uint16_t ADCMeasure(void);
uint16_t ADC_GetVoltage(ADC_BUFFER_IDX_T id);
uint16_t ADC_GetTimes(ADC_BUFFER_IDX_T id,int times);
int16_t actConvertMiddleADValueToTemperature(uint16_t rv_volt_value );
extern void actGetAdcValue(void);

#define AC_V_READ         ADC_BUFFER_IDX_0
#define CTH_READ          ADC_BUFFER_IDX_1

#define VOLT_TO_FUSER_TEMPRETURE_INDEX_NUM       271
#define FUSER_TEMPRETURE_MAX_                    230    //最大温度
#define FUSER_ADC_SAMPLE_TIMES                   20     //采样次数

typedef struct
{
	int16_t tempreture;
	uint16_t volt;
}FuserTempVoltTable;





#endif
