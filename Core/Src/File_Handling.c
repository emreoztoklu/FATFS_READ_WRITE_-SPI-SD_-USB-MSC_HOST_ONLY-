/*
 * File_Handling_RTOS.c
 *
 *  Created on: 26-June-2020
 *      Author: Controllerstech.com
 */
#include "stm32f4xx_hal.h"
#include "File_Handling.h"

#include "usb_host.h"
#include <string.h>
#include <stdio.h>

#include "STM_flash.h"
#include <machine/endian.h>

unsigned char buffer4[4];
#define LITTLE_TO_BIG_ENDIAN(buff)   (buff[0] |(buff[1]<<8) | (buff[2]<<16) | (buff[3]<<24));

//#include "retarget.h"

extern FILELIST_FileTypeDef FileList;
extern ApplicationTypeDef Appli_state;
USBH_HandleTypeDef hUSBHost;
uint16_t NumObs = 0;


/* ===============>>>>>>>> NO CHANGES AFTER THIS LINE ========================>>>>>>> */
extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */

FILINFO USBHfno;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pUSBHFatFS;
DWORD fre_clust;
uint32_t total, free_space;

/********************************************************************************/
extern UART_HandleTypeDef huart3;
#define UART &huart3
void Send_Uart (char *string){
	HAL_UART_Transmit(UART, (uint8_t *)string, strlen(string), HAL_MAX_DELAY);
}
/********************************************************************************/

int Mount_USB (void){
	if ((fresult = f_mount(&USBHFatFS, USBHPath, 1)) != FR_OK){
		printf ("ERROR!!! in mounting USB ...\r\n");
		return 1;
	}else{
		printf("USB mounted successfully...\r\n");
		return 0;
	}
}

int Unmount_USB (void){
	if ((fresult = f_mount(NULL, USBHPath, 1)) == FR_OK){
		printf("USB UNMOUNTED successfully...\r\n");
		return 0;
	}else{
		printf("ERROR!!! in UNMOUNTING USB \r\n");
		return 1;
	}
}

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_USB (char* pat){
    DIR dir;
    UINT i;
    char *path = malloc(100*sizeof (char));
    sprintf (path, "%s",pat);

    /* Open the directory */
    if (( fresult = f_opendir(&dir, path)) == FR_OK){

    	for (;;){
             /* Read a directory item */
            if ((fresult = f_readdir(&dir, &USBHfno)) != FR_OK || USBHfno.fname[0] == 0)
            	break;  /* Break on error or end of dir */

            if (USBHfno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname)))
            		continue;
            	if (!(strcmp("System Volume Information", USBHfno.fname)))
            		continue;

            	printf("\r\n");
            	printf(">USB:Dir: %s\r\n", USBHfno.fname);

                i = strlen(path);
                sprintf(&path[i], "/%s", USBHfno.fname);

                /* Enter the directory */
                if ((fresult = Scan_USB(path)) != FR_OK)
                	break;
                path[i] = 0;
            }
            else{   /* It is a file. */

               printf("\tFile: %s/%s  %d KB\r\n", path, USBHfno.fname,(int)USBHfno.fsize/1024);

            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}

/* Only supports removing files from home directory */
FRESULT Format_USB (void){
    DIR dir;
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s","/");

                           /* Open the directory */
    if ((fresult = f_opendir(&dir, path)) == FR_OK) {
        for (;;){
                               /* Read a directory item */
            if ((fresult = f_readdir(&dir, &USBHfno)) != FR_OK || USBHfno.fname[0] == 0)
            	break;  /* Break on error or end of dir */

            if (USBHfno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname)))
            		continue;

            	if (!(strcmp("System Volume Information", USBHfno.fname)))
            		continue;


            	if ((fresult = f_unlink(USBHfno.fname)) == FR_DENIED) continue;
            }

            else{   /* It is a file. */
               fresult = f_unlink(USBHfno.fname);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}


