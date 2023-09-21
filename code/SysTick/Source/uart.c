
#include "uart.h"
#include "Board.h"
#include "stdint.h"
#include "adc.h"
#include "error.h"
#include "ram.h"
#include <string.h>
#include "stdio.h"
#include "gpio.h"
#include "timer.h"
#include "security_manage.h"
#include "apm32f00x_fmc.h"

#define true   1
#define false  0



uint8_t rxDataBufUSART3[DATA_BUF_SIZE] = {0};
uint8_t txDataBufUSART3[DATA_BUF_SIZE] = {0};
uint8_t rxDataBufUSART1[DATA_BUF_SIZE] = {0};

//uint8_t  at_data[DATA_BUF_SIZE] = {0};

uint8_t rxDataUSART3[DATA_BUF_SIZE] = {0};

uint8_t ADC_print_value[5] = {0};

uint8_t rxDataPt = 0;
uint8_t g_rxDataPt = 0;
uint8_t rxDataPt_len = 0;                       //有效数据长度
uint8_t debug_rxDataPt = 0;

CalibrationTime g_calibration_time;

static uint16_t s_is_recv_complete = false;
static uint16_t s_is_debug_recv_complete = false;

uint32_t JumpAddress;
typedef  void (*pFunction)(void);
pFunction Jump_To_Application;

extern uint32_t fuser_on_timer;                         //加热时长  单位（百毫秒）
/** USART Init */
void USARTInit(void);
/** Delay */
void Delay(uint32_t count);
/** Data buffer init */
void DataBufInit(void);
/** Compares two buffers */
BOOL BufferCompare(uint8_t *buf1, uint8_t *buf2, uint8_t size);
//void print(uint8_t* pdata);
void NumberToChar(uint16_t r_value,uint8_t* value);                           // 将4位数字转换为字符
 
void NumberToCharsix(uint16_t r_value,uint8_t* value);                        // 将6位数字转换为字符

void NumberToChar_plus_minus(uint16_t r_value,uint8_t* value);                 // 将4位带正负数字转换为字符

static void DividePackHandle(uint8_t *rv_data,uint8_t rv_len);                 //对接收到的串口数据分包处理


/*!
 * @brief       USART Init
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void USARTInit(void)
{
    USART_Config_T usartConfig;

    /**  BaudRate is 57600 */
    usartConfig.baudRate = 57600;
    /**  No interrupt */
    usartConfig.interrupt = USART_INT_RX ;//| USART_INT_IDLE;
    /**  Enable receiver */
    usartConfig.mode = USART_MODE_TX_RX;
    /**  Parity disable */
    usartConfig.parity = USART_PARITY_NONE;
    /**  One stop bit */
    usartConfig.stopBits = USART_STOP_BIT_1;
    /**  Word length is 8bit */
    usartConfig.wordLength = USART_WORD_LEN_8B;
    /**  USART1 configuration */
    USART_Config(USART3, &usartConfig);



    
		usartConfig.baudRate = 115200;
    usartConfig.mode = USART_MODE_TX_RX;
		usartConfig.interrupt = USART_INT_RX; 
    USART_Config(USART1, &usartConfig);


    /**  Enable USART3 */
    USART_Enable(USART3);
		USART_Enable(USART1);
    /**  Enable USART2 */
//    USART_Enable(USART1);
    /**  Enable USART1 Receiver IRQ request */
    NVIC_EnableIRQRequest(USART3_RX_IRQn, 0x02);
	  NVIC_EnableIRQRequest(USART1_RX_IRQn, 0x01);



}

/*!
 * @brief       Delay
 *
 * @param       count:  delay count
 *
 * @retval      None
 *
 * @note
 */
void Delay(uint32_t count)
{
    volatile uint32_t delay = count;

    while(delay--);
}

/*!
 * @brief       Data buffer init
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void DataBufInit(void)
{
    uint8_t i;

    for(i = 0; i < DATA_BUF_SIZE; i++)
    {
        txDataBufUSART3[i] = 0xa0 + i;
        rxDataBufUSART3[i] = 0;
    }
}

/*!
 * @brief       Compares two buffers
 *
 * @param       buf1:    First buffer to be compared
 *
 * @param       buf1:    Second buffer to be compared
 *
 * @param       size:    Buffer size
 *
 * @retval      Return TRUE if buf1 = buf2. If not then return FALSE
 *
 * @note
 */
