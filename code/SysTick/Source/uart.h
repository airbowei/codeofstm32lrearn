#ifndef UART_H
#define UART_H

#include "apm32f00x.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_misc.h"
#include "apm32f00x_eint.h"
#include "apm32f00x_usart.h"
#include "apm32f00x_rcm.h"


#define	BOARD_UPGRADE_FLAG		             ( 0x20000F34 )	/* 003 Use */
#define	BOARD_ADDRESS			                 ( 0x20000F30 )	/* OptionTray AND 003 Use */
#define DEBUG_USART                         USART1
#define USART_COM                           (DEBUG_USART)
#define DEBUG_USART_CLK                     RCM_PERIPH_USART1
#define DEBUG_USART_BAUDRATE                115200
#define SendString(x)                       Serial_SendString((uint8_t*)(x))
#define USART_COM_TXDATA(c)                 (USART_TxData8(USART_COM,c))
#define USART_COM_TXFINISH                  (USART_ReadStatusFlag(USART_COM, USART_FLAG_TXBE) == RESET)

#define FLASH_BASE                  (uint32_t)0x0000

/* 15 Kbytes */
#define FLASH_PAGE_SIZE             (uint32_t)0x1FFF
#if defined (__CC_ARM)|| (__ICCARM__)

#define USER_APP1_ADDRESS           (uint32_t)0x2000
/* 15 KBytes */
#define USER_APP1_END_ADDRESS       (uint32_t)0x8000

//#define USER_APP2_ADDRESS           (uint32_t)0x9000
///* 8 KBytes */
//#define USER_APP2_END_ADDRESS       (uint32_t)0xB000
#else

#define USER_APP1_ADDRESS           (uint32_t)0x5000
/* 8 KBytes */
#define USER_APP1_END_ADDRESS       (uint32_t)0x6800

#define USER_APP2_ADDRESS           (uint32_t)0x6800
/* 8 KBytes */
#define USER_APP2_END_ADDRESS       (uint32_t)0x7FFF

#endif

#define USER_APP1_FLASH_SIZE        (USER_APP1_END_ADDRESS - USER_APP1_ADDRESS + 1)

//#define USER_APP2_FLASH_SIZE        (USER_APP2_END_ADDRESS - USER_APP2_ADDRESS + 1)

#define USER_FLASH_SIZE             (FLASH_PAGE_SIZE - FLASH_BASE + 1)



void USARTInit(void);
void Delay(uint32_t count);
void DataBufInit(void);
BOOL BufferCompare(uint8_t *buf1, uint8_t *buf2, uint8_t size);
void USART3RxIsr(void);
void USART1RxIsr(void);
uint8_t actSendDateUsart3(uint8_t* pdata,uint32_t len);
extern uint8_t actSendDateUsart1(uint8_t* pdata,uint32_t len);
extern uint8_t actSendDateUsart1_32(uint32_t* pdata,uint32_t len);
void actClearRxBuff(uint8_t *xDataBuf,uint16_t num);
uint8_t reqSendHandShake1(void);
uint8_t reqSendHandShake2(void);
uint8_t reqSendInquireStatus(uint16_t cth_temp, uint16_t ac_v);
uint8_t reqSendCleanError(void);
void CmdReceiveProcess(void);
void DebugProcess(void);
extern void actPrint1(uint8_t* data,uint32_t leng);

void Serial_SendString(uint8_t *str);
void Send_Char(uint8_t character);
void Jump_to_App(uint8_t Application);

#define DATA_BUF_SIZE       							(31)

#define TX_DATA_BUF_SIZE             			(12)  //����������󳤶�
#define MIN_CMD_LEN                  			(4)   //��С���ȣ���CMD + addr + length + status������
#define CRC_VALUE_LEN               		 	(1)   //crcУ�鳤��


#define CMD_HANDSHAKE_FIRST_LENGTH   			(0x05) //��һ�����ַ������ݳ���
#define CMD_HANDSHAKE_SECOND_LENGTH 		 	(0x00) //�ڶ������ַ������ݳ���
#define CMD_INQUIRE_STATUS_LENGTH    			(0x07) //��ѯ״̬�������ݳ���

