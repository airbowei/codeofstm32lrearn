#include "adc.h"
#include "Board.h"
#include "stdint.h"
#include "ram.h"
#include "apm32f00x.h"
#include "security_manage.h"

/** ADC GPIO port and pin */
#define ADC_CH0_GPIO_PIN            GPIO_PIN_5
#define ADC_CH1_GPIO_PIN            GPIO_PIN_6
#define ADC_CH2_GPIO_PIN            GPIO_PIN_4
#define ADC_CH_GPIO_PORTC           GPIOC
#define VOLTAGE_REFERENCE           5035

const FuserTempVoltTable c_volt_to_tmepreture_table[VOLT_TO_FUSER_TEMPRETURE_INDEX_NUM];


/*!
 * @brief       ADC校准
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void ADCCalibration(void)
{
    uint8_t i;
    int8_t offset = 0;
    uint16_t adcData = 0;

    ADC_SetOffset(0);
    ADC_EnableCompensation();
    ADC_ClearStatusFlag(ADC_FLAG_CC);
    ADC_Enable();

		ADC_StartConversion();
    for (i = 0; i < 10; i++)
    {
        

        while (ADC_ReadStatusFlag(ADC_FLAG_CC) == RESET);

        ADC_ClearStatusFlag(ADC_FLAG_CC);
    }

    ADC_Disable();

    adcData = ADC_ReadData();
    offset = (int8_t)(0x800 - adcData);
    ADC_SetOffset(offset);

    ADC_DisableCompensation();
    ADC_ClearStatusFlag(ADC_FLAG_CC);
}

/*!
 * @brief       ADC_GPIO_Init
 *
 * @param       channel : Specifies the channel
 *
 * @retval      None
 *
 * @note
 */
void ADC_GPIO_Init(ADC_CHANNEL_T channel)
{
    GPIO_Config_T gpioConfig;

    /** ADC GPIO configuration */
    gpioConfig.intEn = GPIO_EINT_DISABLE;
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;

    if (channel == ADC_CHANNEL_4)
    {
        gpioConfig.pin = GPIO_PIN_3;
        GPIO_Config(GPIOD, &gpioConfig);
    }
    else if (channel == ADC_CHANNEL_5)
    {
        gpioConfig.pin = GPIO_PIN_5;
        GPIO_Config(GPIOD, &gpioConfig);
    }
}

/*!
 * @brief       ADCInit
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void ADCInit(void)
{
    ADC_Config_T adcConfig;

    /** ADC GPIO configuration */
    ADC_GPIOConfig();

    ADC_SetMode(ADC_MODE_SINGLE_END);

    /** ADC configuration */
    ADC_ConfigStructInit(&adcConfig);
    adcConfig.div = ADC_DIV_4;
    adcConfig.channel = ADC_CHANNEL_1;
    adcConfig.convMode = ADC_CONV_MODE_CONTINUOUS;
    adcConfig.scanMode = ADC_SCAN_MODE_ENABLE;
    adcConfig.interrupt = ADC_INT_CC;
    ADC_Config(&adcConfig);


    /** ADC Calibration */
    ADCCalibration();

    ADC_Enable();

    ADC_StartConversion();
}

/*!
 * @brief       ADCMeasure
 *
 * @param       channel : Specifies the channel
 *
 * @retval      None
 *
 * @note
 */
uint16_t ADCMeasure(void)
{

    uint16_t adcData = 0;
    uint16_t voltage = 0;


		while (ADC_ReadStatusFlag(ADC_FLAG_CC) == RESET);

		ADC_ClearStatusFlag(ADC_FLAG_CC);

		adcData = ADC_ReadData();

		/**  voltage(mV) =  adcData * (3300mV / 4095) */
	
		voltage = (adcData * VOLTAGE_REFERENCE) / 4095;
	
		return voltage;

}

void ADC_GPIOConfig(void)
{
    GPIO_Config_T gpioConfig;
    
    gpioConfig.intEn = GPIO_EINT_DISABLE;
    gpioConfig.mode = GPIO_MODE_IN_FLOATING;
	// teset
    gpioConfig.pin = ADC_CH0_GPIO_PIN | ADC_CH1_GPIO_PIN;
    GPIO_Config(ADC_CH_GPIO_PORTC, &gpioConfig);
}


