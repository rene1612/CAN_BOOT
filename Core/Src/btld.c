/**
  ******************************************************************************
  * STM32 Bootloader
  * This is inspired by : https://github.com/akospasztor/stm32-bootloader
  ******************************************************************************
  * @authors Akos Pasztor, Francois Rainville
  * @initial file   bootloader.c
  * @file 	btld.c
  * @brief  Bootloader implementation
  *	        This file contains the functions of the bootloader. The bootloader 
  *	        implementation uses the official HAL library of ST.
  * @see    Please refer to README for detailed information.
  ******************************************************************************
  * Copyright (c) 2018 Akos Pasztor.                    https://akospasztor.com
  * Copyright (c) 2018 Fran�ois Rainville.
  ******************************************************************************
**/

#include <btld.h>
#include "stm32f1xx_hal.h"
#include <string.h>

extern CRC_HandleTypeDef hcrc;


/* Private typedef -----------------------------------------------------------*/
typedef void (*pFunction)(void);

/* Private variables ---------------------------------------------------------*/
//static uint32_t flash_ptr = APP_ADDRESS;

extern const _DEV_CRC_REGS* pDevCRCRegs;

extern _BL_CTRL_REGS_TYPE bl_ctrl_reg;

const uint32_t crc32_tab[] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
  };
/*
 * A function that calculates the CRC-32 based on the table above is
 * given below for documentation purposes. An equivalent implementation
 * of this function that's actually used in the kernel can be found
 * in sys/libkern.h, where it can be inlined.
 */
uint32_t crc32(const void *buf, uint32_t size)
{
#define M1 0xffffff
#define M2 0xffffff00
	const uint8_t *p = buf;
	uint32_t crc;

	crc = ~0U;

  while(size--) {
    crc=((crc<<8)&M2)^crc32_tab[((crc>>24)&0xff)^*p++];
  }

  return(crc);
}


/* Initialize bootloader and flash -------------------------------------------*/
void Bootloader_Init(void){
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    //__HAL_RCC_FLASH_CLK_ENABLE();

    /* Clear flash flags */
    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

    HAL_FLASH_Lock();
}


/* Erase flash ---------------------------------------------------------------*/
uint8_t btld_EraseFlash(_FLASH_AREA_TYPE flash_area){
    //uint32_t NbrOfPages = 0;
    uint32_t PageError  = 0;
    FLASH_EraseInitTypeDef  pEraseInit;
    HAL_StatusTypeDef       status = HAL_OK;

	switch(flash_area) {
		case BOOTLOADER:
			if( pDevCRCRegs->bl_fl_flags & BL_PROTECTION_WRP ) {
				status = BL_WRITE_ERROR;
			}else {
				pEraseInit.NbPages = DEV_BL_SIZE / FLASH_PAGE_SIZE;
		        pEraseInit.PageAddress = DEV_BL_ADDRESS; //FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
			}
			break;

		case APPLICATION:
			if( pDevCRCRegs->app_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				pEraseInit.NbPages = DEV_APP_SIZE / FLASH_PAGE_SIZE;
		        pEraseInit.PageAddress = DEV_APP_ADDRESS; //FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
			}
			break;

		case APPLICATION_CONFIG:
			if( pDevCRCRegs->app_config_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				pEraseInit.NbPages = DEV_APP_CONFIG_SIZE / FLASH_PAGE_SIZE;
		        pEraseInit.PageAddress = DEV_APP_CONFIG_FL_ADDRESS; //FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
			}
			break;

		case DEVICE_CONFIG:
			if( pDevCRCRegs->dev_config_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				pEraseInit.NbPages = DEV_CONFIG_FL_SIZE / FLASH_PAGE_SIZE;
		        pEraseInit.PageAddress = DEV_CONFIG_FL_ADDRESS; //FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
			}
			break;

		case BL_DEV_CRC:
			pEraseInit.NbPages = DEV_CRC_FL_SIZE / FLASH_PAGE_SIZE;
			pEraseInit.PageAddress = DEV_CRC_FL_ADDRESS; //FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
			break;

		default:
			status = BL_ADDR_ERROR;
			break;
	}

    HAL_FLASH_Unlock();

    /* Get the number of pages to erase */

    if(status == HAL_OK)
    {
        //pEraseInit.Banks = FLASH_BANK_2;
        //pEraseInit.NbPages = NbrOfPages;
        //pEraseInit.Page = FLASH_PAGE_NBPERBANK - pEraseInit.NbPages;
        pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
        status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    }

    HAL_FLASH_Lock();
    
    return (status == HAL_OK) ? BL_OK : BL_ERASE_ERROR;
}