#define TX_FRAME_CMD         						     (0)
#define TX_FRAME_PARAM1      							   (1)
#define TX_FRAME_PARAM2      							   (2)
#define TX_FRAME_PARAM3     							   (3)
#define TX_FRAME_PARAM4      							   (4)
#define TX_FRAME_PARAM5      							   (5)
#define TX_FRAME_PARAM6      							   (6)
#define TX_FRAME_PARAM7     						 	   (7)
#define TX_FRAME_PARAM8      							   (8)
#define TX_FRAME_PARAM9      							   (9)
#define TX_FRAME_PARAM10     							   (10)

#define    CMD_HANDSHAKE_FIRST_HEAD          0xB5   //mcu���͡�����ָ��1������ָ��ͷ
#define    CMD_HANDSHAKE_SECOND_HEAD         0xB6   //mcu���͡�����ָ��2������ָ��ͷ
#define    CMD_INQUIRE_STATUS_HEAD           0x11   //mcu���͡���ѯ״̬������ָ��ͷ
#define    CMD_CLEAN_ERROR_HEAD              0x12   //mcu���͡�������󡱷���ָ��ͷ

#define    CMD_EC_HANDSHAKE_FIRST_HEAD       0xB5   //���淢�͡�����ָ��1��ָ��ͷ
#define    CMD_EC_HANDSHAKE_SECOND_HEAD      0xB6   //���淢�͡�����ָ��2��ָ��ͷ
#define    CMD_EC_INQUIRE_STATUS_HEAD        0x11   //���淢�͡���ѯ״̬��ָ��ͷ
#define    CMD_EC_CLEAN_ERROR_HEAD           0x12   //���淢�͡��������ָ��ͷ
#define    CMD_EC_FW_UPGRADE_HEAD            0xF1   //���淢�͡��̼�������ָ��ͷ

#define    CMD_HANDSHAKE_FIRST_DATE_LENGTH   (5)    //����ָ��1���ݳ���
#define    CMD_HANDSHAKE_SECOND_DATE_LENGTH  (0)    //����ָ��2���ݳ���
#define    CMD_INQUIRE_DATE_LENGTH   (7)            //��ѯ״̬���ݳ���
#define    CMD_CLEAN_ERROR_LENGTH   (0)             //����������ݳ���

#define    CMD_HANDSHAKE_FIRST_CRC_LENGTH    (1)    //����ָ��1CRC����
#define    CMD_HANDSHAKE_SECOND_CRC_LENGTH   (1)    //����ָ��2CRC����
#define    CMD_INQUIRE_CRC_LENGTH  (1)              //��ѯ״̬CRC����
#define    CMD_CLEAN_ERROR_CRC_LENGTH  (1)              //�������CRC����

#define    CMD_HANDSHAKE_FIRST_DATE_TOTAL_LENGTH   (10)    //����ָ��1�����ܳ���
#define    CMD_HANDSHAKE_SECOND_DATE_TOTAL_LENGTH  (15)    //����ָ��2�����ܳ���
#define    CMD_INQUIRE_DATE_TOTAL_LENGTH   (8)            //��ѯ״̬�����ܳ���
#define    CMD_CLEAN_ERROR_TOTAL_LENGTH   (6)             //������������ܳ���

#define    SAFETY_MCU_ADDRESS                0x03   //��ȫоƬ��ַ ������Ҫ����





typedef enum 
{
	CMD_HANDSHAKE_FIRST = 0,    //���淢�͡�����ָ��1��
	CMD_HANDSHAKE_SECOND,       //���淢�͡�����ָ��2��
	CMD_INQUIRE_STATUS,         //���淢�͡���ѯ״̬��
}CMD_TYPE;                    //ָ������

typedef enum 
{
	STATUS_OK = 0x01,
	CRC_ERR = 0x02,
}CMD_STATUS;



//pFunction Jump_To_Application;
#endif
