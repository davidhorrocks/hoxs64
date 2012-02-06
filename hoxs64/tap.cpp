#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "CDPI.h"
#include "resource.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "register.h"
#include "tap.h"

TAP64::TAP64()
{
	pTapeHeader=NULL;
	tape_length=0;
}
TAP64::~TAP64()
{
	UnloadTAP();
}

void TAP64::UnloadTAP()
{
	if (pTapeHeader)
		GlobalFree(pTapeHeader);
	pTapeHeader=NULL;
	pTapeData=NULL;
	tape_length=0;
}

HRESULT TAP64::LoadTAPFile(TCHAR *filename)
{
HANDLE hfile=0;
DWORD file_size;
BOOL r;
DWORD bytes_read;

	UnloadTAP();

	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not open tape file %s."),filename);
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size || file_size > 5000000L || file_size < 50L){
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("%s is not a valid raw tape file."),filename);
	}

	pTapeHeader = (RAWTAPE *) GlobalAlloc(GMEM_FIXED, file_size);
	if (pTapeHeader == NULL)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("Could not allocate memory for tape file %s."),filename);
	}
	r=ReadFile(hfile, pTapeHeader, file_size, &bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0 || bytes_read!=file_size)
	{
		GlobalFree(pTapeHeader);
		pTapeHeader=NULL;
		return SetError(E_FAIL,TEXT("Could not read from tape file %s."),filename);
	}
	if (_strnicmp((char *)&pTapeHeader->Signature[0],"C64-TAPE-RAW", sizeof(pTapeHeader->Signature))!=0)
	{
		GlobalFree(pTapeHeader);
		pTapeHeader=NULL;
		return SetError(E_FAIL,TEXT("%s is not a valid raw tape file."),filename);
	}

	if (pTapeHeader->Version !=0 && pTapeHeader->Version !=1)
	{		
		GlobalFree(pTapeHeader);
		pTapeHeader=NULL;
		return SetError(E_FAIL,TEXT("%s is in version %d format.\nOnly versions 0 and 1 are supported.")
			, filename ,(int)pTapeHeader->Version);
	}
	pTapeData = &pTapeHeader->data[0];
	tape_length = file_size - sizeof(RAWTAPE) + 1;
	return S_OK;
}


/*********************************************************************************/

Tape64::Tape64()
{
	TapeEvent = NULL;
	bMotorOn = false;
	bPlayDown = false;
	bEOT = true;
	bEOD = false;
	Eject();
}

void Tape64::PressPlay()
{
	bEOD = false;
	bPlayDown=true;
}
void Tape64::PressStop()
{
	bPlayDown=false;
}

void Tape64::Eject()
{
	if (pTapeHeader)
	{
		GlobalFree(pTapeHeader);
		pTapeHeader=NULL;
		pTapeData=NULL;
	}
	tape_position=0;
	tape_length=0;
	tape_pulse_length=0;
	bEOT = true;
	nextTapeTickClock=0;
}

void Tape64::SetMotorWrite(bool motor, bit8 write)
{
	bMotorOn = motor;
}


void Tape64::Rewind()
{
	tape_position=0;
	tape_pulse_length=0;
	if (tape_length > 0 && pTapeHeader!=NULL)
	{
		bEOT = false;
	}
	else
		bEOT = true;
}


HRESULT Tape64::InsertTAPFile(TCHAR *filename)
{
HRESULT hr;

	Eject();
	
	hr = LoadTAPFile(filename);
	if (FAILED(hr))
		return hr;

	Rewind();
	return S_OK;
}

void Tape64::Tick(ICLK sysclock)
{
ICLK clocks;
	if (!bMotorOn || pTapeHeader==NULL || !bPlayDown || bEOT)
	{
		CurrentClock = sysclock;
		nextTapeTickClock = sysclock + 0x10000000;
		return;
	}

	clocks = (sysclock - CurrentClock);
	while (((ICLKS)clocks) > 0)
	{
		if (tape_pulse_length > clocks)
		{
			tape_pulse_length-= clocks;
			CurrentClock = sysclock;
			nextTapeTickClock = sysclock + tape_pulse_length;
			return;
		}
		clocks -= tape_pulse_length;
		CurrentClock = sysclock - clocks;
		if (TapeEvent && !bEOD)
			TapeEvent->Pulse(CurrentClock);
		
		if (tape_position < tape_length)
		{
			tape_pulse_length = (bit32)pTapeData[tape_position] * (bit32)8;
			if (tape_pulse_length == 0)
			{
				if (pTapeHeader->Version==0)
				{
					tape_position++;
					tape_pulse_length = 32768;
				}
				else
				{
					tape_pulse_length =*( (bit32 *) (&pTapeData[tape_position]));
					tape_pulse_length>>=8;
					tape_position+=4;
					if (tape_pulse_length == 0)
						tape_pulse_length = 1;
				}
			}
			else
				tape_position++;
		}
		else
		{
			//Frosty2 doesnt like the play button being instantly released after the last pulse.
			if (!bEOD)
			{
				bEOD = true;
				tape_pulse_length = 0x130A490;
			}
			else
			{
				tape_pulse_length=0;
				bEOT = true;
				bEOD = false;
				if (TapeEvent)
					TapeEvent->EndOfTape(CurrentClock);
				CurrentClock = sysclock;
				return;
			}
		}
	}
	nextTapeTickClock = sysclock + tape_pulse_length;
}
