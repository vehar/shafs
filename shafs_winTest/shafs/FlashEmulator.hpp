//FlashEmulator
#ifndef _FLASH_EMULATOR_H_
#define _FLASH_EMULATOR_H_

#include "stdio.h"
#include "stdint.h"
#include <string.h>
#include <windows.h>
#include <fstream>
#include <iostream>

#define NUM_CHUNKS_PER_SECTOR (16)
#define NUM_SECTOR_PER_BLOCK (16)
#define NUM_OF_BLOCKS (4)

#define CHUNK_SIZE (256) //= PAGE SIZE
#define SECTOR_SIZE (CHUNK_SIZE*NUM_CHUNKS_PER_SECTOR)
#define BLOCK_SIZE (SECTOR_SIZE*NUM_SECTOR_PER_BLOCK)
#define TOTAL_SIZE (BLOCK_SIZE*NUM_OF_BLOCKS)

char filePath[];

void FlashLowLevelWrite(uint8_t *FlashPageBuff, uint32_t FlashWrAddr, uint16_t numBytes);
void FlashLowLevelRead(uint8_t *FlashPageBuff, uint32_t FlashRdAddr, uint16_t numBytes);

void FlashEmyInit(void);
void FlashEmyWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void FlashEmyRead (uint8_t* pBuffer, uint32_t ReadeAddr, uint16_t NumByteToRead);


#endif //_FLASH_EMULATOR_H_