BOOL BufferCompare(uint8_t *buf1, uint8_t *buf2, uint8_t size)
{
    uint8_t i;

    for(i = 0; i < size; i++)
    {
        if(buf1[i] != buf2[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*!
 * @brief       USART3 receiver interrupt service routine
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void USART3RxIsr(void)
{
		volatile uint8_t temp;//临时变量=
		USART_EnableInterrupt(USART3, USART_INT_IDLE);
		USART_ClearIntFlag(USART3, USART_INT_FLAG_IDLE);
    if(USART_ReadIntFlag(USART3, USART_INT_FLAG_RX) == SET)
    {
        if(rxDataPt < DATA_BUF_SIZE)
        {
            rxDataBufUSART3[rxDataPt++] = USART_RxData8(USART3);
        }

    }


		if(SET == USART_ReadStatusFlag(USART3, USART_FLAG_IDLEE))
    {
			USART_ClearIntFlag(USART3, USART_INT_FLAG_IDLE);
			s_is_recv_complete = true;
			g_rxDataPt = rxDataPt;
			rxDataPt = 0;
				temp = USART3->STS;
				temp = USART3->DATA;

			USART_DisableInterrupt(USART3, USART_INT_IDLE);
		}

}

void USART1RxIsr(void)
{
		volatile uint8_t temp;//临时变量
		USART_EnableInterrupt(USART1, USART_INT_IDLE);
		USART_ClearIntFlag(USART1, USART_INT_FLAG_IDLE);
    if(USART_ReadIntFlag(USART1, USART_INT_FLAG_RX) == SET)
    {
        if(debug_rxDataPt < DATA_BUF_SIZE)
        {
            rxDataBufUSART1[debug_rxDataPt++] = USART_RxData8(USART1);
        }

    }


		if(SET == USART_ReadStatusFlag(USART1, USART_FLAG_IDLEE))
    {
			USART_ClearIntFlag(USART1, USART_INT_FLAG_IDLE);
			s_is_debug_recv_complete = true;
				temp = USART1->STS;
				temp = USART1->DATA;

			USART_DisableInterrupt(USART1, USART_INT_IDLE);
		}
	
}

/*!
 * @brief       USART3 Send DATE 
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
uint8_t actSendDateUsart3(uint8_t* pdata,uint32_t len)
{
	uint8_t a = 0;
	for(a = 0; a < len; a++)
	{
			/** Wait until end of transmission */
			while(USART_ReadStatusFlag(USART3, USART_FLAG_TXBE) == RESET);
			USART_TxData8(USART3, pdata[a]);


	}

	return 1;
	
}
/*!
 * @brief       USART3 Send PC  串口调试 
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
uint8_t actSendDateUsart1(uint8_t* pdata,uint32_t len)
{
	uint8_t a = 0;
	for(a = 0; a < len; a++)
	{
			/** Wait until end of transmission */
			while(USART_ReadStatusFlag(USART1, USART_FLAG_TXBE) == RESET)
			{
					
				//Wait until end of transmission
			}
			
			pdata[a] = pdata[a];
			USART_TxData9(USART1, pdata[a]);


	}

	return 1;
	
}
/*!
 * @brief       actPrint1 add \r\n
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void actPrint1(uint8_t* data,uint32_t leng)
{
	uint8_t int_huan_hang1[2] = "\n";
	actSendDateUsart1(&data[0],leng);
	actSendDateUsart1(&int_huan_hang1[0],2);
}

uint8_t actSendDateUsart1_32(uint32_t* pdata,uint32_t len)
{
	uint8_t a = 0;
	for(a = 0; a < len; a++)
	{
			/** Wait until end of transmission */
			while(USART_ReadStatusFlag(USART1, USART_FLAG_TXBE) == RESET);
			pdata[a] = pdata[a];
			USART_TxData9(USART1, pdata[a]);


	}

	return 1;
	
}

/*!
 * @brief       CRC校验
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
static uint8_t actCrc8(const uint8_t* pucData,uint16_t usDataLen)
{
	unsigned char i;
	unsigned char crc = 0xff;
	unsigned char c = 0;
	unsigned char poly = 0x8c;
	
	while(usDataLen-- != 0)
	{
		c = *pucData;
		crc ^= c;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x01)
			{
				crc >>= 1;
				crc ^=poly;
				
			}
			else
			{
				crc >>= 1;
				
			}
		}
		pucData++;
	}
	return (crc);
}

/*!
 * @brief       actcheckDate()
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
static BOOL actcheckDate(const uint8_t *pcDataBuf,uint8_t uslen)
{
	uint8_t recvCRC;
	uint8_t currCRC;
	BOOL at_return = FALSE;
	
	if((uslen > 1) && (uslen <= DATA_BUF_SIZE))
	{
		recvCRC = pcDataBuf[uslen - 1];
		currCRC = actCrc8(pcDataBuf,uslen-1);
		if(recvCRC != currCRC)
		{
			printf("CRC ERROR\n");
			at_return = FALSE;
		}
		else
		{
			at_return = TRUE;
		}
	}
	return (at_return);
}



/*!
 * @brief       回复引擎发送“握手指令1”
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

uint8_t reqSendHandShake1()
{
	uint8_t at_tx_buffer[TX_DATA_BUF_SIZE] = {0};
	
	uint16_t Cmdlen = 0;
	
	/*总长度 = Cmd + addr + len + status + crc*/
	Cmdlen = CMD_HANDSHAKE_FIRST_DATE_LENGTH + MIN_CMD_LEN + CMD_HANDSHAKE_FIRST_CRC_LENGTH;     // 数据长度 + 固定的头四个字节 + crc
	
	at_tx_buffer[TX_FRAME_CMD] = CMD_HANDSHAKE_FIRST_HEAD;          // 握手指令1 CMD
	at_tx_buffer[TX_FRAME_PARAM1] = SAFETY_MCU_ADDRESS;             // 安全MCU的地址
	at_tx_buffer[TX_FRAME_PARAM2] = CMD_HANDSHAKE_FIRST_LENGTH;     // 握手指令1数据包长度
	at_tx_buffer[TX_FRAME_PARAM3] = STATUS_OK;                      // 指令接收状态
	
	//mcu003版本号
	at_tx_buffer[TX_FRAME_PARAM4] =	0x00 ;
	at_tx_buffer[TX_FRAME_PARAM5] =	0x00 ;
	at_tx_buffer[TX_FRAME_PARAM6] =	0x00 ;
	at_tx_buffer[TX_FRAME_PARAM7] =	0x01 ;
	
	at_tx_buffer[TX_FRAME_PARAM8] =	g_ac_v_flag_value ;             //打印机额定电压标志位AC_V_FLAG，获取GPIO输入，若0:110V  1:220V 
	printf("g_ac_v_flag_value = %d\n",at_tx_buffer[TX_FRAME_PARAM8]);

	at_tx_buffer[Cmdlen - CRC_VALUE_LEN ] = actCrc8(at_tx_buffer,Cmdlen - CRC_VALUE_LEN);
	
	
	if(actSendDateUsart3(&at_tx_buffer[0],Cmdlen))
	{
		return 1;
	}
		return 0;
}