FRESULT Write_File (char *name, char *data){

	/**** check whether the file exists or not ****/
	if ((fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
	    return fresult;
	}

	else{
	    /* Create a file with read write access and open it */
	    if ((fresult = f_open(&USBHFile, name, FA_OPEN_EXISTING | FA_WRITE)) != FR_OK){
	    	printf ("\r\nERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
	        return fresult;
	    }

	    else{
	    	printf ("\r\nOpening file-->  *%s*  To WRITE data in it\r\n", name);

	    	if ((fresult = f_write(&USBHFile, data, strlen(data), &bw)) != FR_OK){
	    		printf ("\r\nERROR!!! No. %d while writing to the FILE *%s*\r\n", fresult, name);
	    	}

	    	/* Close file */
	    	if ((fresult = f_close(&USBHFile)) != FR_OK){
	    		printf ("\r\nERROR!!! No. %d in closing file *%s* after writing it\r\n", fresult, name);
	    	}

	    	else{
	    		printf ("\r\nFile *%s* is WRITTEN and CLOSED successfully\r\n", name);
	    	}
	    }
	    return fresult;
	}
}


void toggleinfoled(GPIO_TypeDef* Portx, uint16_t Portnumber, int delay){

	int isOn;
	int delay1;

	isOn = !isOn;

	if(isOn == 1)
	  delay1 = delay;
	else
	  delay1 = delay;

	HAL_GPIO_WritePin(Portx, Portnumber, isOn);
	HAL_Delay(delay1);
}


FRESULT Read_File (char *name){
	/**** check whether the file exists or not ****/

	if ((fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf("ERRROR!!! *%s* does not exists\r\n", name);
	    return fresult;
	}
	else{
/********************************************************************************/
/* Open file to read */
		if ((fresult = f_open(&USBHFile, name, FA_OPEN_ALWAYS|FA_READ)) != FR_OK){
			printf("ERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
		    return fresult;
		}
		else {
	    	printf ("Opening file-->  *%s*  To READ data from it\r\n", name);
		}

//char* bufferX = 0x080186A0;
/* Read data from the file
   see the function details for the arguments */
/********************************************************************************/

        BYTE byte_buffer[4096];
        BYTE *small_buffer;

	    FRESULT fr;          /* FatFs function common result code */
	    UINT br, bw;         /* File read/write count */

	    uint32_t file_size = f_size(&USBHFile);


	    if(file_size == 0 || file_size < sizeof(byte_buffer)){
	    	printf("%s file size is : %d byte  ""not enough buffer size is %d byte""\r\n", name, (int)file_size, (int)4096);

	    	if((small_buffer = (BYTE*)malloc(file_size*sizeof(BYTE))) == NULL){
	    		printf("Dinamic Memory is not allocated\r\n");

	    	}
	    	else {
		    	printf("Dinamic size is : %p \r\n", small_buffer);

		    	while (&USBHFile != f_eof(&USBHFile)){
		    		memset((void*)small_buffer, 0 , f_size(&USBHFile));
					if((fresult = f_read(&USBHFile, small_buffer, file_size, &br)) != FR_OK){
						printf("\r\n>USB : Read Error \r\n");
						break;
					}
		    		if(!br) break;

		    		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
		    		printf("\r\n");

		    		for (int i = 0; i< f_size(&USBHFile); i++){
		    			if(!i)
		    				printf("%08X ", 0);

		    			if(i){
		    				printf(" ");
		    				if(!(i % 16))
		    					printf(" \r\n%08X ", i);
		    			}
		    			printf("%02X", *(BYTE*)(small_buffer + i));

		    		}
		    		memset((void*)small_buffer, 0 , f_size(&USBHFile));
		    		free((void*)small_buffer);
		    		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		    		printf("\r\n");
		    	}
	    	}
/*********************************************************************************/
	    }
	    else{
			int i ,k;
			for (k = 0; k < f_size(&USBHFile)/sizeof(byte_buffer); k++){
			  if((fresult = f_read(&USBHFile, byte_buffer, sizeof(byte_buffer), &br)) != FR_OK){
				  printf("\r\n>USB : Read Error \r\n");
				  break;
			  }

			  //toggleinfoled(GPIOD, GPIO_PIN_13, 500);
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

			  /*V.1*/
			  for(i = 0; i < sizeof(byte_buffer) ; i++){
				  if(!k && !i)
					  printf("%08X ", 0);
				  	  //printf("%08X ", 15);
				  if(k || i){
					printf(" ");
					if(!(i % 16)){
						printf(" \r\n%08X ", i + (k*4096));
					}
				  }
				  printf("%02X", *(BYTE*)(byte_buffer + i));

			  }
			  memset(byte_buffer, 0, sizeof(byte_buffer));
			  f_lseek(&USBHFile, (k + 1) * 4096);
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

			}
			printf("\r\n");
	    }

/********************************************************************************/
/* Close file */
		if ((fresult = f_close(&USBHFile)) != FR_OK){
			printf ("\r\nERROR!!! No. %d in closing file *%s*\r\n", fresult, name);
		}
		else{
			printf ("\r\nFile *%s* CLOSED successfully\r\n", name);
		}

	    return fresult;
	}
}


FRESULT Create_File (char *name){

	if ((fresult = f_stat (name, &USBHfno)) == FR_OK){
		printf ("\r\nERROR!!! *%s* already exists!!!!\n use Update_File \n\n",name);
	    return fresult;
	}
	else{
		if ((fresult = f_open(&USBHFile, name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE)) != FR_OK){
			printf ("\r\nERROR!!! No. %d in creating file *%s*\r\n", fresult, name);
		    return fresult;
		}else{
			printf ("\r\n*%s* created successfully\n Now use Write_File to write data\r\n",name);
		}

		if ((fresult = f_close(&USBHFile)) != FR_OK){
			printf ("\r\nERROR No. %d in closing file *%s*\r\n", fresult, name);
		}else{
			printf ("\r\nFile *%s* CLOSED successfully\r\n", name);
		}
	}
    return fresult;
}

FRESULT Update_File (char *name, char *data){

	/**** check whether the file exists or not ****/
	if ((fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
	    return fresult;
	}
	else{
	/* Create a file with read write access and open it */
	    if ((fresult = f_open(&USBHFile, name, FA_OPEN_APPEND | FA_WRITE)) != FR_OK){
	    	printf ("\r\nERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
	        return fresult;
	    }

	    else{
			printf ("\r\nOpening file-->  *%s*  To UPDATE data in it\r\n", name);

		/* Writing text */
			if ((fresult = f_write(&USBHFile, data, strlen (data), &bw)) != FR_OK){
				printf ("\r\nERROR!!! No. %d in writing file *%s*\r\n", fresult, name);
			}else {
				printf ("\r\n*%s* UPDATED successfully\r\n", name);
			}

		/* Close file */
			if ((fresult = f_close(&USBHFile)) != FR_OK){
				printf ("\r\nERROR!!! No. %d in closing file *%s*\r\n", fresult, name);
			}else {
				printf ("\r\nFile *%s* CLOSED successfully\r\n", name);
			}
	    }
	}
    return fresult;
}

FRESULT Remove_File (char *name){
	/**** check whether the file exists or not ****/
	if ((fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
		return fresult;
	}
	else{
		if ((fresult = f_unlink (name)) == FR_OK){
			printf ("\r\n*%s* has been removed successfully\r\n", name);
		}else{
			printf ("\r\nERROR No. %d in removing *%s*\r\n",fresult, name);
		}
	}
	return fresult;
}

FRESULT Create_Dir (char *name){
    if ((fresult = f_mkdir(name)) == FR_OK){
    	printf ("\r\n*%s* has been created successfully\r\n", name);
    }
    else{
    	printf ("\r\nERROR No. %d in creating directory *%s*\r\n", fresult,name);
    }
    return fresult;
}

void Check_USB_Details (void){
    /* Check free space */
    f_getfree(USBHPath, &fre_clust, &pUSBHFatFS);

    total = (uint32_t)((pUSBHFatFS->n_fatent - 2) * pUSBHFatFS->csize * 0.5);
    printf ("\r\nUSB  Total Size: \t%d MB \r\n",(int)total/1024);

    free_space = (uint32_t)(fre_clust * pUSBHFatFS->csize * 0.5);
    printf ("USB Free Space: \t%d MB \r\n",(int)free_space/1024);
}


FRESULT StorageParse(void){
  FRESULT res = FR_OK;
  FILINFO fno;
  DIR dir;
  char *fn;

  if ((res = f_opendir(&dir, USBHPath)) == FR_OK){
	  FileList.ptr = 0;
	  while(Appli_state == APPLICATION_READY){

		  if ((res = f_readdir(&dir, &fno))!= FR_OK || fno.fname[0] == 0) { break; }
		  if(fno.fname[0] == '.') { continue; }

		  fn = fno.fname;

		  if(FileList.ptr < FILEMGR_LIST_DEPDTH){
			  if((fno.fattrib & AM_DIR) == 0){
				  if((strstr(fn, "wav")) || (strstr(fn, "WAV"))){
					  strncpy((char *)FileList.file[FileList.ptr].name, (char *)fn, FILEMGR_FILE_NAME_SIZE);
					  FileList.file[FileList.ptr].type = FILETYPE_FILE;
					  FileList.ptr++;
				  }
			  }
		  }
	  }
  }

  NumObs = FileList.ptr;
  f_closedir(&dir);
  return res;
}

uint16_t GetObjectNumber(void){
	if (StorageParse() != FR_OK)
		return -1;
	return NumObs;
}


