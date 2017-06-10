#include "shafs.hpp"

#ifdef _WIN32
	#include "FlashEmulator.hpp"
#else
	#include "w25x20.h"
#endif 

#define SHAFS_RAM_VOLUME (10)
shafsHndl_t RamFs[SHAFS_RAM_VOLUME]; //image
shafsChunkHeader_t sh_wr;

//1 - clear; 0 - dirty
uint8_t chunkState[CHUNK_AMOUNT_TO_BIT]; //1bit per chunk
uint8_t FlashPageBuff[CHUNK_SIZE];

void FlashLowLevelWrite(uint8_t *FlashPageBuff, uint32_t FlashWrAddr, uint16_t numBytes)
{
#ifdef _WIN32
	 FlashEmyWrite(FlashPageBuff, FlashWrAddr,numBytes);
#else
	SPI_Flash_Write_Page(FlashPageBuff, FlashWrAddr,numBytes);
#endif 
}

void FlashLowLevelRead(uint8_t *FlashPageBuff, uint32_t FlashRdAddr, uint16_t numBytes)
{
#ifdef _WIN32
	 FlashEmyRead(FlashPageBuff, FlashRdAddr,numBytes);
#else
	SPI_Flash_Read(FlashPageBuff, FlashRdAddr,numBytes);
#endif 
}


void shafs_init(void)
{
	memset(chunkState, 0xFF, CHUNK_AMOUNT_TO_BIT);
}

void shafs_SetChunkClear(uint16_t num)
{
	BIT_SET_VAL(chunkState, num, 1);
}
void shafs_SetChunkDirty(uint16_t num)
{
	BIT_SET_VAL(chunkState, num, 0);
}
uint8_t shafs_GetChunkState(uint16_t num)
{
	return BIT_TEST(chunkState, num); //Checking a bit
}

/*
Changing the nth bit to x
Setting the nth bit to either 1 or 0 can be achieved with the following:
number ^= (-x ^ number) & (1 << n);
Bit n will be set if x is 1, and cleared if x is 0.
*/

/*
// a=target variable, b=bit number to act upon 0-n 
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

// x=target variable, y=mask 
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) (((x) & (y)) == (y))

*/

uint32_t LastWrAddr = 0;

//TODO:
//дописать аллокатор и поиск чистого сдедующего адреса
uint8_t shafsRamTailIdx = 0;

uint32_t shafs_openFile(shafsFile_t file)
{
	uint32_t retval;
	uint32_t f_index = 0; 

	for(f_index = 0; f_index < shafsRamTailIdx; f_index++)
	{
		if(RamFs[f_index].file.name == file.name)
		{
			retval = f_index; //return index of file struct in ram
			goto EXIT;
		} //file exist! 
	}
			 //else register file in ram 
	if(RamFs[shafsRamTailIdx].file.name == 0x00)
	{
		RamFs[shafsRamTailIdx].file = file;
		RamFs[shafsRamTailIdx].ramIdx = shafsRamTailIdx;
		RamFs[shafsRamTailIdx].file.lenght = 0; //no written data yet
		//find free space and calc adr
		if(shafsRamTailIdx == 0)
		{
			RamFs[shafsRamTailIdx].startAddrPhy = 0; //temp
			RamFs[shafsRamTailIdx].endAddrPhy = 0; //temp
		}
		else
		{
			RamFs[shafsRamTailIdx].startAddrPhy = LastWrAddr; //prev
			RamFs[shafsRamTailIdx].endAddrPhy = 0; //temp
		}
		retval = shafsRamTailIdx; //return index of file struct in ram
		
		shafsRamTailIdx++;
	}
EXIT:
	return retval;
}

void shafs_read(shafsFile_t file, uint8_t* data)
{
	uint32_t i = shafs_openFile(file);
	uint32_t startAddrPhy = RamFs[i].startAddrPhy;
	
	FlashLowLevelRead(data, startAddrPhy, file.lenght);
}

shafsHndl_t tmpRamFs;

uint16_t lastChunk = 0;

void shafs_write(shafsFile_t file, uint8_t* data)
{
	uint16_t chunkNeeded = 0;
	uint32_t FlashWrAddr = 0;
	uint16_t chunkCntToWrite = 0;
	uint8_t  dataInChunk = 0;
	uint32_t i = 0;
	
	if((file.lenght == 0) || (file.lenght >= TOTAL_SIZE)) return;// check for valid lenght of file
	
	i = shafs_openFile(file); //if ok - first open the file
	
	tmpRamFs = RamFs[i];
	tmpRamFs.file.lenght += file.lenght;

	// calc addr to start writing
	if(tmpRamFs.endAddrPhy)//existing, previously written file
	{
		//check next free space to write
		if(shafs_GetChunkState(lastChunk) == 1)//if next space is free
		{
			FlashWrAddr = tmpRamFs.endAddrPhy; //continue writing 
			//mark prev header as "obsolete"
			// ...
		}
		else //need to finf new free space
		{
			// ...
			FlashWrAddr = LastWrAddr; //jump to next free space
		}
		
	}
	else //new file
	{		
		FlashWrAddr = tmpRamFs.startAddrPhy; // == the end of prev file
	}
int t = MAX_DATA_PER_CHUNK; //244
int e = SHAFS_HEADER_SIZE; //

	//calc full and partial chunks to write all data
	chunkNeeded = file.lenght/MAX_DATA_PER_CHUNK;
	sh_wr.chunkRemain = file.lenght % MAX_DATA_PER_CHUNK;
	
	//set header attributes
	sh_wr.header = FLASH_FILE_ACTIVE;
	sh_wr.fileName = file.name;
	

	//sh_wr.totalChunks = chunkNeeded+1;
	
	do{
		//sh_wr.currentChunk = chunkCntToWrite; //start write from zero chunk
		sh_wr.prevChunk = (tmpRamFs.endAddrPhy/256) ;		
		//calc data volume to write in current chunk
		if(chunkNeeded == 0)
		{
			if(sh_wr.chunkRemain == 0){dataInChunk = sh_wr.chunkRemain;}// less then MAX_DATA_PER_CHUNK
			else if(sh_wr.chunkRemain > 0){dataInChunk = sh_wr.chunkRemain;}// more than 1 chunk was
		}		
		else if(chunkNeeded > 0)
		{
			dataInChunk = MAX_DATA_PER_CHUNK; //write full chunk
		}
		

		memcpy(FlashPageBuff, data/*+FlashWrAddr*/, dataInChunk); // write data to chunk
		memcpy(FlashPageBuff+dataInChunk, &sh_wr, SHAFS_HEADER_SIZE);// write header 		

		FlashLowLevelWrite(FlashPageBuff, FlashWrAddr, dataInChunk+SHAFS_HEADER_SIZE);
		
		//chunkCntToWrite++;

		shafs_SetChunkDirty(lastChunk);
		lastChunk++;

		//FIX!!!
		FlashWrAddr += (MAX_DATA_PER_CHUNK+SHAFS_HEADER_SIZE); //Set wr adr to next chunk

	LastWrAddr = FlashWrAddr;
	tmpRamFs.endAddrPhy = FlashWrAddr; //save end file adress

	}while(chunkNeeded--);
	
	RamFs[i] = tmpRamFs;

	memset(FlashPageBuff, 0, CHUNK_SIZE);
}