/*
 * STM_flash.h
 *
 *  Created on: 31 Ara 2021
 *      Author: emre öztoklu
 */

#ifndef INC_STM_FLASH_H_
#define INC_STM_FLASH_H_

#include <stdint.h>


uint32_t 	write_STM32_Flash		(uint32_t StartSectorAddress, uint32_t* data, uint16_t numberofwords);
void  		read_STM32_Flash		(uint32_t StartSectorAddress, uint32_t* RxBuffer, uint16_t numberofwords);
void  		write_STM32_FLASH_NUM	(uint32_t StartSectorAddress, float Number);
float 		read_STM32_FLASH_NUM	(uint32_t StartSectorAddress);
void  		Conver_To_Str			(uint32_t* Data, char* Buffer);

#endif /* INC_STM_FLASH_H_ */




