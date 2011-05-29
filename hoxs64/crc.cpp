#include <windows.h>
#include <assert.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "crc.h"


CRC32::CRC32()
{
	Init(CRC32POLY, 0xffffffff, 0xffffffff, true);
}

CRC32::CRC32(DWORD poly, DWORD init, DWORD xorOut, bool isReflected)
{
	Init(poly, init, xorOut, isReflected);
}

CRC32::~CRC32()
{
}

DWORD CRC32::Reflect(DWORD b)
{
	DWORD r=0;
	int i;
	for (i = 0; i<32; i++)
	{
		r <<= 1;
		if (b & 0x1)
			r |= 1;
		b >>= 1;
	}
	return r;
}

void CRC32::Init()
{
	reg = init;
}

void CRC32::Init(DWORD poly, DWORD init, DWORD xorOut, bool isReflected)
{

	if (isReflected)
	{
		poly = Reflect(poly);
		init = Reflect(init);
		xorOut = Reflect(xorOut);
	}



	CRC32::isReflected = isReflected;
	CRC32::poly = poly;
	CRC32::init = init;
	CRC32::xorOut = xorOut;

	

	DWORD i,j;
	bool oneOut;
	if (isReflected)
	{
		for (i=0; i<256; i++)
		{
			reg = i;
			for (j=0; j<8; j++)
			{
				oneOut = (reg & 1) !=0;
				reg >>= 1;
				if (oneOut)
					reg ^= poly;
			
			}
			CrcTable[i] = reg;
		}	
	}
	else
	{
		for (i=0; i<256; i++)
		{
			reg = i<<24;
			for (j=0; j<8; j++)
			{
				oneOut = (reg & 0x80000000) !=0;
				reg <<= 1;
				if (oneOut)
					reg ^= poly;
			
			}
			CrcTable[i] = reg;
		}	
	}

	reg = init;
}

void CRC32::ProcessByte(BYTE b)
{
	if (isReflected)
		reg = CrcTable[(reg ^ b) & 0xFFL] ^ (reg >> 8);
	else
		reg = CrcTable[((reg>>24) ^ b) & 0xFFL] ^ (reg << 8);
}



DWORD CRC32::Value()
{
	return reg ^ xorOut;
}



CRC32Alloc::CRC32Alloc()
{
	pCRC32 = new CRC32();
	if (pCRC32)
		isOK=true;
	else
		isOK=false;
}

CRC32Alloc::~CRC32Alloc()
{
	if (pCRC32)
	{
		delete pCRC32;
		pCRC32=0L;
	}
}

CRC32Alloc::CRC32Alloc(DWORD poly, DWORD init, DWORD xorOut, bool isReflected)
{
	pCRC32 = new CRC32(poly, init, xorOut, isReflected);
	if (pCRC32)
		isOK=true;
	else
		isOK=false;

}