/*!
 * @brief       回复引擎发送“握手指令2”
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

uint8_t reqSendHandShake2()
{
	uint8_t at_tx_buffer[TX_DATA_BUF_SIZE];
	
	uint16_t Cmdlen = 0;
	
	/*总长度 = Cmd + addr + len + status + crc*/
	Cmdlen = CMD_HANDSHAKE_SECOND_DATE_LENGTH + MIN_CMD_LEN + CMD_HANDSHAKE_SECOND_CRC_LENGTH;   // 数据长度 + 固定的头四个字节 + crc
	
	at_tx_buffer[TX_FRAME_CMD] = CMD_HANDSHAKE_SECOND_HEAD;        // 握手指令2 CMD
	at_tx_buffer[TX_FRAME_PARAM1] = SAFETY_MCU_ADDRESS;            // 安全MCU的地址
	at_tx_buffer[TX_FRAME_PARAM2] = CMD_HANDSHAKE_SECOND_LENGTH;   // 握手指令2数据包长度
	at_tx_buffer[TX_FRAME_PARAM3] = STATUS_OK;                     // 指令接收状态
	
	at_tx_buffer[Cmdlen - CRC_VALUE_LEN ] = actCrc8(at_tx_buffer,Cmdlen - CRC_VALUE_LEN);
	
	if(actSendDateUsart3(&at_tx_buffer[0],Cmdlen))
	{
		return 1;
	}
	return 0;
}


/*!
 * @brief       回复引擎发送“查询状态”
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

uint8_t reqSendInquireStatus(uint16_t voltage_cth, uint16_t ac_v)
{
	uint8_t at_tx_buffer[TX_DATA_BUF_SIZE];
	uint16_t at_cth_read = 0;
	uint16_t at_ac_v_read = 0;
	
	uint16_t Cmdlen = 0;
	
	/*总长度 = Cmd + addr + len + status + crc*/
	/*4:异常代码（1）  + AC交流电源电压值（2） + MCU检测定影温度值（1） */
	Cmdlen = CMD_INQUIRE_DATE_LENGTH + MIN_CMD_LEN + CMD_INQUIRE_CRC_LENGTH; // 数据长度 + 固定的头四个字节 + crc
	
	at_cth_read = voltage_cth;
	at_ac_v_read = ac_v;
	
	at_tx_buffer[TX_FRAME_CMD] = CMD_INQUIRE_STATUS_HEAD;           // 查询状态 CMD
	at_tx_buffer[TX_FRAME_PARAM1] = SAFETY_MCU_ADDRESS;             // 安全MCU的地址
	at_tx_buffer[TX_FRAME_PARAM2] = CMD_INQUIRE_STATUS_LENGTH;      // “查询状态”数据包长度
	at_tx_buffer[TX_FRAME_PARAM3] = STATUS_OK;                      // 指令接收状态
	
	at_tx_buffer[TX_FRAME_PARAM4] = (g_error_code >> 8) & 0xff;       //异常代码高八位
	at_tx_buffer[TX_FRAME_PARAM5] = g_error_code & 0xff;             //异常代码低八位
	at_tx_buffer[TX_FRAME_PARAM6] = (at_ac_v_read >> 8) & 0xff;      // AC交流电源电压值高八位 （单位V）
	at_tx_buffer[TX_FRAME_PARAM7] = at_ac_v_read & 0xff;             // AC交流电源电压值低八位 （单位V）   
	
	at_tx_buffer[TX_FRAME_PARAM8] = (at_cth_read >> 8) & 0xff;       // 温度对应的电压值高八位             
	at_tx_buffer[TX_FRAME_PARAM9] = at_cth_read & 0xff;              // 温度对应的电压值低八位  
	
	at_tx_buffer[TX_FRAME_PARAM10] = g_AC_detection_flag;       // mcu是否完成ADC detection
	at_tx_buffer[Cmdlen - CRC_VALUE_LEN ] = actCrc8(at_tx_buffer,Cmdlen - CRC_VALUE_LEN);
			
	if(actSendDateUsart3(&at_tx_buffer[0],Cmdlen))
	{
//		if((g_error_code == OUT_TEMP_ERROR_CODE) || (g_error_code == OUT_HEAT_TIME_ERROR_CODE) 
//		|| (g_error_code == AC_V_HIGH_ERROR_CODE) || (g_error_code == AC_V_LOW_ERROR_CODE) 
//		||(g_error_code == SLEEP_HEAT_ERROR_CODE)|| (g_error_code == EXCESSIVE_TEMP_ERROR_ERROR_CODE))
//		{
//			actFuserRelayOff();
//			printf("UART:close Relay\n");
//		}
		return 1;
		
	}

	return 0;
}

/*!
 * @brief       回复引擎发送“清除错误”
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

uint8_t reqSendCleanError()
{
	uint8_t at_tx_buffer[TX_DATA_BUF_SIZE];
	
	uint16_t Cmdlen = 0;
	
	/*总长度 = Cmd + addr + len + status + crc*/
	Cmdlen = CMD_CLEAN_ERROR_LENGTH + MIN_CMD_LEN + CMD_CLEAN_ERROR_CRC_LENGTH;   // 数据长度 + 固定的头四个字节 + crc
	
	at_tx_buffer[TX_FRAME_CMD] = CMD_CLEAN_ERROR_HEAD;             // 握手指令2 CMD
	at_tx_buffer[TX_FRAME_PARAM1] = SAFETY_MCU_ADDRESS;            // 安全MCU的地址
	at_tx_buffer[TX_FRAME_PARAM2] = CMD_CLEAN_ERROR_LENGTH;        // 握手指令2数据包长度
	at_tx_buffer[TX_FRAME_PARAM3] = STATUS_OK;                     // 指令接收状态
	
	at_tx_buffer[Cmdlen - CRC_VALUE_LEN ] = actCrc8(at_tx_buffer,Cmdlen - CRC_VALUE_LEN);
	
	if(actSendDateUsart3(&at_tx_buffer[0],Cmdlen))
	{
		return 1;
	}
	return 0;
}


