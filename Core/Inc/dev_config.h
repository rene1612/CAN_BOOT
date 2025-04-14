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

	#define SYSMEM_ADDRESS				(uint32_t)0x1FFFF000UL    /* Address of System Memory (ST Bootloader) */

	/* MCU RAM size, used for checking accurately whether flash contains valid application */
	#define SRAM_SIZE					(uint32_t)0x00005000UL	// 20 kB
	#define FLASH_SIZE					(uint32_t)0x00020000UL	// 128kB

	#define _MAGIC_RAM_ADDRESS_			(uint32_t)(SRAM_BASE + SRAM_SIZE - 4)	/* last Address of RAM as magic marker */
	#define _MAGIC_RAM_DWORD_			0x12345678	/* the magic DWORD */

	#define DEV_BL_ADDRESS				(uint32_t)0x08000000UL	/* Start address of bootloader */
	#define DEV_BL_SIZE					(uint32_t)0x00008000UL	/* Lenght of the bootloader (32KB) */

	#define DEV_APP_ADDRESS				(uint32_t)0x08008000UL    /* Start address of application space in flash */
	//#define DEV_APP_END_ADDRESS			(uint32_t)0x0801EFFF    /* End address of application space (addr. of last byte) */
	//#define DEV_APP_SIZE				(uint32_t)(((DEV_APP_END_ADDRESS - DEV_APP_ADDRESS) + 1)) /* Size of application in DWORD (32bits or 4bytes) */
	#define DEV_APP_SIZE				(uint32_t)0x00017000UL 	/* Size of application in DWORD (32bits or 4bytes) */

	#define DEV_APP_CONFIG_FL_ADDRESS	(uint32_t)0x0801F000UL	/* Start address of app-config */
	#define DEV_APP_CONFIG_SIZE			(uint32_t)0x00000800UL	/* Lenght of the app-config (2KB) */

	#define DEV_CONFIG_FL_ADDRESS		(uint32_t)0x0801F800UL	/* Start address of config */
	#define DEV_CONFIG_FL_SIZE			(uint32_t)0x00000400UL	/* Lenght of the bootloader (1KB) */

	#define DEV_CRC_FL_ADDRESS			(uint32_t)0x0801FC00UL	/* Start address of crc flash area */
	#define DEV_CRC_FL_SIZE				(uint32_t)0x00000400UL	/* Lenght of the crc flash area (1KB) */

#else
	#error"WRONG MCU for project"
#endif


typedef enum
{
	BMS_BLK_BOARD					= 0x10,
	BMS_MEASURE_BOARD				= 0x20,
	BMS_BALANCE_BOARD				= 0x30,
	BMS_PECHARGE_BOARD				= 0x40,
}_BOARD_TYPE;


typedef enum
{
	_1000_Kbit					= 9,
	_500_Kbit					= 8,
	_250_Kbit					= 36,
	_125_Kbit					= 72,
	_50_Kbit					= 180,
	_20_Kbit					= 450,
	_10_Kbit					= 900,
}_CAN_BIT_RATE;


typedef enum
{
	ACK = 0x11,
	NACK= 0x13
}_REPLAY_TYPE;


typedef enum
{
	NO_CMD = 0,
	\
	/* SYSK-COMMANDS (Boardtype: ALL) */
	SYS_RESET_CMD,
	SYS_APP_RESET_CMD,
	SYS_BOOT_CMD,
	\
	ALIVE_CMD=6,
	\
	SYS_WRITE_REG_CMD,
	SYS_READ_REG_CMD,
	\
	REPLAY_AKC_NACK_CMD=0x11,
	REPLAY_DATA_CMD=0x13,
	\
	/* BMS-BLK-COMMANDS (Boardtype: BMS_BLK_BOARD) */
	PB_SET_CMD = 0x20,
	PB_SET_OE_CMD,
	NEEY_SET_CMD,
	NEEY_GET_INFO_CMD,
	BMS_GET_CELL_DATA_CMD,
	BMS_GET_BLK_DATA_CMD,
	BMS_GET_NEEY_DATA_CMD,
	BMS_GET_ALL_DATA_CMD,
	WS2815_SET_CMD,
	WS2815_ENABLE_CMD,
	\
	/* BMS-MEASURE-COMMANDS (Boardtype: BMS_MEASURE_BOARD) */
	ADC_OFFSET_CAL_CMD=0x40,
	ADC_GAIN_CAL_CMD,
	ADC_READ_REG_CMD,
	ADC_WRITE_REG_CMD,
	ADC_OFFSET_READ_CMD,
	ADC_GAIN_READ_CMD,
	BMSM_GET_ALL_DATA_CMD=0x48,
	BMSM_GET_SINGLE_DATA_CMD,
	BMSM_GET_VOLTAGE_DATA_CMD,
	BMSM_GET_CURRENT_DATA_CMD,
	\
	SET_RELAY_CMD=0x50,
	GET_RELAY_CMD,
	\
	/* BMS-PRECHARGE-COMMANDS (Boardtype: BMS_PRECHARGE_BOARD) */
	BMSP_SET_CMD=0x60,
	BMSP_PC_DONE_REPLAY,
	\
	SYS_ALERT_MSG=0xAA,
	SYS_WARN_MSG=0xAB,
	\
	END_CMD
}_CAN_CMD;

