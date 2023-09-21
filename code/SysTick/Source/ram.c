
/********************************************************************************/
/*包含头文件                                                                     */
/********************************************************************************/

//#define RAM
#include "ram.h"
//#undef RAM

/********************************************************************************/
/*宏定义                                                                        */
/********************************************************************************/

/********************************************************************************/
/*静态常亮定义                                                                   */
/********************************************************************************/

/********************************************************************************/
/*内部函数原型声明                                                               */
/********************************************************************************/
uint32_t g_Power_Status_Flag = 0;                //该标志位表示ADC已经经过ADC电压滤波才能经行判错

uint16_t g_voltage_average_cth_v = 0;
int16_t  g_voltage_average_cth_temp = 0;
uint16_t g_voltage_average_ac_v = 0;
int16_t  g_voltage_average_ac_v_to_220v = 0;
int16_t  g_power_flag = 0;                        //是否是上电第一次，用于控制只能在上电后进入一次自动开机检测
//uint16_t g_init_time_flag = 0;                    //因为auto_shot上电5秒后才被初始化完成，用于控制5秒后才开始自动关机检测
//uint16_t g_auto_shut_200ms_flag = 0;            
int16_t  g_fuser_temp_high_times = 0;             //用于计时温度过高检测周期时间
int16_t  g_fuser_temp_excessive_error_times = 0;  //用于计时温度误差过大检测周期时间
int16_t  g_fuser_ac_error_times = 0;              //用于计时电源异常检测周期时间
int16_t  g_fuser_voltage_times = 0;               //用于计时电源异常检测周期时间
int16_t  g_fuser_sleep_heat_times = 0;            //用于计时休眠下检测加热信号的周期时间

int16_t  g_fuser_ac_low_error_flag = 0;           //用于计时电源过低异常检测次数
int16_t  g_fuser_ac_high_error_flag = 0;          //用于计时电源过高异常检测次数
int16_t  g_fuser_ac_normal_flag = 0;              //用于计时电源正常检测次数

uint16_t g_out_temperature_protect_value = 0;     //过温保护电压阈值
uint16_t g_out_heating_time_value = 10;           //加热时长阈值
uint16_t g_SOC_temperature_adc = 0;               //soc检测到的定影温度的电压值:由于adc转换为温度是非线性曲线，所以只能比较cth采得的电压值
uint16_t g_start_5v_flag = 0;                     //soc发送过来的EC_5v开始标志位
uint8_t g_AC_detection_flag = 0;                 //mcu检测ADC完成，发送给soc

//uint8_t fuser_on_time_flag = 0;                   // 开始计时标志位
uint32_t fuser_on_timer = 0;                    	//加热时长  单位（百毫秒）
uint8_t fuser_aculate_time_flag = 0;              // 可以开始计算当前时间标志位
uint8_t error_flash_flag = 0;                     //是否保存错误到flash标志位

uint16_t g_error_code = 0;                        // 错误码保存
uint16_t g_SOC_error_code = 0;                    // SOC发送的错误码保存
uint16_t g_voltage_cth = 0;                       //CTH电压
uint16_t g_voltage_ac_v = 0;                      //AC电压

//gpio保存变量
uint16_t g_sleep_value = 0;                      //睡眠信号
uint16_t g_ac_v_flag_value = 0;                  //ac_v_flag信号  交流电压标志位
uint16_t g_fuser_on_time = 0;                     //定影加热信号
uint16_t g_voltage_formula_parameter1 = 0;      //电压公式参数1
uint16_t g_voltage_formula_parameter2 = 0;      //电压公式参数2

//log 信息输出
uint8_t log_print[32] = {0};

//报错测试指令相关变量
uint16_t g_test_voltage = 0;      	//电压测试报错
uint16_t g_test_voltage_flag = 0;   //电压测试报错标志位
uint16_t g_test_temp = 0;      			//过温测试报错
uint16_t g_test_temp_flag = 0;      //过温测试报错标志位
uint16_t g_test_heat_time = 0;      //加热时长测试报错
uint16_t g_test_heat_time_flag = 0; //加热时长测试报错标志位
uint16_t g_test_temp_exessive_flag = 0;  //温差测试报错标志位
uint16_t g_test_sleep_heat_flag = 0; //睡眠下有加热报错标志位

uint8_t receive_threshold_time_value_flag = 0;  //收到加热时长阈值标志位，可以开始判断加热时长是否异常

VoltageRange voltage_range;          //输入电压范围
HeatTime     heat_time;              //加热时长阈值


