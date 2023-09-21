
#include "gpio.h"
#include "Board.h"
#include "ram.h"
#include "apm32f00x_gpio.h"
//#include "Board_APM32F003_MINI.h"


void GpioInit()
{
		GPIO_Config_T gpioConfig;

    /**  GPIO configuration ����ʱ�����  PC4 �� ������ѹ��־λ���  PC7 */
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;
    gpioConfig.pin = FUSER_ON_TIME_PIN | AC_V_FLAG_PIN;
    gpioConfig.speed = GPIO_SPEED_10MHz;
    GPIO_Config(FUSER_ON_TIME_PORT, &gpioConfig);
	
	   /**  GPIO configuration PSU�����źţ�PSU_24Vʹ�ܣ�   PA3 */
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;
    gpioConfig.pin = BOARD_PSU_SLEEP_PIN;
    gpioConfig.speed = GPIO_SPEED_10MHz;
    GPIO_Config(BOARD_PSU_SLEEP_PORT, &gpioConfig);
	
		 /**  GPIO configuration �̵����Ͽ��ź�  PC3 */
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

//�̵����Ͽ�
void actFuserRelayOff(void)
{
		GPIO_SetBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
}

//�̵������Ͽ�
void actFuserRelayOn(void)
{
		GPIO_ClearBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
}

//��ȡsleep״̬
uint16_t actSleepQuery(void)
{
	return GPIO_ReadInputBit(BOARD_PSU_SLEEP_PORT, BOARD_PSU_SLEEP_PIN);
}

//��ȡAC_V_FLAG״̬
uint16_t actAcFlagQuery(void)
{
	return GPIO_ReadInputBit(AC_V_FLAG_PORT, AC_V_FLAG_PIN);
}

//��ȡFUSER_ON_TIME״̬
uint16_t actFuserOnTimeQuery(void)
{
	return GPIO_ReadInputBit(FUSER_ON_TIME_PORT, FUSER_ON_TIME_PIN);
}

