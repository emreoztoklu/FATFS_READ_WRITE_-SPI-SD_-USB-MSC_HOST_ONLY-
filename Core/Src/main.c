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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "retarget.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*****************************************************************************************************************/
const char error_list [20][100]  = {
		{ "(0) Succeeded "},
		{ "(1) A hard error occurred in the low level disk I/O layer "},
		{ "(2) Assertion failed "},
		{ "(3) The physical drive cannot work "},
		{ "(4) Could not find the file "},
		{ "(5) Could not find the pathSD "},
		{ "(6) The pathSD name format is invalid "},
		{ "(7) Access denied due to prohibited access or directory full "},
		{ "(8) Access denied due to prohibited access "},
		{ "(9) The file,directory object is invalid "},
		{ "(10) The physical drive is write protected "},
		{ "(11) The logical drive number is invalid "},
		{ "(12) The volume has no work area "},
		{ "(13) There is no valid FAT volume "},
		{ "(14) The f_mkfs() aborted due to any problem "},
		{ "(15) Could not get a grant to access the volume within defined period "},
		{ "(16) The operation is rejected according to the file sharing policy "},
		{ "(17) LFN working buffer could not be allocated "},
		{ "(18) Number of open files > _FS_LOCK "},
		{ "(19) Given parameter is invalid "},
};

/*USB*/
extern ApplicationTypeDef Appli_state;


/*SD CARD*/
FRESULT fresult;	// RESULT  of storage
UINT br, bw;		// file  read/write count

FATFS 	fs;			// file system
FILINFO finfo;		// file info

FIL 	fil; 		// file handler
FIL 	fil_write; 		// file handler

DIR     dir;
DWORD file_size;

/*Capacity related variables*/
FATFS		*pfs;
DWORD 		fre_clust;
uint32_t 	total_space, free_space;



char char_buffer[4096];	// to storage data
BYTE byte_buffer[4096];


const char *pathSD_name1 = {"1:ERA\\sample.bin"};
const char *pathSD_name2 = {"1:File1.txt"};
const char *pathSD_name3 = {"1:write_sample.bin"};

const char *pathUSB_name1 = {"0:ERA\\sample.bin"};
const char *pathUSB_name2 = {"0:File1.txt"};
const char *pathUSB_name3 = {"0:write_sample.bin"};

/*to find the size of data in buffer*/
int bufsize(char *buf){
	int i = 0;
	while(*buf++ != '\0') i++;
	return i;
}

/*clear buffer*/
void bufclear(void){
	for(int i = 0; i < 1024 ; i++)
		char_buffer[i]= '\0';
}



FRESULT scan_files (const char*pat, FILINFO *fno){
	/* Start node to be scanned (***also used as work area***) */
    FRESULT res;
    DIR dir;
    UINT i;
    FILINFO *fno_t = fno;

    /*Copy the pat to path*/
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s",pat);

    /* Open the directory */
    if ((res = f_opendir(&dir, path)) == FR_OK){
        for (;;) {
        	/* Read a directory item */
        	if ((res = f_readdir(&dir, fno_t)) != FR_OK || fno_t->fname[0] == 0)
        			break;  /* Break on error or end of dir */

        	if (fno_t->fattrib & AM_DIR) {   /* It is a directory */
        		if (!(strcmp ("SYSTEM~1", fno_t->fname)))
        		     continue;
        		if (!(strcmp("System Volume Information", fno_t->fname)))
        		     continue;

            	char *buf = malloc(30*sizeof(char));
            	sprintf (buf, "Dir: %s\r\n", fno_t->fname);
            	printf(buf);
            	free(buf);

        		i = strlen(path);
                sprintf(&path[i], "/%s", fno_t->fname);

                /* Enter the directory */
                if ((res = scan_files(path, fno)) != FR_OK)
                	break;

                path[i] = 0;
            }

            else {  /* It is a file. */
                printf("%s/%s size:%d KB \r\n", path, fno_t->fname, (int)fno_t->fsize/1024);
/*
                T* ---> const T*   legal in C
				const T* ---> T*   legal in C but C++ not
*/

                if (!memcmp("ERA", path+4, 3)){
                	if ( !memcmp("sample.bin", fno_t->fname, strlen(fno_t->fname)))
                		file_size = fno_t->fsize;
                }

            }
        }
        f_closedir(&dir);
    }

    return res;
}


/*****************************************************************************************************************/
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
  MX_USART3_UART_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  RetargetInit(&huart3);

  /* Wait for SD module reset */
  bufclear();
  HAL_Delay(500);


  printf("*****************************************************************\r\n");
  /*1
   * Mount SC Card*/

  if ((fresult = f_mount(&fs, USERPath, 1)) != FR_OK){
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
	  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
  }
  else {
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
	  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
  }