/* Flash Begin ---------------------------------------------------------------*/
uint8_t btld_FlashBegin(_FLASH_AREA_TYPE flash_area){
    /* Reset flash destination address */
    uint8_t       status = BL_OK;


    if (bl_ctrl_reg.FlashInProgress) {
    	return BL_ERROR;
    }
    else
    {
        bl_ctrl_reg.flash_ptr = 0x00000000;
        bl_ctrl_reg.FlashInProgress = 1;
    }

	switch(flash_area) {
		case BOOTLOADER:
			if( pDevCRCRegs->bl_fl_flags & BL_PROTECTION_WRP ) {
				status = BL_WRITE_ERROR;
			}else {
				bl_ctrl_reg.flash_ptr = DEV_BL_ADDRESS;
				bl_ctrl_reg.flash_start_addr = DEV_BL_ADDRESS;
				bl_ctrl_reg.flash_end_addr = DEV_BL_ADDRESS + DEV_BL_SIZE;
			}
			break;

		case APPLICATION:
			if( pDevCRCRegs->app_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				bl_ctrl_reg.flash_ptr = DEV_APP_ADDRESS;
				bl_ctrl_reg.flash_start_addr = DEV_APP_ADDRESS;
				bl_ctrl_reg.flash_end_addr = DEV_APP_ADDRESS + DEV_APP_SIZE;
			}
			break;

		case APPLICATION_CONFIG:
			if( pDevCRCRegs->app_config_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				bl_ctrl_reg.flash_ptr = DEV_APP_CONFIG_FL_ADDRESS;
				bl_ctrl_reg.flash_start_addr = DEV_APP_CONFIG_FL_ADDRESS;
				bl_ctrl_reg.flash_end_addr = DEV_APP_CONFIG_FL_ADDRESS + DEV_APP_CONFIG_SIZE;
			}
			break;

		case DEVICE_CONFIG:
			if( pDevCRCRegs->dev_config_fl_flags ) {
				status = BL_WRITE_ERROR;
			}else {
				bl_ctrl_reg.flash_ptr = DEV_CONFIG_FL_ADDRESS;
				bl_ctrl_reg.flash_start_addr = DEV_CONFIG_FL_ADDRESS;
				bl_ctrl_reg.flash_end_addr = DEV_CONFIG_FL_ADDRESS + DEV_CONFIG_FL_SIZE;
			}
			break;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			status = BL_ADDR_ERROR;
	        bl_ctrl_reg.FlashInProgress = 0;
			break;
	}

    /* Unlock flash */
    HAL_FLASH_Unlock();

	return status;
}


