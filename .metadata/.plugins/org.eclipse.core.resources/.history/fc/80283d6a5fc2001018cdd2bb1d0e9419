/*
 * LTC6811.c
 *
 *  Created on: Nov 8, 2025
 *      Author: racin
 */


#include <stdint.h>
#include "LTC6811.h"
#include <string.h>
#include <stdio.h>
#include <main.h>
#include <math.h>
#include "define.h"
/*@brief 6811 conversion command variables
 */
uint8_t ADCV[2]; //!< Cell Voltage conversion command.
uint8_t ADAX[2]; //!< GPIO conversion command.
uint8_t CVST[2]; //!< Cell Voltage selftest command
uint8_t AXST[2]; //!< GPIO selftest command
uint8_t CLRAUX[2]; //clear Auxiliary register

uint8_t wakeup = 0x00;

void LTC6811_initialize()
{
  set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL, CHST_SC);
  //set_selftest(MD_NORMAL, ST_1);
}


void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG, uint8_t CHST)
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
}


void LTC6811_initialize()
{
  set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL, CHST_ITMP);
  //set_selftest(MD_NORMAL, ST_1);
}


void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG, uint8_t CHST)
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
}
