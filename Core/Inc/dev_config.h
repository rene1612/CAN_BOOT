/**
  ******************************************************************************
  * STM32 CAN Bootloader
  * This is inspired by : https://github.com/akospasztor/stm32-bootloader
  ******************************************************************************
  * @authors René Schönrock
  * @initial file   bootloader.h
  * @file 	dev_config.h
  * @brief  Bootloader header
  *	        This file contains the bootloader configuration parameters,
  *	        function prototypes and other required macros and definitions.
  * @see    Please refer to README for detailed information.
  ******************************************************************************
  * Copyright (c) 2024 René Schönrock. (rene.schoenrock@online.de)
  *
  ******************************************************************************
**/

#ifndef __DEV_CONFIG_H
#define __DEV_CONFIG_H

//#include <stdint.h>

/*** Bootloader Configuration *************************************************/
#if defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)

#define SYSMEM_ADDRESS			(uint32_t)0x1FFFF000    /* Address of System Memory (ST Bootloader) */

/* MCU RAM size, used for checking accurately whether flash contains valid application */
#define SRAM_SIZE				(uint32_t)0x00005000	// 20 kB
#define FLASH_SIZE				(uint32_t)0x00020000	// 128kB

#define DEV_CONFIG_FL_ADDRESS	(uint32_t)0x0801F800	/* Start address of config */
#define DEV_CONFIG_FL_SIZE		(uint32_t)0x00000400	/* Lenght of the bootloader (1KB) */

#define _MAGIC_RAM_ADDRESS_		(uint32_t)(SRAM_BASE + SRAM_SIZE - 4)	/* last Address of RAM as magic marker */
#define _MAGIC_RAM_DWORD_		0x12345678	/* Lenght of the bootloader (1KB) */

#else
#error"WRONG MCU for project"
#endif



/******************************************************************************/
/* Defines -------------------------------------------------------------------*/
#define DEFAULT_ALIVE_TIMEOUT_10MS	15
#define DEAULT_CAN_BITRATE			500000UL

#define __DEV_ID__					0xFF
#define __BOARD_VERSION__			0x0100
#define BOARD_MF_DAY				8
#define BOARD_MF_MONTH				12
#define BOARD_MF_YEAR				2023
#define __BOARD_MF_DATE__			((BOARD_MF_DAY<<24 ) | (BOARD_MF_MONTH<<18) | BOARD_MF_YEAR)


typedef enum
{
	BMS_BLK_BOARD					= 0x10,
	BMS_MEASURE_BOARD				= 0x20,
}_BOARD_TYPE;


#pragma pack(push,1)

/**
 * @struct	REG
 * @brief	Dev-Config stuct.
 *
 * @note	Der Registersatz wird im Flash gehalten
 */
 typedef struct
 {
	 /**
	  * @var	uint8_t dev_id
	  * @brief	Register mit der Gerätekennung
	  * @see	__DEV_ID__
	  * @see	DEV_ID_REG
	  * @see	config.h
	  */
	uint8_t			dev_id;

	_BOARD_TYPE		board_type;

	uint16_t		board_version;

	/**
	* @var	uint32_t		can_bitrate
	* @brief
	*/
	uint32_t		can_bitrate;

	/**
	* @var	uint32t sw_release_date
	* @brief	Register mit dem Datum der Softwareversion
	* Formatierung:
	*	- Byte 0 -> Tag
	*	- BYTE 1 -> Monat
	*	- BYTE 2 -> Jahr
	*	- BYTE 3 -> Jahr
	* @see	__BOARD_MF_DATE__
	* @see	SW_REL_DATE_REG
	* @see	config.h
	*/
	uint32_t		board_mf_date;
 }_DEV_CONFIG_REGS;

#pragma pack(pop)


/* Functions -----------------------------------------------------------------*/

#endif /* __DEV_CONFIG_H */
