/*
 * USB_control.h
 *
 *  Created on: Nov 16, 2025
 *      Author: Egquus
 */

#ifndef INC_USB_CONTROL_H_
#define INC_USB_CONTROL_H_

void USB_control(char *broadcaster, uint8_t *usb_data, uint8_t data_size);
void USB_transmit(uint8_t type, uint8_t *ids_values, uint8_t value_count);

#endif /* INC_USB_CONTROL_H_ */
