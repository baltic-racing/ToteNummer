/*
 * usb_control.h
 *
 *  Created on: Dec 5, 2025
 *      Author: Lilly
 */

#ifndef INC_USB_CONTROL_H_
#define INC_USB_CONTROL_H_

#include <stdint.h>

uint8_t CDC_SendBlocking(uint8_t *buf, uint16_t len, uint32_t timeout_ms);
void USB_control(const char *broadcaster, uint8_t *usb_data, uint8_t data_size_bytes);

// Optional: Counters
extern volatile uint32_t usb_not_configured, usb_busy, usb_fail, usb_ok;
extern volatile uint32_t usb_bad_len, usb_too_big, usb_bad_src, usb_throttled;


void USB_transmit(uint8_t type, const uint8_t *data_shit, uint8_t shit_count);

/*
//void USB_control(const char *broadcaster, uint8_t *usb_data, uint8_t data_size);
void USB_control(const char *broadcaster, uint8_t *usb_data, uint8_t byte_len);
void USB_transmit(uint8_t type, uint8_t *ids_values, uint8_t value_count);
*/

#endif /* INC_USB_CONTROL_H_ */
