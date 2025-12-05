/*
 * bms.h
 *
 *  Created on: Nov 29, 2025
 *      Author: finja
 */

#ifndef INC_BMS_H_
#define INC_BMS_H_


#include "main.h"
#include "stdint.h"
#include "stdbool.h"

void BMS(void);
void BMS_init(void);
void convertVoltage(void);
void sortTemperature(uint8_t selTemp);
void convertTemperature(uint8_t selTemp);
void CAN_interrupt(void);
void send_usb(void);
void checkIMD(void);
void checkPEC(uint8_t pec);
uint8_t getbalancingKP(uint16_t minVoltage);

#endif /* INC_BMS_H_ */

