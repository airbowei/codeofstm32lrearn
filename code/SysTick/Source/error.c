#include "error.h"
#include "Board.h"
#include "apm32f00x_gpio.h"
#include "ram.h"
#include "adc.h"
#include "gpio.h"
#include "uart.h"
#include "apm32f00x_fmc.h"
#include "security_manage.h"
#include "stdio.h"

/********************************************************************************/
/*宏定义                                                                        */
/********************************************************************************/
#define FMC_ADDR1          (0x7C00)
#define FMC_PAGE_SIZE1      (1024)
/********************************************************************************/
/*静态常亮定义                                                                   */
/********************************************************************************/
  typedef enum
{
	eRANGE_FIRST  = 0,
	eRANGE_SECOND = 1,
	eRANGE_THIRD  = 2,
	eRANGE_FORTH  = 3,
}EVOLTAGE_RANGE;                
/********************************************************************************/
/*内部函数原型声明                                            */
/********************************************************************************/
static uint16_t actFuserOnTimeError(void);
static uint16_t actFuserExcessiveTempErrorError(void);
static int16_t AcVError(void);
static uint16_t actSleepHeatError(void);
static uint16_t actFuserOnTimeError(void);
static uint16_t actSleepHeatError(void);
static void actWriteErrorInfo( uint8_t *data,uint32_t addr);

static uint16_t fuser_temp_excessive_times_flag = 0;
static uint16_t fuser_temp_error_times_flag = 0;
static uint16_t fuser_sleep_heat_error_times_flag = 0;

static uint16_t fuser_out_temp_first_flag = 1;         //上电第一次检测过温错误标志位 //硬件要求第一次上电延时2秒后再开始检测
static uint16_t fuser_excessive_temp_first_flag = 1;   //上电第一次检测温度误差错误标志位 //硬件要求第一次上电延时2秒后再开始检测
static uint16_t actFuserTimeError(EVOLTAGE_RANGE rv_range);

extern CalibrationTime g_real_time;


#define FUSER_HEAT_TIME_MAX             									1000
#define FUSER_TEMP_EXCESSIVE_MAX       										5
#define FUSER_OUT_FUSER_TEMP_ERROR     									 	259

#define FUSER_ERROR_INFO_SIZE           									8     //错误信息大小
#define FUSER_DETECTION_INTERVAL_TIME   									1000  //MS  错误检测间隔时间
#define FUSER_MV_TO_V  									                  1000  //1000mv 用于同一单位

//温差检测
#define FUSER_DETECTION_TEMP_EXCESSIVE_VOLTAGE            1000 //温差检测不同公式分界电压
#define FUSER_DETECTION_TEMP_EXCESSIVE_TIMES              3 //连续检测error次数超过3次就报错

//AC电压检测
#define FUSER_AC_LOW_VOLTAGE_110             							760  //单位10V
#define FUSER_AC_HIGH_VOLTAGE_110             						1600 //单位10V
#define FUSER_AC_LOW_VOLTAGE_220             							1600 //单位10V
#define FUSER_AC_HIGH_VOLTAGE_220            							3100 //单位10V
#define FUSER_DETECTION_AC_VOLTAGE_TIMES                  3 //连续检测error次数超过3次就报错

//休眠加热
#define FUSER_DETECTION_SLEEP_HEAT_TIME   								100  //MS  错误检测间隔时间

//过温
#define FUSER_DETECTION_TEMP_OUT_TIMES                    5 //连续检测error次数超过5次就报错
#define FUSER_DETECTION_TEMP_OUT_TIME   								  100  //MS  错误检测间隔时间
/*!
 * @brief       actWriteErrorInfo
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        Write Error Info to Flash 
 */
static void actWriteErrorInfo( uint8_t *data,uint32_t addr)
{
	int  i = 0;
	/**  Unlock flash controler  */
	FMC_Unlock();
	/**  Erase page  */
	FMC_ErasePage(addr);
	for(i = 0; i < FUSER_ERROR_INFO_SIZE; i++)
	{
		addr += 1;
		FMC_ProgramWord(addr, data[i]);
	}
	
	/**  Lock flash controler  */
	FMC_Lock();
}

/*!
 * @brief       actEraseErrorInfo
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        Erase Error Info to Flash 
 */
