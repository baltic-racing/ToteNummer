/*
 * define.h
 *
 *  Created on: Nov 15, 2025
 *      Author: finja
 */

#ifndef SRC_DEFINE_H_
#define SRC_DEFINE_H_

#define NUM_STACK 12						 //total slaves
#define NUM_CELLS_STACK 12					 //Cells per stack Attention LTC6811 CH all modus
#define NUM_GPIO_STACK 6					 //GPIOs per slave
#define NUM_CELLS NUM_CELLS_STACK *NUM_STACK //Cells per accu container
#define NUM_GPIO NUM_GPIO_STACK *NUM_STACK   //GPIOs per slave


#define MAX_VOLTAGE 42500					// Wert in 0,1 mV
#define MIN_VOLTAGE 25000					// es gehen nur Vielfache von 16
//#define blancing_Voltage 42000

#define MAX_TS_VOLTAGE 554
#define MIN_TS_VOLTAGE 343

#define MAX_Temp 58611
#define MIN_Temp 1


#define BYTES_IN_REG 6
#define CMD_LEN 4 + (8 * NUM_STACK)

#define NUM_RX_BYT 8
#define CELL_IN_REG 3
#define AUX_IN_REG 3

#define error_max 5

#define volt_detect_time 700	//nicht regelkonform
#define temp_detect_time 1200
#define precharge_check_time 3000
#define switch_time 100

#define MIN_IMD_RES 400 // minimaler ISOwiderstand in kOhm

#define TSON_RESET_TIME 300

#define NTC_BETA 3892



#endif /* SRC_DEFINE_H_ */
