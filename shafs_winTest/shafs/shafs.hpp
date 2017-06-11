
#include "stdint.h"
#include <string.h>

#ifdef _WIN32
#include "FlashEmulator.hpp"
#else
#include "w25x20.h"
#endif 


#ifndef _SHAFS_H_
#define _SHAFS_H_

//---------------------------------------------BIT_OPERATIONS-----------------------------------------------------
#define BITS 8
#define BIT_SET(  p, n) (p[(n)/BITS] |=  (0x80>>((n)%BITS)))
#define BIT_CLEAR(p, n) (p[(n)/BITS] &= ~(0x80>>((n)%BITS)))

#define BIT_SET_VAL(arr, n, state) (arr[(n)/BITS] = arr[(n)/BITS] & ((~(1 << ((n)%BITS)))) | (state << ((n)%BITS)))
#define BIT_TEST(arr, n)           ( ( ((const char*)&(arr))[(n)>>3] & 0x80 >> ((n)&0x07)) >> (7-((n)&0x07) ) )
/*
//Sample usage:
    unsigned char arr[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    for (int ix = 0; ix < 64; ++ix){printf("bit %d is %d\n", ix, BIT_TEST(arr, ix));}
*/
//---------------------------------------------BIT_OPERATIONS-----------------------------------------------------


#define NUM_CHUNKS_PER_SECTOR (16)
#define NUM_SECTOR_PER_BLOCK (16)
#define NUM_OF_BLOCKS (4)

#define CHUNK_SIZE (256) //= PAGE SIZE
#define SECTOR_SIZE (CHUNK_SIZE*NUM_CHUNKS_PER_SECTOR)
#define BLOCK_SIZE (SECTOR_SIZE*NUM_SECTOR_PER_BLOCK)
#define TOTAL_SIZE (BLOCK_SIZE*NUM_OF_BLOCKS)

#define TOTAL_CHUNKS (TOTAL_SIZE/CHUNK_SIZE)
#define CHUNK_AMOUNT_TO_BIT (TOTAL_CHUNKS/8) //1bit per chunk


enum CHUNK_STATE
{
	FILE_DELETED = 0,
	FLASH_FILE_ACTIVE = 0x5555,
	RAM_FILE_ACTIVE = 0x11
};
	
typedef struct 
{
	//uint16_t currentChunk; 
	uint16_t prevChunk; //!!
	uint8_t chunkRemain;
	//uint16_t totalChunks;
	uint8_t fileName;
	uint16_t header;
	
} shafsChunkHeader_t; 

typedef struct 
{
	uint8_t  name;
	uint32_t lenght;
} shafsFile_t; 

typedef struct 
{
shafsFile_t file;
	uint8_t  ramIdx;
	uint32_t startAddrPhy;
	uint32_t endAddrPhy;
} shafsHndl_t; 


#define SHAFS_HEADER_SIZE (sizeof(shafsChunkHeader_t))
#define MAX_DATA_PER_CHUNK (CHUNK_SIZE-SHAFS_HEADER_SIZE)	

void shafs_init(void);
uint16_t shafs_getFreeSpace(void);
void shafs_write(shafsFile_t file, uint8_t* data);
void shafs_read(shafsFile_t file, uint8_t* data);

#endif //_SHAFS_H_