void actEraseErrorInfo(void)
{	

	/**  Unlock flash controler  */
	FMC_Unlock();
	/**  Erase page  */
	FMC_ErasePage(FMC_ADDR1);
	
	/**  Lock flash controler  */
	FMC_Lock();
}


/*!
 * @brief       actReadErrorInfo
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        Read Error Info to Flash 
 */
void actReadErrorInfo(void)
{	
	uint32_t *read_data = NULL;
	uint32_t data_temp = 0;
	uint32_t read_addr = 0;
	uint8_t  data[FUSER_ERROR_INFO_SIZE];
	int i = 0;
	
	read_addr = FMC_ADDR1;
	/**  Read and verify data  */
//	data = (uint32_t *)FMC_ADDR1;
	for(i = 0; i < FUSER_ERROR_INFO_SIZE; i++)
	{
		
		read_data = (uint32_t *)read_addr;
		data_temp =   *read_data;
		data[i] = data_temp;
		read_addr += 4;
	}
	actSendDateUsart1(&data[0],FUSER_ERROR_INFO_SIZE);
//	actSendDateUsart1_32(&read_data[0],32);
}


///*!
// * @brief       定影加热时长异常检测
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */

static uint16_t actFuserOnTimeError(void)
{



if((STATUS_NOT_SLEEP == g_sleep_value) && (1 == receive_threshold_time_value_flag))
{
	if(g_fuser_on_time == 1)
	{
		if(fuser_on_timer >= (heat_time.g_heat_time_forth * FUSER_MV_TO_V))
		{
			return actFuserTimeError(eRANGE_FORTH);
		}
		else if(fuser_on_timer >= (heat_time.g_heat_time_third * FUSER_MV_TO_V))
		{
			return actFuserTimeError(eRANGE_THIRD);
		}
		else if(fuser_on_timer >= (heat_time.g_heat_time_second * FUSER_MV_TO_V))
		{
			return actFuserTimeError(eRANGE_SECOND);
		}
		else if(fuser_on_timer >= (heat_time.g_heat_time_first * FUSER_MV_TO_V))
		{
			return actFuserTimeError(eRANGE_FIRST);
		}
	 }
 }
	return 0;
}

///*!
// * @brief       加热时长异常判断
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */

static uint16_t actFuserTimeError(EVOLTAGE_RANGE rv_range)
{
	uint16_t at_AcVoltage = 0;
	uint32_t at_fuser_ontime = 0; 
	at_fuser_ontime = fuser_on_timer;
	
	if(1 == g_test_heat_time_flag)
	{
		at_fuser_ontime = g_test_heat_time * FUSER_HEAT_TIME_MAX;
		return 1;
	}
	if(g_voltage_formula_parameter1 != 0)
	{
		at_AcVoltage = ( ((g_avaliable_ad_value[ePOWER_SAMPLE] * 100) / g_voltage_formula_parameter1) + g_voltage_formula_parameter2);
	}
	else
	{
		return 0;
	}
//	if(g_ac_v_flag_value == 1)
//	{	
		switch (rv_range)
		{
		case eRANGE_FORTH:
			if(at_AcVoltage >= voltage_range.g_ac_voltage_range_forth)
			{
				printf("ERROR:at_AcVoltage >= %d at_AcVoltage is %d,fuser_on_timer is %u (s)\n",voltage_range.g_ac_voltage_range_forth,at_AcVoltage,at_fuser_ontime );
				return 1;
			}
			break;
		case eRANGE_THIRD:
			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_third) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_forth))
			{
				printf("ERROR:%d <= at_AcVoltage <=%d at_AcVoltage is %d,fuser_on_timer is %u (s)\n",voltage_range.g_ac_voltage_range_third,voltage_range.g_ac_voltage_range_forth,at_AcVoltage,at_fuser_ontime );
				return 1;
			}
			break;
		case eRANGE_SECOND:
			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_second) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_third))
			{
				printf("ERROR:%d <= at_AcVoltage <=%d at_AcVoltage is %d,fuser_on_timer is %u (s)\n",voltage_range.g_ac_voltage_range_second,voltage_range.g_ac_voltage_range_third,at_AcVoltage,at_fuser_ontime );
				return 1;
			}
			break;
		case eRANGE_FIRST:
			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_first) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_second))
			{
				printf("ERROR:%d <= at_AcVoltage <=%d at_AcVoltage is %d,fuser_on_timer is %u (s)\n",voltage_range.g_ac_voltage_range_first,voltage_range.g_ac_voltage_range_second,at_AcVoltage,at_fuser_ontime );
				return 1;
			}
			break;
								
		default:
			break;
		}

