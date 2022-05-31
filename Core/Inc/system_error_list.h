/*
 * error_list.h
 *
 *  Created on: May 26, 2022
 *      Author: arge12
 */

#ifndef INC_SYSTEM_ERROR_LIST_H_
#define INC_SYSTEM_ERROR_LIST_H_

const char error_list [20][66]  = {
		{ "Succeeded"},
		{ "A hard error occurred in the low level disk I/O layer"},
		{ "Assertion failed"},
		{ "The physical drive cannot work"},
		{ "Could not find the file"},
		{ "Could not find the pathSD"},
		{ "The pathSD name format is invalid"},
		{ "Access denied due to prohibited access or directory full"},
		{ "Access denied due to prohibited access"},
		{ "The file,directory object is invalid"},
		{ "The physical drive is write protected"},
		{ "The logical drive number is invalid"},
		{ "The volume has no work area"},
		{ "There is no valid FAT volume"},
		{ "The f_mkfs() aborted due to any problem"},
		{ "Could not get a grant to access the volume within defined period"},
		{ "The operation is rejected according to the file sharing policy"},
		{ "LFN working buffer could not be allocated"},
		{ "Number of open files > _FS_LOCK"},
		{ "Given parameter is invalid"},
};



#endif /* INC_SYSTEM_ERROR_LIST_H_ */
