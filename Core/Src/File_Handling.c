#include "stm32f4xx_hal.h"
#include "File_Handling.h"

#include "usb_host.h"
#include <string.h>
#include <stdio.h>

//#include "STM_flash.h"
//#include <machine/endian.h>

unsigned char buffer4[4];
#define LITTLE_TO_BIG_ENDIAN(buff)   (buff[0] |(buff[1]<<8) | (buff[2]<<16) | (buff[3]<<24));

//#include "retarget.h"

extern FILELIST_FileTypeDef FileList;
extern ApplicationTypeDef Appli_state;
USBH_HandleTypeDef hUSBHost;
uint16_t NumObs = 0;


/* ===============>>>>>>>> NO CHANGES AFTER THIS LINE ========================>>>>>>> */
extern const char error_list[20][66];

extern uint8_t retUSBH; /* Return value for USBH */
extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */

FILINFO USBHfno;
FRESULT USB_fresult;  // result
UINT USB_br, USB_bw;  // File read/write count

/**** capacity related *****/
FATFS *pUSBHFatFS;
DWORD fre_clust;
uint32_t USB_total, USB_free_space;

/********************************************************************************/
extern UART_HandleTypeDef huart3;
#define UART &huart3
void Send_Uart (char *string){
	HAL_UART_Transmit(UART, (uint8_t *)string, strlen(string), HAL_MAX_DELAY);
}
/********************************************************************************/

int Mount_USB (void){
	if ((USB_fresult = f_mount(&USBHFatFS, USBHPath, 0)) != FR_OK){
		USB_fresult < 21 ? printf("\r\n>USB: Result: (%s) \r\n",error_list[USB_fresult]) : printf(">USB: fresult: %d \r\n", USB_fresult);
		  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
		  return 1;
	  }
	  else {
		  printf("\r\n>USB:(%s) USB STICK MOUNTED!\r\n", error_list[USB_fresult]);
		  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
		  return 0;
	  }
}

int Unmount_USB (void){
	if ((USB_fresult = f_mount(NULL, USBHPath, 1)) != FR_OK){
		  USB_fresult < 21 ? printf("\r\n>USB: Result: (%s) \r\n",error_list[USB_fresult]) : printf(">USB: fresult: %d \r\n", USB_fresult);
		  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
		  return 1;
	  }
	  else {
		  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
		  printf("\r\n>USB:(%s) USB STICK UNMOUNTED! \r\n", error_list[USB_fresult]);
		  return 0;
	  }
}

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_USB (char* pat){
    DIR dir;
    UINT i;
    char *path = malloc(50*sizeof (char));
    sprintf (path, "%s",pat);

    /* Open the directory */
    if (( USB_fresult = f_opendir(&dir, path)) == FR_OK){
    	for (;;){
             /* Read a directory item */
            if ((USB_fresult = f_readdir(&dir, &USBHfno)) != FR_OK || USBHfno.fname[0] == 0)
            	break;  /* Break on error or end of dir */

            /* It is a directory */
            if (USBHfno.fattrib & AM_DIR)     {
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname)))
            		continue;
            	if (!(strcmp("System Volume Information", USBHfno.fname)))
            		continue;

            	printf("\r\n");
            	printf(">USB:Dir: %s\r\n", USBHfno.fname);

                i = strlen(path);
                sprintf(&path[i], "/%s", USBHfno.fname);

                /* Enter the directory */
                if ((USB_fresult = Scan_USB(path)) != FR_OK)
                	break;
                path[i] = 0;
            }
            else{   /* It is a file. */
               printf("    %3dKB  File Name: \"%s/%s\"\r\n",(int)USBHfno.fsize/1024, path, USBHfno.fname);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return USB_fresult;
}

/* Only supports removing files from home directory */
FRESULT Format_USB (void){
    DIR dir;
    char *path = malloc(30*sizeof (char));
    sprintf (path, "%s",USBHPath);

    /* Open the directory */
    if ((USB_fresult = f_opendir(&dir, path)) == FR_OK) {
        for (;;){
            /* Read a directory item */
            if ((USB_fresult = f_readdir(&dir, &USBHfno)) != FR_OK || USBHfno.fname[0] == 0)
            	break;  /* Break on error or end of dir */

            /* It is a directory */
            if (USBHfno.fattrib & AM_DIR){
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname)))
            		continue;
            	if (!(strcmp("System Volume Information", USBHfno.fname)))
            		continue;
            	if ((USB_fresult = f_unlink(USBHfno.fname)) == FR_DENIED)
            		continue;
            }
            /* It is a file. */
            else{
               sprintf(path,"%s/%s",USBHPath, USBHfno.fname);
               USB_fresult = f_unlink(USBHfno.fname);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return USB_fresult;
}