//	}
//	else if(g_ac_v_flag_value == 0)
//	{	
//		switch (rv_range)
//		{
//		case eRANGE_FORTH:
//			if(at_AcVoltage >= voltage_range.g_ac_voltage_range_forth)
//			{
//				printf("ERROR:at_AcVoltage >= 120 at_AcVoltage is %d,fuser_on_timer is %u (s)\n",at_AcVoltage,at_fuser_ontime );
//				return 1;
//			}
//			break;
//		case eRANGE_THIRD:
//			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_third) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_forth))
//			{
//				printf("ERROR:110 <= at_AcVoltage <=120 at_AcVoltage is %d,fuser_on_timer is %u (s)\n",at_AcVoltage,at_fuser_ontime );
//				return 1;
//			}
//			break;
//		case eRANGE_SECOND:
//			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_second) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_third))
//			{
//				printf("ERROR:100 <= at_AcVoltage <=110 at_AcVoltage is %d,fuser_on_timer is %u (s)\n",at_AcVoltage,at_fuser_ontime );
//				return 1;
//			}
//			break;
//		case eRANGE_FIRST:
//			if((at_AcVoltage >= voltage_range.g_ac_voltage_range_first) && (at_AcVoltage <= voltage_range.g_ac_voltage_range_second))
//			{
//				printf("ERROR:90 <= at_AcVoltage <=100 at_AcVoltage is %d,fuser_on_timer is %u (s)\n",at_AcVoltage,at_fuser_ontime );
//				return 1;
//			}
//			break;
//								
//		default:
//			break;
//		}

//	}

return 0;
}

///*!
// * @brief       定影温差过大检测
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */

static uint16_t actFuserExcessiveTempErrorError(void)
{
	int16_t at_value = 0;  
	int16_t at_value1 = 0; 
if(STATUS_NOT_SLEEP == g_sleep_value)
{	
	if(((g_fuser_temp_excessive_error_times >= FUSER_DETECTION_INTERVAL_TIME) && (fuser_excessive_temp_first_flag == 0)) 
		|| ((fuser_excessive_temp_first_flag == 1) && (1 == g_start_5v_flag))) //上电后第一次检测需要等EC_5v给电后再开始测试，以后每次需要间隔1秒检测一次
	{
		fuser_excessive_temp_first_flag = 0;
		g_fuser_temp_excessive_error_times = 0;      //计时1秒清零
		if(g_avaliable_ad_value[eTEMPRATURE_SAMPLE] > FUSER_DETECTION_TEMP_EXCESSIVE_VOLTAGE)
		{
			if(g_avaliable_ad_value[eTEMPRATURE_SAMPLE] != 0)
			{
//			at_value = ((g_avaliable_ad_value[eTEMPRATURE_SAMPLE]- g_SOC_temperature_adc) * 100  / g_avaliable_ad_value[eTEMPRATURE_SAMPLE]);//（Vx1 = (Vsoc - Vmcu) / Vmcu）乘以100，将5%放大100倍
//			at_value1 = ((g_SOC_temperature_adc - g_avaliable_ad_value[eTEMPRATURE_SAMPLE]) * 100 / g_avaliable_ad_value[eTEMPRATURE_SAMPLE]);//（Vx1 = (Vsoc - Vmcu) / Vmcu）乘以100，将5%放大100倍
				at_value  = abs(g_avaliable_ad_value[eTEMPRATURE_SAMPLE]- g_SOC_temperature_adc)* 100 / g_avaliable_ad_value[eTEMPRATURE_SAMPLE];//（Vx1 = (Vsoc - Vmcu) / Vmcu）乘以100，将5%放大100倍;
			}
			else
			{
				printf("ERROR:g_avaliable_ad_value[eTEMPRATURE_SAMPLE] is 0 v  ");
			}
		}
		else
		{
			at_value = abs(g_avaliable_ad_value[eTEMPRATURE_SAMPLE]- g_SOC_temperature_adc)/10;  //(Vx2 = Vsoc - Vmcu)除以10，将单位转换为mv,本来单位为10mv
//		at_value1 = (g_SOC_temperature_adc - g_avaliable_ad_value[eTEMPRATURE_SAMPLE])/10;	//(Vx2 = Vsoc - Vmcu)除以10，将单位转换为mv，本来单位为10mv
		}
		//电压大于1000mv时，温度转换的电压误差(Vsoc - Vmcu)/Vmcu > 5% => (Vsoc - Vmcu)*100/Vmcu > 5
		//电压小于1000mv时，温度转换的电压误差Vsoc - Vmcu > 5mv
		if((at_value >= FUSER_TEMP_EXCESSIVE_MAX)
		  || (1 == g_test_temp_exessive_flag))  
		{
			fuser_temp_excessive_times_flag++;//检测周期为1秒			
			if(fuser_temp_excessive_times_flag >= FUSER_DETECTION_TEMP_EXCESSIVE_TIMES)
			{
				fuser_temp_excessive_times_flag = 0;
				return 1;
			}
		}

		else
		{
			fuser_temp_excessive_times_flag = 0;
		}

	}
} 
return 0;
}