/*!
 * @brief       清空接收buff
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

void actClearRxBuff(uint8_t *xDataBuf,uint16_t num)
{
	uint8_t x ;
	if(xDataBuf[0] == rxDataBufUSART3[0])
	{
		rxDataPt = 0;
	}
	else
	{
		debug_rxDataPt = 0;
	}
	for(x = 0; x < num; x++)
	{
		xDataBuf[x] = 0;
	}
}

/*!
 * @brief       分包处理
 *
* @param       rv_data:接收到的数据   rv_len：数据长度
 *
 * @retval      None
 *
 * @note
 */
static void DividePackHandle(uint8_t *rv_data,uint8_t rv_len)
{
	int i = 0;
	int x = 0;
	int cmd_flag = 0;
	
	for(i = 0; i <= rv_len; i++)
		{
			if(((rv_data[i] == CMD_EC_HANDSHAKE_FIRST_HEAD) && (SAFETY_MCU_ADDRESS == rv_data[i+1])) || (1 == cmd_flag)
				|| ((rv_data[i] == CMD_EC_HANDSHAKE_FIRST_HEAD) && (0 == cmd_flag)))
			{
				cmd_flag       = 1;
				if(x <= CMD_HANDSHAKE_FIRST_DATE_TOTAL_LENGTH - 1)
				{
					rxDataUSART3[x] = rv_data[i];
					x++;
				}
				rxDataPt_len = CMD_HANDSHAKE_FIRST_DATE_TOTAL_LENGTH;
			}
			else if(((rv_data[i] == CMD_EC_HANDSHAKE_SECOND_HEAD) && (SAFETY_MCU_ADDRESS == rv_data[i+1])) || (2 == cmd_flag))	
			{
				cmd_flag       = 2;
				if(x <= CMD_HANDSHAKE_SECOND_DATE_TOTAL_LENGTH - 1)
				{
					rxDataUSART3[x] = rv_data[i];
					x++;
				}		
				rxDataPt_len = CMD_HANDSHAKE_SECOND_DATE_TOTAL_LENGTH;
			}
			else if(((rv_data[i] == CMD_EC_INQUIRE_STATUS_HEAD) && (SAFETY_MCU_ADDRESS == rv_data[i+1])) || (3 == cmd_flag))	
			{
				cmd_flag       = 3;
				if(x <= CMD_INQUIRE_DATE_TOTAL_LENGTH - 1)
				{
					rxDataUSART3[x] = rv_data[i];
					x++;
				}		
				rxDataPt_len = CMD_INQUIRE_DATE_TOTAL_LENGTH;
			}
			else if(((rv_data[i] == CMD_EC_CLEAN_ERROR_HEAD) && (SAFETY_MCU_ADDRESS == rv_data[i+1])) || (3 == cmd_flag))	
			{
				cmd_flag       = 3;
				if(x <= CMD_CLEAN_ERROR_TOTAL_LENGTH - 1)
				{
					rxDataUSART3[x] = rv_data[i];
					x++;
				}		
				rxDataPt_len = CMD_CLEAN_ERROR_TOTAL_LENGTH;
			}
			else
			{
			
			}
//			actSendDateUsart1(&rv_data[i],1);
		}

		
}

/*!
 * @brief       接收信息处理
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */

