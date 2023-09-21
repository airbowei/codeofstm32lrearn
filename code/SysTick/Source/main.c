/*!
 * @file        main.c
 *
 * @brief       Main program body
 *
 * @version     V1.0.1
 *
 * @date        2022-04-11
 *
 * @attention
 *
 *  Copyright (C) 2018-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be usefull and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */
#include "power_manage.h"
#include "Board.h"
#include "main.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_rcm.h"
#include "stdint.h"
#include "ram.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "adc.h"
#include "error.h"
#include "system_apm32f00x.h"
#include "stdio.h"

#define VERSION1 "##                0.0.0.1                 ##\n"


typedef enum
{
    IAP_INTERRUPT_VECTOR    = ((uint32_t)0), //!< IAP interrupt vector
    APP1_INTERRUPT_VECTOR   = ((uint32_t)1), //!< APP1 interrupt vector
    APP2_INTERRUPT_VECTOR   = ((uint32_t)2), //!< APP2 interrupt vector
} INTERRUPT_VECTOR_STATE_T;

///**@} end of group Application1_Enumerations */

///** @defgroup Application1_Variables Variables
//  @{
//*/

#if defined (__CC_ARM)
/* Indicates that the current interrupt vector*/
volatile INTERRUPT_VECTOR_STATE_T FLASH_interrupt_vector __attribute__((at(0x20000004)));

#elif defined (__ICCARM__)
#pragma location = 0x20000004
__root volatile uint32_t FLASH_interrupt_vector = APP1_INTERRUPT_VECTOR;

#else
/* Indicates that the current interrupt vector */
uint32_t FLASH_interrupt_vector __attribute__((section(".my_data"))) = APP1_INTERRUPT_VECTOR;

#endif
/** System tick */
uint32_t sysTick = 0;
/* global variable*/
uint32_t sysTick_ms = 0;

/** Function declare */
void reqFuserRelayInit(void);
void reqValueUpdate(void);
void Version(void);
void NVIC_EnableIRQRequest_Init(void);
/* enum */
/* static */
//static EPOWER_STATUS s_power_status = ePOWER_STATUS_OFF;
/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
int main(void)
{
//	uint32_t *p = NULL;//read flag pointer
	FLASH_interrupt_vector = APP1_INTERRUPT_VECTOR;
    /** Setup SysTick Timer for 1 msec interrupts */
	NVIC_EnableIRQRequest_Init();
	__enable_irq();
	DataBufInit();
	USARTInit();
	TMR4Init();
	SysTick_Config(RCM_GetMasterClockFreq() / 1000);
	RCM_ConfigHIRCDiv(RCM_HIRC_DIV_2);
	/* LedInit */
	reqFuserRelayInit();
	reqPowerManageInit();
	//Board_OutputHigh(POWER_ON_OFF);
	/**  Unlock flash controler  */
	//reqCleanPowerStatusFlag();
	GpioInit();
	ADCInit();
	Version();
    while(1)
    {
			CmdReceiveProcess();        //收到SOC指令回复函数
			if(sysTick >= 1) // MS
			{
				DebugProcess();
				sysTick = 0;
				sysTick_ms++;
				reqValueUpdate();           //GPIO、ADC值更新函数
				FuserHeaterTimeCount();     //定影加热时长计时函数							
				reqPowerManage();           //开关机管理
				Aculate_Real_Time();        //开始计算当前时间
				actErrorDetection();        //错误检测
			}
			
	}
}
void NVIC_EnableIRQRequest_Init(void)
{
	NVIC_EnableIRQRequest(USART3_RX_IRQn, 0x02);
	NVIC_EnableIRQRequest(USART1_RX_IRQn, 0x01);
	NVIC_EnableIRQRequest(SysTick_IRQn, 0x04);
	NVIC_EnableIRQRequest(TMR4_IRQn, 0x03);
}
/*!
 * @brief       GPIO、ADC值更新函数
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void reqValueUpdate(void)
{
	actGetAdcValue();           //更新ADC值
	actGetGpioValue();          //更新GPIO值
}
/*!
 * @brief       Led init
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void reqFuserRelayInit(void)
{
    Board_LedInit(BOARD_LED2);
    //Board_LedInit(BOARD_LED3);

    //Board_LedOff(BOARD_LED2);
	Board_LedOn(BOARD_LED2);
    //Board_LedOff(BOARD_LED3);
}


void Version(void)
{
	printf("############################################\n");
	printf("##        APM32f003f6u6 Version is:       ##\n");
	printf(VERSION1);
	printf("##compile time is [%s] [%s]##\n",__DATE__,__TIME__);
	printf("############################################\n");
}

int fputc(int ch, FILE *f)
{
   while (USART_ReadStatusFlag(USART1, USART_FLAG_TXBE) == RESET);//等待先前的字符发送完成   
   USART_TxData8(USART1, (uint8_t) ch);    //发送字符
   return ch;
}	

