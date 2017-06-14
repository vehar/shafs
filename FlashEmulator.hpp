//FlashEmulator
#ifndef _FLASH_EMULATOR_H_
#define _FLASH_EMULATOR_H_

#include "stdio.h"
#include "stdint.h"
#include <string.h>
#include <windows.h>
#include <fstream>
#include <iostream>

#include "shafs.h"

char filePath[];

#ifdef __cplusplus
extern "C" {
#endif
	
	void FlashLowLevelWrite(uint8_t *FlashPageBuff, uint32_t FlashWrAddr, uint16_t numBytes);
	void FlashLowLevelRead(uint8_t *FlashPageBuff, uint32_t FlashRdAddr, uint16_t numBytes);

	void FlashEmyInit(void);
	void FlashEmyWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
	void FlashEmyRead (uint8_t* pBuffer, uint32_t ReadeAddr, uint16_t NumByteToRead);

#ifdef __cplusplus
}
#endif

#endif //_FLASH_EMULATOR_H_
 