///*!
// * @brief       定影温度过大检测(过温)
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */

static uint16_t actOutFuserTempError(void)
{
uint16_t at_tempratrue = 0;
at_tempratrue = g_avaliable_ad_value[eTEMPRATURE_SAMPLE];

//测试报错专用	
if(1 == g_test_temp_flag)
{
	at_tempratrue = g_test_temp;
}

if(STATUS_NOT_SLEEP == g_sleep_value)
{		
	if(((g_fuser_temp_high_times >= FUSER_DETECTION_TEMP_OUT_TIME) && (fuser_out_temp_first_flag == 0) )
		|| ((fuser_out_temp_first_flag == 1) && (1 == g_start_5v_flag)))  //上电后第一次检测需要等EC_5v给电后再开始测试，以后每次需要间隔100毫秒检测一次
	{
		fuser_out_temp_first_flag = 0;
		g_fuser_temp_high_times = 0;
		if(at_tempratrue <= g_out_temperature_protect_value)   //电压越小温度越大
		{		
				fuser_temp_error_times_flag++; //检测周期为0.1秒
			if(fuser_temp_error_times_flag >= FUSER_DETECTION_TEMP_OUT_TIMES )
			{
				fuser_temp_error_times_flag = 0;
				printf("ERROR:actOutFuserTempError is %u mv\n",at_tempratrue );
				return 1;
			}	
		}

		else
		{
			fuser_temp_error_times_flag = 0;   //温度未超过指定阈值时，错误次数清零，以保证报错时是连续5次检测到温度超过阈值。 
			
		}
	}
}
return 0;
}

///*!
// * @brief       睡眠时有加热异常检测
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */

static uint16_t actSleepHeatError(void)
{
if(g_test_sleep_heat_flag == 1)
{
		return 1;
}
if(STATUS_SLEEP == g_sleep_value)
{
	fuser_out_temp_first_flag = 1;         //休眠后，也需要等待2s再进行过温检测。上电第一次检测过温错误标志位 //硬件要求第一次上电延时2秒后再开始检测
	fuser_excessive_temp_first_flag = 1;   //休眠后，也需要等待2s再进行温差检测.上电第一次检测温度误差错误标志位 //硬件要求第一次上电延时2秒后再开始检测
	if(g_fuser_sleep_heat_times >= FUSER_DETECTION_SLEEP_HEAT_TIME)
	{
		g_fuser_sleep_heat_times = 0;
//		input_IO = GPIO_ReadInputBit(FUSER_ON_TIME_PORT, FUSER_ON_TIME_PIN);   // 加热时长IO口  0:不加热 1:加热
		if(1 == g_fuser_on_time)
		{
			fuser_sleep_heat_error_times_flag++;
			if(fuser_sleep_heat_error_times_flag >= FUSER_DETECTION_AC_VOLTAGE_TIMES)
			{
					fuser_sleep_heat_error_times_flag = 0;
					printf("ERROR:sleep_heat");
					return 1;
			}
		}
		else
		{
			fuser_sleep_heat_error_times_flag = 0;
		}
	}

}

return 0;
}

