
#include "power_manage.h"
#include "Board.h"
#include "stdint.h"
#include "apm32f00x_fmc.h"


#define FMC_ADDR          (0x0000)
#define FMC_PAGE_SIZE     (1024)

//#define POWER_EN   				BOARD_PA3
//#define AUTO_SHUT  				BOARD_PC5
//#define POWER_ON_OFF			BOARD_PC3

#define ABNORMAL_SHUTDOWN		1

/* enum */
typedef enum
{
	WAIT_PRESS,
	WAIT_RELEASE,
} EKEY_STATUS;

typedef enum
{
	ePOWER_STATUS_ON,
	ePOWER_STATUS_OFF,
	ePOWER_STATUS_MAX
} EPOWER_STATUS;

/* static variable */
static EKEY_STATUS s_key_status = WAIT_PRESS;
static uint16_t s_key_release_count = 0;
static uint16_t s_key_count = 0;
static EPOWER_STATUS s_power_status = ePOWER_STATUS_OFF;
static BOOL s_key_press = FALSE;
static BOOL s_key_long_press = FALSE;
static BOOL s_key_long_press_handled = FALSE;
static uint8_t last_power_status_flg = ABNORMAL_SHUTDOWN;
static BOOL s_power_auto_on_flg = FALSE;

//global varialbe
 


static void actPowerKeyPressScan( void );
static void actPowerOnCheck(void);
static void actPowerOffCheck(void);
static void actPowerAutoOffCheck(void);
static void actSetPowerStatusFlag( void );//write FMC
static void actCleanPowerStatusFlag( void );//clean FMC


void reqPowerManageInit(void)
{
	Board_KeyInit(BOARD_KEY1, 0);
	Board_InPutInit(POWER_EN);//power en
	Board_InPutInit(AUTO_SHUT);//auto shut
	
	Board_OutPutInit(POWER_ON_OFF);//power on/off	
	Board_OutputLow(POWER_ON_OFF);
	
}


void actPowerKeyPressScan( void )
{
	if( Board_ReadKeyState(BOARD_KEY1) == 0 )
	{			
		s_key_count++;
		s_key_release_count = 0;
	}
	else
	{
		s_key_count = 0;
		s_key_release_count++;
	}
	
	switch( s_key_status )
	{
		case WAIT_PRESS:			
			if( s_key_count > 2)
			{
				s_key_press = TRUE;
				s_key_status = WAIT_RELEASE;
			}
			break;
			
		case WAIT_RELEASE:
			if ( (s_key_count >= 2000) && (s_key_long_press_handled == FALSE) )//此处计时需要更改为2s目前不准
			{
				s_key_release_count = 0;
				s_key_long_press_handled = TRUE;
				s_key_long_press = TRUE;
			}
			
			if( s_key_release_count >= 20) 
			{
				s_key_release_count = 0;
				s_key_long_press_handled = FALSE;
				s_key_status = WAIT_PRESS;
			}
			break;
			
		default:
			break;
			
	}
	
}

/*!
 * @brief       actPowerManage
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void reqPowerManage(void)
{
	actPowerKeyPressScan();
	
	switch(s_power_status)
	{
		//如果当前为关机状态，判断是否有开机请求
		case ePOWER_STATUS_OFF:
			actPowerOnCheck();
		break;
		
		//如果当前为开机状态
		case ePOWER_STATUS_ON:
			actPowerOffCheck();
			//actPowerAutoOffCheck();
		break;
		
		default:
		break;
	}
}
/*!
 * @brief       actPowerOnCheck
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void actPowerOnCheck(void)
{
	uint32_t *p = NULL;//read flag pointer
	
	p = (uint32_t *)FMC_ADDR;
	if(p != NULL)
	{
		last_power_status_flg = p[0];
	}
	
	if( HIGH == Board_ReadIoState(POWER_EN) && s_power_auto_on_flg == FALSE )//上电自动开机模式
	{
		s_power_auto_on_flg = TRUE;
		Board_OutputHigh(POWER_ON_OFF);
		s_power_status = ePOWER_STATUS_ON;
		//actSetPowerStatusFlag();
		s_key_press = FALSE;
		printf("Power on automatically\n");
	}
	else
	{
		/*if( last_power_status_flg == ABNORMAL_SHUTDOWN )//上次异常断电，上电直接开机
		{
			Board_OutputHigh(POWER_ON_OFF);
			s_power_status = ePOWER_STATUS_ON;
		}
		else*/
		//{
			if( s_key_press == TRUE )
			{
				s_key_press = FALSE;
				Board_OutputHigh(POWER_ON_OFF);
				//actSetPowerStatusFlag();
				s_power_status = ePOWER_STATUS_ON;
			}
//			else
//			{
//				s_key_press_hold = FALSE;//ingnore press&hold 
//			}
		//}
	}
}

/*!
 * @brief       actPowerOffCheck
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void actPowerOffCheck(void)
{
//	if( s_key_press == TRUE )	
//	{
		if( LOW == Board_ReadIoState(AUTO_SHUT) )
		{
			//actCleanPowerStatusFlag();
						
			Board_OutputLow(POWER_ON_OFF);
			s_key_press = FALSE;
			s_power_status = ePOWER_STATUS_OFF;
			printf("AUTO_SHUT automatically power off\n");
		}
//	}
	
	if( s_key_long_press == TRUE )
	{
		//actCleanPowerStatusFlag();
		
		Board_OutputLow(POWER_ON_OFF);
		s_key_press = FALSE;
		s_key_long_press = FALSE;
		s_power_status = ePOWER_STATUS_OFF;
		printf("press and hold the key to shut down\n");
	}
}

/*!
 * @brief       actPowerAutoOffCheck
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        
 */
void actPowerAutoOffCheck(void)
{
	if( LOW == Board_ReadIoState(AUTO_SHUT) )
	{	
		Board_OutputLow(POWER_ON_OFF);
		//actCleanPowerStatusFlag();
		//last_power_status_flg = 0;
		s_power_status = ePOWER_STATUS_OFF;
	}
}

/*!
 * @brief       actCleanPowerStatusFlag
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        Set Power Status Flag = OFF(0)
 */
void actCleanPowerStatusFlag( void )
{		
	/**  Unlock flash controler  */
	FMC_Unlock();
	/**  Erase page  */
    FMC_ErasePage(FMC_ADDR);
	FMC_ProgramWord(FMC_ADDR, 0);
	/**  Lock flash controler  */
	FMC_Lock();
}
/*!
 * @brief       actSetPowerStatusFlag
 *
 * @param       None
 *
 * @retval      None
 *
 * @note        Set Power Status Flag = ON(1)
 */
void actSetPowerStatusFlag( void )
{
	/**  Unlock flash controler  */
	FMC_Unlock();
	/**  Erase page  */
	FMC_ErasePage(FMC_ADDR);
	FMC_ProgramWord(FMC_ADDR, 1);
	/**  Lock flash controler  */
	FMC_Lock();
}
