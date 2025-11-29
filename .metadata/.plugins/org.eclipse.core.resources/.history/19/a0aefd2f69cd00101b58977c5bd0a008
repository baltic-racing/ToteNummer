/*
 * bms.c
 *
 *  Created on: Nov 29, 2025
 *      Author: finja
 */


#include "bms.h"
#include "LTC6811.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "can.h"
#include "adc.h"
#include "usbd_cdc_if.h"
#include "string.h"
#include "define.h"
#include "gpio.h"

uint8_t precharge = 0;

extern uint8_t charging;

uint16_t ts_volt_can = 0;
uint16_t blancing_Voltage = 42000;
uint16_t max_voltage = MAX_VOLTAGE;
uint16_t balanceMargin = 100; //in 0.1mV
uint8_t volt_stamp = 0;
uint8_t temp_stamp = 0;
uint8_t imd_stamp = 0;

uint8_t IMD_ERROR = 0;
uint8_t AMS_ERROR = 0;
uint8_t PEC_ERROR = 0;

extern uint8_t ts_on;
extern uint8_t ts_start;
extern uint8_t sc_state;
extern uint16_t current;

uint32_t ICValue = 0;
float Duty = 0;
uint16_t imdStatValue = 0;
uint8_t IMDcheck = 0;

uint16_t cellVoltages[NUM_CELLS] = {0};	//cell voltages in 0.1[mV]
int16_t cellTemperatures[NUM_CELLS] = {0}; //cell temperatures in 0.1[°C]
uint8_t cfg[NUM_STACK][6] = {{0}}; //0x38 disables the GPIO1..3 pulldown so GPIO1..3 can be used for measurement
uint16_t slaveGPIOs[NUM_GPIO] = {0};
uint16_t temperature[NUM_CELLS] = {0};

uint8_t usb_data[NUM_CELLS*2 + 2 + 1] = {0};
uint8_t usb_voltages[NUM_CELLS_STACK*NUM_STACK] = {0};
uint8_t usb_temperatures[NUM_CELLS_STACK*NUM_STACK] = {0};

uint8_t AMS0_databytes[8];
uint8_t AMS1_databytes[8];


uint16_t OV_flag[NUM_STACK];
uint16_t UV_flag[NUM_STACK];
uint8_t r_statb[NUM_STACK][6];

uint32_t can_cnt = 0; //can counter to adjust timings
uint64_t last20 =0;
uint64_t last100 =0;
uint64_t last500 = 0;
uint32_t volt_error_time = 0;
uint32_t temp_error_time = 0;
uint32_t pec_error_time = 0;
extern uint32_t ivt_error_time;

uint8_t temp_error = 0;
uint8_t count_selTemp = 0;

uint32_t current_time = 0;

uint8_t index_i = 0;
uint8_t index_k = 0;
uint8_t cell_number_temp_min = 0;
uint8_t cell_number_temp_max = 0;
uint8_t cell_number_volt_min = 0;
uint8_t cell_number_volt_max = 0;

extern uint8_t dc_current[8];

/* 1 ms interrupt
 * HLCK 96 MHz
 * APB1 48 MHz
 */


void BMS_init()
{
	LTC6811_initialize();
}

static void BMS_WaitMs(uint32_t ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < ms)
    {
    }
}

