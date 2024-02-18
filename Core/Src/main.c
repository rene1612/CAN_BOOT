/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "crc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include "dev_config.h"
/* Includes ------------------------------------------------------------------*/

/*

		Flash memory organisation

		0x8020000 -> +-------------+
					 | 0x801FC00   |	\
					 |    to       |	 \
					 | 0x802FFFF   |	  |- Reserved for checksum and other informations
					 |   1 KB      |	 /
					 | data        |	/
		0x801FC00 -> +-------------+
					 | 0x801F800   |	\
					 |    to       |	 \
					 | 0x801FBFF   |	  |- Bootloader and device Config-Stuff (Dev-Addr->Can-ID, can-baudrate, Boardtype, Boardversion ...)
					 |   1 KB      |	 /
					 | data        |	/
		0x801F800 -> +-------------+
					 | 0x801F000   |	\
					 |    to       |	 \
					 | 0x801F7FF   |	  |- Applicaton Configdata (Tempsensor ID-Lookup table,
					 |   2 KB      |	 /
					 | data        |	/
		0x801F000 -> +-------------+
					 |             |	\
					 |             |	 \
					 |             |	  |
					 |             |	  |
					 | 0x8008000   |	  |
					 |    to       |	  |
					 | 0x801EFFF   |	  |- Contain the application software
					 |   92 KB     |	  |
					 | application |	  |
					 |             |	  |
					 |             |	  |
					 |             |	 /
					 |             |	/
		0x8008000 -> +-------------+
					 |             |	\
					 | 0x8000000   |	 \
					 |    to       |	  |
					 | 0x8007FFF   |	  |- Contain bootloader software (this project)
					 |   32 KB     |	  |
					 | bootloader  |	 /
					 |             |	/
		0x8000000 -> +-------------+

Bootloader version 1.0.0 CRC32 = 0xC2106B18

 */
#include <string.h>
#include "btld.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRUE 1
#define FALSE 0
#define G_MAX_MS_COUNTER 4294967296

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
__attribute__((__section__(".board_info"))) const unsigned char BOARD_NAME[10] = "BOOT";

__attribute__((__section__(".sw_info"))) const _SW_INFO_REGS sw_info_regs = {
  		__SW_RELEASE__,
  		__SW_RELEASE_DATE__,
  		0x0000000000000000,
  		"no tag"
  };

__attribute__((__section__(".dev_config"))) const _DEV_CONFIG_REGS dev_config_regs = {
		__DEV_ID__,
		__BOARD_TYPE__,
		__BOARD_VERSION__,
		DEAULT_BL_CAN_BITRATE,
		DEAULT_APP_CAN_BITRATE,
		__BOARD_MF_DATE__
};

__attribute__((__section__(".dev_crc_regs"))) const _DEV_CRC_REGS dev_crc_regs = {
		~0U,	//BL crc32
		DEV_BL_SIZE,
		~0U,	//APP crc32
		~0U,
		~0U,	//APP_CONFIG crc32
		~0U,
		~0U,	//DEV_CONFIG crc32
		sizeof(_DEV_CONFIG_REGS),
		BL_PROTECTION_WRP,	// BL Flash Flags
		BL_PROTECTION_NONE,	// APP Flash Flags
		BL_PROTECTION_NONE,	// APP_CONFIG Flash Flags
		BL_PROTECTION_NONE	// DEV_CONFIG Flash Flags
};

//const _DEV_CONFIG_REGS* pDevConfig = (const _DEV_CONFIG_REGS*)DEV_CONFIG_FL_ADDRESS;
const _DEV_CONFIG_REGS* pDevConfig = (const _DEV_CONFIG_REGS*)&dev_config_regs;

//const _DEV_CRC_REGS* pDevCRCRegs = (const _DEV_CRC_REGS*)DEV_CRC_FL_ADDRESS;
const _DEV_CRC_REGS* pDevCRCRegs = (const _DEV_CRC_REGS*)&dev_crc_regs;

uint8_t led_state = 1;
uint8_t G_LedUpdate=0;

_BL_CTRL_REGS_TYPE bl_ctrl_reg;

uint32_t G_mSCounter=0;


CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               TxData[8];
uint8_t               RxData[8];
uint32_t              TxMailbox;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void config_can_filter(void);
void CAN_TX_Cplt(CAN_HandleTypeDef* phcan);
void Run_Application();

