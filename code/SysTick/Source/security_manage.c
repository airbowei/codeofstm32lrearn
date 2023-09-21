
#include "adc.h"
#include "security_manage.h"

/* global variable*/
uint16_t g_avaliable_ad_value[eSAMPLE_TYPE_MAX] = {0};
int16_t cth_temp = 0;
uint16_t ac_v = 0;
uint8_t g_fuser_ad_value_index = 0;
BOOL g_is_sampling_ok = FALSE;
uint16_t g_fuser_ad_value[eSAMPLE_TYPE_MAX][eSAMPLE_MAX] = {0};

static uint16_t actSeletMiddleADValue( SAMPLE_TYPE rv_type );



void reqSecurityManage(void)
{
	//cth_v = ADC_GetVoltage(CTH_READ);
	if( (g_fuser_ad_value_index == eSAMPLE_3)
		&& (g_is_sampling_ok == FALSE) )
	{
		g_is_sampling_ok = TRUE;
	}
	
	
	g_fuser_ad_value[ePOWER_SAMPLE][g_fuser_ad_value_index] = ADC_GetVoltage(AC_V_READ); //ADC_GetTimes(AC_V_READ,20);

	g_fuser_ad_value[eTEMPRATURE_SAMPLE][g_fuser_ad_value_index] = ADC_GetVoltage(CTH_READ);//ADC_GetTimes(CTH_READ,20);
//	printf("g_fuser_ad_value[eTEMPRATURE_SAMPLE][g_fuser_ad_value_index] is %d\n",g_fuser_ad_value[eTEMPRATURE_SAMPLE][g_fuser_ad_value_index]);	
	if(g_is_sampling_ok == FALSE)
	{
		g_avaliable_ad_value[eTEMPRATURE_SAMPLE] = g_fuser_ad_value[eTEMPRATURE_SAMPLE][g_fuser_ad_value_index];
		g_avaliable_ad_value[ePOWER_SAMPLE] = g_fuser_ad_value[ePOWER_SAMPLE][g_fuser_ad_value_index];		
	}
	else
	{
		g_avaliable_ad_value[eTEMPRATURE_SAMPLE] = actSeletMiddleADValue(eTEMPRATURE_SAMPLE);
		g_avaliable_ad_value[ePOWER_SAMPLE] = actSeletMiddleADValue(ePOWER_SAMPLE);
	}
//	cth_temp = actConvertMiddleADValueToTemperature(g_avaliable_ad_value);	
	g_fuser_ad_value_index++;
	if(g_fuser_ad_value_index > eSAMPLE_3)
	{
		g_fuser_ad_value_index = eSAMPLE_1;
	}
	

}



static uint16_t actSeletMiddleADValue( SAMPLE_TYPE rv_type )
{
	uint16_t at_middle_value = 0;
	//printf("samp1 = %d,samp2 = %d,samp3 = %d\n",g_fuser_ad_value[eTEMPRATURE_SAMPLE][eSAMPLE_1],g_fuser_ad_value[eTEMPRATURE_SAMPLE][eSAMPLE_2],g_fuser_ad_value[eTEMPRATURE_SAMPLE][eSAMPLE_3]);
	if( ( (g_fuser_ad_value[rv_type][eSAMPLE_2] <= g_fuser_ad_value[rv_type][eSAMPLE_1]) &&
		  (g_fuser_ad_value[rv_type][eSAMPLE_1] <= g_fuser_ad_value[rv_type][eSAMPLE_3]) )
	||  ( (g_fuser_ad_value[rv_type][eSAMPLE_3] <= g_fuser_ad_value[rv_type][eSAMPLE_1]) &&
		  (g_fuser_ad_value[rv_type][eSAMPLE_1] <= g_fuser_ad_value[rv_type][eSAMPLE_2]) ) )
	{
		at_middle_value = g_fuser_ad_value[rv_type][eSAMPLE_1];

	}
	else if( ( (g_fuser_ad_value[rv_type][eSAMPLE_1] <= g_fuser_ad_value[rv_type][eSAMPLE_2]) &&
		  (g_fuser_ad_value[rv_type][eSAMPLE_2] <= g_fuser_ad_value[rv_type][eSAMPLE_3]) )
	||  ( (g_fuser_ad_value[rv_type][eSAMPLE_3] <= g_fuser_ad_value[rv_type][eSAMPLE_2]) &&
		  (g_fuser_ad_value[rv_type][eSAMPLE_2] <= g_fuser_ad_value[rv_type][eSAMPLE_1]) ) )
	{
		at_middle_value = g_fuser_ad_value[rv_type][eSAMPLE_2];
	}
	else
	{
		at_middle_value = g_fuser_ad_value[rv_type][eSAMPLE_3];
	}
	return at_middle_value;
}


