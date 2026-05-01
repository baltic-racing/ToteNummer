/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "gpio.h"
#include "main.h"
#include "define.h"
#include "gpio.h"
#include "tim.h"
#include "bms.h"

extern uint8_t AMS0_databytes[8];          // CAN-Datenarray für AMS0 Nachricht
extern uint8_t AMS1_databytes[8];          // CAN-Datenarray für AMS1 Nachricht

extern uint16_t cellVoltages[NUM_CELLS];   // Zellspannungen aus BMS
extern uint16_t temperature[NUM_CELLS];     // Temperaturen aus BMS
extern int16_t ltcTemps_c10[NUM_STACK];     // LTC Temperaturen in 0,1 °C

extern uint8_t precharge;                  // Status Precharge
extern uint16_t adc_accu_volt;              // Akkuspannung vom ADC
extern uint16_t imdStatValue;               // IMD Statuswert

extern uint16_t ts_volt_can;                // Traktionssystemspannung für CAN
extern int16_t temp_c10;                    // Temperatur in 0,1 °C

extern uint8_t ts_ready;                    // TS Ready Status
extern uint8_t IMD_ERROR;                   // IMD Fehlerstatus
extern uint8_t AMS_ERROR;                   // AMS Fehlerstatus

uint8_t DIC0_databytes[8];                  // Empfangene/gesendete DIC Daten
uint8_t dc_current[8];                      // CAN-Daten für DC-Strom

uint32_t current_data = 0;                  // Rohwert vom IVT Stromsensor
int32_t current = 0;                        // Stromwert, signed weil Strom negativ sein kann

uint8_t ts_on = 0;                          // TS Einschaltanforderung
uint8_t ts_start = 0;                       // TS Startstatus
uint8_t charging = 0;                       // Ladezustand
uint8_t ams_status = 0;                     // Laufender Statuszähler
uint8_t switch_on = 0;                      // Schalterstatus

uint32_t capacity_data = 0;                 // Kapazitätswert vom IVT
uint32_t ivt_error_time = 0;                // Zeitpunkt der letzten IVT Nachricht

uint8_t counter = 0;                        // Allgemeiner Zähler

/*
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;
uint32_t TxMailbox2;
*/

#define STOP 0                              // IVT Stop-Modus
#define RUN  1                              // IVT Run-Modus

#define IVT_MSG_RESULT_T   4                // IVT Temperaturkanal
#define IVT_MSG_RESULT_W   5                // IVT Leistungskanal
#define IVT_MSG_RESULT_As  6                // IVT Kapazität in As
#define IVT_MSG_RESULT_Wh  7                // IVT Energie in Wh

// transmit CAN Message
void CAN_TX(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data)
{
    CAN_TxHeaderTypeDef header = {0};       // CAN Header wird lokal erstellt
    uint32_t mailbox;                       // Mailbox, die HAL zum Senden benutzt

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
        return;                             // Wenn keine Mailbox frei ist, nicht blockieren

    header.StdId = id;                      // CAN Standard-ID setzen
    header.IDE = CAN_ID_STD;                // Standard-ID verwenden, keine Extended-ID
    header.RTR = CAN_RTR_DATA;              // Normale Datennachricht
    header.DLC = 8;                         // Immer 8 Datenbytes senden

    if (HAL_CAN_AddTxMessage(hcan, &header, data, &mailbox) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOD, LED_RD_Pin, GPIO_PIN_RESET); // Fehler-LED einschalten
    }
}

void IVT_MODE(uint8_t mode)
{
    uint8_t data[8];                        // Datenarray für IVT Befehl

    data[0] = 0x34;                         // IVT Befehl für Modus setzen
    data[1] = mode;                         // STOP oder RUN
    data[2] = 0x01;                         // IVT Parameter
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;

    CAN_TX(&hcan2, 0x411, data);            // Befehl an IVT über CAN2 senden
}

void IVT_ACTIVATE(uint8_t channel)
{
    uint8_t data[8];                        // Datenarray für IVT Aktivierung

    data[0] = 0x20 | channel;               // Gewünschten IVT Kanal aktivieren
    data[1] = 0x02;                         // IVT Konfigurationswert
    data[2] = 0x00;
    data[3] = 0x14;                         // Zykluszeit/Parameter für IVT
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;

    CAN_TX(&hcan2, 0x411, data);            // Aktivierungsbefehl an IVT senden
}

