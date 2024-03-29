#include <windows.h>
#include <commctrl.h>
#include <string>
#include "wfs.h"
#include <tchar.h>
#include "dx_version.h"
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "hexconv.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"


RAM64::RAM64() noexcept
{
	Zero64MemoryPointers();
}

RAM64::~RAM64()
{
	Free64Memory();
}

void RAM64::InitReset(bool poweronreset)
{
	if (!poweronreset)
	{
		return;
	}

	// The copy from mMemoryRestore covers the case where a saved state that was loaded from file has overwritten the ROMs.
	memcpy(mMemory, mMemoryRestore, MEM_TOTALSIZE);
	int i;
	bool usecommon = false;
	bool userandom = true;
	bool isnewc64 = true;
	uniform_int_distribution<int> dist_byte(0, 255);

	for (i=0; i <= 0xFFFF; i++)
	{
		if (usecommon)
		{
			if (isnewc64)
			{
				mMemory[i] = ((i & 0x80) == 0 ? 0xff : 0x00);
			}
			else
			{
				mMemory[i] = ((i & 0x40) != 0 ? 0xff : 0x00);
			}
		}
		else
		{
			switch((i & 0x0380) >> 7)
			{
			case 0:
				mMemory[i] = 0xff;
				break;
			case 1:
				mMemory[i] = 0;
				break;
			case 2:
				mMemory[i] = 0xff;
				break;
			case 3:
				mMemory[i] = 0;
				break;
			case 4:
				if ((i & 0x8000) == 0)
				{
					mMemory[i] = 0x99;
				}
				else
				{
					mMemory[i] = 0x66;
				}

				break;
			case 5:
				if ((i & 0x8000) == 0)
				{
					mMemory[i] = 0x66;
				}
				else
				{
					mMemory[i] = 0x99;
				}

				break;
			case 6:
				if ((i & 0x8000) == 0)
				{
					mMemory[i] = 0x99;
				}
				else
				{
					mMemory[i] = 0x66;
				}

				break;
			case 7:
				if ((i & 0x8000) == 0)
				{
					mMemory[i] = 0x66;
				}
				else
				{
					mMemory[i] = 0x99;
				}

				break;
			default:
				break;
			}
		}
	}

	if (userandom)
	{
		for (i=0; i <= 0xFF; i++)
		{
			mMemory[i << 8] = dist_byte(this->randengine_main);
		}
	}

	if (usecommon)
	{
		for (i=0 ; i<1024 ; i++)
		{
			mColorRAM[i] = 0xf;
		}
	}
	else
	{
		for (i=0 ; i<1024 ; i++)
		{
			mColorRAM[i] = ((i & 1) == 0) ^ ((i & 8) == 0) ? 0x3 : 0xc;
		}
	}

	LoadResetPattern();
}

void RAM64::Reset(bool poweronreset)
{
	InitReset(poweronreset);
}


