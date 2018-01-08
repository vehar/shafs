#include "shafs.h"

#ifdef _WIN32
	#include "FlashEmulator.hpp"
#else
	#include "w25x20.h"
#endif 

#define SHAFS_RAM_VOLUME (5) //suport for n different files
shafsHndl_t RamFs[SHAFS_RAM_VOLUME]; //image

shafsChunkHeader_t sh_wr;

//1 - clear; 0 - dirty
uint8_t chunkState[CHUNK_AMOUNT_TO_BIT]; //1bit per chunk
uint8_t FlashPageBuff[CHUNK_SIZE];
uint8_t shafsRamTailIdx = 0;
uint32_t LastWrAddr = 0;

void FlashLowLevelWrite(uint8_t *PageBuff, uint32_t FlashWrAddr, uint16_t numBytes)
{
#ifdef _WIN32
	 FlashEmyWrite(PageBuff, FlashWrAddr,numBytes);
#else
	SPI_Flash_Write_Page(FlashPageBuff, FlashWrAddr,numBytes);
#endif 
}

void FlashLowLevelRead(uint8_t *PageBuff, uint32_t FlashRdAddr, uint16_t numBytes)
{
#ifdef _WIN32
	 FlashEmyRead(PageBuff, FlashRdAddr,numBytes);
#else
	SPI_Flash_Read(FlashPageBuff, FlashRdAddr,numBytes);
#endif 
}