///*!
// * @brief       220电压异常检测
// *
// * @param       None
// *
// * @retval      None 1欠压  -1过压
// *
// * @note
// */

static int16_t AcVError(void)
{
//	int16_t at_input_IO = 0;
	uint16_t at_AcVoltage = 0;
	
	if((STATUS_NOT_SLEEP == g_sleep_value)&& (1 == g_start_5v_flag))
	{
		if((g_fuser_ac_error_times >= FUSER_DETECTION_INTERVAL_TIME) && (1 == g_Power_Status_Flag ))   //g_Power_Status_Flag 该标志位表示ADC已经经过ADC电压滤波才能经行判错
		{
	//		printf("the time is %d %d %d %d %d %d \n",g_real_time.year,g_real_time.month,g_real_time.day,g_real_time.hour,g_real_time.minute,g_real_time.second);
			g_fuser_ac_error_times = 0;
			at_AcVoltage = ( ((g_avaliable_ad_value[ePOWER_SAMPLE] * 100) / g_voltage_formula_parameter1) + g_voltage_formula_parameter2);
//				at_input_IO = GPIO_ReadInputBit(AC_V_FLAG_PORT, AC_V_FLAG_PIN);   // 交流电压标志位检测IO口测试

			//7125:AC_V_FLAG信号为0;110V:AC = (ADC*51.4)+12.5  =>  AC = (ADC/(1/51.4))+12.5 => AC = (ADC/(10000/51.4))+12.5 10000/51.4是soc发过来的参数1的值  AC = (ADC * 100/(10000/51.4))+12.5 AC单位为10V
			//7125:AC_V_FLAG信号为1;220V:AC = (ADC*103)+12.5   =>  AC = (ADC/(1/103))+12.5  => AC = (ADC/(10000/103))+12.5  10000/103是soc发过来的参数1的值   AC = (ADC * 100/(10000/103))+12.5  AC单位为10V
				if(g_ac_v_flag_value == 0)
				{					
						//测试使用
					if(1 == g_test_voltage_flag)
					{
						at_AcVoltage = g_test_voltage;
					}
					
					if(at_AcVoltage < FUSER_AC_LOW_VOLTAGE_110)  //应为76 ，放大10倍进行比较
					{
						g_fuser_ac_normal_flag = 0;	
						g_fuser_ac_low_error_flag++;
						if(g_fuser_ac_low_error_flag >= FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_low_error_flag = 0;	
													
							printf("ERROE:110 AC voltage is lower than 76 \n");		
							return 1;
						}
					}
					else if(at_AcVoltage > FUSER_AC_HIGH_VOLTAGE_110)  
					{
						g_fuser_ac_normal_flag = 0;
						g_fuser_ac_high_error_flag++;
						if(g_fuser_ac_high_error_flag >= FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_high_error_flag = 0;

							printf("ERROE:110 AC voltage is higher than 160 \n");		
							return -1;
						}
					}
					else
					{
						g_fuser_ac_high_error_flag = 0;
						g_fuser_ac_low_error_flag = 0;
						g_fuser_ac_normal_flag++;
						if(g_fuser_ac_normal_flag > FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_normal_flag = 1;
						}
						return 0;
					}
				}
				else if(g_ac_v_flag_value == 1)
				{
					//at_AcVoltage = 3200;
					printf("at_AcVoltage = %d\n",at_AcVoltage);
					
					//测试使用
					if(1 == g_test_voltage_flag)
					{
						at_AcVoltage = g_test_voltage;
					}
					
					if(at_AcVoltage < FUSER_AC_LOW_VOLTAGE_220)  //应为160 ，测试需要暂时该为160
					{
						g_fuser_ac_normal_flag = 0;						
						g_fuser_ac_low_error_flag++;
						if(g_fuser_ac_low_error_flag >= FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_low_error_flag = 0;
							printf("ERROE:220 AC voltage is lower than 160 \n");		
							return 1;
						}
					}
					else if(at_AcVoltage > FUSER_AC_HIGH_VOLTAGE_220)  
					{
						g_fuser_ac_normal_flag = 0;						
						g_fuser_ac_high_error_flag++;
						if(g_fuser_ac_high_error_flag >= FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_high_error_flag = 0;
							printf("ERROE:220 AC voltage is higher than 310 \n");		
							return -1;
						}
					}
					else
					{
						g_fuser_ac_high_error_flag = 0;
						g_fuser_ac_low_error_flag = 0;		
						g_fuser_ac_normal_flag++;		
						if(g_fuser_ac_normal_flag > FUSER_DETECTION_AC_VOLTAGE_TIMES)
						{
							g_fuser_ac_normal_flag = 1;
						}
						return 0;
					}
				}
		}
	}//g_sleep_value
return 0;
}