FRESULT Write_File (char *name, char *data){

	/**** check whether the file exists or not ****/
	if ((USB_fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
	    return USB_fresult;
	}

	else{
	    /* Create a file with read write access and open it */
	    if ((USB_fresult = f_open(&USBHFile, name, FA_OPEN_EXISTING | FA_WRITE)) != FR_OK){
	    	printf ("\r\nERROR!!! No. %d in opening file *%s*\r\n", USB_fresult, name);
	        return USB_fresult;
	    }

	    else{
	    	printf ("\r\nOpening file-->  *%s*  To WRITE data in it\r\n", name);

	    	if ((USB_fresult = f_write(&USBHFile, data, strlen(data), &USB_bw)) != FR_OK){
	    		printf ("\r\nERROR!!! No. %d while writing to the FILE *%s*\r\n", USB_fresult, name);
	    	}

	    	/* Close file */
	    	if ((USB_fresult = f_close(&USBHFile)) != FR_OK){
	    		printf ("\r\nERROR!!! No. %d in closing file *%s* after writing it\r\n", USB_fresult, name);
	    	}

	    	else{
	    		printf ("\r\nFile *%s* is WRITTEN and CLOSED successfully\r\n", name);
	    	}
	    }
	    return USB_fresult;
	}
}





FRESULT Read_File (char *name){
	/**** check whether the file exists or not ****/

	if ((USB_fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf(">USB:ERRROR!!! *%s* does not exists\r\n", name);
	    return USB_fresult;
	}
	else{
/********************************************************************************/
/* Open file to read */
		if ((USB_fresult = f_open(&USBHFile, name, FA_OPEN_ALWAYS|FA_READ)) != FR_OK){
			printf(">USB:ERROR!!! No. %d in opening file *%s*\r\n", USB_fresult, name);
		    return USB_fresult;
		}
		else {
	    	printf (">USB:FILE OPENED => \"%s\" to READ data from this file\r\n", name);
		}

//char* bufferX = 0x080186A0;
/* Read data from the file
   see the function details for the arguments */
/********************************************************************************/
		#define BUFFER_SIZE 4096

	    if(f_size(&USBHFile) == 0 || f_size(&USBHFile) < BUFFER_SIZE){
	    	printf(">USB:\"%s\" \tfile size: %d Byte INFO:\"not enough buffer size is %d byte\"\r\n", name, (int)f_size(&USBHFile), (int)BUFFER_SIZE);
	    	printf(">USB:Dinamic Memory will be allocated Size:%d\r\n", (int)f_size(&USBHFile));

	    	BYTE *small_buffer;

	    	if((small_buffer = (BYTE*)malloc(f_size(&USBHFile)*sizeof(BYTE))) == NULL){
	    		printf(">USB:Dinamic Memory is not allocated\r\n");
	    	}
	    	else {
		    	printf(">USB:Dinamic address: %p \r\n", small_buffer);
		    	while (!f_eof(&USBHFile)){
		    		memset((void*)small_buffer, 0 , f_size(&USBHFile));
					if((USB_fresult = f_read(&USBHFile, small_buffer, (int)f_size(&USBHFile), &USB_br)) != FR_OK){
						printf("\r\n>USB:Read Error \r\n");
						break;
					}
		    		if(!USB_br) break;
		    		printf("\r\n**********************************************************\r\n");

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
		    		printf("\r\n**********************************************************\r\n");
		    	}
	    	}
/*********************************************************************************/
	    }
	    else{

  		  	BYTE * pbuffer;

  		    if((pbuffer = (BYTE*)malloc(BUFFER_SIZE*sizeof(BYTE))) == NULL){
  		    	printf(">USB:Dinamic Memory is not allocated\r\n");
  		    }
  		    else{
				int i ,k;
				for (k = 0; k < f_size(&USBHFile)/BUFFER_SIZE; k++){
				  if((USB_fresult = f_read(&USBHFile, pbuffer, BUFFER_SIZE, &USB_br)) != FR_OK){
					  printf("\r\n>USB : Read Error \r\n");
					  break;
				  }

				  /*V.1*/
				  for(i = 0; i < BUFFER_SIZE ; i++){
					  if(!k && !i)
						  printf("%08X ", 0);
						  //printf("%08X ", 15);
					  if(k || i){
						printf(" ");
						if(!(i % 16)){
							printf(" \r\n%08X ", i + (k*BUFFER_SIZE));
						}
					  }
					  printf("%02X", *(BYTE*)(pbuffer + i));
				  }
				  memset(pbuffer, 0, BUFFER_SIZE);
				  f_lseek(&USBHFile, (k + 1) * BUFFER_SIZE);
				}
				free((void*)pbuffer);
				printf("\r\n");
  		    }
	    }

/********************************************************************************/
/* Close file */
		if ((USB_fresult = f_close(&USBHFile)) != FR_OK){
			printf ("\r\n>USB:ERROR!!! No. %d in closing file *%s*\r\n", USB_fresult, name);
		}
		else{
			printf ("\r\n>USB:\"%s\" FILE CLOSED\r\n", name);
		}

	    return USB_fresult;
	}
}


FRESULT Create_File (char *name){

	if ((USB_fresult = f_stat (name, &USBHfno)) == FR_OK){
		printf ("\r\nERROR!!! *%s* already exists!!!!\n use Update_File \n\n",name);
	    return USB_fresult;
	}
	else{
		if ((USB_fresult = f_open(&USBHFile, name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE)) != FR_OK){
			printf ("\r\nERROR!!! No. %d in creating file *%s*\r\n", USB_fresult, name);
		    return USB_fresult;
		}else{
			printf ("\r\n*%s* created successfully\n Now use Write_File to write data\r\n",name);
		}

		if ((USB_fresult = f_close(&USBHFile)) != FR_OK){
			printf ("\r\nERROR No. %d in closing file *%s*\r\n", USB_fresult, name);
		}else{
			printf ("\r\nFile *%s* CLOSED successfully\r\n", name);
		}
	}
    return USB_fresult;
}

FRESULT Update_File (char *name, char *data){

	/**** check whether the file exists or not ****/
	if ((USB_fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
	    return USB_fresult;
	}
	else{
	/* Create a file with read write access and open it */
	    if ((USB_fresult = f_open(&USBHFile, name, FA_OPEN_APPEND | FA_WRITE)) != FR_OK){
	    	printf ("\r\nERROR!!! No. %d in opening file *%s*\r\n", USB_fresult, name);
	        return USB_fresult;
	    }

	    else{
			printf ("\r\nOpening file-->  *%s*  To UPDATE data in it\r\n", name);

		/* Writing text */
			if ((USB_fresult = f_write(&USBHFile, data, strlen (data), &USB_bw)) != FR_OK){
				printf ("\r\nERROR!!! No. %d in writing file *%s*\r\n", USB_fresult, name);
			}else {
				printf ("\r\n*%s* UPDATED successfully\r\n", name);
			}

		/* Close file */
			if ((USB_fresult = f_close(&USBHFile)) != FR_OK){
				printf ("\r\nERROR!!! No. %d in closing file *%s*\r\n", USB_fresult, name);
			}else {
				printf ("\r\nFile *%s* CLOSED successfully\r\n", name);
			}
	    }
	}
    return USB_fresult;
}

FRESULT Remove_File (char *name){
	/**** check whether the file exists or not ****/
	if ((USB_fresult = f_stat (name, &USBHfno)) != FR_OK){
		printf ("\r\nERROR!!! *%s* does not exists\r\n", name);
		return USB_fresult;
	}
	else{
		if ((USB_fresult = f_unlink (name)) == FR_OK){
			printf ("\r\n*%s* has been removed successfully\r\n", name);
		}else{
			printf ("\r\nERROR No. %d in removing *%s*\r\n",USB_fresult, name);
		}
	}
	return USB_fresult;
}

FRESULT Create_Dir (char *name){

	DIR dir;
	if((USB_fresult = f_opendir(&dir, name)) != FR_OK){
	    if ((USB_fresult = f_mkdir(name)) != FR_OK){
			USB_fresult < 21 ? printf("\r\n>USB: Result: (%s) \r\n",error_list[USB_fresult]) : printf(">USB : fresult: %d \r\n", USB_fresult);
	    	return USB_fresult;
	    }
	    else{
	    	printf ("\r\n>USB:(%s) File created successfully\r\n", error_list[USB_fresult]);
	    }
	    return USB_fresult;
	}
	return USB_fresult;
}

void Check_USB_Details (void){
    /* Check free space */
	if ((USB_fresult = f_getfree(USBHPath, &fre_clust, &pUSBHFatFS)) != FR_OK){
		USB_fresult < 21 ? printf(">USB: fresult: %s \r\n",error_list[USB_fresult]) :
				  printf(">USB: fresult: %d \r\n", USB_fresult);
	}
	else {
	    USB_total = (uint32_t)((pUSBHFatFS->n_fatent - 2) * pUSBHFatFS->csize * 0.5);
	    printf (">USB:Total Size:%.2fMB\r\n",(float)((USB_total/1024)));

	    USB_free_space = (uint32_t)(fre_clust * pUSBHFatFS->csize * 0.5);
	    printf (">USB:Free Space:%.2fMB\r\n",(float)((USB_free_space/1024)));
	}
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