/* Flash Begin ---------------------------------------------------------------*/
uint32_t btld_GetMaxFlashSize(_FLASH_AREA_TYPE flash_area){

	switch(flash_area) {
		case BOOTLOADER:
			return (uint32_t)DEV_BL_SIZE;

		case APPLICATION:
			return (uint32_t)DEV_APP_SIZE;

		case APPLICATION_CONFIG:
			return (uint32_t)DEV_APP_CONFIG_SIZE;

		case DEVICE_CONFIG:
			return (uint32_t)DEV_CONFIG_FL_SIZE;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

	return 0;
}


/* Flash Begin ---------------------------------------------------------------*/
uint8_t btld_GeFlashFlags(_FLASH_AREA_TYPE flash_area){

	switch(flash_area) {
		case BOOTLOADER:
			return (uint8_t)pDevCRCRegs->bl_fl_flags;

		case APPLICATION:
			return (uint8_t)pDevCRCRegs->app_fl_flags;

		case APPLICATION_CONFIG:
			return (uint8_t)pDevCRCRegs->app_config_fl_flags;

		case DEVICE_CONFIG:
			return (uint8_t)pDevCRCRegs->dev_config_fl_flags;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

	return 0;
}


/* Program 32bit data into flash ---------------------------------------------*/
uint8_t btld_FlashNext_32(uint32_t data, uint32_t* index){


	if ( !bl_ctrl_reg.FlashInProgress )
    {
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    if( !(bl_ctrl_reg.flash_ptr <= (bl_ctrl_reg.flash_end_addr - 4)) || (bl_ctrl_reg.flash_ptr < bl_ctrl_reg.flash_start_addr) )
    {
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, bl_ctrl_reg.flash_ptr, (uint64_t) data) == HAL_OK)
    {
        /* Check the written value */
        if(*(uint32_t*)bl_ctrl_reg.flash_ptr != data)
        {
            /* Flash content doesn't match source content */
            HAL_FLASH_Lock();
            return BL_WRITE_ERROR;
        }
        /* Increment Flash destination address */
        bl_ctrl_reg.flash_ptr += 4;
        *index=(bl_ctrl_reg.flash_ptr - bl_ctrl_reg.flash_start_addr);
    }
    else
    {
        /* Error occurred while writing data into Flash */
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    return BL_OK;
}


/* Program 64bit data into flash ---------------------------------------------*/
uint8_t btld_FlashNext(uint64_t data){

	if ( !bl_ctrl_reg.FlashInProgress )
    {
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    if( !(bl_ctrl_reg.flash_ptr <= (bl_ctrl_reg.flash_end_addr - 8)) || (bl_ctrl_reg.flash_ptr < bl_ctrl_reg.flash_start_addr) )
    {
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }
    
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, bl_ctrl_reg.flash_ptr, data) == HAL_OK)
    {
        /* Check the written value */
        if(*(uint64_t*)bl_ctrl_reg.flash_ptr != data)
        {
            /* Flash content doesn't match source content */
            HAL_FLASH_Lock();
            return BL_WRITE_ERROR;
        }   
        /* Increment Flash destination address */
        bl_ctrl_reg.flash_ptr += 8;
    }
    else
    {
        /* Error occurred while writing data into Flash */
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }
    
    return BL_OK;
}


/* Finish flash programming --------------------------------------------------*/
uint8_t btld_FlashAppSize(void){
	HAL_StatusTypeDef returnedERR=HAL_OK;

    /* Unlock flash */
    HAL_FLASH_Unlock();

	returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->app_length, (uint64_t)(bl_ctrl_reg.flash_ptr - DEV_APP_ADDRESS));
    /* Lock flash */
    HAL_FLASH_Lock();

	return returnedERR;
}


/* Finish flash programming --------------------------------------------------*/
void btld_FlashEnd(void){

    /* Lock flash */
    HAL_FLASH_Lock();
}


/* Configure flash write protection ------------------------------------------------*/
uint8_t btld_ConfigProtection(_FLASH_AREA_TYPE flash_area, uint32_t protection){
    FLASH_OBProgramInitTypeDef  OBStruct = {0};
    HAL_StatusTypeDef           status = HAL_ERROR;
    
    status = HAL_FLASH_Unlock();
    status |= HAL_FLASH_OB_Unlock();

    /* Bank 1 */

    OBStruct.OptionType = OPTIONBYTE_RDP;

    if(protection & BL_PROTECTION_RDP)
    {
    	/* Enable RDP protection */
    	OBStruct.RDPLevel = OB_RDP_LEVEL_1;


    }
    else
    {
        /* Remove RDP protection */
    	OBStruct.RDPLevel = OB_RDP_LEVEL_0;

    }
    status |= HAL_FLASHEx_OBProgram(&OBStruct);



    if(status == HAL_OK)
    {
        /* Loading Flash Option Bytes - this generates a system reset. */ 
        HAL_FLASH_OB_Launch();
    }
    
    status |= HAL_FLASH_OB_Lock();
    status |= HAL_FLASH_Lock();

    return (status == HAL_OK) ? BL_OK : BL_OBP_ERROR;
}


/* Check if application fits into user flash ---------------------------------------*/
uint8_t btld_CheckForSize(_FLASH_AREA_TYPE flash_area, uint32_t size){

 	switch(flash_area) {

		case BOOTLOADER:
			return (DEV_BL_SIZE >= size) ? BL_OK : BL_SIZE_ERROR;

		case APPLICATION:
			return (DEV_APP_SIZE >= size) ? BL_OK : BL_SIZE_ERROR;

		case APPLICATION_CONFIG:
			return (DEV_APP_CONFIG_SIZE >= size) ? BL_OK : BL_SIZE_ERROR;

		case DEVICE_CONFIG:
			return (DEV_CONFIG_FL_SIZE >= size) ? BL_OK : BL_SIZE_ERROR;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

	return BL_SIZE_ERROR;
}


/* Verify checksum of bootloader ---------------------------------*/
uint8_t btld_ValidateFlArea(_FLASH_AREA_TYPE flash_area) {

	uint32_t fl_length = btld_GetFlLength(flash_area);
	uint32_t fl_addr;

	if(fl_length == 0) {
		return BL_ERROR;
	}else{
		switch (flash_area) {
			case BOOTLOADER:
				fl_addr= DEV_BL_ADDRESS;
				break;

			case APPLICATION:
				fl_addr= DEV_APP_ADDRESS;
				break;

			case APPLICATION_CONFIG:
				fl_addr= DEV_APP_CONFIG_FL_ADDRESS;
				break;

			case DEVICE_CONFIG:
				fl_addr= DEV_CONFIG_FL_ADDRESS;
				break;

			case BL_DEV_CRC:
				//flash_ptr = DEV_CRC_FL_ADDRESS;
				//break;
			default:
				return BL_ADDR_ERROR;

		}
		uint32_t fl_cs_calc=btld_CalcChecksum(fl_addr, fl_length);
		uint32_t fl_cs_fl=btld_GetFlChecksum(flash_area);
		if ( fl_cs_fl != fl_cs_calc ) {
			return BL_CHKS_ERROR;
		}
	}
	return BL_OK;
}


uint32_t btld_GetBootChecksum(void){

    uint32_t calculatedCrc = 0;

#ifdef __USE_HAL_CRC
    calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t*)BL_ADDRESS, (BL_SIZE/4));
#else
    calculatedCrc = crc32((uint32_t*)DEV_BL_ADDRESS, (DEV_BL_SIZE/4));
#endif

    return calculatedCrc;

}


