/*
 * STM_flash.c
 *
 *  Created on: 31 Ara 2021
 *      Author: arge12
 */

#include "STM_flash.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"


#define  MIN_ADDRESS_SECTOR0 0x08000000
#define  MAX_ADDRESS_SECTOR0 0x08003FFF

#define  MIN_ADDRESS_SECTOR1 0x08004000
#define  MAX_ADDRESS_SECTOR1 0x08007FFF

#define  MIN_ADDRESS_SECTOR2 0x08008000
#define  MAX_ADDRESS_SECTOR2 0x0800BFFF

#define  MIN_ADDRESS_SECTOR3 0x0800C000
#define  MAX_ADDRESS_SECTOR3 0x0800FFFF

#define  MIN_ADDRESS_SECTOR4 0x08010000		// AVAS voice 0x0801 86A0
#define  MAX_ADDRESS_SECTOR4 0x0801FFFF

#define  MIN_ADDRESS_SECTOR5 0x08020000
#define  MAX_ADDRESS_SECTOR5 0x0803FFFF

#define  MIN_ADDRESS_SECTOR6 0x08040000
#define  MAX_ADDRESS_SECTOR6 0x0805FFFF

#define  MIN_ADDRESS_SECTOR7 0x08060000
#define  MAX_ADDRESS_SECTOR7 0x0807FFFF

#define  MIN_ADDRESS_SECTOR8 0x08080000
#define  MAX_ADDRESS_SECTOR8 0x0809FFFF

#define  MIN_ADDRESS_SECTOR9 0x080A0000
#define  MAX_ADDRESS_SECTOR9 0x080BFFFF

#define  MIN_ADDRESS_SECTOR10 0x080C0000
#define  MAX_ADDRESS_SECTOR10 0x080DFFFF

#define  MIN_ADDRESS_SECTOR11 0x080E0000
#define  MAX_ADDRESS_SECTOR11 0x080FFFFF

static uint32_t GetSector(uint32_t Sectoraddress){
	uint32_t sector = 0;

	if(Sectoraddress < MAX_ADDRESS_SECTOR0 && Sectoraddress >= MIN_ADDRESS_SECTOR0){
		sector = FLASH_SECTOR_0;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR1 && Sectoraddress >= MIN_ADDRESS_SECTOR1){
		sector = FLASH_SECTOR_1;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR2 && Sectoraddress >= MIN_ADDRESS_SECTOR2){
		sector = FLASH_SECTOR_2;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR3 && Sectoraddress >= MIN_ADDRESS_SECTOR3){
		sector = FLASH_SECTOR_3;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR4 && Sectoraddress >= MIN_ADDRESS_SECTOR4){
		sector = FLASH_SECTOR_4;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR5 && Sectoraddress >= MIN_ADDRESS_SECTOR5){
		sector = FLASH_SECTOR_5;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR6 && Sectoraddress >= MIN_ADDRESS_SECTOR6){
		sector = FLASH_SECTOR_6;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR7 && Sectoraddress >= MIN_ADDRESS_SECTOR7){
		sector = FLASH_SECTOR_7;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR8 && Sectoraddress >= MIN_ADDRESS_SECTOR8){
		sector = FLASH_SECTOR_8;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR9 && Sectoraddress >= MIN_ADDRESS_SECTOR9){
		sector = FLASH_SECTOR_9;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR10 && Sectoraddress >= MIN_ADDRESS_SECTOR10){
		sector = FLASH_SECTOR_10;
	}
	else if(Sectoraddress < MAX_ADDRESS_SECTOR11 && Sectoraddress >= MIN_ADDRESS_SECTOR11){
		sector = FLASH_SECTOR_11;
	}

return sector;
}


uint8_t bytes_temp[4];
void float2Byte ( uint8_t* ftoa_bytes_temp, float float_variable){
	union {
		float a;
		uint8_t bytes[4];
	}thing;

	thing.a = float_variable;

	for(int i = 0; i < 4 ; i++){
		ftoa_bytes_temp[i] = thing.bytes[i];
	}
}


float Byte2float(uint32_t* ftoa_bytes_temp){
	union{
		float a;
		uint8_t bytes[4];
	}thing;

	for(int i = 0; i < 4; i++){
		thing.bytes[i] = ftoa_bytes_temp[i];
	}

//	float float_variable =  thing.a;
//	return float_variable;

	return thing.a;
}

uint32_t write_STM32_Flash(uint32_t StartSectorAddress, uint32_t* data, uint16_t numberofwords){

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTORError;
	int sofar = 0;

	/*Unlock the flash to enable the flash control register  to access */
	HAL_FLASH_Unlock();
		//1) Erase the user Flash area
		//2) Get the number of sector to erase from 1.st sector

	uint32_t StartSector		 = GetSector(StartSectorAddress);
	uint32_t EndSector_Address   = StartSector + numberofwords * 4;   //4 bytes
	uint32_t EndSector	     	 = GetSector(EndSector_Address);

	/*Fill EraseInit Structer*/

	EraseInitStruct.TypeErase 		= FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange 	= FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector			= StartSector;
	EraseInitStruct.NbSectors		= (EndSector-StartSector)+1;

	  /* Note: If an erase operation in Flash memory also concerns data in the data
	   *  or instruction cache, you have to make sure that these data are rewritten
	   *  before they are accessed during code execution.
	   *  If this cannot be done safely, it is recommended to flush the caches by
	   *  setting the DCRST and ICRST bits in the FLASH_CR register.
	  */

	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK){
		return HAL_FLASH_GetError();
	}

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR)
	  */

	while (sofar < numberofwords){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartSectorAddress, data[sofar]) == HAL_OK){
			StartSectorAddress += 4;  // 4 bytes forward WORD
			// use StartPageAddress += 2 for half word(H-WORD) and 8 for double word (DWORD)
			sofar++;
		}
		else {
			return HAL_FLASH_GetError(); /* Error occurred while writing data in Flash memory*/
		}
	}

	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/

	HAL_FLASH_Lock();

	  return 0;
}

void read_STM32_Flash(uint32_t StartSectorAddress, uint32_t* RxBuffer, uint16_t numberofwords){

	while(1){
		*RxBuffer = *(__IO uint32_t*) StartSectorAddress;  // __IO is volatile
		StartSectorAddress += 4;
		RxBuffer++;

		if(!(numberofwords--))
			break;
	}
}


void Conver_To_Str(uint32_t* Data, char* Buffer){

	int numberofbytes = ((strlen((char*)Data)/ 4) + ((strlen((char*)Data) % 4) != 0)) * 4;

	for(int i = 0; i < numberofbytes; i++){
		Buffer[i] = Data[i / 4] >> (8 * (i % 4));
	}
}

void write_STM32_FLASH_NUM(uint32_t StartSectorAddress, float Number){
	float2Byte(bytes_temp, Number);
	write_STM32_Flash(StartSectorAddress, (uint32_t*)bytes_temp, 1);
}

float read_STM32_FLASH_NUM(uint32_t StartSectorAddress){
	uint8_t buffer[4];
	float value;
	read_STM32_Flash(StartSectorAddress,(uint32_t*)buffer, 1);

	return value = Byte2float((uint32_t *)buffer);
}