void shafs_eraze(void)
{
#ifdef _WIN32
	memset(chunkState, 0xFF, CHUNK_AMOUNT_TO_BIT);
#endif
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

uint8_t shafs_SearchFreeChunk(void)
{
	uint8_t num = 0;
	for(uint8_t i = 0; i < CHUNK_AMOUNT_TO_BIT; i++)
	{
		if(shafs_GetChunkState(i) == 0){num = i; break;}
	}
	return num;
}
// exec on start for load in RAM entire structure


uint16_t shafs_getFreeSpace(void)
{
	uint16_t chunkCnt = 0;
	for(uint16_t i = 0; i < TOTAL_CHUNKS; i++)
	{
		if(shafs_GetChunkState(i) == 1){chunkCnt++;}
	}
return chunkCnt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t shafs_scan(void)
{
	uint32_t byteAmount = 0;
	uint16_t chunkCnt = 0;
	int16_t chunkIdx = 0;
	uint16_t idx = 0;
	uint32_t totalFileSize = 0;
	uint8_t error = 0;
	uint32_t startAddrPhy = 0;
	uint32_t endAddrPhy = 0;

	uint8_t tName = 0;
	uint16_t tLenght = 0;

	startAddrPhy = 0;

	uint8_t chunkArr[CHUNK_SIZE];
	byteAmount = CHUNK_SIZE;

//search for chunk chain
do
{
	memset(chunkArr, 0, CHUNK_SIZE);
	FlashLowLevelRead(chunkArr, startAddrPhy*CHUNK_SIZE, byteAmount);
	startAddrPhy++;

	//search for chunk header to restore names+length of files
	for(idx = 0; idx < CHUNK_SIZE; idx++)
	{
		if(chunkArr[idx] == (FLASH_FILE_ACTIVE & 0xFF))
		{
			if (chunkArr[idx+1] == (FLASH_FILE_ACTIVE & 0xFF)) //Header valid
			{
				tName = chunkArr[idx-1];
				tLenght = chunkArr[idx-2];
				
				shafs_SetChunkDirty(chunkCnt);
				chunkCnt++;
				LastWrAddr += chunkCnt*CHUNK_SIZE;
				error = 0;

				//search for unic name in ramFs arr
				for(uint8_t i = 0; i < SHAFS_RAM_VOLUME; i++)
				{
					if(RamFs[i].file.name == tName)// existing file
					{
						RamFs[i].file.lenght += tLenght;
						RamFs[i].endAddrPhy = (startAddrPhy)*CHUNK_SIZE;
						goto exit;
					}
				}

				//search for free space to add new file
				for(uint8_t i = 0; i < SHAFS_RAM_VOLUME; i++)
				{
					if(RamFs[i].file.name == 0)// existing file
					{
						RamFs[i].file.name = tName;
						RamFs[i].file.lenght = tLenght;
						RamFs[i].endAddrPhy = (startAddrPhy)*CHUNK_SIZE; //Set wr adr to next chunk ;tLenght
						RamFs[i].ramIdx = i;
						shafsRamTailIdx++;
						goto exit;
					}
				}
		}
		}
		else
		{
				error = 1; //no header found in chunk
		}
	}
	
exit: while(0);

}while(startAddrPhy <= TOTAL_CHUNKS); //search all chunks

return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////


//TODO:
//дописать аллокатор и поиск чистого сдедующего адреса

uint32_t shafs_openFile(shafsFile_t file)
{
	uint32_t retval = 0;
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

uint8_t pBuff[CHUNK_SIZE*10];

uint8_t shafs_read(shafsFile_t file, uint8_t* data)
{
	uint32_t byteAmount = 0;
	uint16_t chunkCnt = 0;
	int16_t chunkIdx = 0;
	uint16_t idx = 0;
	uint32_t totalFileSize = 0;
	uint8_t tBuff[CHUNK_SIZE];
	uint8_t error = 0;
	uint32_t startAddrPhy = 0;

	uint32_t i = shafs_openFile(file);

	startAddrPhy = RamFs[i].endAddrPhy - CHUNK_SIZE; //set read ptr to start of last file chunk
	
	uint8_t chunkArr[CHUNK_SIZE];
	shafsFile_t chunkIdxArr[CHUNK_SIZE];
	byteAmount = CHUNK_SIZE;

//search for chunk chain
do
{
	memset(chunkArr, 0, CHUNK_SIZE);
	FlashLowLevelRead(chunkArr, startAddrPhy, byteAmount);

	//search for 1-st chunk header to restore chain of file chunks
	for(idx = 0; idx < CHUNK_SIZE; idx++)
	{
		if(chunkArr[idx] == (FLASH_FILE_ACTIVE & 0xFF))
		{
			if (chunkArr[idx+1] == (FLASH_FILE_ACTIVE & 0xFF)) //Header valid
			{
				error = 0;
				chunkIdxArr[chunkCnt].name = startAddrPhy/CHUNK_SIZE;// save chunk idx
				chunkIdxArr[chunkCnt].lenght = chunkArr[idx-2];
				chunkCnt++;
				totalFileSize += chunkArr[idx-2];
				break;
			};
		}
		else
		{
			error = 1; //no header found in 1-st chunk
		}
	}

	chunkIdx = (chunkArr[idx-4] + (chunkArr[idx-3]<<8)) - 1;
	if(error){return -1;} //something wrong vs FLASH or RamFs image
	startAddrPhy = chunkIdx * CHUNK_SIZE;

}while(chunkIdx>=0);

chunkCnt -= 1;
byteAmount = 0;
uint32_t t = 0;
do
{
	t += byteAmount;
	startAddrPhy = (chunkIdxArr[chunkCnt].name) * CHUNK_SIZE;
	byteAmount = chunkIdxArr[chunkCnt].lenght;
	FlashLowLevelRead(tBuff, startAddrPhy, byteAmount);

	for(uint32_t i = 0; i < byteAmount; i++) //PATCH:
	{
		pBuff[i+t] = tBuff[i];
	}

}while(chunkCnt--);

memcpy(data, pBuff, totalFileSize);
return 0;
}


shafsHndl_t tmpRamFs;



uint8_t shafs_write(shafsFile_t file, uint8_t* data)
{
	uint16_t chunkNeeded = 0;
	uint32_t FlashWrAddr = 0;
	uint16_t chunkCntToWrite = 0;
	uint8_t  dataInChunk = 0;
	uint32_t i = 0;
	uint16_t lastChunk = LastWrAddr/CHUNK_SIZE;


	if((file.lenght == 0) || (file.lenght >= TOTAL_SIZE)) return -1;// check for valid lenght of file
	
	i = shafs_openFile(file); //if ok - first open the file
	
	tmpRamFs = RamFs[i];
	tmpRamFs.file.lenght += file.lenght;

	// calc addr to start writing
	if(tmpRamFs.endAddrPhy)//existing, previously written file
	{
		//check next free space to write
		if(shafs_GetChunkState(lastChunk) == 1)//if next space is free
		{
			//FlashWrAddr = tmpRamFs.endAddrPhy; //continue writing 
			FlashWrAddr = LastWrAddr; //jump to next free space
			//mark prev header as "obsolete"
			// ...
		}
		else //need to find new free space
		{
			// ...
			FlashWrAddr = LastWrAddr; //jump to next free space
			//FlashWrAddr = shafs_SearchFreeChunk() * CHUNK_SIZE;
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

		//if write a full chunk - mark it
		if(chunkNeeded != 0){sh_wr.chunkRemain = MAX_DATA_PER_CHUNK;}
		else{sh_wr.chunkRemain = file.lenght % MAX_DATA_PER_CHUNK;};

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

	return 0;
}