void CmdReceiveProcess(void)
{
	int i = 0;
	uint16_t at_voltage_average_cth = 0;
	uint16_t at_voltage_average_ac_v = 0;
	
	uint16_t  at_SOC_temperature_adc_H = 0;
	uint16_t  at_SOC_temperature_adc_L = 0;
	
	if(true == s_is_recv_complete)
	{		
//			printf("data lenth1 is %d\n",rxDataPt);
//			for(i = 0; i<at_rxDataPt; i++)
//			{
//				printf("%x ",rxDataBufUSART3[i]);
//				printf("-%d- ",at_rxDataPt);
//			}
//			printf("\n");
			
		s_is_recv_complete = false;
	
		for(i = 0; i<g_rxDataPt; i++)
		{
			rxDataUSART3[i] = rxDataBufUSART3[i];
		}
	
//		if(rxDataUSART3[0] == CMD_EC_FW_UPGRADE_HEAD)
//		{
//			printf("APP1 jump to boot\n");
////			Delay(200);
//			*(uint32_t *)BOARD_UPGRADE_FLAG = 0xA5A5;
//			*(uint32_t *)BOARD_ADDRESS = 3;

//			NVIC_DisableIRQRequest(USART1_RX_IRQn);
//			NVIC_DisableIRQRequest(USART3_RX_IRQn);
//			NVIC_DisableIRQRequest(SysTick_IRQn);
//			NVIC_DisableIRQRequest(TMR4_IRQn);
//			__disable_irq();	
//			Jump_to_App(1);
//		}
			//DividePackHandle( &at_data[0],g_rxDataPt );
		if(SAFETY_MCU_ADDRESS != rxDataUSART3[1])
		{
			actClearRxBuff(&rxDataBufUSART3[0],32);
			actClearRxBuff(&rxDataUSART3[0],32);
			return;
		}
		
		if(actcheckDate(rxDataBufUSART3,g_rxDataPt))
		{	
			switch(rxDataUSART3[0])
			{
				case CMD_EC_HANDSHAKE_FIRST_HEAD:                //收到“握手指令1”的回复
					{
						g_calibration_time.year   = rxDataUSART3[3];
						g_calibration_time.month  = rxDataUSART3[4];
						g_calibration_time.day    = rxDataUSART3[5];
						g_calibration_time.hour   = rxDataUSART3[6];
						g_calibration_time.minute = rxDataUSART3[7];
						g_calibration_time.second = rxDataUSART3[8];
						reqSendHandShake1();
						printf("UART:handshake one successed\n");
						TimeInit();
						fuser_aculate_time_flag = 1;
					}
					break;
				case CMD_EC_HANDSHAKE_SECOND_HEAD:               //收到“握手指令2”的回复
					{
						g_voltage_formula_parameter1            = rxDataUSART3[3];
						g_voltage_formula_parameter2            = rxDataUSART3[4];
						g_out_temperature_protect_value         =(rxDataUSART3[5] << 8) | (rxDataUSART3[6]);
	//					g_out_heating_time_value              = rxDataUSART3[7];
						
						//输入电压范围
						voltage_range.g_ac_voltage_range_first  = rxDataUSART3[7];
						voltage_range.g_ac_voltage_range_second = rxDataUSART3[8];
						voltage_range.g_ac_voltage_range_third  = rxDataUSART3[9];
						voltage_range.g_ac_voltage_range_forth  = rxDataUSART3[10];
						
						//加热时长阈值
						heat_time.g_heat_time_first             = rxDataUSART3[11];     
						heat_time.g_heat_time_second             = rxDataUSART3[12];  
						heat_time.g_heat_time_third             = rxDataUSART3[13];  
						heat_time.g_heat_time_forth             = rxDataUSART3[14];  					
						
						reqSendHandShake2();
						
						receive_threshold_time_value_flag = 1;
						printf("g_ac_voltage_range_first = %d,\n g_ac_voltage_range_second = %d,\n g_ac_voltage_range_third = %d,\n g_ac_voltage_range_forth = %d.\n",
						voltage_range.g_ac_voltage_range_first,voltage_range.g_ac_voltage_range_second,voltage_range.g_ac_voltage_range_third,voltage_range.g_ac_voltage_range_forth);	
						
						printf("g_heat_time_first = %d,\n g_heat_time_second = %d,\n g_heat_time_third = %d,\n g_heat_time_forth = %d.\n",
						heat_time.g_heat_time_first,heat_time.g_heat_time_second,heat_time.g_heat_time_third,heat_time.g_heat_time_forth);	
						
						printf("g_voltage_formula_paprameter1 = %d,g_voltage_formula_paprameter2 = %d\n",g_voltage_formula_parameter1,g_voltage_formula_parameter2);					
						printf("g_out_temperature_protect_value is %dmv\n",g_out_temperature_protect_value);					
						printf("UART:handshake two successed\n");
						
					}
					break;
				case CMD_EC_INQUIRE_STATUS_HEAD:                 //收到“查询状态”的回复
					{
	//					printf("UART:is living\n");		
						g_SOC_error_code = ( rxDataUSART3[3] << 8);
						g_SOC_error_code = g_SOC_error_code | rxDataUSART3[4];
						
						
						at_SOC_temperature_adc_H = rxDataUSART3[5] ;
						at_SOC_temperature_adc_L = rxDataUSART3[6] ;
						
						g_SOC_temperature_adc = (at_SOC_temperature_adc_H << 8) | (at_SOC_temperature_adc_L);
						g_start_5v_flag = rxDataUSART3[7];
						
						if(g_fuser_ac_error_times >= 900)
						{
							//打印soc采集的温度电压
							printf("SOC sample temp is %d \n",g_SOC_temperature_adc);	
							//打印003采集的温度
							g_voltage_average_ac_v = g_avaliable_ad_value[eTEMPRATURE_SAMPLE];
							printf("mcu sample temp is %d \n",g_voltage_average_ac_v);				
						}
						
						
						at_voltage_average_cth = g_avaliable_ad_value[eTEMPRATURE_SAMPLE]; 
	//				at_voltage_average_cth_temp = actConvertMiddleADValueToTemperature(at_voltage_average_cth);
						at_voltage_average_ac_v = g_avaliable_ad_value[ePOWER_SAMPLE];
						reqSendInquireStatus(at_voltage_average_cth, at_voltage_average_ac_v);
					}
					break;
				case CMD_EC_CLEAN_ERROR_HEAD:                 //收到“清除错误”的回复
					{
						g_error_code = 0;
						actEraseErrorInfo();
						reqSendCleanError();
						printf("UART:clean error successed\n");
					}
					break;		
				case CMD_EC_FW_UPGRADE_HEAD:                 //收到“升级“
					{
						printf("APP1 jump to boot\n");
						*(uint32_t *)BOARD_UPGRADE_FLAG = 0xA5A5;
						*(uint32_t *)BOARD_ADDRESS = 3;

						NVIC_DisableIRQRequest(USART1_RX_IRQn);
						NVIC_DisableIRQRequest(USART3_RX_IRQn);
						NVIC_DisableIRQRequest(SysTick_IRQn);
						NVIC_DisableIRQRequest(TMR4_IRQn);
						__disable_irq();	
						Jump_to_App(1);
					}
					break;						
				default:
					break;
			}
		//	actSendDateUsart1(&rxDataBufUSART3[0],rxDataPt);

		}
		else
		{
			printf("data lenth is %d\n",g_rxDataPt);
			for(i = 0; i<g_rxDataPt; i++)
			{
				printf("%x ",rxDataBufUSART3[i]);
			}
			printf("\n");
		}
		actClearRxBuff(&rxDataBufUSART3[0],32);
		actClearRxBuff(&rxDataUSART3[0],32);
	}

}