void send_confirm_msg(uint8_t status);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
 {
	 int DataIdx;
	 for (DataIdx = 0; DataIdx < len; DataIdx++)
	 {
		 ITM_SendChar(*ptr++);
	 }
	 return len;
 }

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	bl_ctrl_reg.loader_mode=0;
	bl_ctrl_reg.FlashInProgress=0;
	uint8_t led_counter=0;
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
  MX_CAN_Init();
  MX_CRC_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

	uint32_t bl_cs_fl=btld_GetFlChecksum(BOOTLOADER);
	uint32_t bl_cs_calc=btld_CalcChecksum(DEV_BL_ADDRESS, DEV_BL_SIZE);
	uint32_t dev_cfg_cs_fl=btld_GetFlChecksum(DEVICE_CONFIG);
	uint32_t dev_cfg_length = btld_GetFlLength(DEVICE_CONFIG);
	uint32_t dev_cfg_cs_calc;


	if (dev_cfg_length) {
		dev_cfg_cs_calc = btld_CalcChecksum(DEV_CONFIG_FL_ADDRESS, dev_cfg_length);
	}

	if ( bl_cs_fl == ~0U ) {	//first time boot?, no valid crc for bootloader and dev_config in flash

		btld_StoreFlChecksum(BOOTLOADER, bl_cs_calc);

		if ( dev_cfg_cs_fl == ~0U ) {
			btld_StoreFlChecksum(DEVICE_CONFIG, dev_cfg_cs_calc);
		}

		//fist time run of bootloader > we want to stay in bootloader and dont run APP???
		bl_ctrl_reg.loader_mode=1;

	} else { //verify bootloader crc??? and //check for dev_config integrity

		if ( (bl_cs_calc != bl_cs_fl) || (dev_cfg_cs_calc != dev_cfg_cs_fl) ) {
			//error bootloader or dev_config maybe corrupted

			//signal with led??? (todo)
			Error_Handler();
		}

		//if we have initial or default dev-config, stay in loader mode??
		if (pDevConfig->dev_id == 0xFF && pDevConfig->board_type == 0xFF ) {
			bl_ctrl_reg.loader_mode=1;
		}

		if(btld_CheckForApplication()==BL_NO_APP) {
			bl_ctrl_reg.loader_mode=1;
		} else {
			if ( btld_ValidateFlArea(APPLICATION) != BL_OK ) {
				bl_ctrl_reg.loader_mode=1;
				led_state = 5;
			}
		}
	}

  /* Configura CAN RX FILTER */
  //config_can_filter();

  /* Start CAN recieve Interrupt */
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);

  HAL_TIM_Base_Start_IT(&htim4); // 1mS


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  if (*(uint32_t *)_MAGIC_RAM_ADDRESS_ == _MAGIC_RAM_DWORD_) {
	  //we have an app based bootevent stay in bootloader and don´t run app
	  *(uint32_t *)_MAGIC_RAM_ADDRESS_ = 0U;
	  bl_ctrl_reg.loader_mode=1;
  }


  while (1)
  {
	//uint8_t LOADED=0;

	/* Loader Mode */
	if(bl_ctrl_reg.loader_mode){

		/* Erase Flash */
		if(bl_ctrl_reg.loader_cmd == BL_erease_FLASH_CMD){
			if(btld_EraseFlash(bl_ctrl_reg.flash_area)==HAL_OK){
				/* Erase Success */
				//Send CAN message to know we are in loader mode
				TxData[2]=0xFF;
			}else{
				/* Erase failed */
				//Send CAN message to know we are in loader mode
				TxData[2]=0x0F;
			}
			TxData[0]=BL_erease_FLASH_CMD;
			TxData[1]=bl_ctrl_reg.flash_area;
			TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
			TxHeader.IDE=CAN_ID_EXT;
			TxHeader.RTR=CAN_RTR_DATA;
			TxHeader.DLC=3;

			HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);

			bl_ctrl_reg.loader_cmd=0;
		}

		/* Run Application command */
		if(bl_ctrl_reg.loader_cmd == BL_run_APP_CMD){
			//btld_JumpToApp();
			Run_Application();
		}

		/* Calculate and send CRC */
		if(bl_ctrl_reg.loader_cmd == BL_ask_for_CRC_CMD){
			uint32_t checksum;
			checksum=btld_GetFlChecksum(bl_ctrl_reg.flash_area);

			/* Send Available flash size for application */
			TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
			TxHeader.IDE=CAN_ID_EXT;
			TxHeader.RTR=CAN_RTR_DATA;
			TxHeader.DLC=6;
			TxData[0]=BL_ask_for_CRC_CMD;
			TxData[1]=bl_ctrl_reg.flash_area;

			TxData[2]=(checksum & 0xFF000000)>>24;
			TxData[3]=(checksum & 0x00FF0000)>>16;
			TxData[4]=(checksum & 0x0000FF00)>>8;
			TxData[5]=(checksum & 0x000000FF);

			/*TxData[1]=(~checksum & 0xFF000000)>>24;
			TxData[2]=(~checksum & 0x00FF0000)>>16;
			TxData[3]=(~checksum & 0x0000FF00)>>8;
			TxData[4]=(~checksum & 0x000000FF);*/

			HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);

			bl_ctrl_reg.loader_cmd=0;
		}

		/* Start Falshing and request new data, respond success  */
		if (bl_ctrl_reg.loader_cmd == BL_start_FLASH_CMD) {

			if (btld_FlashBegin(bl_ctrl_reg.flash_area) == BL_OK) {
				TxData[2]=0xFF;	//Success
				led_state = 4;
			} else {
				TxData[2]=0x0F;	//Error
				//led_state = err; (todo)
			}
			//bl_ctrl_reg.FlashInProgress = 1;

			// request next WORD
			/* Send success */
			TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
			TxHeader.IDE=CAN_ID_EXT;
			TxHeader.RTR=CAN_RTR_DATA;
			TxHeader.DLC=3;
			TxData[0]=BL_start_FLASH_CMD;
			TxData[1]=bl_ctrl_reg.flash_area;

			HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);

			bl_ctrl_reg.loader_cmd=0;
		}

		/* End Falshing and respond success/error */
		if (bl_ctrl_reg.loader_cmd == BL_done_FLASH_CMD  && bl_ctrl_reg.FlashInProgress){
			btld_FlashEnd();
			//store permanently to crc-flash area
			if (btld_SaveFlParam() == HAL_OK) {
				TxData[2]=0xFF;	//Success
				led_state = 1;
			} else {
				TxData[2]=0x0F;	//Success
				led_state = 3;
			}

			bl_ctrl_reg.FlashInProgress=0;
			bl_ctrl_reg.loader_cmd=0;

			// request next WORD
			/* Send success */
			TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
			TxHeader.IDE=CAN_ID_EXT;
			TxHeader.RTR=CAN_RTR_DATA;
			TxHeader.DLC=3;
			TxData[0]=BL_done_FLASH_CMD;
			TxData[1]=bl_ctrl_reg.flash_area;

			HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);
		}

		/*  */
		if (bl_ctrl_reg.loader_cmd == BL_disable_RW_PROTECTION_CMD  || bl_ctrl_reg.loader_cmd == BL_enable_RW_PROTECTION_CMD){

			if (bl_ctrl_reg.loader_cmd == BL_disable_RW_PROTECTION_CMD) {
				btld_ConfigProtection(bl_ctrl_reg.flash_area, BL_PROTECTION_NONE);
			}else if (bl_ctrl_reg.loader_cmd == BL_enable_RW_PROTECTION_CMD) {
				btld_ConfigProtection(bl_ctrl_reg.flash_area, BL_PROTECTION_RDP);
			}

			/* Send success */
			TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
			TxHeader.IDE=CAN_ID_EXT;
			TxHeader.RTR=CAN_RTR_DATA;
			TxHeader.DLC=3;
			TxData[0]=bl_ctrl_reg.loader_cmd;
			TxData[1]=0xFF;	//Success
			TxData[2]=bl_ctrl_reg.flash_area;

			HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);

			bl_ctrl_reg.loader_cmd=0;
		}


		// LED CONTROL
		if (G_LedUpdate) {
			// every 500mS update the LED Status
			//static uint8_t led_state = 1;
			led_counter++;

			switch (led_state) {
			case 1:
				if (led_counter%4==0 ) {
					//HAL_GPIO_WritePin(GPIOB, LED_RED_Pin,1);
					HAL_GPIO_TogglePin(GPIOB, LED_GREEN_Pin);
					//HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin,0);
					//led_state++;
				}
				break;
			case 2:
				//if (led_counter%4==0 ) {
				//	HAL_GPIO_WritePin(GPIOB, LED_RED_Pin,0);
					HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin,1);
				//	HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin,0);
				//	led_state++;
				//}
				break;
			case 3:
				//if (led_counter%2==0 ) {
				//	HAL_GPIO_WritePin(GPIOB, LED_RED_Pin,0);
					HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin,0);
				//	HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin,1);
				//	led_state = 1;
				//}
				break;
			case 5:
				if (led_counter%3==0 ) {
					//HAL_GPIO_WritePin(GPIOB, LED_RED_Pin,1);
					HAL_GPIO_TogglePin(GPIOB, LED_GREEN_Pin);
					//HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin,0);
					//led_state++;
				}
				break;
			default:
				HAL_GPIO_TogglePin(GPIOB, LED_GREEN_Pin);
				break;
			}
			G_LedUpdate=0;
		}

	}else{
		/* Jump to app if timeout*/
		if(G_mSCounter>50){
			btld_JumpToApp();
		}
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* Interrupt callback functions-----------------------------------------------*/

/* HAL_TIM_PeriodElapsedCallback ---------------------------------------------*/
/* Interrupt callback to manage timer interrupts							  */
/* htim19 1ms																  */
/*----------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	/* htim4 1ms */
	if (&htim4 == htim) {

		static uint16_t cpt2=0;

		G_mSCounter++;

		if(G_mSCounter % 200 == 0){
			G_LedUpdate = 1;
		}

		if(G_mSCounter>=G_MAX_MS_COUNTER){
			G_mSCounter=0;
		}

		cpt2++;

		/* 1 S, reinitiate cpt2 */
		if(cpt2>=1000){
			cpt2=0;
		}

		/* 100 mS */
		if(cpt2%100==0){
			/* Main loop -----------------------------------------------------*/

			/* Input Read ------------------------------------------------*/

			/*------------------------------------------------ Input Read */

			/* Control loop ----------------------------------------------*/


			/*---------------------------------------------- Control loop */

			/* Output Write ----------------------------------------------*/
			if(bl_ctrl_reg.loader_mode){
				//Send CAN message to know we are in loader mode
				TxHeader.ExtId=bl_ctrl_reg.tx_heartbeat_can_id;
				TxHeader.IDE=CAN_ID_EXT;
				TxHeader.RTR=CAN_RTR_DATA;
				TxHeader.DLC=1;
				TxData[0]=0xFF;

				HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);
			}
			/*---------------------------------------------- Output Write */

			/*----------------------------------------------------- Main loop */
		}
	}
}