/*!
 * @brief       ADC poll for conversion
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
uint16_t ADC_GetVoltage(ADC_BUFFER_IDX_T id)
{
    uint16_t Voltage = 0;
    uint32_t AdcValue = 0;
 // uint8_t str[100];
 // uint8_t bufferIndex;
		int i;
		uint16_t Vol[FUSER_ADC_SAMPLE_TIMES + 1] = {0};


    if(ADC_ReadStatusFlag(ADC_FLAG_CC) == SET)
    {
        ADC_ClearStatusFlag(ADC_FLAG_CC);
				
//				AdcValue = ADC_ReadBufferData(id);
				for(i = 0; i < FUSER_ADC_SAMPLE_TIMES; i++)
				{
					Vol[i] = ADC_ReadBufferData(id);
					AdcValue = AdcValue + Vol[i];
					
				}
				AdcValue = AdcValue / FUSER_ADC_SAMPLE_TIMES;
				Voltage = (AdcValue * VOLTAGE_REFERENCE) / 4095;
        
    }
		return Voltage;
}

/*!
 * @brief       ADC get many times and get avreage
 *
 * @param       id:channl id   times:get times
 *
 * @retval      None
 *
 * @note
 */

uint16_t ADC_GetTimes(ADC_BUFFER_IDX_T id,int times)
{
	uint16_t Vol[20] = {0};
	uint32_t Voltages = 0;
	int i;
	for(i = 0; i < times; i++)
	{
		Vol[i] = ADC_GetVoltage(id);
		Voltages = Voltages + Vol[i];
		
	}
	(uint16_t)Voltages = Voltages / times;
	return Voltages;
}



/*!
 * @brief       将ADC电压值转换为定影温度值
 *
 * @param       rv_volt_value:电压值
 *
 * @retval      None
 *
 * @note
 */
int16_t actConvertMiddleADValueToTemperature(uint16_t rv_volt_value )
{
	int16_t at_temperature;
	uint16_t at_index;
	
	for(at_index = 0; at_index < VOLT_TO_FUSER_TEMPRETURE_INDEX_NUM; at_index++)
	{
		if(rv_volt_value >= c_volt_to_tmepreture_table[at_index].volt)
		{
			break;
		}
	}
	
	if(at_index == VOLT_TO_FUSER_TEMPRETURE_INDEX_NUM)
	{
		at_temperature = 230; //最大温度值 
	}
	else
	{
		at_temperature = c_volt_to_tmepreture_table[at_index].tempreture;
	}
	
	return at_temperature;
}


/*!
 * @brief       将ADC电压值转换输入电压值
 *
 * @param       rv_volt_value:电压值
 *
 * @retval      None
 *
 * @note
 */
int16_t actADValueToIntoValue(uint16_t rv_volt_value )
{
	int16_t Value;
	Value = rv_volt_value / 14; //单位 V
	return Value;
}

/*!
 * @brief       将ADC电压值转换输入电压值
 *
 * @param       rv_volt_value:电压值
 *
 * @retval      None
 *
 * @note
 */
void actGetAdcValue(void)
{
//	g_voltage_cth = ADC_GetTimes(CTH_READ,20); 
//	g_voltage_ac_v = ADC_GetTimes(AC_V_READ,20);
	if(g_fuser_voltage_times >= 100)
	{
		
		reqSecurityManage();
		g_fuser_voltage_times = 0;
		g_Power_Status_Flag = 1;  //该标志位表示ADC已经经过ADC电压滤波才能经行判错
		
	}
}