///*!
// * @brief       异常检测
// *
// * @param       None
// *
// * @retval      None 
// *
// * @note
// */
void actErrorDetection(void)
{
	  uint8_t data_error[8];
    uint32_t addr = 0;
		int16_t tmp = 0;

		tmp = AcVError();
	if(FUSER_DETECTION_AC_VOLTAGE_TIMES == g_fuser_ac_normal_flag)
	{
			g_AC_detection_flag = 1;
		
	}
	if(g_error_code == 0)
	{	
		if(1 == actFuserOnTimeError())
		{
			g_error_code = OUT_HEAT_TIME_ERROR_CODE;
			printf("EEROR:OUT_HEAT_TIME_ERROR_CODE\n");
		}
		if(1 == actFuserExcessiveTempErrorError())
		{
			g_error_code = EXCESSIVE_TEMP_ERROR_ERROR_CODE;
			printf("EEROR:EXCESSIVE_TEMP_ERROR_ERROR_CODE\n");
		}
		
		
		if(-1 == tmp)
		{
			g_error_code = AC_V_HIGH_ERROR_CODE;
			printf("EEROR:AC_V_HIGH_ERROR_CODE\n");
		}
		else if(1 == tmp)
		{
			g_error_code = AC_V_LOW_ERROR_CODE;
			printf("EEROR:AC_V_LOW_ERROR_CODE\n");
		}
			
		if(1 == actOutFuserTempError())
		{
			g_error_code = OUT_TEMP_ERROR_CODE;	
			printf("EEROR:OUT_TEMP_ERROR_CODE\n");		
		}
		
		if(1 == actSleepHeatError())
		{
			g_error_code = SLEEP_HEAT_ERROR_CODE;
			printf("EEROR:SLEEP_HEAT_ERROR_CODE\n");		
		}
		
		if((g_error_code == OUT_TEMP_ERROR_CODE) || (g_error_code == OUT_HEAT_TIME_ERROR_CODE) 
		|| (g_error_code == AC_V_HIGH_ERROR_CODE)|| (g_error_code == AC_V_LOW_ERROR_CODE) 
		||(g_error_code == SLEEP_HEAT_ERROR_CODE)|| (g_error_code == EXCESSIVE_TEMP_ERROR_ERROR_CODE))
		{
			actFuserRelayOff();
			printf("UART:close Relay\n");
		}
	}


	if((g_error_code != 0) && (error_flash_flag == 0) && (1 == fuser_aculate_time_flag))
	{
		error_flash_flag = 1;
		//actWriteErrorInfo( &data_error[0],addr);
	FMC_Unlock();
		addr = FMC_ADDR1;
		FMC_ErasePage(addr);

		data_error[0] = g_real_time.year;
		FMC_ProgramWord(addr, data_error[0]);
		addr+= 4;
		data_error[1] = g_real_time.month;
		FMC_ProgramWord(addr, data_error[1]);
		addr+= 4;
		data_error[2] = g_real_time.day;
		FMC_ProgramWord(addr, data_error[2]);
		addr+= 4;
		data_error[3] = g_real_time.hour;
		FMC_ProgramWord(addr, data_error[3]);
		addr+= 4;
		data_error[4] = g_real_time.minute;
		FMC_ProgramWord(addr, data_error[4]);
		addr+= 4;
		data_error[5] = g_real_time.second;
		FMC_ProgramWord(addr, data_error[5]);
		addr+= 4;
		data_error[6] = g_error_code << 8;
		FMC_ProgramWord(addr, data_error[6]);
		addr+= 4;
		data_error[7] = g_error_code;
		FMC_ProgramWord(addr, data_error[7]);

	FMC_Lock();
	}
		
}