void BMS()		// Battery Management System function for main loop.
{
	uint8_t pec = 0;
	static uint8_t selTemp = 0;
	//uint16_t VOV = MAX_VOLTAGE/16;					// Formeln aus Datenblatt S.65
	//uint16_t VUV = (MIN_VOLTAGE/16)-1;
	get_ts_ready();

	//precharge = 1 when complete and 0 when still charging
	precharge |= ADC_TS_Voltage(MAX_TS_VOLTAGE, MIN_TS_VOLTAGE);

	for (uint8_t i = 0; i < NUM_STACK; i++)
	{
		//Balancing with flags
/*
		cfg[i][0] = 0x3C | ((selTemp << 6) & 0xC0);		//cfg : Databytes in config register of the LTC6811
		cfg[i][1] = 0x00 | VUV;
		cfg[i][2] = 0x00 | (VOV<<4) | (VUV>>4);
		cfg[i][3] = 0x00 | (VOV>>4);
		cfg[i][4] = 0x00 | OV_flag[i];
		cfg[i][5] = 0x00 | (OV_flag[i]>>8);

*/
		//Balancing without flags
		cfg[i][0] = 0x3C | ((selTemp << 6) & 0xC0);		//cfg : Databytes in config register of the LTC6811
		cfg[i][1] = 0x00;
		cfg[i][2] = 0x00;
		cfg[i][3] = 0x00;
		cfg[i][4] = 0x00;
		cfg[i][5] = 0x00;

		if(charging == 1)
		{
			balanceMargin = ((max_voltage - blancing_Voltage) * getbalancingKP(blancing_Voltage))/10;

			if(balanceMargin > 100)
			{
				if(selTemp < 5)
				{
					for(uint8_t j = 0; j < 8; j++)
					{
						if(cellVoltages[i * NUM_STACK + j] - blancing_Voltage > balanceMargin)cfg[i][4] |= 1 << j;
					}
					for(uint8_t j = 0; j < 3; j++)
					{
						if(cellVoltages[i * NUM_STACK + j + 8] - blancing_Voltage > balanceMargin)cfg[i][5] |= 1 << j;
					}
				}
			}

		}
	}

	LTC6811_wrcfg((uint8_t(*)[6])cfg);		// Write config
	BMS_WaitMs(3);

	LTC6811_adcv();										// measure voltages
	BMS_WaitMs(3);

	pec = LTC6811_rdcv(0, (uint16_t(*)[12])cellVoltages);	//read voltages
	BMS_WaitMs(3);

	LTC6811_clraux();
	BMS_WaitMs(3);

	LTC6811_adax();										// measure 3 celltemp
	BMS_WaitMs(3);

	pec = LTC6811_rdaux(0, (uint16_t(*)[6])slaveGPIOs);	// read celltemp
	BMS_WaitMs(3);

	convertVoltage();
	convertTemperature(selTemp);

	checkPEC(pec);

	//HAL_GPIO_WritePin(GPIOC, LED_RD_Pin, AMS_ERROR); //Aktuell led defekt 30.5.25

	if(IMDcheck == 0){
		if(HAL_GetTick()>2000){
			IMDcheck =1;
		}

	}
	if(IMDcheck){
		checkIMD();
	}


	if(HAL_GetTick() - ivt_error_time >= volt_detect_time){
		AMS_ERROR = 1;
	}

	if (selTemp < 3)
		selTemp++;		// Variable for cycling the multiplexers for temp measurement.
	else
		selTemp = 0;

	if(AMS_ERROR == 1 || IMD_ERROR == 1 )//|| PEC_ERROR == 1)
	{
		sc_state = 1;
		ts_on = 0;
		ts_start = 0;
		HAL_GPIO_WritePin(GPIOC, SC_STATE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(TS_ACTIVATE_GPIO_Port, TS_ACTIVATE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIR_P_SW_GPIO_Port, AIR_P_SW_Pin, GPIO_PIN_RESET);
	}

	can_put_data();
	send_usb();
}


void convertVoltage()		//convert and sort Voltages
{
	ts_volt_can = 0;
	for(uint8_t i = 0; i < NUM_CELLS; i++)
	{
		usb_voltages[i] = cellVoltages[i]/1000;
		ts_volt_can = ts_volt_can + cellVoltages[i]/100;
	}

	static uint16_t cell_max;
	static uint16_t cell_min;

	static uint8_t prev_number_volt_max = 0;
	static uint8_t prev_number_volt_min = 0;

	cell_max = cellVoltages[0];
	cell_min = cellVoltages[0];


	for(uint8_t k = 0; k < NUM_STACK; k++)
	{
		for(uint8_t i = 0; i < NUM_CELLS_STACK; i++)
		{
			if(cellVoltages[i + k * 12] > cell_max && (cellVoltages[i + k * 12] < 45000 || cellVoltages[i + k * 12] > 60000) && (i+1)%12 != 0)
			{
				cell_max = cellVoltages[i + k * 12];
				cell_number_volt_max = i + k * 12;
				dc_current[3] = cell_number_volt_max;
			}
			else if(cellVoltages[i + k * 12] < cell_min && (cellVoltages[i + k * 12] > 23000 || cellVoltages[i + k * 12] < 5000) && (i+1)%12 != 0)
			{
				cell_min = cellVoltages[i + k * 12];
				cell_number_volt_min = i + k * 12;
				dc_current[0] = cell_number_volt_min;
			}
		}
	}

	if(!((cell_min < MIN_VOLTAGE && cell_number_volt_min == prev_number_volt_min) || (cell_max > MAX_VOLTAGE && cell_number_volt_max == prev_number_volt_max)))
		volt_error_time = HAL_GetTick();
	/*
	if(!(cell_min < MIN_VOLTAGE || cell_max > MAX_VOLTAGE))
		volt_error_time = HAL_GetTick();
*/
	if(HAL_GetTick() - volt_error_time >= volt_detect_time)
		AMS_ERROR = 1;

	blancing_Voltage = cell_min;
	max_voltage = cell_max;

	AMS1_databytes[0] = cell_min;
	AMS1_databytes[1] = (cell_min >> 8);
	AMS1_databytes[2] = cell_max;
	AMS1_databytes[3] = (cell_max >> 8);

	prev_number_volt_max = cell_number_volt_max;
	prev_number_volt_min = cell_number_volt_min;
}

uint16_t calculateTemperature(uint16_t voltageCode, uint16_t referenceCode)		//convert temp
{

	uint16_t denom = referenceCode - voltageCode;
	//if((referenceCode - voltageCode) != 0 && voltageCode != 0)
	if (denom >= 2000 && voltageCode != 0)
	{
		uint32_t convert_R = (voltageCode * 100000)/(referenceCode - voltageCode);
		return 1000.0 / ((1.0 / 298.15) - (log(10000.0 / convert_R) / NTC_BETA)) - 273150.0;
	}
	else return 0xFFFF;
}

void CAN_interrupt()
{
	if (HAL_GetTick()>= last20 + 20)
	{
		CAN_50(AMS0_databytes);
		last20 = HAL_GetTick();
	}
	if (HAL_GetTick()>= last100 + 100)
	{
		CAN_10(AMS1_databytes);

		HAL_GPIO_TogglePin(GPIOA, WDI_Pin);		// toggle watchdog
		HAL_GPIO_TogglePin(GPIOC, LED_GN_Pin);	// toggle LED
		last100 = HAL_GetTick();
		send_usb();
	}
}

uint16_t find_me = 0;
uint8_t test2 = 0;

void sortTemperature(uint8_t selTemp)
{
	//uint8_t indexOffset[12] = {9, 4, 11, 7, 6, 1, 0, 3, 10, 2, 5, 8};
	uint8_t indexOffset[12] = {11, 4, 1, 9, 6, 3, 0, 5, 10, 2, 7, 8};
	for(uint8_t k = 0; k < NUM_STACK; k++)
	{
		for(uint8_t j = 0; j < 3; j++)
		{
			if(selTemp == 0 && j == 0)
			{
				temperature[k * NUM_CELLS_STACK + indexOffset[j + selTemp * 3]] = 0xFFFE;
			}
			else
			{
				uint16_t curr_temp = calculateTemperature(slaveGPIOs[j + k * 6], slaveGPIOs[5 + k * NUM_GPIO_STACK]);
				temperature[k * NUM_CELLS_STACK + indexOffset[j + selTemp * 3]] = curr_temp;

			}

		}
	}
}

void convertTemperature(uint8_t selTemp)		// sort temp
{
	sortTemperature(selTemp);

	if(selTemp == 3)
	{
		static uint16_t temp_min;
		static uint16_t temp_max;

		temp_min = temperature[0];
		temp_max = temperature[0];

		static uint8_t prev_number_temp_max = 0;
		static uint8_t prev_number_temp_min = 0;

		for(uint8_t k = 0; k < NUM_STACK; k++)
			{
				for(uint8_t i = 0; i < NUM_CELLS_STACK; i++)
				{
					if(temperature[i + k * 12] > temp_max && ((i+1)%12 != 0 && (i+1)%10 != 0))
						{
							temp_max = temperature[i + k * 12];
							cell_number_temp_max = i + k * 12;
							dc_current[2] = cell_number_temp_max;
						}
					else if(temperature[i + k * 12] < temp_min && ((i+1)%12 != 0 && (i+1)%10 != 0))
					{
						temp_min = temperature[i + k * 12];
						index_i = i;
						index_k = k;
						cell_number_temp_min = i + k * 12;
						dc_current[1] = cell_number_temp_min;
					}

					if ((i+1)%10 != 0) {
						usb_temperatures[i + k * 12] = temperature[i + k * 12]/1000;
					}
					else if (temperature[i + k * 12] < 60000) {
						usb_temperatures[i + k * 12] = temperature[i + k * 12]/1000;
					}
				}
			}

		/*
		if(!(temp_min < MIN_Temp || temp_max > MAX_Temp))
					temp_error_time = HAL_GetTick();
*/
		if(!((temp_min < MIN_Temp && prev_number_temp_min == cell_number_temp_min) || (temp_max > MAX_Temp && prev_number_temp_max == cell_number_temp_max)))
					temp_error_time = HAL_GetTick();

		if(HAL_GetTick() - temp_error_time >= temp_detect_time){
			AMS_ERROR = 1;
		}

		AMS1_databytes[4] = temp_min;
		AMS1_databytes[5] = (temp_min >> 8);
		AMS1_databytes[6] = temp_max;
		AMS1_databytes[7] = (temp_max >> 8);

		prev_number_temp_max = cell_number_temp_max;
		prev_number_temp_min = cell_number_temp_min;
	}

}

