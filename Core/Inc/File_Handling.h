/*
 * File_Handling_RTOS.h
 *
 *  Created on: 30-April-2020
 *      Author: Controllerstech
 */

#ifndef FILE_HANDLING_H_
#define FILE_HANDLING_H_

#include "fatfs.h"
#include "string.h"
#include "stdio.h"

#define FILEMGR_LIST_DEPDTH                        24
#define FILEMGR_FILE_NAME_SIZE                     40
#define FILEMGR_FULL_PATH_SIZE                     256
#define FILEMGR_MAX_LEVEL                          4
#define FILETYPE_DIR                               0
#define FILETYPE_FILE                              1

typedef struct _FILELIST_LineTypeDef {
  uint8_t type;
  uint8_t name[FILEMGR_FILE_NAME_SIZE];
}FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
  FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
  uint16_t              ptr;
}FILELIST_FileTypeDef;


typedef struct {
  uint32_t ChunkID;       /* 0 */
  uint32_t FileSize;      /* 4 */
  uint32_t FileFormat;    /* 8 */
  uint32_t SubChunk1ID;   /* 12 */
  uint32_t SubChunk1Size; /* 16*/
  uint16_t AudioFormat;   /* 20 */
  uint16_t NbrChannels;   /* 22 */
  uint32_t SampleRate;    /* 24 */

  uint32_t ByteRate;      /* 28 */
  uint16_t BlockAlign;    /* 32 */
  uint16_t BitPerSample;  /* 34 */
  uint32_t SubChunk2ID;   /* 36 */
  uint32_t SubChunk2Size; /* 40 */
}WAVE_FormatTypeDef;


uint16_t GetObjectNumber(void);

/* mounts the USB*/
int Mount_USB (void);

/* unmounts the USB*/
int Unmount_USB (void);

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_USB (char* pat);

/* Only supports removing files from home directory. Directory remover to be added soon */
FRESULT Format_USB (void);

/* write the data to the file
 * @ name : is the path to the file*/
FRESULT Write_File (char *name, char *data);

/* read data from the file
 * @ name : is the path to the file*/
FRESULT Read_File (char *name);

/* creates the file, if it does not exists
 * @ name : is the path to the file*/
FRESULT Create_File (char *name);

/* Removes the file from the USB
 * @ name : is the path to the file*/
FRESULT Remove_File (char *name);

/* creates a directory
 * @ name: is the path to the directory
 */
FRESULT Create_Dir (char *name);

/* checks the free space in the USB*/
void Check_USB_Details (void);

/* updates the file. write pointer is set to the end of the file
 * @ name : is the path to the file
 */
FRESULT Update_File (char *name, char *data);

char * Get_FilenameUart (void);
void Send_Uart (char *string);




#endif /* FILE_HANDLING_RTOS_H_ */