/* HAL_CAN_ErrorCallback -----------------------------------------------------*/
/* Interrupt callback to manage Can Errors                                    */
/*----------------------------------------------------------------------------*/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *phcan){
	//uint32_t transmitmailbox;
	int retry=0;

	if((phcan->ErrorCode & HAL_CAN_ERROR_TX_ALST0) || (phcan->ErrorCode & HAL_CAN_ERROR_TX_TERR0)){
		HAL_CAN_AbortTxRequest(phcan,CAN_TX_MAILBOX0);
		retry=1;
	}
	if((phcan->ErrorCode & HAL_CAN_ERROR_TX_ALST1) || (phcan->ErrorCode & HAL_CAN_ERROR_TX_TERR1)){
		HAL_CAN_AbortTxRequest(phcan,CAN_TX_MAILBOX1);
		retry=1;
	}
	if((phcan->ErrorCode & HAL_CAN_ERROR_TX_ALST2) || (phcan->ErrorCode & HAL_CAN_ERROR_TX_TERR2)){
		HAL_CAN_AbortTxRequest(phcan,CAN_TX_MAILBOX2);
		retry=1;
	}

	HAL_CAN_ResetError(phcan);

	if(retry==1){
		HAL_CAN_DeactivateNotification(phcan,CAN_IT_TX_MAILBOX_EMPTY);
		HAL_CAN_ActivateNotification(phcan,CAN_IT_TX_MAILBOX_EMPTY);
		//HAL_CAN_AddTxMessage(phcan,&(CanTxList.SendMsgBuff.header),CanTxList.SendMsgBuff.Data,&transmitmailbox);
	}
}