/* Verify checksum of application located in flash ---------------------------------*/
uint32_t btld_CalcChecksum(uint32_t start_addr, uint32_t Length){

    uint32_t calculatedCrc = 0;

#ifdef __USE_HAL_CRC
    //calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t*)APP_ADDRESS, (APP_SIZE/4));
    calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t *)start_addr, (Length/4));
#else
    calculatedCrc = crc32((uint32_t *)start_addr, (Length/4));
#endif
   return calculatedCrc;
}

/* Verify checksum of application located in flash ---------------------------------*/
uint32_t btld_GetFlChecksum(_FLASH_AREA_TYPE flash_area) {

 	switch(flash_area) {

		case BOOTLOADER:
			return (uint32_t)pDevCRCRegs->bl_crc32;

		case APPLICATION:
			return (uint32_t)pDevCRCRegs->app_crc32;

		case APPLICATION_CONFIG:
			return (uint32_t)pDevCRCRegs->app_config_crc32;

		case DEVICE_CONFIG:
			return (uint32_t)pDevCRCRegs->dev_config_crc32;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

 	return (uint32_t)~0U;
}



/* Save checksum and length to flash crc area ----------------------------------------------------------*/
uint8_t btld_SaveFlParam(void){

	HAL_StatusTypeDef returnedERR=HAL_OK;
	_DEV_CRC_REGS dev_crc_regs;
	uint32_t* p_dev_crc_regs=(uint32_t*)&dev_crc_regs;
	uint32_t length = 0;
	uint32_t calculatedCrc;
	int i;

	//copy crc flash mem to temp ram variable before ereasing flash
	memcpy((void*)&dev_crc_regs, (void*)pDevCRCRegs, sizeof(_DEV_CRC_REGS));

	//get and calc the new parameters for the spezefic flash area
	length =  (uint32_t)(bl_ctrl_reg.flash_ptr - bl_ctrl_reg.flash_start_addr);
	calculatedCrc=btld_CalcChecksum(bl_ctrl_reg.flash_start_addr, length);

	//set the new params in the temp ram struct
 	switch(bl_ctrl_reg.flash_area) {

		case BOOTLOADER:
			dev_crc_regs.bl_length = length;
			dev_crc_regs.bl_crc32 = calculatedCrc;
			break;

		case APPLICATION:
			dev_crc_regs.app_length = length;
			dev_crc_regs.app_crc32 = calculatedCrc;
			break;

		case APPLICATION_CONFIG:
			dev_crc_regs.app_config_length = length;
			dev_crc_regs.app_config_crc32 = calculatedCrc;
			break;

		case DEVICE_CONFIG:
			dev_crc_regs.dev_config_length = length;
			dev_crc_regs.dev_config_crc32 = calculatedCrc;
			break;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			return HAL_ERROR;
	}

 	//erease the flash page that holds the dev crc and length params
 	btld_EraseFlash(BL_DEV_CRC);

	HAL_FLASH_Unlock();

	//write back the updated, temp ram struct to dev crc flash
	//@note: be aware when modifying the dev crc struct, we need a 4 Byte alignment, otherwise the copycode below will fail
	for (i=0; i<(sizeof(_DEV_CRC_REGS)/4); i++) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)((uint32_t*)pDevCRCRegs+i), (uint64_t)p_dev_crc_regs[i] ) != HAL_OK) {
			returnedERR=HAL_ERROR;
			break;
		}
	}

	HAL_FLASH_Lock();

	return returnedERR;
}


