
#ifndef RAM_H
#define RAM_H
/********************************************************************************/
/*����ͷ�ļ�                                                                     */
/********************************************************************************/

#include "Board.h"
#include "stdint.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_rcm.h"
#include "stdio.h"
/********************************************************************************/
/*�궨��                                                                        */
/********************************************************************************/

/********************************************************************************/
/*��̬��������                                                                   */
/********************************************************************************/

/********************************************************************************/
/*�����ṹ�������嶨��                                                              */
/********************************************************************************/

extern uint32_t g_Power_Status_Flag;               //�ñ�־λ��ʾADC�Ѿ�����ADC��ѹ�˲����ܾ����д�
//uint16_t g_voltage = 0;
extern uint16_t g_voltage_average_cth_v;
extern int16_t  g_voltage_average_cth_temp;
extern uint16_t g_voltage_average_ac_v;
extern int16_t  g_voltage_average_ac_v_to_220v;

extern int16_t g_power_flag;                        //�Ƿ����ϵ��һ�Σ����ڿ���ֻ�����ϵ�����һ���Զ��������

//extern uint16_t g_init_time_flag;                    //��Ϊauto_shot�ϵ�5���ű���ʼ����ɣ����ڿ���5���ſ�ʼ�Զ��ػ����

//extern uint16_t g_auto_shut_200ms_flag; 
extern int16_t  g_fuser_temp_high_times;             //���ڼ�ʱ�¶ȹ��߼������ʱ��
extern int16_t  g_fuser_temp_excessive_error_times;  //���ڼ�ʱ�¶�������������ʱ��
extern int16_t  g_fuser_ac_error_times;              //���ڼ�ʱ��Դ�쳣�������ʱ��
extern int16_t  g_fuser_voltage_times ;              //���ڼ�ʱ��Դ�쳣�������ʱ��
extern int16_t  g_fuser_sleep_heat_times;            //���ڼ�ʱ�����¼Ӳ�ײ��źŵ�����ʱ��

extern int16_t  g_fuser_ac_low_error_flag;           //���ڼ�ʱ��Դ�����쳣������
extern int16_t  g_fuser_ac_high_error_flag;          //���ڼ�ʱ��Դ�����쳣������
extern int16_t  g_fuser_ac_normal_flag;              //���ڼ�ʱ��Դ����������

extern uint16_t g_out_temperature_protect_value;     //���±�����ֵ
extern uint16_t g_out_heating_time_value;            //����ʱ����ֵ
extern uint16_t g_SOC_temperature_adc;               //soc��⵽�Ķ�Ӱ�¶�ֵ
extern uint16_t g_start_5v_flag;                     //soc���͹�����EC_5v��ʼ��־λ
extern uint8_t g_AC_detection_flag;                 //mcu���ADC��ɣ����͸�soc

//extern uint8_t fuser_on_time_flag;                   // ��ʼ��ʱ��־λ
extern uint32_t fuser_on_timer;                      //����ʱ��  ��λ���ٺ��룩

extern uint16_t g_error_code;                        // �쳣������
extern uint16_t g_SOC_error_code;                    // SOC���͵Ĵ����뱣��
extern uint16_t g_voltage_cth;                       //CTH��ѹ
extern uint16_t g_voltage_ac_v;                      //AC��ѹ

//gpio�������
extern uint16_t g_sleep_value;                      //˯���ź�
extern uint16_t g_ac_v_flag_value;                  //ac_v_flag�ź�  ������ѹ��־λ
extern uint16_t g_fuser_on_time;                     //��Ӱ�����ź�
extern uint8_t fuser_aculate_time_flag;              // ���Կ�ʼ���㵱ǰʱ���־λ
//extern uint16_t g_heat_time_code;                          // ����ʱ���쳣
//extern uint16_t g_ac_v_high_code;                         // AC��ѹ�����쳣
//extern uint16_t g_ac_v_low_out_time_code;                 // AC��ѹ�����쳣
//extern uint16_t g_excessive_temp_code;                    // �²�����쳣
//extern uint16_t g_sleep_heat_code;                        // ˯�����м�����Ϊ�쳣

//ʱ���
//extern uint8_t g_calibration_time_year;
//extern uint8_t g_calibration_time_month;
//extern uint8_t g_calibration_time_day;
//extern uint8_t g_calibration_time_hour;
//extern uint8_t g_calibration_time_minute;
//extern uint8_t g_calibration_time_second;

extern uint8_t error_flash_flag;   
extern uint16_t g_voltage_formula_parameter1;      //��ѹ��ʽ����1
extern uint16_t g_voltage_formula_parameter2;      //��ѹ��ʽ����2
//log ��Ϣ���
extern uint8_t log_print[32];

//�������ָ����ر���
extern uint16_t g_test_voltage;         //��ѹ���Ա���
extern uint16_t g_test_voltage_flag;    //��ѹ���Ա����־λ
extern uint16_t g_test_temp;      			//���²��Ա���
extern uint16_t g_test_temp_flag;     	//���²��Ա����־λ
extern uint16_t g_test_heat_time;      	//����ʱ�����Ա���
extern uint16_t g_test_heat_time_flag; 	//����ʱ�����Ա����־λ
extern uint16_t g_test_temp_exessive_flag;  //�²���Ա����־λ
extern uint16_t g_test_sleep_heat_flag; //˯�����м��ȱ����־λ
extern uint8_t receive_threshold_time_value_flag;  //�յ�����ʱ����ֵ��־λ�����Կ�ʼ�жϼ���ʱ���Ƿ��쳣

#define STATUS_SLEEP             1
#define STATUS_NOT_SLEEP         0

//ʱ���
typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}CalibrationTime;

//�����ѹ��Χ
typedef struct
{
	uint16_t g_ac_voltage_range_first;  //AC��ѹ�߽�1
	uint16_t g_ac_voltage_range_second; //AC��ѹ�߽�2
	uint16_t g_ac_voltage_range_third;  //AC��ѹ�߽�3
	uint16_t g_ac_voltage_range_forth;  //AC��ѹ�߽�4
	
}VoltageRange;

//����ʱ����ֵ
typedef struct
{
	uint16_t g_heat_time_first;        //������ֵ1
	uint16_t g_heat_time_second;       //������ֵ2
	uint16_t g_heat_time_third;  			 //������ֵ3
	uint16_t g_heat_time_forth;        //������ֵ4
	
}HeatTime;



extern VoltageRange voltage_range;          //�����ѹ��Χ
extern HeatTime     heat_time;              //����ʱ����ֵ
#endif
