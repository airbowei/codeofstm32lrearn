
#include "gpio.h"
#include "Board.h"
#include "ram.h"
#include "apm32f00x_gpio.h"
//#include "Board_APM32F003_MINI.h"


void GpioInit()
{
		GPIO_Config_T gpioConfig;

    /**  GPIO configuration 加热时长检测  PC4 和 交流电压标志位检测  PC7 */
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;
    gpioConfig.pin = FUSER_ON_TIME_PIN | AC_V_FLAG_PIN;
    gpioConfig.speed = GPIO_SPEED_10MHz;
    GPIO_Config(FUSER_ON_TIME_PORT, &gpioConfig);
	
	   /**  GPIO configuration PSU休眠信号（PSU_24V使能）   PA3 */
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;
    gpioConfig.pin = BOARD_PSU_SLEEP_PIN;
    gpioConfig.speed = GPIO_SPEED_10MHz;
    GPIO_Config(BOARD_PSU_SLEEP_PORT, &gpioConfig);
	
		 /**  GPIO configuration 继电器断开信号  PC3 */
    gpioConfig.mode = GPIO_MODE_OUT_PP;
    gpioConfig.pin = FUSER_RELAY_OFF_PIN;
    gpioConfig.speed = GPIO_SPEED_10MHz;
    GPIO_Config(FUSER_RELAY_OFF_PORT, &gpioConfig);
}

void actGetGpioValue(void)
{
	g_sleep_value     = actSleepQuery();
	g_ac_v_flag_value = actAcFlagQuery();
	g_fuser_on_time   = actFuserOnTimeQuery();
}

//继电器断开
void actFuserRelayOff(void)
{
		GPIO_SetBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
}

//继电器不断开
void actFuserRelayOn(void)
{
		GPIO_ClearBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
}

//获取sleep状态
uint16_t actSleepQuery(void)
{
	return GPIO_ReadInputBit(BOARD_PSU_SLEEP_PORT, BOARD_PSU_SLEEP_PIN);
}

//获取AC_V_FLAG状态
uint16_t actAcFlagQuery(void)
{
	return GPIO_ReadInputBit(AC_V_FLAG_PORT, AC_V_FLAG_PIN);
}

//获取FUSER_ON_TIME状态
uint16_t actFuserOnTimeQuery(void)
{
	return GPIO_ReadInputBit(FUSER_ON_TIME_PORT, FUSER_ON_TIME_PIN);
}