/* Save checksum to flash ----------------------------------------------------------*/
uint8_t btld_StoreFlChecksum(_FLASH_AREA_TYPE flash_area, uint32_t checksum){
	//uint32_t calculatedCrc = 0;
	HAL_StatusTypeDef returnedERR=HAL_OK;

	HAL_FLASH_Unlock();

 	switch(flash_area) {

		case BOOTLOADER:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->bl_crc32, (uint64_t)checksum);
			break;

		case APPLICATION:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->app_crc32, (uint64_t)checksum);
			break;

		case APPLICATION_CONFIG:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->app_config_crc32, (uint64_t)checksum);
			break;

		case DEVICE_CONFIG:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->dev_config_crc32, (uint64_t)checksum);
			break;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

	HAL_FLASH_Lock();

	return returnedERR;
}



/* Save checksum to flash ----------------------------------------------------------*/
uint8_t btld_SaveFlChecksum(void){
	uint32_t calculatedCrc = 0;
	HAL_StatusTypeDef returnedERR=HAL_OK;
	uint32_t length = 0;

	length =  (uint32_t)(bl_ctrl_reg.flash_ptr - bl_ctrl_reg.flash_start_addr);

	calculatedCrc=btld_CalcChecksum(bl_ctrl_reg.flash_start_addr, length);

	returnedERR = btld_StoreFlChecksum(bl_ctrl_reg.flash_area, calculatedCrc);

	return returnedERR;
}


