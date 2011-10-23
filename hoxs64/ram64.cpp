#include <windows.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "hexconv.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"

//bit8 vic_de00_byte;


RAM64::RAM64()
{
	Zero64MemoryPointers();
}

RAM64::~RAM64()
{
	Free64Memory();
}

/*
void RAM64::Reset()
{
int i;
	for (i=0 ; i<=0xFFFF ; i++)
	{
		if ((i & 64)==0)
			mMemory[i] = 0;
		else
			mMemory[i] = 255;
	}
	for (i=0 ; i<1024 ; i++)
	{
		mColorRAM[i] = 0;
	}
}
*/

void RAM64::Reset()
{
int i;
	for (i=0 ; i<=0xFFFF ; i++)
	{
		switch((i & 0x0380) >> 7)
		{
		case 0:
			if ((i & 127) == 0)
			{
				mMemory[i] = G::Rand(0, 255);
			}
			else
				mMemory[i] = 0xff;
			break;
		case 1:
			mMemory[i] = 0;
			break;
		case 2:
			if ((i & 127) == 0)
			{
				mMemory[i] = G::Rand(0, 255);
			}
			else
				mMemory[i] = 0xff;
			break;
		case 3:
			mMemory[i] = 0;
			break;
		case 4:
			if ((i & 127) == 0)
			{
				mMemory[i] = G::Rand(0, 255);
			}
			else
				if ((i & 0x8000) == 0)
					mMemory[i] = 0x99;
				else
					mMemory[i] = 0x66;
				//mMemory[i] = 0xFF;
			break;
		case 5:
			if ((i & 0x8000) == 0)
				mMemory[i] = 0x99;
			else
				mMemory[i] = 0x66;
			//mMemory[i] = 0x00;
			break;
		case 6:
			if ((i & 127) == 0)
			{
				mMemory[i] = G::Rand(0, 255);
			}
			else
			if ((i & 0x8000) == 0)
				mMemory[i] = 0x99;
			else
				mMemory[i] = 0x66;
				//mMemory[i] = 0xFF;
			break;
		case 7:
			if ((i & 0x8000) == 0)
				mMemory[i] = 0x99;
			else
				mMemory[i] = 0x66;
			//mMemory[i] = 0x00;
			break;
		}
	}
	for (i=0 ; i<1024 ; i++)
	{
		mColorRAM[i] = 0;
	}

	LoadResetPattern();
}


