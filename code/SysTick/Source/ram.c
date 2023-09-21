
/********************************************************************************/
/*����ͷ�ļ�                                                                     */
/********************************************************************************/

//#define RAM
#include "ram.h"
//#undef RAM

/********************************************************************************/
/*�궨��                                                                        */
/********************************************************************************/

/********************************************************************************/
/*��̬��������                                                                   */
/********************************************************************************/

/********************************************************************************/
/*�ڲ�����ԭ������                                                               */
/********************************************************************************/
uint32_t g_Power_Status_Flag = 0;                //�ñ�־λ��ʾADC�Ѿ�����ADC��ѹ�˲����ܾ����д�

uint16_t g_voltage_average_cth_v = 0;
int16_t  g_voltage_average_cth_temp = 0;
uint16_t g_voltage_average_ac_v = 0;
int16_t  g_voltage_average_ac_v_to_220v = 0;
int16_t  g_power_flag = 0;                        //�Ƿ����ϵ��һ�Σ����ڿ���ֻ�����ϵ�����һ���Զ��������
//uint16_t g_init_time_flag = 0;                    //��Ϊauto_shot�ϵ�5���ű���ʼ����ɣ����ڿ���5���ſ�ʼ�Զ��ػ����
//uint16_t g_auto_shut_200ms_flag = 0;            
int16_t  g_fuser_temp_high_times = 0;             //���ڼ�ʱ�¶ȹ��߼������ʱ��
int16_t  g_fuser_temp_excessive_error_times = 0;  //���ڼ�ʱ�¶�������������ʱ��
int16_t  g_fuser_ac_error_times = 0;              //���ڼ�ʱ��Դ�쳣�������ʱ��
int16_t  g_fuser_voltage_times = 0;               //���ڼ�ʱ��Դ�쳣�������ʱ��
int16_t  g_fuser_sleep_heat_times = 0;            //���ڼ�ʱ�����¼������źŵ�����ʱ��

int16_t  g_fuser_ac_low_error_flag = 0;           //���ڼ�ʱ��Դ�����쳣������
int16_t  g_fuser_ac_high_error_flag = 0;          //���ڼ�ʱ��Դ�����쳣������
int16_t  g_fuser_ac_normal_flag = 0;              //���ڼ�ʱ��Դ����������

uint16_t g_out_temperature_protect_value = 0;     //���±�����ѹ��ֵ
uint16_t g_out_heating_time_value = 10;           //����ʱ����ֵ
uint16_t g_SOC_temperature_adc = 0;               //soc��⵽�Ķ�Ӱ�¶ȵĵ�ѹֵ:����adcת��Ϊ�¶��Ƿ��������ߣ�����ֻ�ܱȽ�cth�ɵõĵ�ѹֵ
uint16_t g_start_5v_flag = 0;                     //soc���͹�����EC_5v��ʼ��־λ
uint8_t g_AC_detection_flag = 0;                 //mcu���ADC��ɣ����͸�soc

//uint8_t fuser_on_time_flag = 0;                   // ��ʼ��ʱ��־λ
uint32_t fuser_on_timer = 0;                    	//����ʱ��  ��λ���ٺ��룩
uint8_t fuser_aculate_time_flag = 0;              // ���Կ�ʼ���㵱ǰʱ���־λ
uint8_t error_flash_flag = 0;                     //�Ƿ񱣴����flash��־λ

uint16_t g_error_code = 0;                        // �����뱣��
uint16_t g_SOC_error_code = 0;                    // SOC���͵Ĵ����뱣��
uint16_t g_voltage_cth = 0;                       //CTH��ѹ
uint16_t g_voltage_ac_v = 0;                      //AC��ѹ

//gpio�������
uint16_t g_sleep_value = 0;                      //˯���ź�
uint16_t g_ac_v_flag_value = 0;                  //ac_v_flag�ź�  ������ѹ��־λ
uint16_t g_fuser_on_time = 0;                     //��Ӱ�����ź�
uint16_t g_voltage_formula_parameter1 = 0;      //��ѹ��ʽ����1
uint16_t g_voltage_formula_parameter2 = 0;      //��ѹ��ʽ����2

//log ��Ϣ���
uint8_t log_print[32] = {0};

//�������ָ����ر���
uint16_t g_test_voltage = 0;      	//��ѹ���Ա���
uint16_t g_test_voltage_flag = 0;   //��ѹ���Ա����־λ
uint16_t g_test_temp = 0;      			//���²��Ա���
uint16_t g_test_temp_flag = 0;      //���²��Ա����־λ
uint16_t g_test_heat_time = 0;      //����ʱ�����Ա���
uint16_t g_test_heat_time_flag = 0; //����ʱ�����Ա����־λ
uint16_t g_test_temp_exessive_flag = 0;  //�²���Ա����־λ
uint16_t g_test_sleep_heat_flag = 0; //˯�����м��ȱ����־λ

uint8_t receive_threshold_time_value_flag = 0;  //�յ�����ʱ����ֵ��־λ�����Կ�ʼ�жϼ���ʱ���Ƿ��쳣

VoltageRange voltage_range;          //�����ѹ��Χ
HeatTime     heat_time;              //����ʱ����ֵ