/*!
 * @brief       Debug
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void DebugProcess(void)
{
	int16_t input_IO_test1 = 0;
	int16_t output_IO_test1 = 0;
	
	uint8_t input_IN_flag = 0;
	uint8_t input_OUT_flag = 0;
	
	uint8_t debug_data[DATA_BUF_SIZE];
	uint8_t debug_data1[DATA_BUF_SIZE] ="ac_v";  //方便一直输出003检测的电源电压
	
	uint8_t sleep[DATA_BUF_SIZE] = "sleep";
	uint8_t sleep_H[12] = "不休眠 ";
	uint8_t sleep_L[12] = "休眠 ";
	
	uint8_t times[DATA_BUF_SIZE] = "times";
	uint8_t times_H[15] = "定影加热低电平 ";
	uint8_t times_L[28] = "定影加热高电平 单位 百毫秒";
	
	uint8_t acflag[DATA_BUF_SIZE] = "acflag";
	uint8_t acflag_H[12] = "220V 电源 ";
	uint8_t acflag_L[12] = "110V 电源 ";
	
	uint8_t auto_shot[DATA_BUF_SIZE] = "auto_shot";
	uint8_t auto_shot_H[12] = "不自动关机 ";
	uint8_t auto_shot_L[12] = "自动关机 ";
	
	uint8_t pnl_key[7] = "pnl_key";
	uint8_t pnl_key_H[8] = "未按下 ";
	uint8_t pnl_key_L[8] = "按下 ";
	
	uint8_t power_on[DATA_BUF_SIZE] = "power_on";
	uint8_t power_on_H[5] = "开机 ";
	
	uint8_t power_off[DATA_BUF_SIZE] = "power_off";
	uint8_t power_off_L[5] = "关机 ";	
	
	uint8_t relay_on[DATA_BUF_SIZE] = "relay_on";
	uint8_t relay_on_H[15] = "切断定影加热 ";
	
	uint8_t relay_off[DATA_BUF_SIZE] = "relay_off";
	uint8_t relay_off_L[16] = "不切断定影加热 ";	
	
	uint8_t ac_v[DATA_BUF_SIZE] = "ac_v";
	uint8_t ac_v_char[DATA_BUF_SIZE] = "ac_v 检测电压 mv：";
	uint8_t ac_v_char_To_220[DATA_BUF_SIZE] = " 转换为220v v：";
	
	uint8_t cth_v[DATA_BUF_SIZE] = "cth_v";
	uint8_t cth_v_char[DATA_BUF_SIZE] = "cth_v 检测电压 mv：";
	uint8_t cth_v_char_To_temp[DATA_BUF_SIZE] = " 转换为温度：";
	
	uint8_t read_error_cmd[DATA_BUF_SIZE] = "error";
	uint8_t read_boot_cmd[DATA_BUF_SIZE] = "boot";
	uint8_t ac_v_high_error_cmd[DATA_BUF_SIZE] = "ac_high";
	uint8_t ac_v_low_error_cmd[DATA_BUF_SIZE] =  "ac_low";
	uint8_t ac_v_temp_high_error_cmd[DATA_BUF_SIZE] =  "tmp_high";
	uint8_t ac_v_heat_time_error_cmd[DATA_BUF_SIZE] =  "heat_time";
	uint8_t ac_v_temp_exessive_error_cmd[DATA_BUF_SIZE] =  "temp_exe";
	uint8_t sleep_heat_error_cmd[DATA_BUF_SIZE] =  "slep_heat";
//	uint8_t read_error_info[14] = "错误信息为：";
	
	uint8_t int_huan_hang1[2] = "\n";
	
	uint8_t intput1[DATA_BUF_SIZE] = " IN is 1 \n";
	uint8_t intput0[DATA_BUF_SIZE] = " IN is 0 \n";
	
	uint8_t output1[DATA_BUF_SIZE] = " OUT is 1 \n";
	uint8_t output0[DATA_BUF_SIZE] = " OUT is 0 \n";
	
		if(BufferCompare(&debug_data1[0],&ac_v[0],4))                   //ac_v交流电压
		{

			
		}


	if(true == s_is_debug_recv_complete)
	{  
		actClearRxBuff(&debug_data[0],32);
		memcpy(&debug_data[0],&rxDataBufUSART1[0],32);
		actClearRxBuff(&rxDataBufUSART1[0],32);
		s_is_debug_recv_complete = false;
		
		if(BufferCompare(&debug_data[0],&sleep[0],5))
		{
			input_IO_test1 = GPIO_ReadInputBit(BOARD_PSU_SLEEP_PORT, BOARD_PSU_SLEEP_PIN);   // PSU休眠信号IO口测试
			input_IN_flag =1;
			if( 1 == input_IO_test1)
			{
				 actSendDateUsart1(&sleep_H[0],8);
			}
			else
			{
				actSendDateUsart1(&sleep_L[0],6);
			}
			actSendDateUsart1(&sleep[0],5);
		 }
		else if(BufferCompare(&debug_data[0],&times[0],5))
		{
//			input_IO_test1 = GPIO_ReadInputBit(FUSER_ON_TIME_PORT, FUSER_ON_TIME_PIN);   // 加热时长IO口测试
			input_IN_flag =1;
//			input_IO_test1 = 0;//验证
			if( 0 == g_fuser_on_time)
			{
				 actSendDateUsart1(&times_H[0],15);
			}
			else
			{
				actSendDateUsart1(&times_L[0],30);	
				actClearRxBuff(&ADC_print_value[0],6);
				NumberToCharsix(fuser_on_timer,&ADC_print_value[0]);	
				actSendDateUsart1(&ADC_print_value[0],6);
				actClearRxBuff(&ADC_print_value[0],6);
			}
			actSendDateUsart1(&times[0],6);
		}
		else if(BufferCompare(&debug_data[0],&acflag[0],6))
		{
			input_IO_test1 = GPIO_ReadInputBit(AC_V_FLAG_PORT, AC_V_FLAG_PIN);   // 交流电压标志位检测IO口测试
			input_IN_flag =1;
			if( 1 == input_IO_test1)
			{
				 actSendDateUsart1(&acflag_H[0],10);
			}
			else
			{
				actSendDateUsart1(&acflag_L[0],10);
			}
			actSendDateUsart1(&acflag[0],6);
		}
		else if(BufferCompare(&debug_data[0],&auto_shot[0],6))
		{
			input_IO_test1 = GPIO_ReadInputBit(POWER_AUTO_SHUT_PORT, POWER_AUTO_SHUT_PIN);   //AUTO_SHOUT
			input_IN_flag =1;
			if( 1 == input_IO_test1)
			{
				 actSendDateUsart1(&auto_shot_H[0],10);
			}
			else
			{
				actSendDateUsart1(&auto_shot_L[0],10);
			}
			actSendDateUsart1(&auto_shot[0],6);
		}
		
		else if(BufferCompare(&debug_data[0],&pnl_key[0],7))
		{
			input_IO_test1 = GPIO_ReadInputBit(BOARD_KEY1_GPIO_PORT, BOARD_KEY1_GPIO_PIN);   //PNL_KEY
			input_IN_flag =1;
			if( 1 == input_IO_test1)
			{
				 actSendDateUsart1(&pnl_key_H[0],8);
			}
			else
			{
				actSendDateUsart1(&pnl_key_L[0],6);
			}
			actSendDateUsart1(&pnl_key[0],7);
		}
		else if(BufferCompare(&debug_data[0],&power_on[0],8))                 // /** Turn 5V上电 */
		{
			GPIO_SetBit(POWER_ON_OFF_PORT, BOARD_POWER_ON_OFF_PIN);
			input_OUT_flag =1;
			output_IO_test1 = 1;
			actSendDateUsart1(&power_on_H[0],5);
			
			actSendDateUsart1(&power_on[0],8);
		}
		else if(BufferCompare(&debug_data[0],&power_off[0],9))                // /** Turn 5V掉电 */
		{
			GPIO_ClearBit(POWER_ON_OFF_PORT, BOARD_POWER_ON_OFF_PIN);
			input_OUT_flag =1;
			output_IO_test1 = 0;
			actSendDateUsart1(&power_off_L[0],5);			
			
			actSendDateUsart1(&power_off[0],9);
		}
		else if(BufferCompare(&debug_data[0],&relay_on[0],8))                ///** Turn 继电器断开信号 */
		{
			GPIO_SetBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
			input_OUT_flag =1;
			output_IO_test1 = 1;
			actSendDateUsart1(&relay_on_H[0],15);	
			
			actSendDateUsart1(&relay_on[0],8);
		}
		else if(BufferCompare(&debug_data[0],&relay_off[0],9))              ///** Turn 继电器不使能信号 */
		{
			GPIO_ClearBit(FUSER_RELAY_OFF_PORT, FUSER_RELAY_OFF_PIN);
			input_OUT_flag =1;
			output_IO_test1 = 0;
			actSendDateUsart1(&relay_off_L[0],16);				
			
			actSendDateUsart1(&relay_off[0],9);
		}
		else if(BufferCompare(&debug_data[0],&ac_v[0],4))                   //ac_v交流电压
		{
			g_voltage_average_ac_v = g_avaliable_ad_value[ePOWER_SAMPLE];
			actClearRxBuff(&ADC_print_value[0],5);
			NumberToChar(g_voltage_average_ac_v,&ADC_print_value[0]);
			g_voltage_average_ac_v_to_220v =  ( ((g_avaliable_ad_value[ePOWER_SAMPLE] * 100) / g_voltage_formula_parameter1) + g_voltage_formula_parameter2);
			printf("ac_v detection voltage is %d，convert to 220v is %d\n",g_voltage_average_ac_v,g_voltage_average_ac_v_to_220v);

			
		
			
//			actSendDateUsart1(&ac_v_char[0],20);
//			actSendDateUsart1(&ADC_print_value[0],5);
//			actClearRxBuff(&ADC_print_value[0],5);
//			
//			NumberToChar(g_voltage_average_ac_v_to_220v,&ADC_print_value[0]);
//			actSendDateUsart1(&ac_v_char_To_220[0],20);
//			
//			actSendDateUsart1(&ADC_print_value[0],5);
//			actClearRxBuff(&ADC_print_value[0],5);
//			
//			actSendDateUsart1(&int_huan_hang1[0],2);
			
		}
		else if(BufferCompare(&debug_data[0],&cth_v[0],4))                 //cth_v温度
		{
			g_voltage_average_cth_v = g_avaliable_ad_value[eTEMPRATURE_SAMPLE];
			actClearRxBuff(&ADC_print_value[0],5);
			NumberToChar(g_voltage_average_cth_v,&ADC_print_value[0]);
			g_voltage_average_cth_temp = actConvertMiddleADValueToTemperature(g_voltage_average_cth_v);
			

			actSendDateUsart1(&cth_v_char[0],20);
			actSendDateUsart1(&ADC_print_value[0],5);
			actClearRxBuff(&ADC_print_value[0],5);
			
			NumberToChar_plus_minus(g_voltage_average_cth_temp,&ADC_print_value[0]);
			actSendDateUsart1(&cth_v_char_To_temp[0],20);
			
			actSendDateUsart1(&ADC_print_value[0],5);
			actClearRxBuff(&ADC_print_value[0],5);
			
			actSendDateUsart1(&int_huan_hang1[0],2);
			
		}
		
		else if(BufferCompare(&debug_data[0],&read_error_cmd[0],5))              ///** 读取Flash错误信息 */
		{
			
//			actSendDateUsart1(&read_error_info[0],14);				
			actReadErrorInfo();
//			actSendDateUsart1(&int_huan_hang1[0],2);

		}
		else if(BufferCompare(&debug_data[0],&read_boot_cmd[0],4))              ///** 跳转回bootlodoer */
		{
			
			printf("APP1 jump to boot\n");
			//Delay(200);
			*(uint32_t *)BOARD_UPGRADE_FLAG = 0xA5A5;
			*(uint32_t *)BOARD_ADDRESS = 3;
			NVIC_DisableIRQRequest(USART1_RX_IRQn);
			NVIC_DisableIRQRequest(USART3_RX_IRQn);
			NVIC_DisableIRQRequest(SysTick_IRQn);
			NVIC_DisableIRQRequest(TMR4_IRQn);
			Jump_to_App(1);

		}
		else if(BufferCompare(&debug_data[0],&ac_v_high_error_cmd[0],7))              ///** 制造电压过高错误指令 */
		{
			g_test_voltage = 3200;      //电压测试报错
			g_test_voltage_flag = 1;      //电压测试报错标志位
			printf("now ac_voltage = 3200（320V）\n");
		}	
		else if(BufferCompare(&debug_data[0],&ac_v_low_error_cmd[0],6))              ///** 制造电压过低错误指令 */
		{
			g_test_voltage = 750;      //电压测试报错
			g_test_voltage_flag = 1;      //电压测试报错标志位
			printf("now ac_voltage = 750（75V）\n");
		}				
		else if(BufferCompare(&debug_data[0],&ac_v_temp_high_error_cmd[0],8))        ///** 制造过温错误指令 */
		{
			g_test_temp = 240;      //过温测试报错
			g_test_temp_flag = 1;      //过温测试报错标志位
			printf("now temp_voltage = 240mv\n");
		}	
		else if(BufferCompare(&debug_data[0],&ac_v_heat_time_error_cmd[0],9))         ///** 加热时长异常错误指令 */
		{
			g_test_heat_time = 46;      //加热时长测试报错
			g_test_heat_time_flag = 1; //加热时长测试报错标志
			printf("now heat_time = 46s\n");
		}			
		else if(BufferCompare(&debug_data[0],&ac_v_temp_exessive_error_cmd[0],8))     ///** 温差过大异常错误指令 */
		{
			g_test_temp_exessive_flag = 1; //加热时长测试报错标志
			printf("now temp_exessive error \n");
		}
		else if(BufferCompare(&debug_data[0],&sleep_heat_error_cmd[0],9))     ///**睡眠下有加热异常错误指令 */
		{
			g_test_sleep_heat_flag = 1; //加热时长测试报错标志
			printf("now sleep heat error \n");
		}
		
		/**串口输出信息**/
		//intput
		if(input_IN_flag == 1)
		{
			if((1 == input_IO_test1 ))
			{
				actSendDateUsart1(&intput1[0],11);
			}
			else if((0 == input_IO_test1 ))
			{
				actSendDateUsart1(&intput0[0],11);
			}	
		}
		
		//output
		if(input_OUT_flag == 1)
		{
			if((1 == output_IO_test1 ))
			{
				actSendDateUsart1(&output1[0],11);
			}
			else if(( 0 == output_IO_test1 ))
			{
				actSendDateUsart1(&output0[0],11);
			}	
		}
		
		input_IN_flag =0;
		input_OUT_flag = 0;
	}

}
//void print(uint8_t* pdata)
//{
//	actSendDateUsart1(&pdata[0],32);
//}

