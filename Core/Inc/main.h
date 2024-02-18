/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/**
 * @struct	REG
 * @brief	Registersatz des Controllers.
 *
 * @note	Der Registersatz wird im RAM und im EEProm gehalten
 */
 typedef struct
 {
/**
 * @var	unsigned int sw_release
 * @brief	Register mit der Softwareversion
 * @see	__SW_RELEASE__
 * @see	SW_REL_REG
 * @see	config.h
 */
 uint16_t		sw_release;

/**
 * @var	unsigned int sw_release_date
 * @brief	Register mit dem Datum der Softwareversion
 * Formatierung:
 *	- Byte 0 -> Tag
 *	- BYTE 1 -> Monat
 *	- BYTE 2 -> Jahr
 *	- BYTE 3 -> Jahr
 * @see	__SW_RELEASE_DATE__
 * @see	SW_REL_DATE_REG
 * @see	config.h
 */
 uint32_t		sw_release_date;

 uint64_t		sw_git_short_hash;

 const char	sw_git_tag[16];
}_SW_INFO_REGS;



/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI1_DATA_STROBE_Pin GPIO_PIN_0
#define SPI1_DATA_STROBE_GPIO_Port GPIOB
#define SPI1_OE_Pin GPIO_PIN_1
#define SPI1_OE_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_12
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_13
#define LED_RED_GPIO_Port GPIOB
#define LED_BLUE_Pin GPIO_PIN_14
#define LED_BLUE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */


#define __DEV_SIGNATURE__			0x12
#define __SW_RELEASE__				0x0100
#define SW_RELEASE_DAY				18
#define SW_RELEASE_MONTH			02
#define SW_RELEASE_YEAR				2024
#define __SW_RELEASE_DATE__			((SW_RELEASE_DAY<<24 ) | (SW_RELEASE_MONTH<<16) | SW_RELEASE_YEAR)



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