HRESULT RAM64::Init(const wchar_t* pwszAppDirectory, Cart* cart)
{
	HANDLE hfile = 0;
	BOOL r;
	DWORD bytes_read;
	std::wstring wsRomPath;
	static const wchar_t ROM_NAME_KERNAL[] = L"kernal.rom";
	static const wchar_t ROM_NAME_BASIC[] = L"basic.rom";
	static const wchar_t ROM_NAME_CHAR[] = L"char.rom";
	randengine_main.seed(rd());

	ClearError();
	m_pCart = cart;
	wsAppDirectory.clear();
	if (pwszAppDirectory != nullptr)
	{
		wsAppDirectory = pwszAppDirectory;
	}

	if (S_OK != Allocate64Memory())
	{
		return E_FAIL;
	}

	hfile = INVALID_HANDLE_VALUE;
	wsRomPath.append(wsAppDirectory);
	Wfs::Path_Append(wsRomPath, ROM_NAME_KERNAL);
	hfile = CreateFileW(Wfs::EnsureLongNamePrefix(wsRomPath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		hfile = CreateFileW(ROM_NAME_KERNAL, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open kernal.rom"));
	}

	r = ReadFile(hfile, mKernal, 0x2000, &bytes_read, NULL);
	CloseHandle(hfile);
	if (r == 0)
	{
		return SetError(E_FAIL, TEXT("Could not read from kernal.rom"));
	}

	if (bytes_read != 0x2000)
	{
		return SetError(E_FAIL, TEXT("Could not read 0x2000 bytes from kernal.rom"));
	}

	hfile = INVALID_HANDLE_VALUE;
	wsRomPath.clear();
	wsRomPath.append(wsAppDirectory);
	Wfs::Path_Append(wsRomPath, ROM_NAME_BASIC);
	hfile = CreateFileW(Wfs::EnsureLongNamePrefix(wsRomPath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		hfile = CreateFileW(ROM_NAME_BASIC, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open basic.rom"));
	}

	r = ReadFile(hfile, mBasic, 0x2000, &bytes_read, NULL);
	CloseHandle(hfile);
	if (r == 0)
	{
		return SetError(E_FAIL, TEXT("Could not read from basic.rom"));
	}

	if (bytes_read != 0x2000)
	{
		return SetError(E_FAIL, TEXT("Could not read 0x2000 bytes from basic.rom"));
	}

	hfile = INVALID_HANDLE_VALUE;
	wsRomPath.clear();
	wsRomPath.append(wsAppDirectory);
	Wfs::Path_Append(wsRomPath, ROM_NAME_CHAR);

	hfile = CreateFileW(Wfs::EnsureLongNamePrefix(wsRomPath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		hfile = CreateFileW(ROM_NAME_CHAR, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open char.rom"));
	}

	r = ReadFile(hfile, mCharGen, 0x1000, &bytes_read, NULL);
	CloseHandle(hfile);
	if (r == 0)
	{
		return SetError(E_FAIL, TEXT("Could not read from char.rom"));
	}

	if (bytes_read != 0x1000)
	{
		return SetError(E_FAIL, TEXT("Could not read 0x1000 bytes from char.rom"));
	}

	memcpy(mMemoryRestore, mMemory, MEM_TOTALSIZE);

	InitMMU();
	return S_OK;
}

void RAM64::LoadResetPattern()
{
	HANDLE hfile = 0;
	BOOL r;
	DWORD bytes_read;
	std::wstring wsRomPath;
	static const wchar_t RAM_NAME_PATTERN[] = L"c64.ram";
	hfile = INVALID_HANDLE_VALUE;
	wsRomPath.clear();
	wsRomPath.append(wsAppDirectory);
	Wfs::Path_Append(wsRomPath, RAM_NAME_PATTERN);
	hfile = CreateFileW(Wfs::EnsureLongNamePrefix(wsRomPath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		hfile = CreateFileW(RAM_NAME_PATTERN, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	if (hfile != INVALID_HANDLE_VALUE)
	{
		r = ReadFile(hfile, mMemory, 0x10000, &bytes_read, NULL);
		CloseHandle(hfile);
	}
}

HRESULT RAM64::Allocate64Memory() 
{
	Free64Memory();
	mMemory = (bit8*)GlobalAlloc(GPTR, MEM_TOTALSIZE);
	if (mMemory != nullptr)
	{
		mMemoryRestore = (bit8*)GlobalAlloc(GPTR, MEM_TOTALSIZE);
		if (mMemoryRestore != nullptr)
		{
			mKernal = mMemory + (INT_PTR)MEM_RAM64;
			mBasic = mKernal + (INT_PTR)MEM_KERNAL;
			mIO = mBasic + (INT_PTR)MEM_BASIC;
			mCharGen = mIO + (INT_PTR)MEM_IO;
		}

		return S_OK;
	}

	return SetError(E_FAIL, TEXT("Allocate64Memory() failed"));
}

void RAM64::Free64Memory() noexcept
{
	if (mMemory)
	{
		GlobalFree(mMemory);
	}

	if (mMemoryRestore)
	{
		GlobalFree(mMemoryRestore);
	}

	Zero64MemoryPointers();
}

void RAM64::Zero64MemoryPointers() noexcept
{
	m_iCurrentCpuMmuIndex = 0;
	mMemory = nullptr;
	mKernal = nullptr;
	mBasic = nullptr;
	mIO = nullptr;
	mCharGen = nullptr;
	mColorRAM = nullptr;
	mMemoryRestore = nullptr;
	miMemory = nullptr;
	miKernal = nullptr;
	miBasic = nullptr;
	miIO = nullptr;
	miCharGen = nullptr;
}

int RAM64::GetCurrentCpuMmuMemoryMap()
{
	return m_iCurrentCpuMmuIndex;
}

bit8* RAM64::GetCpuMmuIndexedPointer(MEM_TYPE mt)
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
	case MT_ROML:
	case MT_ROMH:
	case MT_ROML_ULTIMAX:
	case MT_ROMH_ULTIMAX:
	case MT_EXRAM:
		return 0;
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

MEM_TYPE RAM64::GetCurrentCpuMmuReadMemoryType(bit16 address)
{
	return MMU_MT_read[m_iCurrentCpuMmuIndex][address >> 12];
}

MEM_TYPE RAM64::GetCurrentCpuMmuWriteMemoryType(bit16 address)
{
	return MMU_MT_write[m_iCurrentCpuMmuIndex & 0x1f][address >> 12];
}

void RAM64::InitMMU_0()
{
int i,j;
	
	//CHARGEN,LORAM,HIRAM,GAME,EXROM
	//11111	#1
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
	MMU_MT_read[0x18][0xD]=MT_IO;
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
	MMU_MT_read[0x08][0xD]=MT_RAM;
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
	MMU_MT_read[0x1E][0x9]=MT_ROML;
	MMU_MT_read[0x1E][0x8]=MT_ROML;
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
	MMU_MT_read[0x0E][0x9]=MT_ROML;
	MMU_MT_read[0x0E][0x8]=MT_ROML;
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
	MMU_MT_read[0x14][0xB]=MT_ROMH;
	MMU_MT_read[0x14][0xA]=MT_ROMH;
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
	MMU_MT_read[0x04][0xB]=MT_ROMH;
	MMU_MT_read[0x04][0xA]=MT_ROMH;
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
	MMU_MT_read[0x1C][0xB]=MT_ROMH;
	MMU_MT_read[0x1C][0xA]=MT_ROMH;
	MMU_MT_read[0x1C][0x9]=MT_ROML;
	MMU_MT_read[0x1C][0x8]=MT_ROML;
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
	MMU_MT_read[0x0C][0xB]=MT_ROMH;
	MMU_MT_read[0x0C][0xA]=MT_ROMH;
	MMU_MT_read[0x0C][0x9]=MT_ROML;
	MMU_MT_read[0x0C][0x8]=MT_ROML;
	MMU_MT_read[0x0C][0x7]=MT_RAM;
	MMU_MT_read[0x0C][0x6]=MT_RAM;
	MMU_MT_read[0x0C][0x5]=MT_RAM;
	MMU_MT_read[0x0C][0x4]=MT_RAM;
	MMU_MT_read[0x0C][0x3]=MT_RAM;
	MMU_MT_read[0x0C][0x2]=MT_RAM;
	MMU_MT_read[0x0C][0x1]=MT_RAM;
	MMU_MT_read[0x0C][0x0]=MT_RAM;


	//10001 #9
	MMU_MT_read[0x11][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x11][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x11][0xD]=MT_IO;
	MMU_MT_read[0x11][0xC]=MT_EXRAM;
	MMU_MT_read[0x11][0xB]=MT_EXRAM;
	MMU_MT_read[0x11][0xA]=MT_EXRAM;
	MMU_MT_read[0x11][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x11][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x11][0x7]=MT_EXRAM;
	MMU_MT_read[0x11][0x6]=MT_EXRAM;
	MMU_MT_read[0x11][0x5]=MT_EXRAM;
	MMU_MT_read[0x11][0x4]=MT_EXRAM;
	MMU_MT_read[0x11][0x3]=MT_EXRAM;
	MMU_MT_read[0x11][0x2]=MT_EXRAM;
	MMU_MT_read[0x11][0x1]=MT_EXRAM;
	MMU_MT_read[0x11][0x0]=MT_RAM;
	//00001 #9
	MMU_MT_read[0x01][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x01][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x01][0xD]=MT_IO;
	MMU_MT_read[0x01][0xC]=MT_EXRAM;
	MMU_MT_read[0x01][0xB]=MT_EXRAM;
	MMU_MT_read[0x01][0xA]=MT_EXRAM;
	MMU_MT_read[0x01][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x01][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x01][0x7]=MT_EXRAM;
	MMU_MT_read[0x01][0x6]=MT_EXRAM;
	MMU_MT_read[0x01][0x5]=MT_EXRAM;
	MMU_MT_read[0x01][0x4]=MT_EXRAM;
	MMU_MT_read[0x01][0x3]=MT_EXRAM;
	MMU_MT_read[0x01][0x2]=MT_EXRAM;
	MMU_MT_read[0x01][0x1]=MT_EXRAM;
	MMU_MT_read[0x01][0x0]=MT_RAM;


	//10001 #9
	MMU_MT_read[0x15][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x15][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x15][0xD]=MT_IO;
	MMU_MT_read[0x15][0xC]=MT_EXRAM;
	MMU_MT_read[0x15][0xB]=MT_EXRAM;
	MMU_MT_read[0x15][0xA]=MT_EXRAM;
	MMU_MT_read[0x15][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x15][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x15][0x7]=MT_EXRAM;
	MMU_MT_read[0x15][0x6]=MT_EXRAM;
	MMU_MT_read[0x15][0x5]=MT_EXRAM;
	MMU_MT_read[0x15][0x4]=MT_EXRAM;
	MMU_MT_read[0x15][0x3]=MT_EXRAM;
	MMU_MT_read[0x15][0x2]=MT_EXRAM;
	MMU_MT_read[0x15][0x1]=MT_EXRAM;
	MMU_MT_read[0x15][0x0]=MT_RAM;
	//00001 #9
	MMU_MT_read[0x05][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x05][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x05][0xD]=MT_IO;
	MMU_MT_read[0x05][0xC]=MT_EXRAM;
	MMU_MT_read[0x05][0xB]=MT_EXRAM;
	MMU_MT_read[0x05][0xA]=MT_EXRAM;
	MMU_MT_read[0x05][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x05][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x05][0x7]=MT_EXRAM;
	MMU_MT_read[0x05][0x6]=MT_EXRAM;
	MMU_MT_read[0x05][0x5]=MT_EXRAM;
	MMU_MT_read[0x05][0x4]=MT_EXRAM;
	MMU_MT_read[0x05][0x3]=MT_EXRAM;
	MMU_MT_read[0x05][0x2]=MT_EXRAM;
	MMU_MT_read[0x05][0x1]=MT_EXRAM;
	MMU_MT_read[0x05][0x0]=MT_RAM;


	//10001 #9
	MMU_MT_read[0x19][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x19][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x19][0xD]=MT_IO;
	MMU_MT_read[0x19][0xC]=MT_EXRAM;
	MMU_MT_read[0x19][0xB]=MT_EXRAM;
	MMU_MT_read[0x19][0xA]=MT_EXRAM;
	MMU_MT_read[0x19][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x19][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x19][0x7]=MT_EXRAM;
	MMU_MT_read[0x19][0x6]=MT_EXRAM;
	MMU_MT_read[0x19][0x5]=MT_EXRAM;
	MMU_MT_read[0x19][0x4]=MT_EXRAM;
	MMU_MT_read[0x19][0x3]=MT_EXRAM;
	MMU_MT_read[0x19][0x2]=MT_EXRAM;
	MMU_MT_read[0x19][0x1]=MT_EXRAM;
	MMU_MT_read[0x19][0x0]=MT_RAM;
	//00001 #9
	MMU_MT_read[0x09][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x09][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x09][0xD]=MT_IO;
	MMU_MT_read[0x09][0xC]=MT_EXRAM;
	MMU_MT_read[0x09][0xB]=MT_EXRAM;
	MMU_MT_read[0x09][0xA]=MT_EXRAM;
	MMU_MT_read[0x09][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x09][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x09][0x7]=MT_EXRAM;
	MMU_MT_read[0x09][0x6]=MT_EXRAM;
	MMU_MT_read[0x09][0x5]=MT_EXRAM;
	MMU_MT_read[0x09][0x4]=MT_EXRAM;
	MMU_MT_read[0x09][0x3]=MT_EXRAM;
	MMU_MT_read[0x09][0x2]=MT_EXRAM;
	MMU_MT_read[0x09][0x1]=MT_EXRAM;
	MMU_MT_read[0x09][0x0]=MT_RAM;


	//10001 #9
	MMU_MT_read[0x1D][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x1D][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x1D][0xD]=MT_IO;
	MMU_MT_read[0x1D][0xC]=MT_EXRAM;
	MMU_MT_read[0x1D][0xB]=MT_EXRAM;
	MMU_MT_read[0x1D][0xA]=MT_EXRAM;
	MMU_MT_read[0x1D][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x1D][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x1D][0x7]=MT_EXRAM;
	MMU_MT_read[0x1D][0x6]=MT_EXRAM;
	MMU_MT_read[0x1D][0x5]=MT_EXRAM;
	MMU_MT_read[0x1D][0x4]=MT_EXRAM;
	MMU_MT_read[0x1D][0x3]=MT_EXRAM;
	MMU_MT_read[0x1D][0x2]=MT_EXRAM;
	MMU_MT_read[0x1D][0x1]=MT_EXRAM;
	MMU_MT_read[0x1D][0x0]=MT_RAM;
	//00001 #9
	MMU_MT_read[0x0D][0xF]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x0D][0xE]=MT_ROMH_ULTIMAX;
	MMU_MT_read[0x0D][0xD]=MT_IO;
	MMU_MT_read[0x0D][0xC]=MT_EXRAM;
	MMU_MT_read[0x0D][0xB]=MT_EXRAM;
	MMU_MT_read[0x0D][0xA]=MT_EXRAM;
	MMU_MT_read[0x0D][0x9]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x0D][0x8]=MT_ROML_ULTIMAX;
	MMU_MT_read[0x0D][0x7]=MT_EXRAM;
	MMU_MT_read[0x0D][0x6]=MT_EXRAM;
	MMU_MT_read[0x0D][0x5]=MT_EXRAM;
	MMU_MT_read[0x0D][0x4]=MT_EXRAM;
	MMU_MT_read[0x0D][0x3]=MT_EXRAM;
	MMU_MT_read[0x0D][0x2]=MT_EXRAM;
	MMU_MT_read[0x0D][0x1]=MT_EXRAM;
	MMU_MT_read[0x0D][0x0]=MT_RAM;


	for (i=0 ; i<=0x1f ; i++)
	{
		for (j=0 ; j<=0xf ; j++)
		{
			MEM_TYPE readmt = MMU_MT_read[i][j];
			if (readmt == MT_CHARGEN 
				|| readmt == MT_BASIC
				|| readmt == MT_KERNAL
				)
			{
				MMU_MT_write[i][j]=MT_RAM;
			}
			else
			{
				MMU_MT_write[i][j]=readmt;
			}
		}
	}
}

void RAM64::InitMMU(){
int i,j;
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

	VicMMU_read[0x3][0x3]=mMemory+0xF000;
	VicMMU_read[0x3][0x2]=mMemory+0xE000;
	VicMMU_read[0x3][0x1]=mMemory+0xD000;
	VicMMU_read[0x3][0x0]=mMemory+0xC000;

	VicMMU_read[0x2][0x3]=mMemory+0xB000;
	VicMMU_read[0x2][0x2]=mMemory+0xA000;
	VicMMU_read[0x2][0x1]=mCharGen;
	VicMMU_read[0x2][0x0]=mMemory+0x8000;

	VicMMU_read[0x1][0x3]=mMemory+0x7000;
	VicMMU_read[0x1][0x2]=mMemory+0x6000;
	VicMMU_read[0x1][0x1]=mMemory+0x5000;
	VicMMU_read[0x1][0x0]=mMemory+0x4000;

	VicMMU_read[0x0][0x3]=mMemory+0x3000;
	VicMMU_read[0x0][0x2]=mMemory+0x2000;
	VicMMU_read[0x0][0x1]=mCharGen;
	VicMMU_read[0x0][0x0]=mMemory;
}

void RAM64::ConfigureMMU(bit8 index, bit8 ***p_memory_map_read, bit8 ***p_memory_map_write)
{
	m_iCurrentCpuMmuIndex = index;
	*p_memory_map_read = MMU_read[index & 0x1f];
	*p_memory_map_write = MMU_write[index & 0x1f];
}


/*
* From https://www.floodgap.com/retrobits/ckb/secret/ultimax.html
* 
The Max Machine's price, anaemic RAM and fierce market competition doomed it to failure -- what happened next to Commodore Japan is in the entry for the 
Japanese 64. That wasn't the end of the story, however; as the simultaneous introduction would imply, Commodore wanted to make sure that the Max could be a 
springboard to the 64 and the Ultimax even now haunts the 64 and 128's memory mapping schemes. To allow the 64 to use Ultimax cartridges, if you pull -GAME low 
and leave -EXROM alone, the 64 plops into Ultimax mode (as described in the Programmer's Reference Guide). 4K of RAM is mapped into $0000-$0FFF; low ROM into 
$8000-9FFF (on the Max, this was undoubtedly where MiniBASIC resided); and I/O and high ROM at $D000 and $E000 as usual, with most Ultimax cartridges natively 
living in the $E000 range. The VIC-II, however, sees all 64K in its usual 16K clumps, with ROM banked into the upper 4K of its current "slice" 
(meaning $F000-$FFFF of the cartridge ROM actually "appears" at $3000-$3FFF in the default VIC addressing space as well). LORAM, HIRAM and CHAREN signals are 
ignored, though the VIC-II's VA14/15 banking bits still have some effect. This is the only mode where the VIC-II can access external memory 
(i.e., memory outside of its default 16K slice), engineered to allow the cartridge's sprite and character data to be visible to the VIC-II without copying it 
and wasting what little RAM is present -- particularly important since the Ultimax has no built-in character set! (However, because of the processor's 
restrictions it would be very stupid for software to attempt to use memory higher than $1000.) Because of this complex little internal hackery, you must also 
copy $F000-$FFFF to $3000-$3FFF before running an Ultimax cartridge dump or the game will have scrambled graphics. The character generator can be also turned 
off in Ultimax mode and replaced either by low ROM, high ROM, or bytes fetched by the processor; the -ROML and -ROMH signals control this feature.
*/
void RAM64::ConfigureVICMMU(bit8 index, bit8 ***p_vic_memory_map_read, bit8 **p_vic_3fff_ptr)
{
	if (m_pCart->IsUltimax())
	{		
		bit8 *p = m_pCart->Get_RomH() + 0x1000;
		VicMMU_read[0x0][0x3] = p;
		VicMMU_read[0x1][0x3] = p;
		VicMMU_read[0x2][0x3] = p;
		VicMMU_read[0x3][0x3] = p;

		VicMMU_read[0x0][0x1] = mMemory+0x1000;
		VicMMU_read[0x2][0x1] = mMemory+0x9000;
	}
	else
	{
		VicMMU_read[0x0][0x3] = mMemory+0x3000;
		VicMMU_read[0x1][0x3] = mMemory+0x7000;
		VicMMU_read[0x2][0x3] = mMemory+0xB000;
		VicMMU_read[0x3][0x3] = mMemory+0xF000;

		VicMMU_read[0x0][0x1] = mCharGen;
		VicMMU_read[0x2][0x1] = mCharGen;
	}

	*p_vic_memory_map_read = VicMMU_read[index & 3];
	*p_vic_3fff_ptr = &(*p_vic_memory_map_read)[3][0x0fff];
}