/*!
 * @brief       带正负的数字字符转换
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void NumberToChar_plus_minus(uint16_t r_value,uint8_t* value)
{
	if(r_value > 0x80)
	{
		r_value = ~r_value + 1;
		value[0] = (r_value / 1000) + 0x30;
		value[1] = ((r_value / 100) % 10) + 0x30;
		value[2] = ((r_value / 10) % 10) + 0x30;
		value[3] = (r_value % 10) + 0x30;
		if(r_value > 10)
		{
			value[0] = 0x20;    //0x20：空格  0x2D：-
			value[1] = 0x2D;
		}
		else
		{
			value[0] = 0x20;
			value[1] = 0x20;
			value[2] = 0x2D;
		}
	}
	else
	{
		value[0] = (r_value / 1000) + 0x30;
		value[1] = ((r_value / 100) % 10) + 0x30;
		value[2] = ((r_value / 10) % 10) + 0x30;
		value[3] = (r_value % 10) + 0x30;
	}

}
/*!
 * @brief       正的数字字符转换
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void NumberToChar(uint16_t r_value,uint8_t* value)
{
		value[0] = (r_value / 1000) + 0x30;
		value[1] = ((r_value / 100) % 10) + 0x30;
		value[2] = ((r_value / 10) % 10) + 0x30;
		value[3] = (r_value % 10) + 0x30;

}

/*!
 * @brief       正的6位数字字符转换
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void NumberToCharsix(uint16_t r_value,uint8_t* value)
{
	value[0] = (r_value / 10000) + 0x30;
	value[1] = ((r_value / 1000) % 10) + 0x30;
	value[2] = ((r_value / 100) % 10) + 0x30;
	value[3] = ((r_value / 10) % 10) + 0x30;
	value[4] = (r_value % 10) + 0x30;
}

void Serial_SendString(uint8_t *str)
{
    while (*str != '\0')
    {
        Send_Char(*str);
        str++;
    }
}

void Send_Char(uint8_t character)
{
    USART_COM_TXDATA(character);

    while (USART_COM_TXFINISH)
    {
    }
}



void Jump_to_App(uint8_t Application)
{
    uint32_t address;

    /* Lock the Program memory */
    FMC_Lock();


    if(Application == 0)
    {
        address = USER_APP1_ADDRESS;
    }
    else
    {
        address = FLASH_BASE;
    }

    /* Jump to user application */
    JumpAddress = *(__IO uint32_t *) (address + 4);
    Jump_To_Application = (pFunction) JumpAddress;

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t *) address);

    /* Jump to application */
    Jump_To_Application();
}