typedef enum
{
	NONE				=0x00,
	GET_MAIN_REGS		=0x00,
	GET_SW_INFO_REGS	=0x01,
	GET_DEV_CFG_REGS	=0x02,
	GET_BRD_INFO_REGS	=0x04,
	END
}_CAN_GET_SYS_REG_TYPE;

/******************************************************************************/
/* Defines -------------------------------------------------------------------*/
#define DEFAULT_ALIVE_TIMEOUT_10MS	15
#define DEAULT_BL_CAN_BITRATE		(_CAN_BIT_RATE)_500_Kbit
#define DEAULT_APP_CAN_BITRATE		(_CAN_BIT_RATE)_500_Kbit

#define DEAULT_BOARD_NAME_SIZE		20


#define DEAULT_TRIPP_CAN_ID			(BMS_MEASURE_BOARD + 1)
#define DEAULT_BROADCAST_CAN_ID		0x7F

//#define __BOARD_NAME__ "BMS_MEASURE_BOARD"

//#ifndef __BOARD_NAME__
//	#if (__BOARD_TYPE__) == (BMS_BLK_BOARD)
//		#define __BOARD_NAME__  "BMS_BLK_BOARD"
//	#else
//		#if (__BOARD_TYPE__) == (BMS_MEASURE_BOARD)
//			#define __BOARD_NAME__  "BMS_MEASURE_BOARD"
//		#else
//			#if (__BOARD_TYPE__) == BMS_BALANCE_BOARD
//				#define __BOARD_NAME__  "BMS_BALANCE_BOARD"
//			#else
//				#define __BOARD_NAME__  "NO_NAME"
//			#endif
//		#endif
//	#endif
//#endif

#ifndef __BOARD_MF_DATE__
	#define BOARD_MF_DAY				7
	#define BOARD_MF_MONTH				9
	#define BOARD_MF_YEAR				2023
	#define __BOARD_MF_DATE__			((BOARD_MF_DAY<<24 ) | (BOARD_MF_MONTH<<16) | BOARD_MF_YEAR)
#endif


#pragma pack(push,1)

/**
 * @struct	_GIT_INFO_STRUCT
 * @brief	Git Informationen aus dem letzen Commit (Head).
 *
 * @note
 * @see		gitcommit.h
 */
 typedef struct
 {
	 uint32_t		short_hash;
	 uint32_t		unix_time_stamp;
	 const char 	date_str[12];
	 const char 	info_tag_branch[64];
 }_GIT_INFO_STRUCT;


 /**
  * @struct	_BOARD_INFO_STRUCT
  * @brief	boarname
  *
  * @note
  * @see		BOARD_TYPE
  */
  typedef struct
  {
 	 const char 	board_name[DEAULT_BOARD_NAME_SIZE];
  }_BOARD_INFO_STRUCT;

/**
 * @struct	REG
 * @brief	Registersatz des Controllers.
 *
 * @note	Der Registersatz wird im RAM und im EEProm gehalten
 */
 typedef struct
 {
  const char 		sw_name[20];
  const char		sw_configuration[10];	//DEBUG/RELEASE

/**
 * @var	uint16_t  sw_release
 * @brief	Register mit der Softwareversion
 * @see	__SW_RELEASE__
 * @see	SW_REL_REG
 */
 uint16_t			sw_release;

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
 uint32_t			sw_release_date;

 _GIT_INFO_STRUCT	git;

 }_SW_INFO_REGS;

/**
 * @struct	_DEV_CONFIG_REGS
 * @brief	Dev-Config stuct.
 *
 * @note	Der Registersatz wird im Flash (0x801F800) gehalten
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

	const char 		board_name[20];

	uint16_t		board_version;

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

	/**
	* @var	uint32_t	can_bitrate
	* @brief			CAN-Bitrate des Controllers (default 500000 Bit/s)
	*/
	_CAN_BIT_RATE	bl_can_bitrate;

	/**
	* @var	uint32_t	can_bitrate
	* @brief			CAN-Bitrate des Controllers (default 500000 Bit/s)
	*/
	_CAN_BIT_RATE	app_can_bitrate;


	uint16_t		can_trip_id;

	uint16_t		can_broadcast_id;


 }_DEV_CONFIG_REGS;

#pragma pack(pop)


/* Functions -----------------------------------------------------------------*/

#endif /* __DEV_CONFIG_H */