void IVT_init(void)
{
    static uint32_t t = 0;                  // Speichert die letzte Zeit
    static uint8_t state = 0;               // Speichert aktuellen Initialisierungsschritt

    switch (state)
    {
        case 0:
            t = HAL_GetTick();             // Startzeit merken
            state = 1;                     // Zum nächsten Schritt gehen
            break;

        case 1:
            if (HAL_GetTick() - t >= 1000) // 1 Sekunde warten
            {
                IVT_MODE(STOP);            // IVT zuerst stoppen
                t = HAL_GetTick();         // Zeit neu merken
                state = 2;
            }
            break;

        case 2:
            if (HAL_GetTick() - t >= 100)  // 100 ms warten
            {
                IVT_ACTIVATE(IVT_MSG_RESULT_As); // Kapazitätsmessung aktivieren
                t = HAL_GetTick();
                state = 3;
            }
            break;

        case 3:
            if (HAL_GetTick() - t >= 100)  // 100 ms warten
            {
                IVT_ACTIVATE(IVT_MSG_RESULT_W);  // Leistungsmessung aktivieren
                t = HAL_GetTick();
                state = 4;
            }
            break;

        case 4:
            if (HAL_GetTick() - t >= 100)  // 100 ms warten
            {
                IVT_MODE(RUN);             // IVT wieder starten
                state = 5;                 // Initialisierung fertig
            }
            break;

        case 5:
            break;                         // Fertig, nichts mehr machen
    }
}

void CAN_Send_All_LTC_Temps_Frame(uint8_t frame)
{
    uint8_t data[8];                        // 8 Byte CAN Daten

    if (frame == 0)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            data[2 * i]     = (uint8_t)(ltcTemps_c10[i] & 0xFF);      // Low Byte
            data[2 * i + 1] = (uint8_t)(ltcTemps_c10[i] >> 8);        // High Byte
        }

        CAN_TX(&hcan1, 0x220, data);        // Erste 4 LTC Temperaturen senden
    }
    else if (frame == 1)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            data[2 * i]     = (uint8_t)(ltcTemps_c10[i + 4] & 0xFF);  // Low Byte
            data[2 * i + 1] = (uint8_t)(ltcTemps_c10[i + 4] >> 8);    // High Byte
        }

        CAN_TX(&hcan1, 0x221, data);        // Zweite 4 LTC Temperaturen senden
    }
    else if (frame == 2)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            data[2 * i]     = (uint8_t)(ltcTemps_c10[i + 8] & 0xFF);  // Low Byte
            data[2 * i + 1] = (uint8_t)(ltcTemps_c10[i + 8] >> 8);    // High Byte
        }

        CAN_TX(&hcan1, 0x222, data);        // Dritte 4 LTC Temperaturen senden
    }
}


void can_put_data()
{
    AMS0_databytes[0] = ts_volt_can & 0xFF; 	// Spannung Low Byte
    AMS0_databytes[1] = ts_volt_can >> 8;  		// Spannung High Byte
    AMS0_databytes[2] = current & 0xFF;   		// Strom Low Byte
    AMS0_databytes[3] = current >> 8;     		// Strom High Byte
    AMS0_databytes[4] = imdStatValue & 0xFF; 	// IMD Low Byte
    AMS0_databytes[5] = imdStatValue >> 8;   	// IMD High Byte
	AMS0_databytes[6] =  0  | (ts_ready << 3) | (precharge << 4) | (IMD_ERROR << 6) | (AMS_ERROR << 7);	 // Statusbits in ein Byte packen
	AMS0_databytes[7] = ams_status;				// Laufender Statuszähler
}

void CAN_50(uint8_t precharge_data[])
{
    CAN_TX(&hcan1, 0x200, precharge_data);  // AMS0 Nachricht mit 50 Hz senden

    ams_status++;                           // Statuszähler erhöhen

    if (ams_status == 255)
    {
        ams_status = 0;                     // Zähler zurücksetzen
    }
}