/* Finish flash programming --------------------------------------------------*/
uint8_t btld_SaveFlLength(void){
	HAL_StatusTypeDef returnedERR=HAL_OK;
	uint32_t length;
    /* Unlock flash */

	length =  (uint32_t)(bl_ctrl_reg.flash_ptr - bl_ctrl_reg.flash_start_addr);

	HAL_FLASH_Unlock();

 	switch(bl_ctrl_reg.flash_area) {

		case BOOTLOADER:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->bl_length, (uint64_t)length);
			break;

		case APPLICATION:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->app_length, (uint64_t)length);
			break;

		case APPLICATION_CONFIG:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->app_config_length, (uint64_t)length);
			break;

		case DEVICE_CONFIG:
			returnedERR=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&pDevCRCRegs->dev_config_length, (uint64_t)length);
			break;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

    /* Lock flash */
    HAL_FLASH_Lock();

	return returnedERR;
}


/* Finish flash programming --------------------------------------------------*/
uint32_t btld_GetFlLength(_FLASH_AREA_TYPE flash_area) {

 	switch(flash_area) {

		case BOOTLOADER:
			if (pDevCRCRegs->bl_length <= DEV_BL_SIZE)
				return pDevCRCRegs->bl_length;

		case APPLICATION:
			if (pDevCRCRegs->app_length <= DEV_APP_SIZE)
				return pDevCRCRegs->app_length;

		case APPLICATION_CONFIG:
			if (pDevCRCRegs->app_config_length <= DEV_APP_CONFIG_SIZE)
				return pDevCRCRegs->app_config_length;

		case DEVICE_CONFIG:
			if (pDevCRCRegs->dev_config_length <= DEV_CONFIG_FL_SIZE)
				return pDevCRCRegs->dev_config_length;

		case BL_DEV_CRC:
			//flash_ptr = DEV_CRC_FL_ADDRESS;
			//break;
		default:
			break;
	}

	return 0;
}


///* Finish flash programming --------------------------------------------------*/
//uint32_t btld_GetAppCRC(void){
//	//return *((uint32_t *)APP_CRC_ADDRESS);
//	return pDevCRCRegs->app_crc32;
//}


/* Check for application in user flash ---------------------------------------------*/
uint8_t btld_CheckForApplication(void){

    return ( ((*(__IO uint32_t*)DEV_APP_ADDRESS) - SRAM_SIZE) == 0x20000000 ) ? BL_OK : BL_NO_APP;
}


/* Jump to application -------------------------------------------------------------*/
void btld_JumpToApp(void){
    uint32_t  JumpAddress = *(__IO uint32_t*)(DEV_APP_ADDRESS + 4);
    pFunction Jump = (pFunction)JumpAddress;
    

    HAL_RCC_DeInit();
    HAL_DeInit();
    
    //HAL_NVIC_DisableIRQ();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;
    
#if (SET_VECTOR_TABLE)
    SCB->VTOR = DEV_APP_ADDRESS;
#endif
    
    __set_MSP(*(__IO uint32_t*)DEV_APP_ADDRESS);
    Jump();
}


/* Jump to System Memory (ST Bootloader) -------------------------------------------*/
void btld_JumpToSysMem(void){
    uint32_t  JumpAddress = *(__IO uint32_t*)(SYSMEM_ADDRESS + 4);
    pFunction Jump = (pFunction)JumpAddress;
    
    HAL_RCC_DeInit();
    HAL_DeInit();
    
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;
    
    __HAL_RCC_SYSCFG_CLK_ENABLE();
//    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
    
    __set_MSP(*(__IO uint32_t*)SYSMEM_ADDRESS);
    Jump();
    
    while(1);
}
