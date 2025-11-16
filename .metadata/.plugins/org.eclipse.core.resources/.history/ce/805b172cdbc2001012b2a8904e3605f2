/*
 * Slave_control.c
 *
 *  Created on: Nov 15, 2025
 *      Author: Egquus
 */

#include "main.h"
#include "radio_tx.h"
#include "stdlib.h"
#include "stdbool.h"
#include "usart.h"
#include "gpio.h"
#include "Slave_control.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/*@brief 6811 conversion command variables

*/
uint8_t ADSTAT[2]; //!< Start Status Group ADC command

uint8_t ADCV[2]; //!< Cell Voltage conversion command
uint8_t ADAX[2]; //!< GPIO conversion command
uint8_t CVST[2]; //!< Cell Voltage selftest command
uint8_t AXST[2]; //!< GPIO selftest command
uint8_t CLRAUX[2]; //clear Auxiliary register

uint8_t wakeup = 0x00;

uint8_t pec = 0;

uint8_t data [1];

/*@brief Initializes all command variables
 */

void Slave_control(){

	if(config_watch != 0){
		Config_setup(config_watch);

	}

	LTC_cmd();						// measure order/command

	pec = LTC_readout(LTC_temp, data);	// Read measuring|später dann zum testen via USB auslesen

}
/*________________________________________________________________________________________________________*/
void wakeup_idle()
{
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &wakeup, 1, 1);
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);
}
/*________________________________________________________________________________________________________*/
void Config_setup(uint8_t *config){
	//standard configuration for meassure Cell temperature and Cell voltage
	if(config_watch == 1){
		set_setup(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL, CHST_SC);
	}
	//configuration for read LTC temperature -> Internal Die Temperature (ITMP) via ADC1 [datasheet p. 17/29]
	if(config_watch == 2){
		set_setup(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL, CHST_ITMP);
	}

	return (config = 0);
}

void set_setup(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG, uint8_t CHST)
{
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  ADCV[0] = md_bits | 0x02;
  md_bits = (MD & 0x01) << 7;
  ADCV[1] = md_bits | 0x60 | (DCP << 4) | CH;

  md_bits = (MD & 0x02) >> 1;
  ADAX[0] = md_bits | 0x04;
  md_bits = (MD & 0x01) << 7;
  ADAX[1] = md_bits | 0x60 | CHG;

  CLRAUX[0] = 0x0E;
  CLRAUX[1] = 0x12;

  md_bits = (MD & 0x02) >> 1;
  ADSTAT[0] = md_bits | 0x04;
  md_bits = (MD & 0x01) << 7;
  ADSTAT[1] = md_bits | 0x68 | CHST;
}
/*________________________________________________________________________________________________________*/
void LTC_cmd()
{
	uint8_t cmd[4];
	uint16_t temp_pec;
	//1
	cmd[0] = ADSTAT[0];
	cmd[1] = ADSTAT[1];
	//2
	temp_pec = pec15_calc(2, ADSTAT);
	cmd[2] = (uint8_t)(temp_pec >> 8);
	cmd[3] = (uint8_t)(temp_pec);
	//3
	wakeup_idle();
	//4
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);
	spi_write_array(4, cmd);
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);

	HAL_SPI_Transmit(&hspi3, &wakeup, 1, 1);
	HAL_SPI_Transmit(&hspi3, &wakeup, 1, 1);

}

void LTC_readout(uint8_t reg, uint8_t *data)
{
	//if(reg == LTC_temp){
		uint8_t RDICT[4];
		uint16_t temp_pec;

		wakeup_idle();


		RDICT[0] = 0x80;
		temp_pec = pec15_calc(2, RDICT);
		RDICT[2] = (uint8_t)(temp_pec >> 8);
		RDICT[3] = (uint8_t)(temp_pec);
		HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);
		spi_write_read(RDICT, 4, &data[0], 8);
		HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);
	//}
}

/*________________________________________________________________________________________________________*/
//char *data uint8_t *data , uint8_t len
uint16_t pec15_calc(uint8_t len, uint8_t *data)
 {
 uint16_t remainder, address;

 remainder = 16;//PEC seed
 for (int i = 0; i < len; i++)
 {
 address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
 remainder = (remainder << 8 ) ^ pec15Table[address];
 }
 return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
 }