void CAN_10(uint8_t bms_data[])
{
    static uint8_t step = 0;                // Merkt, welche Nachricht als nächstes kommt

    if (step == 0)
    {
        CAN_TX(&hcan1, 0x201, bms_data);    // AMS1 senden
        CAN_TX(&hcan1, 0x210, dc_current);  // Stromdaten senden
    }
    else if (step == 1)
    {
        CAN_Send_All_LTC_Temps_Frame(0);    // Temperaturframe 0 senden
    }
    else if (step == 2)
    {
        CAN_Send_All_LTC_Temps_Frame(1);    // Temperaturframe 1 senden
    }
    else if (step == 3)
    {
        CAN_Send_All_LTC_Temps_Frame(2);    // Temperaturframe 2 senden
    }

    step++;                                 // Zum nächsten Schritt gehen

    if (step >= 4)
    {
        step = 0;                           // Wieder von vorne anfangen
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;   // CAN Header (ID usw.)
    uint8_t RxData[8];             // Empfangene Datenbytes

    // Nachricht aus FIFO lesen
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
        return;

    // Nachricht von CAN1
    if (hcan->Instance == CAN1)
    {
        // ID 0x500 = Buttons / DIC
        if (RxHeader.StdId == 0x500)
        {
            // Bit 0 = Einschalten
            if ((RxData[0] & 0x01) == 1)
            {
                ts_on = 1;
                switch_on = 1;
            }

            // Bit 7 = Laden aktiv
            charging = (RxData[0] >> 7) & 0x01;
        }
    }

    // Nachricht von CAN2
    if (hcan->Instance == CAN2)
    {
        // ID 0x521 = Strom vom IVT
        if (RxHeader.StdId == 0x521)
        {
            // 4 Bytes zu Stromwert zusammensetzen
            int32_t raw_current =
                ((int32_t)RxData[2] << 24) |
                ((int32_t)RxData[3] << 16) |
                ((int32_t)RxData[4] << 8)  |
                ((int32_t)RxData[5]);

            current = raw_current / 100;   // Strom umrechnen
            ivt_error_time = HAL_GetTick(); // Letzte IVT Nachricht merken

            // Rohdaten speichern
            dc_current[4] = RxData[2];
            dc_current[5] = RxData[3];
            dc_current[6] = RxData[4];
            dc_current[7] = RxData[5];
        }

        // ID 0x527 = Kapazität vom IVT
        if (RxHeader.StdId == 0x527)
        {
            // 2 Bytes zu Wert zusammensetzen
            capacity_data = ((uint16_t)RxData[4] << 8) | RxData[5];

            // Daten speichern
            AMS0_databytes[4] = RxData[4];
            AMS0_databytes[5] = RxData[5];
        }
    }
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */


  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  CAN_FilterTypeDef canfilterconfig1;

  canfilterconfig1.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig1.FilterBank = 0;  // which filter bank to use from the assigned ones
  canfilterconfig1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canfilterconfig1.FilterIdHigh = 0x500<<5;
  canfilterconfig1.FilterIdLow = 0;
  canfilterconfig1.FilterMaskIdHigh = 0x7FF<<5;
  canfilterconfig1.FilterMaskIdLow = 0x0000;
  canfilterconfig1.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig1.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilterconfig1.SlaveStartFilterBank = 14;  // how many filters to assign to the CAN1 (master can)

  HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig1);

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 3; //16;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  //hcan2.Init.TimeSeg1 = CAN_BS1_1TQ;
  //hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = ENABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */
  CAN_FilterTypeDef canfilterconfig_ivt;

    canfilterconfig_ivt.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig_ivt.FilterBank = 14;  // which filter bank to use from the assigned ones
    canfilterconfig_ivt.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilterconfig_ivt.FilterIdHigh = 0x521<<5;
    canfilterconfig_ivt.FilterIdLow = 0;
    canfilterconfig_ivt.FilterMaskIdHigh = 0x7FF<<5;
    canfilterconfig_ivt.FilterMaskIdLow = 0x0000;
    canfilterconfig_ivt.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig_ivt.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig_ivt.SlaveStartFilterBank = 14;  // how many filters to assign to the CAN1 (master can)

    HAL_CAN_ConfigFilter(&hcan2, &canfilterconfig_ivt);

	CAN_FilterTypeDef canfilterconfig_ivt1;
	canfilterconfig_ivt1.FilterActivation = CAN_FILTER_ENABLE;
	  canfilterconfig_ivt1.FilterBank = 15;  // which filter bank to use from the assigned ones
	  canfilterconfig_ivt1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	  canfilterconfig_ivt1.FilterIdHigh = 0x527<<5;
	  canfilterconfig_ivt1.FilterIdLow = 0;
	  canfilterconfig_ivt1.FilterMaskIdHigh = 0x7FF<<5;
	  canfilterconfig_ivt1.FilterMaskIdLow = 0x0000;
	  canfilterconfig_ivt1.FilterMode = CAN_FILTERMODE_IDMASK;
	  canfilterconfig_ivt1.FilterScale = CAN_FILTERSCALE_32BIT;
	  canfilterconfig_ivt1.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&hcan2, &canfilterconfig_ivt1);
  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
