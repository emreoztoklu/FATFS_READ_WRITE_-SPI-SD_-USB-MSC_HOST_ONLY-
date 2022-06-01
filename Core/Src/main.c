/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"
#include <system_error_list.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "retarget.h"
#include "fatfs_sd.h"
#include "File_Handling.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define APP_VERSION_MAX 	0x1U
#define APP_VERSION_MIN 	0x0U
#define APP_VERSION_SMIN 	0x2U

#define __STM32F4xx_HAL_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define __STM32F4xx_HAL_VERSION_SUB1   (0x07U) /*!< [23:16] sub1 version */
#define __STM32F4xx_HAL_VERSION_SUB2   (0x0DU) /*!< [15:8]  sub2 version */
#define __STM32F4xx_HAL_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
int file_copy(const char * src_path, const char *dest_path);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*****************************************************************************************************************/
DIR dir;

/*USB*/
extern ApplicationTypeDef Appli_state;
extern USBH_StatusTypeDef usb_status;
extern USBH_HandleTypeDef hUsbHostFS;

BYTE byte_buffer[4096];
char char_buffer[2];
BYTE *small_buffer;

/*USB HANDLES*/
extern uint8_t retUSBH; /* Return value for USBH */
extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */
extern FILINFO USBHfno;
extern UINT USB_br, USB_bw;  // File read/write count

/*SD HANDLES*/
extern uint8_t retUSER;    /* Return value for USER */
extern char USERPath[4];   /* USER logical drive path */
extern FATFS USERFatFS;    /* File system object for USER logical drive */
extern FIL USERFile;       /* File object for USER */
extern FILINFO SDfno;
extern UINT SD_br, SD_bw;  		// File read/write count


/*******************************************/
/*Flags*/
uint8_t usb_flag;			// 0 is pasive 1 is active
uint8_t usb_wrflag;			// 0 is write 1 is read

extern uint16_t Timer3;
/*******************************************/

char path_SD[20];
char path_USB[20];

void toggleinfoled(GPIO_TypeDef* Portx, uint16_t Portnumber, int delay){
	int isOn;
	int delay1;

	isOn = !isOn;

	if(isOn == TRUE)
	  delay1 = delay;
	else
	  delay1 = delay;

	HAL_GPIO_WritePin(Portx, Portnumber, isOn);
	HAL_Delay(delay1);
}

/*
to find the size of data in buffer
int bufsize(char *buf){
	int i = 0;
	while(*buf++ != '\0') i++;
	return i;
}

*/

/*
clear buffer
char char_buffer[4096];	// to storage data
void bufclear(void){
	for(int i = 0; i < 1024 ; i++)
		char_buffer[i]= '\0';
}
*/



/*****************************************************************************************************************/
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */



