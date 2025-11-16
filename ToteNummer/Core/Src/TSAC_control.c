/*
 * TSAC_control.c
 *
 *  Created on: Oct 18, 2025
 *      Author: Egquus
 */
/* Includes ------------------------------------------------------------------*/
#include "TSAC_control.h"
/* USER CODE BEGIN 0 */
/*Includes*/
#include "main.h"
#include "can.h"

/* Variables */
extern volatile uint8_t BMS_state;

extern uint8_t CAN1_exe;
extern uint8_t CAN2_exe;
extern uint8_t TxData[8];

extern CAN_TxHeaderTypeDef TxHeader;
extern CAN_RxHeaderTypeDef RxHeader;

uint8_t LED_GN_state = 0;
uint8_t LED_YW_state = 0;
uint8_t LED_RD_state = 0;

/*Functions*/

void TSAC_control(){

	CAN_transceive(&hcan1, &CAN1_exe, TxData);

	Slave_control();
}

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
