#include "Board.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_eint.h"
#include "apm32f00x_tmr4.h"
#include "apm32f00x_rcm.h"
#include "apm32f00x_misc.h"
#include "timer.h"
#include "power_manage.h"
#include "ram.h"
/** Timer tick */
uint32_t timer_tick = 0;

extern uint8_t fuser_on_time_flag;               // 开始计时标志位
extern CalibrationTime g_calibration_time;
extern uint32_t sysTick_ms;

CalibrationTime g_real_time;

uint8_t day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

void TimeInit(void)
{
	g_real_time.second = g_calibration_time.second;
	g_real_time.minute = g_calibration_time.minute;
	g_real_time.hour   = g_calibration_time.hour;
	g_real_time.day    = g_calibration_time.day;
	g_real_time.month  = g_calibration_time.month;
	g_real_time.year   = g_calibration_time.year;
}


void Aculate_Real_Time(void)
{
	if(1 == fuser_aculate_time_flag)
	{
		if(sysTick_ms >= 1000 )
		{
			sysTick_ms = 0;
			g_real_time.second++;
		}
		if(g_real_time.second >= 60)
		{
			g_real_time.second = 1;
			g_real_time.minute++;
		}
		if(g_real_time.minute >= 60)
		{
			g_real_time.minute = 1;
			g_real_time.hour++;
		}
		if(g_real_time.hour >= 24)
		{
			g_real_time.hour = 1;
			g_real_time.day++;
		}
		if( ((0 == g_real_time.year % 4)&&(0 != g_real_time.year % 100)) ||(0 == g_real_time.year % 400) )
		{
			if(2 == g_real_time.month)
			{
				if(g_real_time.day >= 29)
				{
					g_real_time.day = 1;
					g_real_time.month++;
				}	
			}
			else
			{
				if(g_real_time.day >= day[g_real_time.month])
				{
					g_real_time.day = 1;
					g_real_time.month++;
				}				
			}
		}
		else
		{
			if(2 == g_real_time.month)
			{
				if(g_real_time.day >= 28)
				{
					g_real_time.day = 1;
					g_real_time.month++;
				}	
			}
			else
			{
				if(g_real_time.day >= day[g_real_time.month])
				{
					g_real_time.day = 1;
					g_real_time.month++;
				}				
			}
		}
		if(g_real_time.month >= 12)
		{
			g_real_time.month = 1;
			g_real_time.year++;
		}
	}	
 	
}

void TMR4Init(void)
{
    /**  Divider = 7, counter = 0xff  */
    TMR4_ConfigTimerBase(7, 0XBC);
    /**  Enable update interrupt  */
    TMR4_EnableInterrupt(TMR4_INT_UPDATE);
    /**  Enable TMR4  */
    TMR4_Enable();

    NVIC_EnableIRQRequest(TMR4_IRQn, 0X03);
}
/*!
 * @brief       TMR4 interrupt service runtine
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void TMR4Isr(void)
{
    if(TMR4_ReadIntFlag(TMR4_INT_UPDATE) == SET)
    {
        TMR4_ClearIntFlag(TMR4_INT_UPDATE);
        timer_tick++;
				g_fuser_temp_high_times++;
				g_fuser_temp_excessive_error_times++;
				g_fuser_ac_error_times++;
				g_fuser_voltage_times++;
				g_fuser_sleep_heat_times++;
			
				//进入休眠模式参数全部清零
				if(g_sleep_value == 1)
				{
					timer_tick                         = 0;
					g_fuser_temp_high_times            = 0;
					g_fuser_temp_excessive_error_times = 0;
					g_fuser_ac_error_times             = 0;
					g_fuser_voltage_times              = 0;
					g_fuser_sleep_heat_times           = 0;
				}
//				if(g_init_time_flag < 7000)
//				{
//					g_init_time_flag++;
//				}
//				if( LOW == Board_ReadIoState(AUTO_SHUT) )
//				{
//					g_auto_shut_200ms_flag = 0;
//				}
//				else if(HIGH == Board_ReadIoState(AUTO_SHUT) )
//				{
//					g_auto_shut_200ms_flag++;
//				}
			
			
			
			}
}

/*!
 * @brief       定影加热时长计时函数
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

void FuserHeaterTimeCount(void)
{
//		int16_t input_IO = 0;
//		input_IO = GPIO_ReadInputBit(FUSER_ON_TIME_PORT, FUSER_ON_TIME_PIN);   // 加热时长IO口测试
		if( 1 == g_fuser_on_time)
		{
//			fuser_on_time_flag = 1; 
			fuser_on_timer++;			

		}
		else
		{
//			 fuser_on_time_flag = 0;     
			 fuser_on_timer = 0;  																											//计时清零	
		}
//	 if(timer_tick >= 1000 )
//	 {
//		timer_tick = 0;
//		if(fuser_on_time_flag == 1)
//		{
//			fuser_on_timer ++;
//		}
//		else
//		{
//			fuser_on_timer = 0;
//		}
//	}
}