/************************************************************************/
  /*Check free space*/
  if ((fresult = f_getfree(USERPath, &fre_clust, &pfs)) != FR_OK)
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
  else
	  printf(">SD : CARD size is calculating \r\n");


  total_space = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
  printf(">SD : total size is : %.2f GB \r\n",(float)(total_space/1024)/1024);

  free_space = (uint32_t)(fre_clust * pfs->csize * 0.5 );
  printf(">SD : Free Space is : %.2f GB \r\n",(float)(free_space/1024)/1024);

  //printf(" File system mount ID : %u\r\n",pfs->id);
  //printf(" Physical drive number : %u\r\n",pfs->drv);

  scan_files(USERPath, &finfo);

  printf("-----------------------------------------------------------------------\r\n");
  printf(">SD : File system type (0:N/A) : %d\r\n", pfs->fs_type);
  printf(">SD : Physical drive number : %d\r\n", pfs->drv);
  printf(">SD : Number of FATs (1 or 2) : %d\r\n", pfs->n_fats);
  printf(">SD : win[] flag (b0:dirty) : %d\r\n", pfs->wflag);
  printf(">SD : FSINFO flags (b7:disabled, b0:dirty) : %d\r\n", pfs->fsi_flag);
  printf(">SD : File system mount ID : %u\r\n", pfs->id);
  printf(">SD : Number of root directory entries (FAT12/16) : %u\r\n", pfs->n_rootdir);
  printf(">SD : Cluster size [sectors] : %u\r\n", pfs->csize);
  printf("-----------------------------------------------------------------------\r\n");
  printf(">SD : Last allocated cluster : %lu\r\n", pfs->last_clst);
  printf(">SD : Number of free clusters : %lu\r\n", pfs->free_clst);
  printf("-----------------------------------------------------------------------\r\n");
  printf(">SD : Number of FAT entries (number of clusters + 2) : %lu\r\n", pfs->n_fatent);
  printf(">SD : Size of an FAT [sectors] : %lu\r\n", pfs->fsize);
  printf(">SD : Volume base sector : %lu\r\n", pfs->volbase);
  printf(">SD : FAT base sector : %lu\r\n", pfs->fatbase);
  printf(">SD : Root directory base sector/cluster : %lu\r\n", pfs->dirbase);
  printf(">SD : Data base sector : %lu\r\n", pfs->database);
  printf(">SD : Current sector appearing in the win[] : %lu\r\n", pfs->winsect);

/************************************************************************/

  /*Open this file on SD, if it is not exist then create this file */
  if ((fresult = f_open(&fil, pathSD_name2,  FA_OPEN_ALWAYS | FA_READ | FA_WRITE)) != FR_OK)
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD :fresult: %d \r\n", fresult);
  else
	  printf(">SD : File handler is opened in Read & Write Mode\r\n");

  /************************************************************************/
  /*write some text in file*/
  /*f_write*/

  f_write(&fil,"EMRE EMRE\n", 10, &bw);


  if ((bw = f_puts("New data: Emre OZTOKLU\n Era-Elektronik\n", &fil))==0)
	  printf(">SD : Write Error size is: %d\r\n", bw);
  else
	  printf(">SD : File1.txt created and data will be written  byte size is : %d \r\n",bw);

/************************************************************************/
  /*Close file handler*/
  if ((fresult = f_close(&fil))!=FR_OK)
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
  else
	  printf(">SD : File is closed!\r\n");

/************************************************************************/

  /*Open file and read*/
  if ((fresult = f_open(&fil, pathSD_name1, FA_READ)) != FR_OK)
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
  else
	  printf(">SD : File handler is opened to read mode\r\n");


  if ((fresult = f_open(&fil_write, pathSD_name3, FA_OPEN_ALWAYS | FA_READ | FA_WRITE)) != FR_OK)
	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
  else
	  printf(">SD : File handler is opened to write mode\r\n");


  scan_files("1:/", &finfo);

/************************************************************************/

  /*read string from the file*/
//  if(!f_gets(char_buffer,sizeof(char_buffer), &fil))				// reading string by f_gets
//	 printf(">Read Error \r\n");
//  else
//	 printf(">Read data from file total buf size 4096\r\n");
//
//  printf(">Data is : %s",char_buffer);
//  printf("\n");


  /*read string from the file*/
	printf("\r\n>SD : Read data from file total buf size 4096bytes\r\n");

	int i ,k;
	for (k = 0; k < file_size/sizeof(byte_buffer); k++){
	  if((fresult = f_read(&fil, byte_buffer, sizeof(byte_buffer), &br)) != FR_OK){
		  printf("\r\n>SD : Read Error \r\n");
		  break;
	  }
	  /*V.1*/
	  for(i = 0; i < sizeof(byte_buffer) ; i++){
		  if(!k && !i)
			printf("%08X ", 15);
		  if(k || i){
			printf(" ");
			if(!(i % 16)){
				printf(" \r\n%08X ", i + (k*4096));
			}
		  }
		  printf("%02X", *(BYTE*)(byte_buffer + i));

		  sprintf(&char_buffer[0],"%c",*(BYTE*)(byte_buffer + i));
		  f_write(&fil_write,(const void *)&char_buffer[0], 1, &bw);

	  }
	  memset(byte_buffer, 0, sizeof(byte_buffer));
	  f_lseek(&fil, (k + 1) * 4096);

	/*V.2*/
	//  printf("%08X ", 15);
	//  for(int i = 0; i < sizeof(byte_buffer) ; i++){
	//	  if(i!=0 && !( i % 4)){
	//  		printf(" ");
	//  		if(!(i % 64)){
	//  			printf("\r\n");
	//  			printf("%08X ", i);
	//  	  	}
	//	  }
	//	  printf("%02X", *(BYTE*)(byte_buffer + i));
	//  }
	}

/************************************************************************/
    /*Close file handler*/
    if ((fresult = f_close(&fil))!=FR_OK)
  	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
    else
  	  printf("\r\n\n>SD : File is closed!\r\n");

    /*Close file handler*/
    if ((fresult = f_close(&fil_write))!=FR_OK)
  	  fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
    else
  	  printf("\r\n\n>SD : Written File is closed!\r\n");

/************************************************************************/

	/* Unmount SDCARD */
	if(f_mount(NULL, "1:", 1) != FR_OK)
		fresult < 21 ? printf(">SD : fresult: %s \r\n",error_list[fresult]) : printf(">SD : fresult: %d \r\n", fresult);
	else{
		printf("\r\n>SD : SD card Unmount!\r\n");
		HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
	}


    bufclear();

    printf("*****************************************************************\r\n");
    printf("*****************************************************************\r\n");



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

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