/* HAL_CAN_TxCpltCallback ----------------------------------------------------*/
/* Interrupt callback to manage Can Tx Ready                                  */
/*----------------------------------------------------------------------------*/
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* phcan){
	CAN_TX_Cplt(phcan);
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* phcan){
	CAN_TX_Cplt(phcan);
}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* phcan){
	CAN_TX_Cplt(phcan);
}

void CAN_TX_Cplt(CAN_HandleTypeDef* phcan){

}

/* HAL_CAN_RxCpltCallback ----------------------------------------------------*/
/* Interrupt callback to manage Can Rx message ready to read                  */
/*----------------------------------------------------------------------------*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* phcan)
{

	CAN_RxHeaderTypeDef rxheader;
	uint8_t data[8];

	HAL_CAN_GetRxMessage(phcan,CAN_RX_FIFO0,&rxheader,data);

	/* Low level Can management ----------------------------------------------*/
	if(rxheader.ExtId==bl_ctrl_reg.rx_cmd_can_id && rxheader.IDE==CAN_ID_EXT){

		switch (data[0]) {

			/* xxxt ----------------------------------------------*/
			case BL_XXX_CMD:
				bl_ctrl_reg.loader_mode=1;
				break;

			/* erease flash cmd ----------------------------------------------*/
			case BL_erease_FLASH_CMD:
				if(bl_ctrl_reg.loader_mode){
					bl_ctrl_reg.flash_area=data[1];
					bl_ctrl_reg.loader_cmd=BL_erease_FLASH_CMD;
				}
				break;

			/* run app cmd ----------------------------------------------*/
			case BL_run_APP_CMD:
				if(bl_ctrl_reg.loader_mode){
					if (btld_CheckForApplication()==BL_OK) {
						bl_ctrl_reg.loader_cmd=BL_run_APP_CMD;
					}
				}
				break;

			/* ask for flash space and flags cmd ----------------------------------------------*/
			case BL_ask_for_FLASH_space_CMD:
				if(bl_ctrl_reg.loader_mode){
					/* Send the available flash size in bytes */
					uint32_t flash_size;

					//bl_ctrl_reg.flash_area=data[1];

					/* APP_SIZE is in DWORD, we multiply by 4 to have the size in bytes */
					flash_size=btld_GetMaxFlashSize(data[1]);	//((END_ADDRESS - ADDRESS) + 1);

					/* Send Available flash size for spezific flash area */
					TxHeader.ExtId=bl_ctrl_reg.tx_feedback_can_id;
					TxHeader.IDE=CAN_ID_EXT;
					TxHeader.RTR=CAN_RTR_DATA;
					TxHeader.DLC=7;
					TxData[0]=BL_ask_for_FLASH_space_CMD;
					TxData[1]=bl_ctrl_reg.flash_area;

					TxData[2]=(flash_size & 0xFF000000)>>24;
					TxData[3]=(flash_size & 0x00FF0000)>>16;
					TxData[4]=(flash_size & 0x0000FF00)>>8;
					TxData[5]=(flash_size & 0x000000FF);
					TxData[6]=btld_GeFlashFlags(data[1]);

					HAL_CAN_AddTxMessage(&hcan,&TxHeader,TxData,&TxMailbox);
 				}
				break;

			/* ask for crc cmd ----------------------------------------------*/
			case BL_ask_for_CRC_CMD:
				if(bl_ctrl_reg.loader_mode){
					bl_ctrl_reg.flash_area=data[1];
					bl_ctrl_reg.loader_cmd=BL_ask_for_CRC_CMD;
				}
				break;

			/* start flash cmd ----------------------------------------------*/
			case BL_start_FLASH_CMD:
				if(bl_ctrl_reg.loader_mode){
					bl_ctrl_reg.flash_area=data[1];
					bl_ctrl_reg.G_index=0;
					bl_ctrl_reg.loader_cmd=BL_start_FLASH_CMD;
				}
				break;

			/* next flash data cmd ----------------------------------------------*/
			case BL_next_FLASH_DATA_CMD:
				if(bl_ctrl_reg.loader_mode && rxheader.DLC >= 5){
					//bl_ctrl_reg.flash_area=data[1];
					bl_ctrl_reg.G_uint32_to_write = (((uint32_t) data[1]))
													| (((uint32_t) data[2]) << 8)
													| (((uint32_t) data[3]) << 16)
													| (((uint32_t) data[4]) << 24);

					//bl_ctrl_reg.loader_cmd=BL_next_FLASH_DATA_CMD;

					HAL_CAN_DeactivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
					HAL_TIM_Base_Stop_IT(&htim4);

					if (bl_ctrl_reg.G_index==((((uint32_t) data[5]) | ((uint32_t) data[6]) << 8))*4) {

						if (btld_FlashNext_32(bl_ctrl_reg.G_uint32_to_write, &bl_ctrl_reg.G_index) == BL_OK) {
							//bl_ctrl_reg.loader_cmd = 0;

							/* Send success // request next WORD */
							send_confirm_msg(0xFF);

						} else {
							// Send failed msg
							// mark flash area dirty ??? (todo)
							/* Send failure */
							send_confirm_msg(0x00);

							//bl_ctrl_reg.loader_cmd = 0;
							btld_FlashEnd();
							bl_ctrl_reg.FlashInProgress = 0;
						}
					} else {
						//bl_ctrl_reg.loader_cmd = 0;

						/* Send success // request next WORD */
						send_confirm_msg(0xFF);
					}

					HAL_TIM_Base_Start_IT(&htim4);
					HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
				}
				break;

			/* flash done cmd ----------------------------------------------*/
			case BL_done_FLASH_CMD:
				if(bl_ctrl_reg.loader_mode){
					bl_ctrl_reg.flash_area=data[1];
					bl_ctrl_reg.loader_cmd=BL_done_FLASH_CMD;
				}
				break;

			/* *Disable/Enable READ/WRITE protection cmd ----------------------------------------------*/
			case BL_enable_RW_PROTECTION_CMD:
			case BL_disable_RW_PROTECTION_CMD:
				if(bl_ctrl_reg.loader_mode && !bl_ctrl_reg.FlashInProgress){
					if (data[2] == 0xEF && data[3] == 0xFE && data[4] == 0x40 && data[5] == 0x20) {
						bl_ctrl_reg.flash_area=data[1];
						bl_ctrl_reg.loader_cmd=data[0];
						/*Disable READ/WRITE protection*/
						btld_ConfigProtection(bl_ctrl_reg.flash_area, BL_PROTECTION_NONE);

						/* Send success // request next WORD */
						//send_confirm_msg(0xFF);
					}
				}
				break;

			default:	//error, unknown command (send error response ???)
				break;
		}

	}
}

