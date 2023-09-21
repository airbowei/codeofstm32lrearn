
#ifndef RAM_H
#define RAM_H
/********************************************************************************/
/*包含头文件                                                                     */
/********************************************************************************/

#include "Board.h"
#include "stdint.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_rcm.h"
#include "stdio.h"
/********************************************************************************/
/*宏定义                                                                        */
/********************************************************************************/

/********************************************************************************/
/*静态常亮定义                                                                   */
/********************************************************************************/

/********************************************************************************/
/*变量结构体联合体定义                                                              */
/********************************************************************************/

extern uint32_t g_Power_Status_Flag;               //该标志位表示ADC已经经过ADC电压滤波才能经行判错
//uint16_t g_voltage = 0;
extern uint16_t g_voltage_average_cth_v;
extern int16_t  g_voltage_average_cth_temp;
extern uint16_t g_voltage_average_ac_v;
extern int16_t  g_voltage_average_ac_v_to_220v;

extern int16_t g_power_flag;                        //是否是上电第一次，用于控制只能在上电后进入一次自动开机检测

//extern uint16_t g_init_time_flag;                    //因为auto_shot上电5秒后才被初始化完成，用于控制5秒后才开始自动关机检测

//extern uint16_t g_auto_shut_200ms_flag; 
extern int16_t  g_fuser_temp_high_times;             //用于计时温度过高检测周期时间
extern int16_t  g_fuser_temp_excessive_error_times;  //用于计时温度误差过大检测周期时间
extern int16_t  g_fuser_ac_error_times;              //用于计时电源异常检测周期时间
extern int16_t  g_fuser_voltage_times ;              //用于计时电源异常检测周期时间
extern int16_t  g_fuser_sleep_heat_times;            //用于计时休眠下加测甲测信号的周期时间

extern int16_t  g_fuser_ac_low_error_flag;           //用于计时电源过低异常检测次数
extern int16_t  g_fuser_ac_high_error_flag;          //用于计时电源过高异常检测次数
extern int16_t  g_fuser_ac_normal_flag;              //用于计时电源正常检测次数

extern uint16_t g_out_temperature_protect_value;     //过温保护阈值
extern uint16_t g_out_heating_time_value;            //加热时长阈值
extern uint16_t g_SOC_temperature_adc;               //soc检测到的定影温度值
extern uint16_t g_start_5v_flag;                     //soc发送过来的EC_5v开始标志位
extern uint8_t g_AC_detection_flag;                 //mcu检测ADC完成，发送给soc

//extern uint8_t fuser_on_time_flag;                   // 开始计时标志位
extern uint32_t fuser_on_timer;                      //加热时长  单位（百毫秒）

extern uint16_t g_error_code;                        // 异常错误码
extern uint16_t g_SOC_error_code;                    // SOC发送的错误码保存
extern uint16_t g_voltage_cth;                       //CTH电压
extern uint16_t g_voltage_ac_v;                      //AC电压

//gpio保存变量
extern uint16_t g_sleep_value;                      //睡眠信号
extern uint16_t g_ac_v_flag_value;                  //ac_v_flag信号  交流电压标志位
extern uint16_t g_fuser_on_time;                     //定影加热信号
extern uint8_t fuser_aculate_time_flag;              // 可以开始计算当前时间标志位
//extern uint16_t g_heat_time_code;                          // 加热时长异常
//extern uint16_t g_ac_v_high_code;                         // AC电压过高异常
//extern uint16_t g_ac_v_low_out_time_code;                 // AC电压过低异常
//extern uint16_t g_excessive_temp_code;                    // 温差过大异常
//extern uint16_t g_sleep_heat_code;                        // 睡眠下有加热行为异常

//时间戳
//extern uint8_t g_calibration_time_year;
//extern uint8_t g_calibration_time_month;
//extern uint8_t g_calibration_time_day;
//extern uint8_t g_calibration_time_hour;
//extern uint8_t g_calibration_time_minute;
//extern uint8_t g_calibration_time_second;

extern uint8_t error_flash_flag;   
extern uint16_t g_voltage_formula_parameter1;      //电压公式参数1
extern uint16_t g_voltage_formula_parameter2;      //电压公式参数2
//log 信息输出
extern uint8_t log_print[32];

//报错测试指令相关变量
extern uint16_t g_test_voltage;         //电压测试报错
extern uint16_t g_test_voltage_flag;    //电压测试报错标志位
extern uint16_t g_test_temp;      			//过温测试报错
extern uint16_t g_test_temp_flag;     	//过温测试报错标志位
extern uint16_t g_test_heat_time;      	//加热时长测试报错
extern uint16_t g_test_heat_time_flag; 	//加热时长测试报错标志位
extern uint16_t g_test_temp_exessive_flag;  //温差测试报错标志位
extern uint16_t g_test_sleep_heat_flag; //睡眠下有加热报错标志位
extern uint8_t receive_threshold_time_value_flag;  //收到加热时长阈值标志位，可以开始判断加热时长是否异常

#define STATUS_SLEEP             1
#define STATUS_NOT_SLEEP         0

//时间戳
typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}CalibrationTime;

//输入电压范围
typedef struct
{
	uint16_t g_ac_voltage_range_first;  //AC电压边界1
	uint16_t g_ac_voltage_range_second; //AC电压边界2
	uint16_t g_ac_voltage_range_third;  //AC电压边界3
	uint16_t g_ac_voltage_range_forth;  //AC电压边界4
	
}VoltageRange;

//加热时长阈值
typedef struct
{
	uint16_t g_heat_time_first;        //加热阈值1
	uint16_t g_heat_time_second;       //加热阈值2
	uint16_t g_heat_time_third;  			 //加热阈值3
	uint16_t g_heat_time_forth;        //加热阈值4
	
}HeatTime;



extern VoltageRange voltage_range;          //输入电压范围
extern HeatTime     heat_time;              //加热时长阈值
#endif
