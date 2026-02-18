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
#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include "usb_control.h"
extern USBD_HandleTypeDef hUsbDeviceFS;

// Existing counters
volatile uint32_t usb_not_configured = 0;
volatile uint32_t usb_busy = 0;
volatile uint32_t usb_fail = 0;
volatile uint32_t usb_ok = 0;

// New counters (for visibility)
volatile uint32_t usb_bad_len = 0;
volatile uint32_t usb_too_big = 0;
volatile uint32_t usb_bad_src = 0;
volatile uint32_t usb_throttled = 0;

// ---- Settings ----
#ifndef USB_TX_THROTTLE_MS
#define USB_TX_THROTTLE_MS 100u   // 10 Hz
#endif

// ---- Pending buffer ("latest wins") ----
// Holds the most recent packet that should be sent.
static uint8_t  pending_packet[64];
static uint8_t  pending_len = 0;
static volatile uint8_t pending_valid = 0;

// Next time we are allowed to send (throttle)
static uint32_t next_allowed_send_ms = 0;


/*
.__________________________________________.
| transmit_state | Description             |
|==========================================|
|		0x03	 | slave_temp			   |
|__________________________________________|
*/

//void USB_control(const char *broadcaster, uint8_t *usb_data, uint8_t data_size)
void USB_control(const char *broadcaster, const uint8_t *usb_data, uint8_t byte_len)
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
    else {
            usb_bad_src++;
            return;
        }

    // must be (ID,H,L) triplets
        if ((byte_len % 3u) != 0u) { usb_bad_len++; return; }

        if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
            usb_not_configured++;
            pending_valid = 0;
            return;
        }

        uint8_t value_count = (uint8_t)(byte_len / 3u);
        uint8_t payload_len = (uint8_t)(value_count * 3u);
        if (payload_len > 60u) { usb_too_big++; return; }

        // Build packet
        uint8_t packet[64];
        uint8_t idx = 0;

        packet[idx++] = 0xAA;
        packet[idx++] = type;
        packet[idx++] = payload_len;

        for (uint8_t i = 0; i < value_count; i++) {
            packet[idx++] = usb_data[i * 3u + 0u]; // ID
            packet[idx++] = usb_data[i * 3u + 1u]; // H
            packet[idx++] = usb_data[i * 3u + 2u]; // L
        }

        packet[idx++] = 0x55;

        // Store as latest pending packet
        memcpy(pending_packet, packet, idx);
        pending_len = idx;
        pending_valid = 1;

        // Optional: kick one immediate attempt (still respects throttle & busy)
        USB_transmit();

}



//void USB_transmit(uint8_t type, uint8_t *ids_values, uint8_t value_count)
void USB_transmit(void)
{
	if (!pending_valid) return;

	    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
	        usb_not_configured++;
	        pending_valid = 0;
	        return;
	    }

	    uint32_t now = HAL_GetTick();

	    // Throttle: send only when allowed
	    if ((int32_t)(now - next_allowed_send_ms) < 0) {
	        usb_throttled++;
	        return;
	    }

	    uint8_t res = CDC_Transmit_FS(pending_packet, pending_len);

	    if (res == USBD_OK) {
	        usb_ok++;
	        pending_valid = 0;
	        next_allowed_send_ms = now + USB_TX_THROTTLE_MS;
	        return;
	    }

	    if (res == USBD_BUSY) {
	        usb_busy++;
	        // keep pending_valid = 1; retry next call
	        return;
	    }

	    usb_fail++;
	    pending_valid = 0; // drop on hard error
}
