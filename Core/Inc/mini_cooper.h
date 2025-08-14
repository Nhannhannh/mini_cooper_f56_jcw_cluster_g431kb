/*
 * mini_cooper.h
 *
 *  Created on: Aug 11, 2025
 *      Author: user
 */

#ifndef INC_MINI_COOPER_H_
#define INC_MINI_COOPER_H_

#include "fdcan.h"
#include <string.h>

#define CRC8_POLYNOMIAL 0x1D
typedef uint8_t CrcCooper_t;

void GenerateCooperCrc8Table(void);
CrcCooper_t GetCooperCrc8(uint8_t *_message, uint8_t _length,
		uint8_t _final_xor);

int32_t map_int(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min,
		int32_t out_max);

// Visit https://github.com/r00li/CarCluster for more CAN frames!
void send_ignition_on(uint32_t delay_before_send);
void send_speed(uint16_t speed, uint32_t delay_before_send);
void send_rpm(uint16_t rpm, uint8_t gear, uint32_t delay_before_send);
void send_blinkers(uint8_t left, uint8_t right, uint32_t delay_before_send);
void send_fuel(uint8_t fuel, uint32_t delay_before_send);
void send_lights(uint8_t mainLights, uint8_t highBeam, uint8_t rearFogLight,
		uint8_t frontFogLight, uint32_t delay_before_send);
void send_button_pressed(uint32_t delay_before_send);
void send_button_released(uint32_t delay_before_send);
void send_backlight_brightness(uint8_t brightness, uint32_t delay_before_send);
void send_acc(uint32_t delay_before_send);

#endif /* INC_MINI_COOPER_H_ */
