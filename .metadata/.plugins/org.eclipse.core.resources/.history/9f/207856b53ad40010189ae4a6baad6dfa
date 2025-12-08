/*
 * usb_control.c
 *
 *  Created on: Dec 5, 2025
 *      Author: Lilly
 */
#include "main.h"
#include "stdlib.h"
#include "stdbool.h"
#include "gpio.h"
#include "bms.h"
#include "LTC6811.h"



/*
.__________________________________________.
| transmit_state | Description             |
|==========================================|
|		0x03	 | slave_temp			   |
|__________________________________________|
*/

void USB_control(const char *broadcaster, uint8_t *usb_data, uint8_t data_size)
{
    uint8_t type = 0x00;

    if (strcmp(broadcaster, "slave") == 0) {
        type = 0x03;   // Slave Telemetrie
    }
    else if (strcmp(broadcaster, "tsac") == 0) {
        type = 0x01;   // TSAC Telemetrie
    }
    else if (strcmp(broadcaster, "debug") == 0) {
        type = 0x02;   // Debug
    }

    USB_transmit(type, usb_data, data_size);
}

void USB_transmit(uint8_t type, uint8_t *ids_values, uint8_t value_count)
{
    uint8_t payload_len = value_count * 2;   // ID + VAL je Wert = 2 Bytes

    if (payload_len > 60) {
        //BMS_state = 3;
        return;
    }

    uint8_t packet[64];
    uint8_t index = 0;

    packet[index++] = 0xAA;          // Start
    packet[index++] = type;          // Type
    packet[index++] = payload_len;   // Payload length

    // IDs und Werte übertragen
    for (int i = 0; i < value_count; i++) {
        packet[index++] = ids_values[i*2];     // ID
        packet[index++] = ids_values[i*2 + 1]; // Value
    }

    packet[index++] = 0x55;          // End

    CDC_Transmit_FS(packet, index);	 //Übermittelte Daten müssen ein pointer sein
}

