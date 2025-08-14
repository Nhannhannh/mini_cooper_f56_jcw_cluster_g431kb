/*
 * mini_cooper.c
 *
 *  Created on: Aug 11, 2025
 *      Author: user
 */

#include "main.h"
#include "mini_cooper.h"

// Visit https://github.com/r00li/CarCluster for more CAN frames!

uint8_t crc8Table[256];

FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData8[8];
uint8_t TxData6[6];
uint8_t TxData5[5];
uint8_t TxData3[3];
uint8_t TxData2[2];

uint8_t rolling_counter = 0;

void GenerateCooperCrc8Table(void)
{
	CrcCooper_t remainder;
	for (uint16_t dividend = 0; dividend < 256; ++dividend)
	{
		remainder = dividend;

		for (uint8_t bit = 8; bit > 0; --bit)
		{
			if (remainder & 0b10000000)
			{
				remainder = (remainder << 1) ^ CRC8_POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
		crc8Table[dividend] = remainder;
	}
}

CrcCooper_t GetCooperCrc8(uint8_t *_message, uint8_t _length,
		uint8_t _final_xor)
{
	uint8_t data;
	CrcCooper_t remainder = 0xFF;

	for (uint8_t byte = 0; byte < _length; ++byte)
	{
		data = _message[byte] ^ remainder;
		remainder = crc8Table[data] ^ (remainder << 8);
	}
	remainder = remainder ^ _final_xor;

	return (remainder);
}

int32_t map_int(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min,
		int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void send_ignition_on(uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	TxHeader.Identifier = 0x12F;
	memset(TxData8, 0, sizeof(TxData8));

	TxData8[1] = 0x80 | rolling_counter;
	TxData8[2] = 0x8A;
	TxData8[3] = 0xDD;
	TxData8[4] = 0xF1;
	TxData8[5] = 0x01;
	TxData8[6] = 0x30;
	TxData8[7] = 0x06;

	TxData8[0] = GetCooperCrc8(TxData8 + 1, 7, 0x44);

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData8) != HAL_OK)
	{
		Error_Handler();
	}

}

void send_speed(uint16_t speed, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_5;
	TxHeader.Identifier = 0x1A1;
	memset(TxData5, 0, sizeof(TxData5));
	uint16_t calculatedSpeed = (double) speed * 64.01;

	TxData5[1] = 0xC0 | rolling_counter;
	TxData5[2] = calculatedSpeed & 0x00FF;
	TxData5[3] = (calculatedSpeed >> 8) & 0x00FF;
	TxData5[4] = (speed == 0 ? 0x81 : 0x91);

	TxData5[0] = GetCooperCrc8(TxData5 + 1, 4, 0xA9);

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData5) != HAL_OK)
	{
		Error_Handler();
	}

}

void send_rpm(uint16_t rpm, uint8_t gear, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	TxHeader.Identifier = 0x0F3;
	memset(TxData8, 0, sizeof(TxData8));

	uint8_t calculatedGear = 0;
	switch (gear)
	{
	case 0:
		calculatedGear = 0;
		break; // Empty
	case 1 ... 9:
		calculatedGear = gear + 4;
		break; // 1-9
	case 11:
		calculatedGear = 2;
		break; // Reverse
	case 12:
		calculatedGear = 1;
		break; // Neutral
	}

	TxData8[1] = 0x60 | rolling_counter;
	TxData8[2] = map_int(rpm, 0, 6900, 0x00, 0x2B); // rpm
	TxData8[3] = 0xC0;
	TxData8[4] = 0xF0;
	TxData8[5] = calculatedGear;
	TxData8[6] = 0xFF;
	TxData8[7] = 0xFF;

	TxData8[0] = GetCooperCrc8(TxData8 + 1, 7, 0x7A);

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData8) != HAL_OK)
	{
		Error_Handler();
	}
}

void send_blinkers(uint8_t left, uint8_t right, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	TxHeader.Identifier = 0x1F6;
	memset(TxData2, 0, sizeof(TxData2));
	TxData2[0] =
			(left == 0 && right == 0) ? 0x80 : (0x81 | left << 4 | right << 5);
	TxData2[1] = 0xF0;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData2) != HAL_OK)
	{
		Error_Handler();
	}
}

void send_backlight_brightness(uint8_t brightness, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	TxHeader.Identifier = 0x202;
	memset(TxData2, 0, sizeof(TxData2));

	TxData2[0] = map_int(brightness, 0, 100, 0, 253);
	TxData2[1] = 0xFF;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData2) != HAL_OK)
	{
		Error_Handler();
	}
}

void send_fuel(uint8_t fuel, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_5;
	TxHeader.Identifier = 0x349;
	memset(TxData5, 0, sizeof(TxData5));

	TxData5[0] = 0x00;
	TxData5[1] = 0x00;
	TxData5[2] = 0x00;
	TxData5[3] = map_int(fuel, 0, 100, 22, 3);
	TxData5[4] = 0x00;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData5) != HAL_OK)
	{
		Error_Handler();
	}
}

void send_lights(uint8_t mainLights, uint8_t highBeam, uint8_t rearFogLight,
		uint8_t frontFogLight, uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_3;
	TxHeader.Identifier = 0x21A;
	memset(TxData3, 0, sizeof(TxData3));
	uint8_t lightStatus = highBeam << 1 | mainLights << 2 | frontFogLight << 5
			| rearFogLight << 6;

	TxData3[0] = lightStatus;
	TxData3[1] = 0xC0;
	TxData3[2] = 0xF7;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData3) != HAL_OK)
	{
		Error_Handler();
	}
}

// Works only for zero speed - safety :)
void send_button_pressed(uint32_t delay_before_send)
{

	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	TxHeader.Identifier = 0x1EE;
	memset(TxData2, 0, sizeof(TxData2));
	TxData2[0] = 0x40;
	TxData2[1] = 0xFF;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData2) != HAL_OK)
	{
		Error_Handler();
	}
}

void send_button_released(uint32_t delay_before_send)
{
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	TxHeader.Identifier = 0x1EE;
	memset(TxData2, 0, sizeof(TxData2));

	TxData2[0] = 0x00;
	TxData2[1] = 0xFF;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData2) != HAL_OK)
	{
		Error_Handler();
	}
}

// Adaptive speed control --> required to activate fuel level gauge
void send_acc(uint32_t delay_before_send)
{
	static uint8_t _rolling_counter = 0;
	HAL_Delay(delay_before_send);

	TxHeader.DataLength = FDCAN_DLC_BYTES_6;
	TxHeader.Identifier = 0x33B;
	memset(TxData6, 0, sizeof(TxData6));

	TxData6[1] = 0xF0 | _rolling_counter;
	TxData6[2] = 0x5C;
	TxData6[3] = 0x70;
	TxData6[4] = 0x00;
	TxData6[5] = 0x00;

	TxData6[0] = GetCooperCrc8(TxData6 + 1, 5, 0x6B);

	_rolling_counter += 4;
	_rolling_counter %= 0x0F;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData6) != HAL_OK)
	{
		Error_Handler();
	}
}
