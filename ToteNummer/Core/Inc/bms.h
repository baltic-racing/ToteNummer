/*
 * bms.h
 *
 *  Created on: Nov 29, 2025
 *      Author: finja und lilly
 */

#ifndef SRC_BMS_H_
#define SRC_BMS_H_

#include "main.h"
#include "stdint.h"
#include "stdbool.h"

void BMS(void);
void BMS_init(void);

void convertVoltage(void);
void sortTemperature(uint8_t selTemp);
void convertTemperature(uint8_t selTemp);

void send_usb(void);

#endif /* SRC_BMS_H_ */