const FuserTempVoltTable c_volt_to_tmepreture_table[VOLT_TO_FUSER_TEMPRETURE_INDEX_NUM] = 
{
{-20,   3260},
{-19,   3258},
{-18,   3256},
{-17,   3253},
{-16,   3251},
{-15,   3249},
{-14,   3247},
{-13,   3244},
{-12,   3242},
{-11,   3239},
{-10,   3236},
{-9 ,   3233},
{-8 ,   3230},
{-7 ,   3227},
{-6 ,   3224},
{-5 ,   3221},
{-4 ,   3217},
{-3 ,   3213},
{-2 ,   3210},
{-1 ,   3206},
{0  ,   3202},
{1  ,   3198},
{2  ,   3194},
{3  ,   3189},
{4  ,   3185},
{5  ,   3180},
{6  ,   3175},
{7  ,   3170},
{8  ,   3165},
{9  ,   3159},
{10 ,   3154},
{11 ,   3148},
{12 ,   3142},
{13 ,   3136},
{14 ,   3130},
{15 ,   3123},
{16 ,   3117},
{17 ,   3110},
{18 ,   3103},
{19 ,   3096},
{20 ,   3088},
{21 ,   3081},
{22 ,   3073},
{23 ,   3065},
{24 ,   3057},
{25 ,   3048},
{26 ,   3039},
{27 ,   3030},
{28 ,   3021},
{29 ,   3012},
{30 ,   3002},
{31 ,   2993},
{32 ,   2982},
{33 ,   2972},
{34 ,   2962},
{35 ,   2951},
{36 ,   2940},
{37 ,   2928},
{38 ,   2917},
{39 ,   2905},
{40 ,   2893},
{41 ,   2881},
{42 ,   2868},
{43 ,   2856},
{44 ,   2843},
{45 ,   2829},
{46 ,   2816},
{47 ,   2802},
{48 ,   2788},
{49 ,   2774},
{50 ,   2760},
{51 ,   2745},
{52 ,   2730},
{53 ,   2715},
{54 ,   2700},
{55 ,   2684},
{56 ,   2668},
{57 ,   2652},
{58 ,   2636},
{59 ,   2620},
{60 ,   2603},
{61 ,   2586},
{62 ,   2569},
{63 ,   2551},
{64 ,   2534},
{65 ,   2516},
{66 ,   2498},
{67 ,   2480},
{68 ,   2462},
{69 ,   2444},
{70 ,   2425},
{71 ,   2407},
{72 ,   2388},
{73 ,   2369},
{74 ,   2349},
{75 ,   2330},
{76 ,   2311},
{77 ,   2291},
{78 ,   2272},
{79 ,   2407},
{80 ,   2232},
{81 ,   2212},
{82 ,   2192},
{83 ,   2172},
{84 ,   2152},
{85 ,   2132},
{86 ,   2111},
{87 ,   2091},
{88 ,   2071},
{89 ,   2050},
{90 ,   2030},
{91 ,   2009},
{92 ,   1989},
{93 ,   1968},
{94 ,   1948},
{95 ,   1927},
{96 ,   1907},
{97 ,   1886},
{98 ,   1866},
{99 ,   1846},
{100,   1825},
{101,   1805},
{102,   1785},
{103,   1764},
{104,   1744},
{105,   1724},
{106,   1704},
{107,   1684},
{108,   1664},
{109,   1654},
{110,   1625},
{111,   1605},
{112,   1586},
{113,   1567},
{114,   1547},
{115,   1528},
{116,   1509},
{117,   1490},
{118,   1472},
{119,   1453},
{120,   1435},
{121,   1416},
{122,   1398},
{123,   1380},
{124,   1362},
{125,   1345},
{126,   1327},
{127,   1310},
{128,   1292},
{129,   1275},
{130,   1258},
{131,   1241},
{132,   1225},
{133,   1208},
{134,   1192},
{135,   1176},
{136,   1160},
{137,   1144},
{138,   1128},
{139,   1113},
{140,   1098},
{141,   1083},
{142,   1068},
{143,   1053},
{144,   1039},
{145,   1024},
{146,   1010},
{147,   996 },
{148,   982 },
{149,   968 },
{150,   955 },
{151,   942 },
{152,   928 },
{153,   915 },
{154,   902 },
{155,   890 },
{156,   877 },
{157,   865 },
{158,   853 },
{159,   840 },
{160,   829 },
{161,   817 },
{162,   806 },
{163,   794 },
{164,   783 },
{165,   772 },
{166,   761 },
{167,   750 },
{168,   740 },
{169,   729 },
{170,   719 },
{171,   709 },
{172,   699 },
{173,   689 },
{174,   679 },
{175,   670 },
{176,   660 },
{177,   651 },
{178,   642 },
{179,   633 },
{180,   624 },
{181,   615 },
{182,   606 },
{183,   598 },
{184,   590 },
{185,   581 },
{186,   573 },
{187,   565 },
{188,   557 },
{189,   550 },
{190,   542 },
{191,   534 },
{192,   527 },
{193,   520 },
{194,   512 },
{195,   505 },
{196,   498 },
{197,   492 },
{198,   485 },
{199,   478 },
{200,   472 },
{201,   465 },
{202,   459 },
{203,   452 },
{204,   446 },
{205,   440 },
{206,   434 },
{207,   428 },
{208,   423 },
{209,   417 },
{210,   411 },
{211,   406 },
{212,   400 },
{213,   395 },
{214,   390 },
{215,   384 },
{216,   379 },
{217,   374 },
{218,   369 },
{219,   365 },
{220,   360 },
{221,   355 },
{222,   350 },
{223,   346 },
{224,   341 },
{225,   337 },
{226,   332 },
{227,   328 },
{228,   324 },
{229,   320 },
{230,   316 },
{231,   311 },
{232,   307 },
{233,   303 },
{234,   300 },
{235,   296 },
{236,   292 },
{237,   288 },
{238,   285 },
{239,   281 },
{240,   278 },
{241,   274 },
{242,   271 },
{243,   267 },
{244,   264 },
{245,   261 },
{246,   257 },
{247,   254 },
{248,   251 },
{249,   248 },
{250,   245 },
};


/*由于热敏电阻的特性，最大测量温度250℃，超过后不准确 */