HRESULT RAM64::Init(TCHAR *szAppDirectory)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read;
TCHAR szRomPath[MAX_PATH+1];
	ClearError();

	if (szAppDirectory==NULL)
		m_szAppDirectory[0] = 0;
	else
		_tcscpy_s(m_szAppDirectory, _countof(m_szAppDirectory), szAppDirectory);

	if (S_OK != Allocate64Memory())
		return E_FAIL;

	hfile=INVALID_HANDLE_VALUE;
	szRomPath[0]=0;
	if (_tmakepath_s(szRomPath, _countof(szRomPath), 0, m_szAppDirectory, TEXT("kernal.rom"), 0)==0)
		hfile=CreateFile(szRomPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		hfile=CreateFile(TEXT("kernal.rom"),GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		return SetError(E_FAIL, TEXT("Could not open kernal.rom"));
	r=ReadFile(hfile, mKernal, 0x2000, &bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0)
		return SetError(E_FAIL, TEXT("Could not read from kernal.rom"));
	if (bytes_read!=0x2000)
		return SetError(E_FAIL, TEXT("Could not read 0x2000 bytes from kernal.rom"));

	hfile=INVALID_HANDLE_VALUE;
	szRomPath[0]=0;
	if (_tmakepath_s(szRomPath, _countof(szRomPath), 0, m_szAppDirectory, TEXT("basic.rom"), 0)==0)
		hfile=CreateFile(szRomPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		hfile=CreateFile(TEXT("basic.rom"),GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		return SetError(E_FAIL, TEXT("Could not open basic.rom"));
	r=ReadFile(hfile, mBasic,0x2000, &bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0)
		return SetError(E_FAIL, TEXT("Could not read from basic.rom"));
	if (bytes_read!=0x2000)
		return SetError(E_FAIL, TEXT("Could not read 0x2000 bytes from basic.rom"));

	hfile=INVALID_HANDLE_VALUE;
	szRomPath[0]=0;
	if (_tmakepath_s(szRomPath, _countof(szRomPath), 0, m_szAppDirectory, TEXT("char.rom"), 0)==0)
		hfile=CreateFile(szRomPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		hfile=CreateFile(TEXT("char.rom"),GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		return SetError(E_FAIL, TEXT("Could not open char.rom"));
	r=ReadFile(hfile,mCharGen,0x1000,&bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0)
		return SetError(E_FAIL, TEXT("Could not read from char.rom"));
	if (bytes_read!=0x1000)
		return SetError(E_FAIL, TEXT("Could not read 0x1000 bytes from char.rom"));

	InitMMU();

	return S_OK;
}

void RAM64::LoadResetPattern()
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read;
TCHAR szRomPath[MAX_PATH+1];
	hfile=INVALID_HANDLE_VALUE;
	szRomPath[0]=0;
	if (_tmakepath_s(szRomPath, _countof(szRomPath), 0, m_szAppDirectory, TEXT("c64.ram"), 0)==0)
		hfile=CreateFile(szRomPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		hfile=CreateFile(TEXT("c64.ram"),GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 

	if (hfile!=INVALID_HANDLE_VALUE)
	{		
		r = ReadFile(hfile, mMemory, 0x10000, &bytes_read, NULL);
		CloseHandle(hfile);
	}
}

HRESULT RAM64::Allocate64Memory(){
bool r=S_OK;
	Free64Memory();
	if (NULL != (mMemory =(bit8 *) VirtualAlloc(NULL, (64 + 8 + 8 + 4 + 4)*1024, MEM_COMMIT, PAGE_READWRITE))){
		mKernal = mMemory+0x10000;
		mBasic = mKernal+0x2000;
		mIO = mBasic+0x2000;
		mCharGen = mIO+0x1000;

		return S_OK;
	}
	Free64Memory();
	return SetError(E_FAIL, TEXT("Memory allocation failed"));;
}

void RAM64::Free64Memory(){
	if(mMemory)
		VirtualFree(mMemory, 0, MEM_RELEASE);
	Zero64MemoryPointers();
}

void RAM64::Zero64MemoryPointers(){
	m_iCurrentCpuMmuIndex=0;
	mMemory=0L;
	mKernal=0L;
	mBasic=0L;
	mIO=0L;
	mCharGen=0L;
	mColorRAM;

	miMemory=0L;
	miKernal=0L;
	miBasic=0L;
	miIO=0L;
	miCharGen=0L;
}

int RAM64::GetCurrentCpuMmuMemoryMap()
{
	return m_iCurrentCpuMmuIndex;
}

bit8 *RAM64::GetCpuMmuIndexedPointer(MEM_TYPE mt)
{
	switch (mt)
	{
	case MT_RAM:
		return miMemory;
	case MT_IO:
		return 0;
	case MT_CHARGEN:
		return miCharGen;
	case MT_BASIC:
		return miBasic;
	case MT_KERNAL:
		return miKernal;
	default:
		return miMemory;
	}
}

MEM_TYPE RAM64::GetCpuMmuReadMemoryType(bit16 address, int memorymap)
{
	if (memorymap < 0)//Use the MMU
		return MMU_MT_read[m_iCurrentCpuMmuIndex][address >> 12];
	else
		return MMU_MT_read[memorymap & 0x1f][address >> 12];
}

MEM_TYPE RAM64::GetCpuMmuWriteMemoryType(bit16 address, int memorymap)
{
	if (memorymap < 0)//Use the MMU
		return MMU_MT_write[m_iCurrentCpuMmuIndex & 0x1f][address >> 12];
	else
		return MMU_MT_write[memorymap & 0x1f][address >> 12];
}

void RAM64::InitMMU_0()
{
int i,j;
	MMU_MT_read[0x1F][0xF]=MT_KERNAL;
	MMU_MT_read[0x1F][0xE]=MT_KERNAL;
	MMU_MT_read[0x1F][0xD]=MT_IO;
	MMU_MT_read[0x1F][0xC]=MT_RAM;
	MMU_MT_read[0x1F][0xB]=MT_BASIC;
	MMU_MT_read[0x1F][0xA]=MT_BASIC;
	MMU_MT_read[0x1F][0x9]=MT_RAM;
	MMU_MT_read[0x1F][0x8]=MT_RAM;
	MMU_MT_read[0x1F][0x7]=MT_RAM;
	MMU_MT_read[0x1F][0x6]=MT_RAM;
	MMU_MT_read[0x1F][0x5]=MT_RAM;
	MMU_MT_read[0x1F][0x4]=MT_RAM;
	MMU_MT_read[0x1F][0x3]=MT_RAM;
	MMU_MT_read[0x1F][0x2]=MT_RAM;
	MMU_MT_read[0x1F][0x1]=MT_RAM;
	MMU_MT_read[0x1F][0x0]=MT_RAM;
	//01111	#1
	MMU_MT_read[0x0F][0xF]=MT_KERNAL;
	MMU_MT_read[0x0F][0xE]=MT_KERNAL;
	MMU_MT_read[0x0F][0xD]=MT_CHARGEN;
	MMU_MT_read[0x0F][0xC]=MT_RAM;
	MMU_MT_read[0x0F][0xB]=MT_BASIC;
	MMU_MT_read[0x0F][0xA]=MT_BASIC;
	MMU_MT_read[0x0F][0x9]=MT_RAM;
	MMU_MT_read[0x0F][0x8]=MT_RAM;
	MMU_MT_read[0x0F][0x7]=MT_RAM;
	MMU_MT_read[0x0F][0x6]=MT_RAM;
	MMU_MT_read[0x0F][0x5]=MT_RAM;
	MMU_MT_read[0x0F][0x4]=MT_RAM;
	MMU_MT_read[0x0F][0x3]=MT_RAM;
	MMU_MT_read[0x0F][0x2]=MT_RAM;
	MMU_MT_read[0x0F][0x1]=MT_RAM;
	MMU_MT_read[0x0F][0x0]=MT_RAM;


	for (i=0 ; i<=0x1f ; i++){

		for (j=0 ; j<=0xf ; j++)
		{
			if (i!=0x1f && i!=0xf)
				if ((i & 0x10)!=0)
					MMU_MT_read[i][j]=MMU_MT_read[0x1f][j];
				else
					MMU_MT_read[i][j]=MMU_MT_read[0x0f][j];
		}
	}

	//11010 #2
	MMU_MT_read[0x1A][0xF]=MT_RAM;
	MMU_MT_read[0x1A][0xE]=MT_RAM;
	MMU_MT_read[0x1A][0xD]=MT_IO;
	MMU_MT_read[0x1A][0xC]=MT_RAM;
	MMU_MT_read[0x1A][0xB]=MT_RAM;
	MMU_MT_read[0x1A][0xA]=MT_RAM;
	MMU_MT_read[0x1A][0x9]=MT_RAM;
	MMU_MT_read[0x1A][0x8]=MT_RAM;
	MMU_MT_read[0x1A][0x7]=MT_RAM;
	MMU_MT_read[0x1A][0x6]=MT_RAM;
	MMU_MT_read[0x1A][0x5]=MT_RAM;
	MMU_MT_read[0x1A][0x4]=MT_RAM;
	MMU_MT_read[0x1A][0x3]=MT_RAM;
	MMU_MT_read[0x1A][0x2]=MT_RAM;
	MMU_MT_read[0x1A][0x1]=MT_RAM;
	MMU_MT_read[0x1A][0x0]=MT_RAM;
	//01010 #2
	MMU_MT_read[0x0A][0xF]=MT_RAM;
	MMU_MT_read[0x0A][0xE]=MT_RAM;
	MMU_MT_read[0x0A][0xD]=MT_CHARGEN;
	MMU_MT_read[0x0A][0xC]=MT_RAM;
	MMU_MT_read[0x0A][0xB]=MT_RAM;
	MMU_MT_read[0x0A][0xA]=MT_RAM;
	MMU_MT_read[0x0A][0x9]=MT_RAM;
	MMU_MT_read[0x0A][0x8]=MT_RAM;
	MMU_MT_read[0x0A][0x7]=MT_RAM;
	MMU_MT_read[0x0A][0x6]=MT_RAM;
	MMU_MT_read[0x0A][0x5]=MT_RAM;
	MMU_MT_read[0x0A][0x4]=MT_RAM;
	MMU_MT_read[0x0A][0x3]=MT_RAM;
	MMU_MT_read[0x0A][0x2]=MT_RAM;
	MMU_MT_read[0x0A][0x1]=MT_RAM;
	MMU_MT_read[0x0A][0x0]=MT_RAM;

	//11011 #2
	MMU_MT_read[0x1B][0xF]=MT_RAM;
	MMU_MT_read[0x1B][0xE]=MT_RAM;
	MMU_MT_read[0x1B][0xD]=MT_IO;
	MMU_MT_read[0x1B][0xC]=MT_RAM;
	MMU_MT_read[0x1B][0xB]=MT_RAM;
	MMU_MT_read[0x1B][0xA]=MT_RAM;
	MMU_MT_read[0x1B][0x9]=MT_RAM;
	MMU_MT_read[0x1B][0x8]=MT_RAM;
	MMU_MT_read[0x1B][0x7]=MT_RAM;
	MMU_MT_read[0x1B][0x6]=MT_RAM;
	MMU_MT_read[0x1B][0x5]=MT_RAM;
	MMU_MT_read[0x1B][0x4]=MT_RAM;
	MMU_MT_read[0x1B][0x3]=MT_RAM;
	MMU_MT_read[0x1B][0x2]=MT_RAM;
	MMU_MT_read[0x1B][0x1]=MT_RAM;
	MMU_MT_read[0x1B][0x0]=MT_RAM;
	//01011 #2
	MMU_MT_read[0x0B][0xF]=MT_RAM;
	MMU_MT_read[0x0B][0xE]=MT_RAM;
	MMU_MT_read[0x0B][0xD]=MT_CHARGEN;
	MMU_MT_read[0x0B][0xC]=MT_RAM;
	MMU_MT_read[0x0B][0xB]=MT_RAM;
	MMU_MT_read[0x0B][0xA]=MT_RAM;
	MMU_MT_read[0x0B][0x9]=MT_RAM;
	MMU_MT_read[0x0B][0x8]=MT_RAM;
	MMU_MT_read[0x0B][0x7]=MT_RAM;
	MMU_MT_read[0x0B][0x6]=MT_RAM;
	MMU_MT_read[0x0B][0x5]=MT_RAM;
	MMU_MT_read[0x0B][0x4]=MT_RAM;
	MMU_MT_read[0x0B][0x3]=MT_RAM;
	MMU_MT_read[0x0B][0x2]=MT_RAM;
	MMU_MT_read[0x0B][0x1]=MT_RAM;
	MMU_MT_read[0x0B][0x0]=MT_RAM;


	//11000 #3
	MMU_MT_read[0x18][0xF]=MT_RAM;
	MMU_MT_read[0x18][0xE]=MT_RAM;
	MMU_MT_read[0x18][0xD]=MT_RAM;
	MMU_MT_read[0x18][0xC]=MT_RAM;
	MMU_MT_read[0x18][0xB]=MT_RAM;
	MMU_MT_read[0x18][0xA]=MT_RAM;
	MMU_MT_read[0x18][0x9]=MT_RAM;
	MMU_MT_read[0x18][0x8]=MT_RAM;
	MMU_MT_read[0x18][0x7]=MT_RAM;
	MMU_MT_read[0x18][0x6]=MT_RAM;
	MMU_MT_read[0x18][0x5]=MT_RAM;
	MMU_MT_read[0x18][0x4]=MT_RAM;
	MMU_MT_read[0x18][0x3]=MT_RAM;
	MMU_MT_read[0x18][0x2]=MT_RAM;
	MMU_MT_read[0x18][0x1]=MT_RAM;
	MMU_MT_read[0x18][0x0]=MT_RAM;
	//01000 #3
	MMU_MT_read[0x08][0xF]=MT_RAM;
	MMU_MT_read[0x08][0xE]=MT_RAM;
	MMU_MT_read[0x08][0xD]=MT_CHARGEN;
	MMU_MT_read[0x08][0xC]=MT_RAM;
	MMU_MT_read[0x08][0xB]=MT_RAM;
	MMU_MT_read[0x08][0xA]=MT_RAM;
	MMU_MT_read[0x08][0x9]=MT_RAM;
	MMU_MT_read[0x08][0x8]=MT_RAM;
	MMU_MT_read[0x08][0x7]=MT_RAM;
	MMU_MT_read[0x08][0x6]=MT_RAM;
	MMU_MT_read[0x08][0x5]=MT_RAM;
	MMU_MT_read[0x08][0x4]=MT_RAM;
	MMU_MT_read[0x08][0x3]=MT_RAM;
	MMU_MT_read[0x08][0x2]=MT_RAM;
	MMU_MT_read[0x08][0x1]=MT_RAM;
	MMU_MT_read[0x08][0x0]=MT_RAM;


	//10110 #4
	MMU_MT_read[0x16][0xF]=MT_KERNAL;
	MMU_MT_read[0x16][0xE]=MT_KERNAL;
	MMU_MT_read[0x16][0xD]=MT_IO;
	MMU_MT_read[0x16][0xC]=MT_RAM;
	MMU_MT_read[0x16][0xB]=MT_RAM;
	MMU_MT_read[0x16][0xA]=MT_RAM;
	MMU_MT_read[0x16][0x9]=MT_RAM;
	MMU_MT_read[0x16][0x8]=MT_RAM;
	MMU_MT_read[0x16][0x7]=MT_RAM;
	MMU_MT_read[0x16][0x6]=MT_RAM;
	MMU_MT_read[0x16][0x5]=MT_RAM;
	MMU_MT_read[0x16][0x4]=MT_RAM;
	MMU_MT_read[0x16][0x3]=MT_RAM;
	MMU_MT_read[0x16][0x2]=MT_RAM;
	MMU_MT_read[0x16][0x1]=MT_RAM;
	MMU_MT_read[0x16][0x0]=MT_RAM;
	//00110 #4
	MMU_MT_read[0x06][0xF]=MT_KERNAL;
	MMU_MT_read[0x06][0xE]=MT_KERNAL;
	MMU_MT_read[0x06][0xD]=MT_CHARGEN;
	MMU_MT_read[0x06][0xC]=MT_RAM;
	MMU_MT_read[0x06][0xB]=MT_RAM;
	MMU_MT_read[0x06][0xA]=MT_RAM;
	MMU_MT_read[0x06][0x9]=MT_RAM;
	MMU_MT_read[0x06][0x8]=MT_RAM;
	MMU_MT_read[0x06][0x7]=MT_RAM;
	MMU_MT_read[0x06][0x6]=MT_RAM;
	MMU_MT_read[0x06][0x5]=MT_RAM;
	MMU_MT_read[0x06][0x4]=MT_RAM;
	MMU_MT_read[0x06][0x3]=MT_RAM;
	MMU_MT_read[0x06][0x2]=MT_RAM;
	MMU_MT_read[0x06][0x1]=MT_RAM;
	MMU_MT_read[0x06][0x0]=MT_RAM;


	//10111 #4
	MMU_MT_read[0x17][0xF]=MT_KERNAL;
	MMU_MT_read[0x17][0xE]=MT_KERNAL;
	MMU_MT_read[0x17][0xD]=MT_IO;
	MMU_MT_read[0x17][0xC]=MT_RAM;
	MMU_MT_read[0x17][0xB]=MT_RAM;
	MMU_MT_read[0x17][0xA]=MT_RAM;
	MMU_MT_read[0x17][0x9]=MT_RAM;
	MMU_MT_read[0x17][0x8]=MT_RAM;
	MMU_MT_read[0x17][0x7]=MT_RAM;
	MMU_MT_read[0x17][0x6]=MT_RAM;
	MMU_MT_read[0x17][0x5]=MT_RAM;
	MMU_MT_read[0x17][0x4]=MT_RAM;
	MMU_MT_read[0x17][0x3]=MT_RAM;
	MMU_MT_read[0x17][0x2]=MT_RAM;
	MMU_MT_read[0x17][0x1]=MT_RAM;
	MMU_MT_read[0x17][0x0]=MT_RAM;
	//00111 #4
	MMU_MT_read[0x07][0xF]=MT_KERNAL;
	MMU_MT_read[0x07][0xE]=MT_KERNAL;
	MMU_MT_read[0x07][0xD]=MT_CHARGEN;
	MMU_MT_read[0x07][0xC]=MT_RAM;
	MMU_MT_read[0x07][0xB]=MT_RAM;
	MMU_MT_read[0x07][0xA]=MT_RAM;
	MMU_MT_read[0x07][0x9]=MT_RAM;
	MMU_MT_read[0x07][0x8]=MT_RAM;
	MMU_MT_read[0x07][0x7]=MT_RAM;
	MMU_MT_read[0x07][0x6]=MT_RAM;
	MMU_MT_read[0x07][0x5]=MT_RAM;
	MMU_MT_read[0x07][0x4]=MT_RAM;
	MMU_MT_read[0x07][0x3]=MT_RAM;
	MMU_MT_read[0x07][0x2]=MT_RAM;
	MMU_MT_read[0x07][0x1]=MT_RAM;
	MMU_MT_read[0x07][0x0]=MT_RAM;


	//10010 #5
	MMU_MT_read[0x12][0xF]=MT_RAM;
	MMU_MT_read[0x12][0xE]=MT_RAM;
	MMU_MT_read[0x12][0xD]=MT_RAM;
	MMU_MT_read[0x12][0xC]=MT_RAM;
	MMU_MT_read[0x12][0xB]=MT_RAM;
	MMU_MT_read[0x12][0xA]=MT_RAM;
	MMU_MT_read[0x12][0x9]=MT_RAM;
	MMU_MT_read[0x12][0x8]=MT_RAM;
	MMU_MT_read[0x12][0x7]=MT_RAM;
	MMU_MT_read[0x12][0x6]=MT_RAM;
	MMU_MT_read[0x12][0x5]=MT_RAM;
	MMU_MT_read[0x12][0x4]=MT_RAM;
	MMU_MT_read[0x12][0x3]=MT_RAM;
	MMU_MT_read[0x12][0x2]=MT_RAM;
	MMU_MT_read[0x12][0x1]=MT_RAM;
	MMU_MT_read[0x12][0x0]=MT_RAM;
	//00010 #5
	MMU_MT_read[0x02][0xF]=MT_RAM;
	MMU_MT_read[0x02][0xE]=MT_RAM;
	MMU_MT_read[0x02][0xD]=MT_RAM;
	MMU_MT_read[0x02][0xC]=MT_RAM;
	MMU_MT_read[0x02][0xB]=MT_RAM;
	MMU_MT_read[0x02][0xA]=MT_RAM;
	MMU_MT_read[0x02][0x9]=MT_RAM;
	MMU_MT_read[0x02][0x8]=MT_RAM;
	MMU_MT_read[0x02][0x7]=MT_RAM;
	MMU_MT_read[0x02][0x6]=MT_RAM;
	MMU_MT_read[0x02][0x5]=MT_RAM;
	MMU_MT_read[0x02][0x4]=MT_RAM;
	MMU_MT_read[0x02][0x3]=MT_RAM;
	MMU_MT_read[0x02][0x2]=MT_RAM;
	MMU_MT_read[0x02][0x1]=MT_RAM;
	MMU_MT_read[0x02][0x0]=MT_RAM;


	//10011 #5
	MMU_MT_read[0x13][0xF]=MT_RAM;
	MMU_MT_read[0x13][0xE]=MT_RAM;
	MMU_MT_read[0x13][0xD]=MT_RAM;
	MMU_MT_read[0x13][0xC]=MT_RAM;
	MMU_MT_read[0x13][0xB]=MT_RAM;
	MMU_MT_read[0x13][0xA]=MT_RAM;
	MMU_MT_read[0x13][0x9]=MT_RAM;
	MMU_MT_read[0x13][0x8]=MT_RAM;
	MMU_MT_read[0x13][0x7]=MT_RAM;
	MMU_MT_read[0x13][0x6]=MT_RAM;
	MMU_MT_read[0x13][0x5]=MT_RAM;
	MMU_MT_read[0x13][0x4]=MT_RAM;
	MMU_MT_read[0x13][0x3]=MT_RAM;
	MMU_MT_read[0x13][0x2]=MT_RAM;
	MMU_MT_read[0x13][0x1]=MT_RAM;
	MMU_MT_read[0x13][0x0]=MT_RAM;
	//00011 #5
	MMU_MT_read[0x03][0xF]=MT_RAM;
	MMU_MT_read[0x03][0xE]=MT_RAM;
	MMU_MT_read[0x03][0xD]=MT_RAM;
	MMU_MT_read[0x03][0xC]=MT_RAM;
	MMU_MT_read[0x03][0xB]=MT_RAM;
	MMU_MT_read[0x03][0xA]=MT_RAM;
	MMU_MT_read[0x03][0x9]=MT_RAM;
	MMU_MT_read[0x03][0x8]=MT_RAM;
	MMU_MT_read[0x03][0x7]=MT_RAM;
	MMU_MT_read[0x03][0x6]=MT_RAM;
	MMU_MT_read[0x03][0x5]=MT_RAM;
	MMU_MT_read[0x03][0x4]=MT_RAM;
	MMU_MT_read[0x03][0x3]=MT_RAM;
	MMU_MT_read[0x03][0x2]=MT_RAM;
	MMU_MT_read[0x03][0x1]=MT_RAM;
	MMU_MT_read[0x03][0x0]=MT_RAM;


	//10000 #5
	MMU_MT_read[0x10][0xF]=MT_RAM;
	MMU_MT_read[0x10][0xE]=MT_RAM;
	MMU_MT_read[0x10][0xD]=MT_RAM;
	MMU_MT_read[0x10][0xC]=MT_RAM;
	MMU_MT_read[0x10][0xB]=MT_RAM;
	MMU_MT_read[0x10][0xA]=MT_RAM;
	MMU_MT_read[0x10][0x9]=MT_RAM;
	MMU_MT_read[0x10][0x8]=MT_RAM;
	MMU_MT_read[0x10][0x7]=MT_RAM;
	MMU_MT_read[0x10][0x6]=MT_RAM;
	MMU_MT_read[0x10][0x5]=MT_RAM;
	MMU_MT_read[0x10][0x4]=MT_RAM;
	MMU_MT_read[0x10][0x3]=MT_RAM;
	MMU_MT_read[0x10][0x2]=MT_RAM;
	MMU_MT_read[0x10][0x1]=MT_RAM;
	MMU_MT_read[0x10][0x0]=MT_RAM;
	//00000 #5
	MMU_MT_read[0x00][0xF]=MT_RAM;
	MMU_MT_read[0x00][0xE]=MT_RAM;
	MMU_MT_read[0x00][0xD]=MT_RAM;
	MMU_MT_read[0x00][0xC]=MT_RAM;
	MMU_MT_read[0x00][0xB]=MT_RAM;
	MMU_MT_read[0x00][0xA]=MT_RAM;
	MMU_MT_read[0x00][0x9]=MT_RAM;
	MMU_MT_read[0x00][0x8]=MT_RAM;
	MMU_MT_read[0x00][0x7]=MT_RAM;
	MMU_MT_read[0x00][0x6]=MT_RAM;
	MMU_MT_read[0x00][0x5]=MT_RAM;
	MMU_MT_read[0x00][0x4]=MT_RAM;
	MMU_MT_read[0x00][0x3]=MT_RAM;
	MMU_MT_read[0x00][0x2]=MT_RAM;
	MMU_MT_read[0x00][0x1]=MT_RAM;
	MMU_MT_read[0x00][0x0]=MT_RAM;


	//11110 #6
	MMU_MT_read[0x1E][0xF]=MT_KERNAL;
	MMU_MT_read[0x1E][0xE]=MT_KERNAL;
	MMU_MT_read[0x1E][0xD]=MT_IO;
	MMU_MT_read[0x1E][0xC]=MT_RAM;
	MMU_MT_read[0x1E][0xB]=MT_BASIC;
	MMU_MT_read[0x1E][0xA]=MT_BASIC;
	MMU_MT_read[0x1E][0x9]=MT_RAM;
	MMU_MT_read[0x1E][0x8]=MT_RAM;
	MMU_MT_read[0x1E][0x7]=MT_RAM;
	MMU_MT_read[0x1E][0x6]=MT_RAM;
	MMU_MT_read[0x1E][0x5]=MT_RAM;
	MMU_MT_read[0x1E][0x4]=MT_RAM;
	MMU_MT_read[0x1E][0x3]=MT_RAM;
	MMU_MT_read[0x1E][0x2]=MT_RAM;
	MMU_MT_read[0x1E][0x1]=MT_RAM;
	MMU_MT_read[0x1E][0x0]=MT_RAM;
	//01110 #6
	MMU_MT_read[0x0E][0xF]=MT_KERNAL;
	MMU_MT_read[0x0E][0xE]=MT_KERNAL;
	MMU_MT_read[0x0E][0xD]=MT_CHARGEN;
	MMU_MT_read[0x0E][0xC]=MT_RAM;
	MMU_MT_read[0x0E][0xB]=MT_BASIC;
	MMU_MT_read[0x0E][0xA]=MT_BASIC;
	MMU_MT_read[0x0E][0x9]=MT_RAM;
	MMU_MT_read[0x0E][0x8]=MT_RAM;
	MMU_MT_read[0x0E][0x7]=MT_RAM;
	MMU_MT_read[0x0E][0x6]=MT_RAM;
	MMU_MT_read[0x0E][0x5]=MT_RAM;
	MMU_MT_read[0x0E][0x4]=MT_RAM;
	MMU_MT_read[0x0E][0x3]=MT_RAM;
	MMU_MT_read[0x0E][0x2]=MT_RAM;
	MMU_MT_read[0x0E][0x1]=MT_RAM;
	MMU_MT_read[0x0E][0x0]=MT_RAM;


	//10100 #7
	MMU_MT_read[0x14][0xF]=MT_KERNAL;
	MMU_MT_read[0x14][0xE]=MT_KERNAL;
	MMU_MT_read[0x14][0xD]=MT_IO;
	MMU_MT_read[0x14][0xC]=MT_RAM;
	MMU_MT_read[0x14][0xB]=MT_RAM;
	MMU_MT_read[0x14][0xA]=MT_RAM;
	MMU_MT_read[0x14][0x9]=MT_RAM;
	MMU_MT_read[0x14][0x8]=MT_RAM;
	MMU_MT_read[0x14][0x7]=MT_RAM;
	MMU_MT_read[0x14][0x6]=MT_RAM;
	MMU_MT_read[0x14][0x5]=MT_RAM;
	MMU_MT_read[0x14][0x4]=MT_RAM;
	MMU_MT_read[0x14][0x3]=MT_RAM;
	MMU_MT_read[0x14][0x2]=MT_RAM;
	MMU_MT_read[0x14][0x1]=MT_RAM;
	MMU_MT_read[0x14][0x0]=MT_RAM;
	//00100 #7
	MMU_MT_read[0x04][0xF]=MT_KERNAL;
	MMU_MT_read[0x04][0xE]=MT_KERNAL;
	MMU_MT_read[0x04][0xD]=MT_CHARGEN;
	MMU_MT_read[0x04][0xC]=MT_RAM;
	MMU_MT_read[0x04][0xB]=MT_RAM;
	MMU_MT_read[0x04][0xA]=MT_RAM;
	MMU_MT_read[0x04][0x9]=MT_RAM;
	MMU_MT_read[0x04][0x8]=MT_RAM;
	MMU_MT_read[0x04][0x7]=MT_RAM;
	MMU_MT_read[0x04][0x6]=MT_RAM;
	MMU_MT_read[0x04][0x5]=MT_RAM;
	MMU_MT_read[0x04][0x4]=MT_RAM;
	MMU_MT_read[0x04][0x3]=MT_RAM;
	MMU_MT_read[0x04][0x2]=MT_RAM;
	MMU_MT_read[0x04][0x1]=MT_RAM;
	MMU_MT_read[0x04][0x0]=MT_RAM;


	//11100 #8
	MMU_MT_read[0x1C][0xF]=MT_KERNAL;
	MMU_MT_read[0x1C][0xE]=MT_KERNAL;
	MMU_MT_read[0x1C][0xD]=MT_IO;
	MMU_MT_read[0x1C][0xC]=MT_RAM;
	MMU_MT_read[0x1C][0xB]=MT_RAM;
	MMU_MT_read[0x1C][0xA]=MT_RAM;
	MMU_MT_read[0x1C][0x9]=MT_RAM;
	MMU_MT_read[0x1C][0x8]=MT_RAM;
	MMU_MT_read[0x1C][0x7]=MT_RAM;
	MMU_MT_read[0x1C][0x6]=MT_RAM;
	MMU_MT_read[0x1C][0x5]=MT_RAM;
	MMU_MT_read[0x1C][0x4]=MT_RAM;
	MMU_MT_read[0x1C][0x3]=MT_RAM;
	MMU_MT_read[0x1C][0x2]=MT_RAM;
	MMU_MT_read[0x1C][0x1]=MT_RAM;
	MMU_MT_read[0x1C][0x0]=MT_RAM;
	//01100 #8
	MMU_MT_read[0x0C][0xF]=MT_KERNAL;
	MMU_MT_read[0x0C][0xE]=MT_KERNAL;
	MMU_MT_read[0x0C][0xD]=MT_CHARGEN;
	MMU_MT_read[0x0C][0xC]=MT_RAM;
	MMU_MT_read[0x0C][0xB]=MT_RAM;
	MMU_MT_read[0x0C][0xA]=MT_RAM;
	MMU_MT_read[0x0C][0x9]=MT_RAM;
	MMU_MT_read[0x0C][0x8]=MT_RAM;
	MMU_MT_read[0x0C][0x7]=MT_RAM;
	MMU_MT_read[0x0C][0x6]=MT_RAM;
	MMU_MT_read[0x0C][0x5]=MT_RAM;
	MMU_MT_read[0x0C][0x4]=MT_RAM;
	MMU_MT_read[0x0C][0x3]=MT_RAM;
	MMU_MT_read[0x0C][0x2]=MT_RAM;
	MMU_MT_read[0x0C][0x1]=MT_RAM;
	MMU_MT_read[0x0C][0x0]=MT_RAM;

	for (i=0 ; i<=0x1f ; i++)
	{
		for (j=0 ; j<=0xf ; j++)
			if (MMU_MT_read[i][j] == MT_CHARGEN 
				|| MMU_MT_read[i][j] == MT_BASIC
				|| MMU_MT_read[i][j] == MT_KERNAL)
				MMU_MT_write[i][j]=MT_RAM;
			else
				MMU_MT_write[i][j]=MMU_MT_read[i][j];
	}
}

void RAM64::InitMMU(){
int i,j;
//-LORAM, -HIRAM, -GAME, -EXROM 

	InitMMU_0();

	miMemory = mMemory;
	miKernal = mKernal - 0xE000;
	miBasic = mBasic - 0xA000;
	miIO = mIO - 0xD000;
	mColorRAM = mIO + 0x0800;
	miCharGen = mCharGen - 0xD000;

	for (i=0 ; i<=0x1f ; i++)
	{
		for (j=0 ; j<=0xf ; j++)
		{
			MMU_read[i][j] = GetCpuMmuIndexedPointer(MMU_MT_read[i][j]);
			MMU_write[i][j] = GetCpuMmuIndexedPointer(MMU_MT_write[i][j]);
		}
	}

	////11111	#1
	//MMU_read[0x1F][0xF]=miKernal;
	//MMU_read[0x1F][0xE]=miKernal;
	//MMU_read[0x1F][0xD]=0;
	//MMU_read[0x1F][0xC]=miMemory;
	//MMU_read[0x1F][0xB]=miBasic;
	//MMU_read[0x1F][0xA]=miBasic;
	//MMU_read[0x1F][0x9]=miMemory;
	//MMU_read[0x1F][0x8]=miMemory;
	//MMU_read[0x1F][0x7]=miMemory;
	//MMU_read[0x1F][0x6]=miMemory;
	//MMU_read[0x1F][0x5]=miMemory;
	//MMU_read[0x1F][0x4]=miMemory;
	//MMU_read[0x1F][0x3]=miMemory;
	//MMU_read[0x1F][0x2]=miMemory;
	//MMU_read[0x1F][0x1]=miMemory;
	//MMU_read[0x1F][0x0]=miMemory;
	////01111	#1
	//MMU_read[0x0F][0xF]=miKernal;
	//MMU_read[0x0F][0xE]=miKernal;
	//MMU_read[0x0F][0xD]=miCharGen;
	//MMU_read[0x0F][0xC]=miMemory;
	//MMU_read[0x0F][0xB]=miBasic;
	//MMU_read[0x0F][0xA]=miBasic;
	//MMU_read[0x0F][0x9]=miMemory;
	//MMU_read[0x0F][0x8]=miMemory;
	//MMU_read[0x0F][0x7]=miMemory;
	//MMU_read[0x0F][0x6]=miMemory;
	//MMU_read[0x0F][0x5]=miMemory;
	//MMU_read[0x0F][0x4]=miMemory;
	//MMU_read[0x0F][0x3]=miMemory;
	//MMU_read[0x0F][0x2]=miMemory;
	//MMU_read[0x0F][0x1]=miMemory;
	//MMU_read[0x0F][0x0]=miMemory;


	//for (i=0 ; i<=0x1f ; i++){

	//	for (j=0 ; j<=0xf ; j++){
	//		if (i!=0x1f && i!=0xf)
	//			if ((i & 0x10)!=0)
	//				MMU_read[i][j]=MMU_read[0x1f][j];
	//			else
	//				MMU_read[i][j]=MMU_read[0x0f][j];
	//	}
	//}

	////11010 #2
	//MMU_read[0x1A][0xF]=miMemory;
	//MMU_read[0x1A][0xE]=miMemory;
	//MMU_read[0x1A][0xD]=0;
	//MMU_read[0x1A][0xC]=miMemory;
	//MMU_read[0x1A][0xB]=miMemory;
	//MMU_read[0x1A][0xA]=miMemory;
	//MMU_read[0x1A][0x9]=miMemory;
	//MMU_read[0x1A][0x8]=miMemory;
	//MMU_read[0x1A][0x7]=miMemory;
	//MMU_read[0x1A][0x6]=miMemory;
	//MMU_read[0x1A][0x5]=miMemory;
	//MMU_read[0x1A][0x4]=miMemory;
	//MMU_read[0x1A][0x3]=miMemory;
	//MMU_read[0x1A][0x2]=miMemory;
	//MMU_read[0x1A][0x1]=miMemory;
	//MMU_read[0x1A][0x0]=miMemory;
	////01010 #2
	//MMU_read[0x0A][0xF]=miMemory;
	//MMU_read[0x0A][0xE]=miMemory;
	//MMU_read[0x0A][0xD]=miCharGen;
	//MMU_read[0x0A][0xC]=miMemory;
	//MMU_read[0x0A][0xB]=miMemory;
	//MMU_read[0x0A][0xA]=miMemory;
	//MMU_read[0x0A][0x9]=miMemory;
	//MMU_read[0x0A][0x8]=miMemory;
	//MMU_read[0x0A][0x7]=miMemory;
	//MMU_read[0x0A][0x6]=miMemory;
	//MMU_read[0x0A][0x5]=miMemory;
	//MMU_read[0x0A][0x4]=miMemory;
	//MMU_read[0x0A][0x3]=miMemory;
	//MMU_read[0x0A][0x2]=miMemory;
	//MMU_read[0x0A][0x1]=miMemory;
	//MMU_read[0x0A][0x0]=miMemory;

	////11011 #2
	//MMU_read[0x1B][0xF]=miMemory;
	//MMU_read[0x1B][0xE]=miMemory;
	//MMU_read[0x1B][0xD]=0;
	//MMU_read[0x1B][0xC]=miMemory;
	//MMU_read[0x1B][0xB]=miMemory;
	//MMU_read[0x1B][0xA]=miMemory;
	//MMU_read[0x1B][0x9]=miMemory;
	//MMU_read[0x1B][0x8]=miMemory;
	//MMU_read[0x1B][0x7]=miMemory;
	//MMU_read[0x1B][0x6]=miMemory;
	//MMU_read[0x1B][0x5]=miMemory;
	//MMU_read[0x1B][0x4]=miMemory;
	//MMU_read[0x1B][0x3]=miMemory;
	//MMU_read[0x1B][0x2]=miMemory;
	//MMU_read[0x1B][0x1]=miMemory;
	//MMU_read[0x1B][0x0]=miMemory;
	////01011 #2
	//MMU_read[0x0B][0xF]=miMemory;
	//MMU_read[0x0B][0xE]=miMemory;
	//MMU_read[0x0B][0xD]=miCharGen;
	//MMU_read[0x0B][0xC]=miMemory;
	//MMU_read[0x0B][0xB]=miMemory;
	//MMU_read[0x0B][0xA]=miMemory;
	//MMU_read[0x0B][0x9]=miMemory;
	//MMU_read[0x0B][0x8]=miMemory;
	//MMU_read[0x0B][0x7]=miMemory;
	//MMU_read[0x0B][0x6]=miMemory;
	//MMU_read[0x0B][0x5]=miMemory;
	//MMU_read[0x0B][0x4]=miMemory;
	//MMU_read[0x0B][0x3]=miMemory;
	//MMU_read[0x0B][0x2]=miMemory;
	//MMU_read[0x0B][0x1]=miMemory;
	//MMU_read[0x0B][0x0]=miMemory;


	////11000 #3
	//MMU_read[0x18][0xF]=miMemory;
	//MMU_read[0x18][0xE]=miMemory;
	//MMU_read[0x18][0xD]=miMemory;
	//MMU_read[0x18][0xC]=miMemory;
	//MMU_read[0x18][0xB]=miMemory;
	//MMU_read[0x18][0xA]=miMemory;
	//MMU_read[0x18][0x9]=miMemory;
	//MMU_read[0x18][0x8]=miMemory;
	//MMU_read[0x18][0x7]=miMemory;
	//MMU_read[0x18][0x6]=miMemory;
	//MMU_read[0x18][0x5]=miMemory;
	//MMU_read[0x18][0x4]=miMemory;
	//MMU_read[0x18][0x3]=miMemory;
	//MMU_read[0x18][0x2]=miMemory;
	//MMU_read[0x18][0x1]=miMemory;
	//MMU_read[0x18][0x0]=miMemory;
	////01000 #3
	//MMU_read[0x08][0xF]=miMemory;
	//MMU_read[0x08][0xE]=miMemory;
	//MMU_read[0x08][0xD]=miCharGen;
	//MMU_read[0x08][0xC]=miMemory;
	//MMU_read[0x08][0xB]=miMemory;
	//MMU_read[0x08][0xA]=miMemory;
	//MMU_read[0x08][0x9]=miMemory;
	//MMU_read[0x08][0x8]=miMemory;
	//MMU_read[0x08][0x7]=miMemory;
	//MMU_read[0x08][0x6]=miMemory;
	//MMU_read[0x08][0x5]=miMemory;
	//MMU_read[0x08][0x4]=miMemory;
	//MMU_read[0x08][0x3]=miMemory;
	//MMU_read[0x08][0x2]=miMemory;
	//MMU_read[0x08][0x1]=miMemory;
	//MMU_read[0x08][0x0]=miMemory;


	////10110 #4
	//MMU_read[0x16][0xF]=miKernal;
	//MMU_read[0x16][0xE]=miKernal;
	//MMU_read[0x16][0xD]=0;
	//MMU_read[0x16][0xC]=miMemory;
	//MMU_read[0x16][0xB]=miMemory;
	//MMU_read[0x16][0xA]=miMemory;
	//MMU_read[0x16][0x9]=miMemory;
	//MMU_read[0x16][0x8]=miMemory;
	//MMU_read[0x16][0x7]=miMemory;
	//MMU_read[0x16][0x6]=miMemory;
	//MMU_read[0x16][0x5]=miMemory;
	//MMU_read[0x16][0x4]=miMemory;
	//MMU_read[0x16][0x3]=miMemory;
	//MMU_read[0x16][0x2]=miMemory;
	//MMU_read[0x16][0x1]=miMemory;
	//MMU_read[0x16][0x0]=miMemory;
	////00110 #4
	//MMU_read[0x06][0xF]=miKernal;
	//MMU_read[0x06][0xE]=miKernal;
	//MMU_read[0x06][0xD]=miCharGen;
	//MMU_read[0x06][0xC]=miMemory;
	//MMU_read[0x06][0xB]=miMemory;
	//MMU_read[0x06][0xA]=miMemory;
	//MMU_read[0x06][0x9]=miMemory;
	//MMU_read[0x06][0x8]=miMemory;
	//MMU_read[0x06][0x7]=miMemory;
	//MMU_read[0x06][0x6]=miMemory;
	//MMU_read[0x06][0x5]=miMemory;
	//MMU_read[0x06][0x4]=miMemory;
	//MMU_read[0x06][0x3]=miMemory;
	//MMU_read[0x06][0x2]=miMemory;
	//MMU_read[0x06][0x1]=miMemory;
	//MMU_read[0x06][0x0]=miMemory;


	////10111 #4
	//MMU_read[0x17][0xF]=miKernal;
	//MMU_read[0x17][0xE]=miKernal;
	//MMU_read[0x17][0xD]=0;
	//MMU_read[0x17][0xC]=miMemory;
	//MMU_read[0x17][0xB]=miMemory;
	//MMU_read[0x17][0xA]=miMemory;
	//MMU_read[0x17][0x9]=miMemory;
	//MMU_read[0x17][0x8]=miMemory;
	//MMU_read[0x17][0x7]=miMemory;
	//MMU_read[0x17][0x6]=miMemory;
	//MMU_read[0x17][0x5]=miMemory;
	//MMU_read[0x17][0x4]=miMemory;
	//MMU_read[0x17][0x3]=miMemory;
	//MMU_read[0x17][0x2]=miMemory;
	//MMU_read[0x17][0x1]=miMemory;
	//MMU_read[0x17][0x0]=miMemory;
	////00111 #4
	//MMU_read[0x07][0xF]=miKernal;
	//MMU_read[0x07][0xE]=miKernal;
	//MMU_read[0x07][0xD]=miCharGen;
	//MMU_read[0x07][0xC]=miMemory;
	//MMU_read[0x07][0xB]=miMemory;
	//MMU_read[0x07][0xA]=miMemory;
	//MMU_read[0x07][0x9]=miMemory;
	//MMU_read[0x07][0x8]=miMemory;
	//MMU_read[0x07][0x7]=miMemory;
	//MMU_read[0x07][0x6]=miMemory;
	//MMU_read[0x07][0x5]=miMemory;
	//MMU_read[0x07][0x4]=miMemory;
	//MMU_read[0x07][0x3]=miMemory;
	//MMU_read[0x07][0x2]=miMemory;
	//MMU_read[0x07][0x1]=miMemory;
	//MMU_read[0x07][0x0]=miMemory;


	////10010 #5
	//MMU_read[0x12][0xF]=miMemory;
	//MMU_read[0x12][0xE]=miMemory;
	//MMU_read[0x12][0xD]=miMemory;
	//MMU_read[0x12][0xC]=miMemory;
	//MMU_read[0x12][0xB]=miMemory;
	//MMU_read[0x12][0xA]=miMemory;
	//MMU_read[0x12][0x9]=miMemory;
	//MMU_read[0x12][0x8]=miMemory;
	//MMU_read[0x12][0x7]=miMemory;
	//MMU_read[0x12][0x6]=miMemory;
	//MMU_read[0x12][0x5]=miMemory;
	//MMU_read[0x12][0x4]=miMemory;
	//MMU_read[0x12][0x3]=miMemory;
	//MMU_read[0x12][0x2]=miMemory;
	//MMU_read[0x12][0x1]=miMemory;
	//MMU_read[0x12][0x0]=miMemory;
	////00010 #5
	//MMU_read[0x02][0xF]=miMemory;
	//MMU_read[0x02][0xE]=miMemory;
	//MMU_read[0x02][0xD]=miMemory;
	//MMU_read[0x02][0xC]=miMemory;
	//MMU_read[0x02][0xB]=miMemory;
	//MMU_read[0x02][0xA]=miMemory;
	//MMU_read[0x02][0x9]=miMemory;
	//MMU_read[0x02][0x8]=miMemory;
	//MMU_read[0x02][0x7]=miMemory;
	//MMU_read[0x02][0x6]=miMemory;
	//MMU_read[0x02][0x5]=miMemory;
	//MMU_read[0x02][0x4]=miMemory;
	//MMU_read[0x02][0x3]=miMemory;
	//MMU_read[0x02][0x2]=miMemory;
	//MMU_read[0x02][0x1]=miMemory;
	//MMU_read[0x02][0x0]=miMemory;


	////10011 #5
	//MMU_read[0x13][0xF]=miMemory;
	//MMU_read[0x13][0xE]=miMemory;
	//MMU_read[0x13][0xD]=miMemory;
	//MMU_read[0x13][0xC]=miMemory;
	//MMU_read[0x13][0xB]=miMemory;
	//MMU_read[0x13][0xA]=miMemory;
	//MMU_read[0x13][0x9]=miMemory;
	//MMU_read[0x13][0x8]=miMemory;
	//MMU_read[0x13][0x7]=miMemory;
	//MMU_read[0x13][0x6]=miMemory;
	//MMU_read[0x13][0x5]=miMemory;
	//MMU_read[0x13][0x4]=miMemory;
	//MMU_read[0x13][0x3]=miMemory;
	//MMU_read[0x13][0x2]=miMemory;
	//MMU_read[0x13][0x1]=miMemory;
	//MMU_read[0x13][0x0]=miMemory;
	////00011 #5
	//MMU_read[0x03][0xF]=miMemory;
	//MMU_read[0x03][0xE]=miMemory;
	//MMU_read[0x03][0xD]=miMemory;
	//MMU_read[0x03][0xC]=miMemory;
	//MMU_read[0x03][0xB]=miMemory;
	//MMU_read[0x03][0xA]=miMemory;
	//MMU_read[0x03][0x9]=miMemory;
	//MMU_read[0x03][0x8]=miMemory;
	//MMU_read[0x03][0x7]=miMemory;
	//MMU_read[0x03][0x6]=miMemory;
	//MMU_read[0x03][0x5]=miMemory;
	//MMU_read[0x03][0x4]=miMemory;
	//MMU_read[0x03][0x3]=miMemory;
	//MMU_read[0x03][0x2]=miMemory;
	//MMU_read[0x03][0x1]=miMemory;
	//MMU_read[0x03][0x0]=miMemory;


	////10000 #5
	//MMU_read[0x10][0xF]=miMemory;
	//MMU_read[0x10][0xE]=miMemory;
	//MMU_read[0x10][0xD]=miMemory;
	//MMU_read[0x10][0xC]=miMemory;
	//MMU_read[0x10][0xB]=miMemory;
	//MMU_read[0x10][0xA]=miMemory;
	//MMU_read[0x10][0x9]=miMemory;
	//MMU_read[0x10][0x8]=miMemory;
	//MMU_read[0x10][0x7]=miMemory;
	//MMU_read[0x10][0x6]=miMemory;
	//MMU_read[0x10][0x5]=miMemory;
	//MMU_read[0x10][0x4]=miMemory;
	//MMU_read[0x10][0x3]=miMemory;
	//MMU_read[0x10][0x2]=miMemory;
	//MMU_read[0x10][0x1]=miMemory;
	//MMU_read[0x10][0x0]=miMemory;
	////00000 #5
	//MMU_read[0x00][0xF]=miMemory;
	//MMU_read[0x00][0xE]=miMemory;
	//MMU_read[0x00][0xD]=miMemory;
	//MMU_read[0x00][0xC]=miMemory;
	//MMU_read[0x00][0xB]=miMemory;
	//MMU_read[0x00][0xA]=miMemory;
	//MMU_read[0x00][0x9]=miMemory;
	//MMU_read[0x00][0x8]=miMemory;
	//MMU_read[0x00][0x7]=miMemory;
	//MMU_read[0x00][0x6]=miMemory;
	//MMU_read[0x00][0x5]=miMemory;
	//MMU_read[0x00][0x4]=miMemory;
	//MMU_read[0x00][0x3]=miMemory;
	//MMU_read[0x00][0x2]=miMemory;
	//MMU_read[0x00][0x1]=miMemory;
	//MMU_read[0x00][0x0]=miMemory;


	////11110 #6
	//MMU_read[0x1E][0xF]=miKernal;
	//MMU_read[0x1E][0xE]=miKernal;
	//MMU_read[0x1E][0xD]=0;
	//MMU_read[0x1E][0xC]=miMemory;
	//MMU_read[0x1E][0xB]=miBasic;
	//MMU_read[0x1E][0xA]=miBasic;
	//MMU_read[0x1E][0x9]=miMemory;
	//MMU_read[0x1E][0x8]=miMemory;
	//MMU_read[0x1E][0x7]=miMemory;
	//MMU_read[0x1E][0x6]=miMemory;
	//MMU_read[0x1E][0x5]=miMemory;
	//MMU_read[0x1E][0x4]=miMemory;
	//MMU_read[0x1E][0x3]=miMemory;
	//MMU_read[0x1E][0x2]=miMemory;
	//MMU_read[0x1E][0x1]=miMemory;
	//MMU_read[0x1E][0x0]=miMemory;
	////01110 #6
	//MMU_read[0x0E][0xF]=miKernal;
	//MMU_read[0x0E][0xE]=miKernal;
	//MMU_read[0x0E][0xD]=miCharGen;
	//MMU_read[0x0E][0xC]=miMemory;
	//MMU_read[0x0E][0xB]=miBasic;
	//MMU_read[0x0E][0xA]=miBasic;
	//MMU_read[0x0E][0x9]=miMemory;
	//MMU_read[0x0E][0x8]=miMemory;
	//MMU_read[0x0E][0x7]=miMemory;
	//MMU_read[0x0E][0x6]=miMemory;
	//MMU_read[0x0E][0x5]=miMemory;
	//MMU_read[0x0E][0x4]=miMemory;
	//MMU_read[0x0E][0x3]=miMemory;
	//MMU_read[0x0E][0x2]=miMemory;
	//MMU_read[0x0E][0x1]=miMemory;
	//MMU_read[0x0E][0x0]=miMemory;


	////10100 #7
	//MMU_read[0x14][0xF]=miKernal;
	//MMU_read[0x14][0xE]=miKernal;
	//MMU_read[0x14][0xD]=0;
	//MMU_read[0x14][0xC]=miMemory;
	//MMU_read[0x14][0xB]=miMemory;
	//MMU_read[0x14][0xA]=miMemory;
	//MMU_read[0x14][0x9]=miMemory;
	//MMU_read[0x14][0x8]=miMemory;
	//MMU_read[0x14][0x7]=miMemory;
	//MMU_read[0x14][0x6]=miMemory;
	//MMU_read[0x14][0x5]=miMemory;
	//MMU_read[0x14][0x4]=miMemory;
	//MMU_read[0x14][0x3]=miMemory;
	//MMU_read[0x14][0x2]=miMemory;
	//MMU_read[0x14][0x1]=miMemory;
	//MMU_read[0x14][0x0]=miMemory;
	////00100 #7
	//MMU_read[0x04][0xF]=miKernal;
	//MMU_read[0x04][0xE]=miKernal;
	//MMU_read[0x04][0xD]=miCharGen;
	//MMU_read[0x04][0xC]=miMemory;
	//MMU_read[0x04][0xB]=miMemory;
	//MMU_read[0x04][0xA]=miMemory;
	//MMU_read[0x04][0x9]=miMemory;
	//MMU_read[0x04][0x8]=miMemory;
	//MMU_read[0x04][0x7]=miMemory;
	//MMU_read[0x04][0x6]=miMemory;
	//MMU_read[0x04][0x5]=miMemory;
	//MMU_read[0x04][0x4]=miMemory;
	//MMU_read[0x04][0x3]=miMemory;
	//MMU_read[0x04][0x2]=miMemory;
	//MMU_read[0x04][0x1]=miMemory;
	//MMU_read[0x04][0x0]=miMemory;


	////11100 #8
	//MMU_read[0x1C][0xF]=miKernal;
	//MMU_read[0x1C][0xE]=miKernal;
	//MMU_read[0x1C][0xD]=0;
	//MMU_read[0x1C][0xC]=miMemory;
	//MMU_read[0x1C][0xB]=miMemory;
	//MMU_read[0x1C][0xA]=miMemory;
	//MMU_read[0x1C][0x9]=miMemory;
	//MMU_read[0x1C][0x8]=miMemory;
	//MMU_read[0x1C][0x7]=miMemory;
	//MMU_read[0x1C][0x6]=miMemory;
	//MMU_read[0x1C][0x5]=miMemory;
	//MMU_read[0x1C][0x4]=miMemory;
	//MMU_read[0x1C][0x3]=miMemory;
	//MMU_read[0x1C][0x2]=miMemory;
	//MMU_read[0x1C][0x1]=miMemory;
	//MMU_read[0x1C][0x0]=miMemory;
	////01100 #8
	//MMU_read[0x0C][0xF]=miKernal;
	//MMU_read[0x0C][0xE]=miKernal;
	//MMU_read[0x0C][0xD]=miCharGen;
	//MMU_read[0x0C][0xC]=miMemory;
	//MMU_read[0x0C][0xB]=miMemory;
	//MMU_read[0x0C][0xA]=miMemory;
	//MMU_read[0x0C][0x9]=miMemory;
	//MMU_read[0x0C][0x8]=miMemory;
	//MMU_read[0x0C][0x7]=miMemory;
	//MMU_read[0x0C][0x6]=miMemory;
	//MMU_read[0x0C][0x5]=miMemory;
	//MMU_read[0x0C][0x4]=miMemory;
	//MMU_read[0x0C][0x3]=miMemory;
	//MMU_read[0x0C][0x2]=miMemory;
	//MMU_read[0x0C][0x1]=miMemory;
	//MMU_read[0x0C][0x0]=miMemory;


	//for (i=0 ; i<=0x1f ; i++){
	//	for (j=0 ; j<=0xf ; j++)
	//		if (MMU_read[i][j] == miCharGen 
	//			|| MMU_read[i][j] == miBasic
	//			|| MMU_read[i][j] == miKernal)
	//			MMU_write[i][j]=miMemory;
	//		else
	//			MMU_write[i][j]=MMU_read[i][j];
	//}


	VicMMU_read[0x3][0x3]=mMemory+0xC000;
	VicMMU_read[0x3][0x2]=mMemory+0xC000;
	VicMMU_read[0x3][0x1]=mMemory+0xC000;
	VicMMU_read[0x3][0x0]=mMemory+0xC000;

	VicMMU_read[0x2][0x3]=mMemory+0x8000;
	VicMMU_read[0x2][0x2]=mMemory+0x8000;
	VicMMU_read[0x2][0x1]=mCharGen-0x1000;
	VicMMU_read[0x2][0x0]=mMemory+0x8000;

	VicMMU_read[0x1][0x3]=mMemory+0x4000;
	VicMMU_read[0x1][0x2]=mMemory+0x4000;
	VicMMU_read[0x1][0x1]=mMemory+0x4000;
	VicMMU_read[0x1][0x0]=mMemory+0x4000;

	VicMMU_read[0x0][0x3]=mMemory;
	VicMMU_read[0x0][0x2]=mMemory;
	VicMMU_read[0x0][0x1]=mCharGen-0x1000;
	VicMMU_read[0x0][0x0]=mMemory;
}

void RAM64::ConfigureMMU(bit8 index, bit8 ***p_memory_map_read, bit8 ***p_memory_map_write)
{
	m_iCurrentCpuMmuIndex = index;
	*p_memory_map_read = MMU_read[index & 0x1f];
	*p_memory_map_write = MMU_write[index & 0x1f];
}


void RAM64::ConfigureVICMMU(bit8 index, bit8 ***p_vic_memory_map_read, bit8 **p_vic_3fff_ptr)
{
	*p_vic_memory_map_read = VicMMU_read[index & 3];
	*p_vic_3fff_ptr = &(**p_vic_memory_map_read)[0x3fff];
}

