/**
  ******************************************************************************
  * STM32 Bootloader
  * This is inspired by : https://github.com/akospasztor/stm32-bootloader
  ******************************************************************************
  * @authors Akos Pasztor, Francois Rainville
  * @initial file   bootloader.h
  * @file 	btld.h
  * @brief  Bootloader header
  *	        This file contains the bootloader configuration parameters,
  *	        function prototypes and other required macros and definitions.
  * @see    Please refer to README for detailed information.
  ******************************************************************************
  * Copyright (c) 2018 Akos Pasztor.                    https://akospasztor.com
  * Copyright (c) 2018 Fran�ois Rainville.
  ******************************************************************************
**/

#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include <stdint.h>
#include "dev_config.h"
#include "can.h"

/*** Bootloader Configuration *************************************************/
#define SET_VECTOR_TABLE        1       /* Automatically set vector table location before launching application */
#define CLEAR_RESET_FLAGS       1       /* If enabled: bootloader clears reset flags. (This occurs only when OBL RST flag is active.)
                                           If disabled: bootloader does not clear reset flags, not even when OBL RST is active. */
#define BL_ADDRESS			(uint32_t)0x08000000	/* Start address of bootloader */
#define BL_SIZE				(uint32_t)0x00008000	/* Lenght of the bootloader (32KB) */

#define APP_ADDRESS			(uint32_t)0x08008000    /* Start address of application space in flash */
#define APP_END_ADDRESS		(uint32_t)0x0801EFFF    /* End address of application space (addr. of last byte) */
#define APP_SIZE			(uint32_t)(((APP_END_ADDRESS - APP_ADDRESS) + 1)) /* Size of application in DWORD (32bits or 4bytes) */

#define DEV_CRC_FL_ADDRESS	(uint32_t)0x0801FC00	/* Start address of crc flash area */
#define DEV_CRC_FL_SIZE		(uint32_t)0x00000400	/* Lenght of the crc flash area (1KB) */

#define APP_CONFIG_FL_ADDRESS	(uint32_t)0x0801F000	/* Start address of app-config */
#define APP_CONFIG_SIZE			(uint32_t)0x00000800	/* Lenght of the app-config (2KB) */


/******************************************************************************/
/* Defines -------------------------------------------------------------------*/
#define FLASH_PAGE_NBPERBANK    64             /* Number of pages per bank in flash */
//#define APP_SIZE	(uint32_t)(((END_ADDRESS - APP_ADDRESS) + 3) / 4) /* Size of application in DWORD (32bits or 4bytes) */

/* Bootloader Error Codes */
enum
{
    BL_OK = 0,
    BL_NO_APP,
    BL_SIZE_ERROR,
    BL_CHKS_ERROR,
    BL_ERASE_ERROR,
    BL_WRITE_ERROR,
    BL_OBP_ERROR
};

/* Flash Protection Types */
enum
{
    BL_PROTECTION_NONE  = 0,
    BL_PROTECTION_WRP   = 0x1,
    BL_PROTECTION_RDP   = 0x2,
    BL_PROTECTION_PCROP = 0x4
};

typedef enum
{
	BOOTLOADER			= 0x10,
	APPLICATION			= 0x20,
	APPLICATION_CONFIG 	= 0x21,
	DEVICE_CONFIG		= 0x30,
	BL_DEV_CRC			= 0x40
}_FLASH_AREA_TYPE;

/**
 * @struct	REG
 * @brief	Bootloader CTRL stuct.
 *
 * @note	Der Registersatz wird im RAM gehalten
 */
 typedef struct
 {
	/**
	* @var	_CAN_BL_CMD_TYPE		bl_can_cmd
	* @brief
	*/
	 _CAN_BL_CMD_TYPE		loader_cmd;

	/**
	* @var	_FLASH_AREA_TYPE		bl_flash_area
	* @brief
	*/
	 _FLASH_AREA_TYPE  		flash_area;
	 uint32_t 				G_uint32_to_write;
	 uint32_t 				G_index;

	 uint8_t 				loader_mode;
	 uint8_t 				FlashInProgress;

}_BL_CTRL_REGS_TYPE;


/**
 * @struct	REG
 * @brief	Dev-Config stuct.
 *
 * @note	Der Registersatz wird im Flash gehalten
 */
 typedef struct
 {
	/**
	* @var	uint32_t		bl_crc32
	* @brief
	*/
	uint32_t		bl_crc32;

	/**
	* @var	uint32_t		bl_length
	* @brief
	*/
	uint32_t		bl_length;

	/**
	* @var	uint32_t		app_crc32
	* @brief
	*/
	uint32_t		app_crc32;

	/**
	* @var	uint32_t		app_length
	* @brief
	*/
	uint32_t		app_length;

	/**
	* @var	uint32_t		dev_config_crc32
	* @brief
	*/
	uint32_t		dev_config_crc32;

	/**
	* @var	uint32_t		dev_config_length
	* @brief
	*/
	uint32_t		dev_config_length;

	/**
	* @var	uint32_t		dev_config_crc32
	* @brief
	*/
	uint32_t		app_config_crc32;

	/**
	* @var	uint32_t		dev_config_length
	* @brief
	*/
	uint32_t		app_config_length;

	uint8_t			bl_fl_flags;
	uint8_t			app_fl_flags;
	uint8_t			dev_config_fl_flags;
	uint8_t			app_config_fl_flags;
}_DEV_CRC_REGS;


/* Functions -----------------------------------------------------------------*/
uint32_t crc32(const void *buf, uint32_t size);
void    Bootloader_Init(void);
uint8_t btld_EraseFlash(void);

void    btld_FlashBegin(void);
uint8_t btld_FlashNext_32(uint32_t data,uint32_t* index);
uint8_t btld_FlashNext(uint64_t data);
void    btld_FlashEnd(void);

uint8_t btld_GetProtectionStatus(void);
uint8_t btld_ConfigProtection(uint32_t protection);

uint8_t btld_CheckSize(uint32_t appsize);
uint32_t btld_GetBootChecksum(void);
uint32_t btld_GetChecksum(uint32_t Length);
uint8_t btld_SaveBlChecksum(void);
uint8_t btld_SaveAppChecksum(void);
uint8_t btld_SaveAppLength(void);
uint32_t btld_GetAppLength(void);
uint32_t btld_GetAppCRC(void);
uint8_t btld_CheckForApplication(void);

void    btld_JumpToApp(void);
void    btld_JumpToSysMem(void);

#endif /* __BOOTLOADER_H */
