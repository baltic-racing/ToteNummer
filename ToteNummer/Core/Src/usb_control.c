/*
 * usb_control.c
 *
 *  Created on: Dec 5, 2025
 *      Author: Lilly
 */
#include <main.h>
#include "stdlib.h"
#include "stdbool.h"
#include "gpio.h"
#include "bms.h"
#include "LTC6811.h"
#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include "usb_control.h"

#include <string.h>   // strcmp
#include <stdint.h>

extern volatile uint8_t BMS_state;
extern USBD_HandleTypeDef hUsbDeviceFS;

#define CMD_STX 0x02

//static volatile uint16_t rxWrite = 0, rxRead = 0;
static volatile uint16_t rxWrite = 0;
static volatile uint16_t rxRead  = 0;

uint8_t debug_tx_buffer[64];
uint8_t debug_tx_len;

uint8_t debug_len = 0;
uint8_t debug_buffer[64];

static uint8_t packet[64];

/*
.______________.
| transmit_state | Description             |
|==========================================|
|      0x03      | slave_temp              |
|______________|
*/

void USB_control(const char *broadcaster, uint8_t *data_shit, uint8_t data_size_bytes)
{
    uint8_t type = 0x00;

	if (strcmp(broadcaster, "ID_TS_Voltage") == 0)
		type = 0x21;
	else if (strcmp(broadcaster, "ID_TS_Current") == 0)
		type = 0x22;
	else if (strcmp(broadcaster, "ID_TS_Currentdrawn") == 0)
		type = 0x23;
	else if (strcmp(broadcaster, "ID_TS_AMS_Status") == 0)
		type = 0x24;
	else if (strcmp(broadcaster, "ID_TS_Cell_Voltages_max") == 0)
		type = 0x25;
	else if (strcmp(broadcaster, "ID_TS_Cell_Voltages_min") == 0)
		type = 0x26;
	else if (strcmp(broadcaster, "ID_TS_Cell_Temprearure_max") == 0)
		type = 0x27;
	else if (strcmp(broadcaster, "ID_TS_Cell_Temprearure_min") == 0)
		type = 0x28;
	else if (strcmp(broadcaster, "ID_TS_Cellnumer_Voltages_max") == 0)
		type = 0x29;
	else if (strcmp(broadcaster, "ID_TS_Cellnumer_Voltages_min") == 0)
		type = 0x2A;
	else if (strcmp(broadcaster, "ID_TS_Cellnumer_Temprearure_max") == 0)
		type = 0x2B;
	else if (strcmp(broadcaster, "ID_TS_Cellnumer_Temprearure_min") == 0)
		type = 0x2C;
	else if (strcmp(broadcaster, "ID_LTC_Temperature") == 0)
		type = 0x2D;
	else
		return;

	USB_transmit(type, data_shit, data_size_bytes);
}

#define CMD_STX 0x02

void USB_transmit(uint8_t type, const uint8_t *data_shit, uint8_t shit_count)
{
    uint8_t payload_len = shit_count;

    if (payload_len > 60)
        return;

    uint8_t idx = 0;
    uint8_t chk = 0;

    packet[idx++] = CMD_STX;
    packet[idx++] = type;
    packet[idx++] = payload_len;

    for (uint8_t i = 0; i < payload_len; i++)
    {
        packet[idx++] = data_shit[i];
    }

    for (uint8_t i = 0; i < idx; i++)
    {
        chk ^= packet[i];
    }

    packet[idx++] = chk;

    CDC_SendBlocking(packet, idx, 3);
}

uint8_t CDC_SendBlocking(uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
    {
        return 0;
    }

    uint32_t start = HAL_GetTick();

    while (CDC_Transmit_FS(buf, len) == USBD_BUSY)
    {
        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return 0;
        }
    }

    return 1;
}