/* Bootloader functions-------------------------------------------------------*/


///*----------------------------------------------------------------------------*/
//void config_can_filter(void){
//   CAN_FilterTypeDef sFilterConfig;
//
//   	   	   	   	   	   	   	  /* Leave mask bits for different messages commands
//   	   	   	   	   	   	   	  |  	  	  	  	  	  	  	  	  	  	  	  	 */
//   uint32_t filterMask=	RXFILTERMASK;
//   uint32_t filterID=	RXFILTERID; // Only accept bootloader CAN message ID
//
//  /*##-2- Configure the CAN Filter ###########################################*/
//  //sFilterConfig.FilterNumber = 0;
//  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
//  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
//  sFilterConfig.FilterIdHigh = filterID >> 13;
//  sFilterConfig.FilterIdLow = (0x00FF & (filterID << 3)) | (1 << 2);
//  sFilterConfig.FilterMaskIdHigh = filterMask >> 13;
//  sFilterConfig.FilterMaskIdLow = (0x00FF & (filterMask << 3)) | (1 << 2);
//  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
//  sFilterConfig.FilterActivation = ENABLE;
//  sFilterConfig.FilterBank = 0;
//  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}

/* Run_Application -----------------------------------------------------------*/
/* Bootloader is done. Prepare the mcu for application-Start and finaly Start */
/*----------------------------------------------------------------------------*/
void Run_Application(void){
	  HAL_TIM_Base_Stop_IT(&htim4);
	  HAL_TIM_Base_DeInit(&htim4);
	  HAL_CAN_DeactivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);
	  HAL_CAN_Stop(&hcan);
	  HAL_CAN_DeInit(&hcan);
	  btld_JumpToApp();
}

/* send_confirm_msg ---------------------------------------------------------*/
/* replay to can-massege                                                     */
/*----------------------------------------------------------------------------*/
void send_confirm_msg(uint8_t status){
	uint16_t index;

	index = bl_ctrl_reg.G_index/4;

	TxHeader.ExtId = bl_ctrl_reg.tx_feedback_can_id;
	TxHeader.IDE = CAN_ID_EXT;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 4;
	TxData[0] = BL_next_FLASH_DATA_CMD;
	TxData[1] = status;	//0xFF = Success, 0x00 = failed
	TxData[2] = index & 0x00FF;
	TxData[3] = (index & 0xFF00)>>8;

	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
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

#ifdef  USE_FULL_ASSERT
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
