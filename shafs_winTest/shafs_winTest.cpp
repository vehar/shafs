// shafs_winTest.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include <windows.h>
#include <fstream>

#include "shafs\FlashEmulator.hpp"
#include "shafs\shafs.hpp"

using namespace std;

//ofstream ftout(filePath, std::ofstream::out | std::ofstream::app);
//ifstream ftin(filePath, std::ofstream::in | std::ofstream::app);

shafsFile_t sf;

#define TEST_ARR_SZ (256*16*16*4)

uint8_t arr[TEST_ARR_SZ] = {1,2,3,4,5,6,7,8,9,10,11};

void TestSet(uint8_t val)
{
	if(val != 0xFF){memset(arr, val, TEST_ARR_SZ);}
	else{for(int i = 0; i < TEST_ARR_SZ; i++){arr[i] = i;}}
}

int main(int argc, char* argv[])
{
	printf("Start\r\n");
	//*
	FlashEmyInit();

	TestSet(0);
	sf.name = 0xA1;
	sf.lenght = 551; //0x14
	shafs_write(sf, arr);

	TestSet(2);
	sf.name = 0xA2;
	sf.lenght = 253; //7
	shafs_write(sf, arr);

	
	TestSet(1);
	TestSet(0x11);
	sf.name = 0xA1;
	sf.lenght = 943; //0x28
	shafs_write(sf, arr);



	TestSet(0x22);
	sf.name = 0xA2;
	sf.lenght = 253; //0x0A
	shafs_write(sf, arr);

	TestSet(1);
	sf.name = 0xA1;
	sf.lenght = 5; //5
	shafs_write(sf, arr);

	FlashEmyRead(arr, 10, 5000);
/*	TestSet(5);
	sf.name = 0xA5;
	sf.lenght = 50; //0x32
	shafs_write(sf, arr);

	TestSet(1);
	sf.name = 0xA1;
	sf.lenght = 890; //0x32
	shafs_write(sf, arr);

	TestSet(5);
	sf.name = 0xA5;
	sf.lenght = 50; //0x32
	shafs_write(sf, arr);
//*/
 
/*
	memset(arr, 0, TEST_ARR_SZ);
	sf.name = 111;
	sf.lenght = 500; //0x23
	shafs_read(sf, arr);
//*/

//	ftout.open(filePath, std::ofstream::out | std::ofstream::in);
//	ftout.close();
/*
	ifstream ftin(filePath, std::ofstream::in | std::ofstream::ate);
	ftin.open(filePath,  std::ofstream::in);
	ftin.clear(); // reset flags 
	ftin.seekg(0,ios::beg);
	ftin.read((char*)arr, 1000);
	ftin.close();
*/

	return 0;  
}
/*
	//FlashEmyWrite(TestBuffIn, 3, 5);
	//FlashEmyRead(TestBuffIn, 3, 40);

    for (int ix = 0; ix < 64; ++ix){BIT_SET_VAL(arr,1,ix);}
	for (int ix = 0; ix < 64; ++ix){if((ix%8) == 0){printf("***\n");}; printf("bit %d is %d\n", ix, BIT_TEST(arr, ix));}
*/
