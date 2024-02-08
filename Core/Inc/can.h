/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "dev_config.h"

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

typedef enum
{
	BL_XXX_CMD						= 0xF0,
	BL_erease_FLASH_CMD				= 0xF1,
	BL_run_APP_CMD 					= 0xF2,
	BL_ask_for_FLASH_space_CMD		= 0xF3,
	BL_ask_for_CRC_CMD				= 0xF4,
	BL_start_FLASH_CMD				= 0xF5,
	BL_next_FLASH_DATA_CMD			= 0xF6,
	BL_done_FLASH_CMD				= 0xF7,
	BL_disable_RW_PROTECTION_CMD	= 0xF8,
	BL_enable_RW_PROTECTION_CMD		= 0xF9,
	BL_start_RD_FLASH_CMD			= 0xFA,
	BL_next_RD_FLASH_DATA_CMD		= 0xFB,
	BL_done_RD_FLASH_CMD			= 0xFC,
	BL_validate_FLASH_CMD			= 0xFD
}_CAN_BL_CMD_TYPE;

#define BMS_BLK_CAN_ID		0x488

#define CANRX_SA 0x01
#define CANTX_SA 0x02
#define CANTX_HA 0x03

#define RX_CMD_CANID 0x00FF00
#define TX_HEARTBEAT_CANID 0x00FF00
#define TX_FEEDBACK_CANID 0x00FE00

#define RXFILTERMASK 0xFFFFFFFC
//#define RXFILTERID   0x0000FF00 + CANRX_SA



extern uint8_t				can_task_scheduler;


#define PROCESS_CAN_SEND_NEW_ADC_DATA 	0x01
#define PROCESS_CAN_ON_MSG				0x02
#define PROCESS_CAN_SEND_REPLAY			0x04

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
uint8_t		process_CAN(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

