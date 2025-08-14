/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "mini_cooper.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_MSG_TIME_SPACING 5
#define OLED_REFRESH_PERIOD 250
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FDCAN_RxHeaderTypeDef rxHeader;
uint8_t rxData[8];

FDCAN_RxHeaderTypeDef rxHeaderTemperature;
uint8_t rxDataTemperature[8];
float temperature_ntc = 0;

extern FDCAN_TxHeaderTypeDef TxHeader;

uint32_t SoftwareTimer100;
uint32_t SoftwareTimer1000;
uint32_t OledSoftwareTimer;

char lcd_line[32];

extern uint8_t rolling_counter;  // alive counter (0 - 14)

volatile uint8_t green_button_pressed = 0;
volatile uint8_t crc8_error = 0;
volatile uint8_t crc8_actual = 0;
volatile uint8_t crc8_appended = 0;
volatile uint8_t counter_prev = 0;
volatile uint8_t counter_error = 0;
volatile uint8_t error_flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_FDCAN1_Init();
	MX_I2C3_Init();
	MX_ADC1_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */
	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 0);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(12, 12);
	ssd1306_WriteString("MINI Cooper (F56)", Font_6x8, White);
	ssd1306_SetCursor(12, 22);
	ssd1306_WriteString("John Cooper Works", Font_6x8, White);
	ssd1306_SetCursor(8, 32);
	ssd1306_WriteString("instrument cluster", Font_6x8, White);
	ssd1306_UpdateScreen();

	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
	FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	// air quality sensor
	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
			0) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	// temperature sensor
	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
			0) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	SoftwareTimer100 = HAL_GetTick();
	SoftwareTimer1000 = HAL_GetTick();
	OledSoftwareTimer = HAL_GetTick();

	GenerateCooperCrc8Table();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{

		if (HAL_GetTick() - SoftwareTimer100 > 100)
		{
			SoftwareTimer100 = HAL_GetTick();
			send_ignition_on(CAN_MSG_TIME_SPACING);
			send_speed(200, CAN_MSG_TIME_SPACING);
			send_rpm(6000, 6, CAN_MSG_TIME_SPACING);
			send_fuel(95, CAN_MSG_TIME_SPACING);
			send_acc(CAN_MSG_TIME_SPACING);

			rolling_counter++;
			rolling_counter %= 0x0F;
		}

		if (HAL_GetTick() - SoftwareTimer1000 > 1000)
		{
			SoftwareTimer1000 = HAL_GetTick();
			send_backlight_brightness(80, CAN_MSG_TIME_SPACING);
			send_lights(1, 1, 1, 1, CAN_MSG_TIME_SPACING);
			send_button_released(CAN_MSG_TIME_SPACING);
		}

		if (green_button_pressed == 1)
		{
			green_button_pressed = 0;
			send_blinkers(1, 1, CAN_MSG_TIME_SPACING);
			send_button_pressed(CAN_MSG_TIME_SPACING);
		}

		if (HAL_GetTick() - OledSoftwareTimer > OLED_REFRESH_PERIOD)
		{
			OledSoftwareTimer = HAL_GetTick();

			ssd1306_SetCursor(2, 46);
			sprintf(lcd_line, "T_amb = %.1f deg. C  ", temperature_ntc);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_SetCursor(2, 56);
			if (error_flag == 1)
			{

				sprintf(lcd_line, "CRC8: %s, RCNT: %s    ",
						crc8_error ? "ERR" : " OK",
						counter_error ? "ERR" : " OK");

				error_flag = 0;
			}
			else
			{
				sprintf(lcd_line, "CRC8:  OK, RCNT:  OK    ");
			}
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_UpdateScreen();
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GREEN_BUTTON_Pin)
	{
		green_button_pressed = 1;
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	}
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retreive Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData)
				!= HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}
		else
		{
			HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);

			if (error_flag == 0)
			{
				crc8_actual = GetCooperCrc8(rxData + 1, 6, 0b11001010);
				crc8_appended = rxData[0];
				crc8_error = (crc8_actual != crc8_appended);
				counter_error = (((rxData[1]) & 0x0F)
						!= ((counter_prev == 0x0E) ?
								0x00 : (counter_prev + 0x01)));
			}
			if (crc8_error || counter_error)
			{
				error_flag = 1;
			}
			counter_prev = (rxData[1]) & 0x0F;
		}

		if (HAL_FDCAN_ActivateNotification(hfdcan,
		FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
	{
		/* Retreive Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rxHeaderTemperature,
				rxDataTemperature) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}
		else if (rxDataTemperature[1] == 0x00) // valid temperature data?
		{
			temperature_ntc = (float) (rxDataTemperature[3]) / 2.0f - 40.0f; // = IPC displayed value (for sure different NTC)
			HAL_GPIO_TogglePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin);
		}
		else
		{
			__NOP(); // some other data?
		}

		if (HAL_FDCAN_ActivateNotification(hfdcan,
		FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
