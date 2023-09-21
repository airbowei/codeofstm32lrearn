/*!
 * @file        Board_APM32F003_MINI.h
 *
 * @brief       This file contains definitions for APM32F003_MINIBOARD's Leds, push-buttons hardware resources
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

#ifndef BOARD_APM32F003_MINI_H
#define BOARD_APM32F003_MINI_H

#include "apm32f00x.h"

#define BOARD_POWER_EN_PORT             GPIOA
#define BOARD_POWER_EN_PIN              GPIO_PIN_1  //自动开机使能  PA1

#define POWER_ON_OFF_PORT   						GPIOA
#define BOARD_POWER_ON_OFF_PIN					GPIO_PIN_2  //电源5V上电   PA2

#define BOARD_PSU_SLEEP_PORT   					GPIOA
#define BOARD_PSU_SLEEP_PIN						  GPIO_PIN_3  //PSU休眠信号（PSU_24V使能）   PA3

#define POWER_AUTO_SHUT_PORT						GPIOD
#define POWER_AUTO_SHUT_PIN							GPIO_PIN_4  //自动关机     PD4

#define FUSER_RELAY_OFF_PORT            GPIOC
#define FUSER_RELAY_OFF_PIN             GPIO_PIN_3  //继电器断开信号  PC3

#define FUSER_ON_TIME_PORT              GPIOC
#define FUSER_ON_TIME_PIN               GPIO_PIN_4  //加热时长检测  PC4

#define AC_V_FLAG_PORT                  GPIOC
#define AC_V_FLAG_PIN                   GPIO_PIN_7  //交流电压标志位检测  PC7

#define BOARD_LED2_GPIO_PORT            GPIOC
#define BOARD_LED2_GPIO_PIN             GPIO_PIN_3

#define BOARD_LED3_GPIO_PORT            GPIOB
#define BOARD_LED3_GPIO_PIN             GPIO_PIN_4

#define BOARD_KEY1_GPIO_PORT            GPIOD
#define BOARD_KEY1_GPIO_PIN             GPIO_PIN_3

#define HIGH ((uint8_t)(1))
#define LOW  ((uint8_t)(0))
/**
 * @brief   Led number definition
 */
typedef enum
{
    BOARD_LED2,
    BOARD_LED3
}BOARD_LED_T;

/**
 * @brief   Key button number definition
 */
typedef enum
{
    BOARD_KEY1
}BOARD_KEY_T;

typedef enum
{
	POWER_EN,
	POWER_ON_OFF,
	AUTO_SHUT,
}BOARD_IO_T;
/** LED */
void Board_LedInit(BOARD_LED_T led);
void Board_LedOn(BOARD_LED_T led);
void Board_LedOff(BOARD_LED_T led);
void Board_LedToggle(BOARD_LED_T led);

/** Key button */
void Board_KeyInit(BOARD_KEY_T key, uint8_t intEnable);
uint8_t Board_ReadKeyState(BOARD_KEY_T key);
/** POWER EN FLAG*/
void Board_InPutInit(BOARD_IO_T pin);
uint8_t Board_ReadIoState(BOARD_IO_T pin);
/** POWER ON/OFF */
void Board_OutPutInit(BOARD_IO_T rv_pin);
void Board_OutputHigh(BOARD_IO_T rv_pin);
void Board_OutputLow(BOARD_IO_T rv_pin);
#endif