int main(void){
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
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */

  RetargetInit(&huart3);
  printf("*******************ERA ELECTRONICS**********************\r\n");
  printf("APPLICATION VERSION	: \"v%d.%d.%d\"\r\n", APP_VERSION_MAX,APP_VERSION_MIN,APP_VERSION_SMIN);
  printf("STM32F4xx_HAL_VERSION	: \"v%d.%d.%d\"\r\n", __STM32F4xx_HAL_VERSION_MAIN,__STM32F4xx_HAL_VERSION_SUB1,__STM32F4xx_HAL_VERSION_SUB2);
  printf("STM32F4xx_CMSIS_VERSION	: \"v%d.%d.%d\"\r\n", __STM32F4xx_CMSIS_VERSION_MAIN,__STM32F4xx_CMSIS_VERSION_SUB1,__STM32F4xx_CMSIS_VERSION_SUB2);
  printf("*******************************************************\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  Timer3 = 3000;			// if the USB is not mounted --check the condition 3sn then go your main loop


  while (1){
    /* USER CODE END WHILE */

	  MX_USB_HOST_Process();
	  if((Appli_state == APPLICATION_READY) && (hUsbHostFS.gState == HOST_CLASS) && (usb_status == USBH_OK)){

		  	Check_USB_Details();   // check space details
		  	printf("---------------------------------------------\r\n");
		  	//Scan_USB(USBHPath);

/*
		  	if(Create_Dir("0://ERA")){
		  		printf(">USB:Directory already exist or Create Error!\r\n");
		  	}else{
		  		for(int i = 1; i<51;i++){
		  			sprintf(path_USB,"0://ERA/ses_%d.txt",i);
			  		file_copy("0://USBERA/USBemre.txt", path_USB);
			  	  }
		  	}
*/
		  	Read_File("0://ERA/ses_35.txt");

		  	if(Mount_SD()){
		  		HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
		  		printf(">SD : There is no any SD CARD\r\n");
		  		Error_Handler();
		  		break;
		  	}

		  	else{
		  	  /*Check free space*/
		  	  Check_SDCARD_Details();
		  	  Format_SD();
/**************************************************************************/
		  	  printf("---------------------------------------------\r\n");

		  	  for ( int i = 1; i<51;i++){
		  		sprintf(path_USB,"0://ERA//ses_%d.txt",i);
		  		sprintf(path_SD,"1://ses_%d.txt",i);
		  		file_copy(path_USB, path_SD);
		  	  }

		  // Scan_SD(USERPath);				// USERPath is SD CARD
		  	  Read_SD_File("1://ses_35.txt");

/**************************************************************************/
		  	  UnMount_SD();
		  	}
		  	Unmount_USB();
		  	hUsbHostFS.gState = HOST_IDLE;
	  }

	  if(hUsbHostFS.gState == HOST_IDLE && !Timer3){
		while(1){
			toggleinfoled(LED1_GPIO_Port, LED1_Pin, 100);
		}
	}

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int file_copy(const char *src_path, const char *dest_path){
	 if (f_stat (src_path, &USBHfno) != FR_OK){
			printf ("\r\n>USB: ERROR!!! File does not exists\r\n");
			return 1;
	  }else{
		  if(f_open(&USBHFile,src_path,FA_OPEN_ALWAYS|FA_READ)!= FR_OK){
			printf ("\r\n>USB: ERROR!!! File does not open\r\n");
			return 1;
		  }
		  else{
	  		  if(f_open(&USERFile,dest_path,FA_OPEN_ALWAYS|FA_CREATE_ALWAYS|FA_READ|FA_WRITE)!= FR_OK){
	  			printf ("\r\n >SD : ERROR!!! File can not Create/Write\r\n");			//1 bak
	  			return 1;
	  		  }
	  		  else{
	  		    if(f_size(&USBHFile) == 0 || f_size(&USBHFile) < sizeof(byte_buffer)){
	  		    	if((small_buffer = (BYTE*)malloc((int)f_size(&USBHFile)*sizeof(BYTE))) == NULL){
	  		    		printf(">USB:Dinamic Memory is not allocated\r\n");
	  		    	}
	  		    	else{
	  			    	while (!f_eof(&USBHFile)){
	  			    		memset((void*)small_buffer, 0 , f_size(&USBHFile));
	  						if(f_read(&USBHFile, small_buffer, (int)f_size(&USBHFile), &USB_br) != FR_OK){
	  							printf(">USB:Read Error\r\n");
	  							return 1;
	  						}
	  			    		if(!USB_br) return 1;

	  			    		for (int i = 0; i< f_size(&USBHFile); i++){
			  					sprintf(&char_buffer[0],"%c",*(BYTE*)(small_buffer + i));
			  					f_write(&USERFile,(const void *)&char_buffer[0], 1, &SD_br);
	  			    		}
	  			    		memset((void*)small_buffer, 0 , f_size(&USBHFile));
	  			    		free((void*)small_buffer);
	  			    	}
	  		    	}
	  		    }
	  		    else{
		  			for (int k = 0; k < f_size(&USBHFile)/sizeof(byte_buffer); k++){
		  				if(f_read(&USBHFile, byte_buffer, sizeof(byte_buffer), &USB_br) != FR_OK){
		  					printf(">SD :Read Error\r\n");
		  					break;
		  				}

		  				for(int i = 0; i < sizeof(byte_buffer) ; i++){
		  					sprintf(&char_buffer[0],"%c",*(BYTE*)(byte_buffer + i));
		  					f_write(&USERFile,(const void *)&char_buffer[0], 1, &SD_br);
		  				}
		  				memset(byte_buffer, 0, sizeof(byte_buffer));
		  				f_lseek(&USBHFile, (k + 1) * 4096);
		  			}

	  		    }
	  		 }
		  }

		f_close(&USBHFile);
		f_close(&USERFile);

	  }
	 return 0;
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

