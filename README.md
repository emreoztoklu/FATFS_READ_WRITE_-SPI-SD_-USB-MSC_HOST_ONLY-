# USB 

This file is about How to read files data on USB over STM32F407 Discovery Kit

  - Compiler IDE: CubeIDE  (Framework v1.26 has issuse because of that it will be good choice if you create firsttime file with CubeMX)
  
  - OTG Mass Storage Class
  - FatFile System
  - Systick (168 Mhz)
  - USB_OTG_FS
  - retarget folder fir (printf and scanf functions)
  - Debug Test: used UART3  
 
 
>SD : fresult: (0) Succeeded  
>SD : CARD size is calculating 
>SD : total size is : 7.89 GB 
>SD : Free Space is : 7.89 GB 
Dir: ERA
1://ERA/emre.txt size:0 KB 
1://ERA/sample.bin size:296 KB 
1://File1.bin size:0 KB 
1://file1.txt size:0 KB 
1://foto.png size:79 KB 
1://sample.bin size:296 KB 
1://write_sample.bin size:296 KB 


>SD : File system type (0:N/A) : 3
>SD : Physical drive number : 1
>SD : Number of FATs (1 or 2) : 2
>SD : win[] flag (b0:dirty) : 0
>SD : FSINFO flags (b7:disabled, b0:dirty) : 0
>SD : File system mount ID : 1
>SD : Number of root directory entries (FAT12/16) : 0
>SD : Cluster size [sectors] : 8
>SD : Last allocated cluster : 252
>SD : Number of free clusters : 2068230
>SD : Number of FAT entries (number of clusters + 2) : 2068482
>SD : Size of an FAT [sectors] : 16161
>SD : Volume base sector : 2048
>SD : FAT base sector : 2494
>SD : Root directory base sector/cluster : 2
>SD : Data base sector : 34816
>SD : Current sector appearing in the win[] : 34816
>SD : File handler is opened in Read & Write Mode
>SD : File1.txt created and data will be written  byte size is : 41 
>SD : File is closed!
>SD : File handler is opened to read mode
>SD : File handler is opened to write mode
Dir: ERA

>SD : Read data from file total buf size 4096bytes
0000000F 80 FF 80 FF 82 FF 82 FF 83 FF 80 FF 88 FF 84 FF  
00000010 8B FF 89 FF 8C FF 8B FF 8C FF 92 FF 94 FF 90 FF  
00000020 93 FF 93 FF 94 FF 96 FF 95 FF 96 FF 94 FF 92 FF  
00000030 9A FF 96 FF 9A FF 96 FF 98 FF 95 FF 90 FF 92 FF  

 
 
/****************************************************/
  
 
00049FD0 18 0A B0 0A 8E 0A 3A 0A 3B 0A 80 09 C1 09 B5 09  
00049FE0 D2 08 1A 08 AA 07 B9 08 54 09 45 09 B9 08 BF 07  
00049FF0 EC 07 90 08 86 09 20 0A 00 0A B3 09 85 09 5B 0A


File *0://USBERA/USBsample.bin* CLOSED successfully

	File: 0://USBfoto.png  79 KB
	File: 0://USBsample.bin  296 KB


>USB:Dir: USBERA
	File: 0://USBERA/USBemre.txt  0 KB
	File: 0://USBERA/USBsample.bin  296 KB
	File: 0://USBFile1.bin  0 KB
	File: 0://USBfile1.txt  0 KB

Enter a any file name to read from exist files:



