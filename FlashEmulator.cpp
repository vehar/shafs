#include "FlashEmulator.hpp"
char filePath[] = "A:\\flashEmu.txt";
 
using namespace std;
ofstream fout(filePath);

void FlashEmyInit(void)
{
	ofstream fout(filePath);
 // allocate memory for file content
  char* buffer = new char[TOTAL_SIZE]; //2MB
  //for(long i = 0; i<TOTAL_SIZE; i++ ){buffer[i] = i;}
  memset(buffer, 0xFF, TOTAL_SIZE);
  fout.write (buffer,TOTAL_SIZE);
  fout.close();
  delete buffer;
}

void FlashEmyWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	
	fout.open(filePath, std::ofstream::out | std::ofstream::in);
	fout.clear(); // reset flags 
	fout.seekp(WriteAddr, ios::beg);
	fout.write((char*)pBuffer, NumByteToWrite);
	//delete pBuffer;
	fout.close();
}

void FlashEmyRead(uint8_t* pBuffer, uint32_t ReadeAddr, uint16_t NumByteToRead)
{
	ifstream fin(filePath);
	fin.open(filePath);
	fin.clear(); // reset flags 
	fin.seekg(ReadeAddr,ios::beg);
	fin.read((char*)pBuffer, NumByteToRead);
	fin.close();
}


