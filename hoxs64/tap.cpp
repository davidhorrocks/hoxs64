#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include "filestream.h"
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "CDPI.h"
#include "resource.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "savestate.h"
#include "register.h"
#include "tap.h"

TAP64::TAP64()
{
	pData=NULL;
	tape_max_counter=0;
}
TAP64::~TAP64()
{
	UnloadTAP();
}

void TAP64::UnloadTAP()
{
	if (pData)
		GlobalFree(pData);
	pData=NULL;
	tape_max_counter=0;
}

HRESULT TAP64::LoadTAPFile(TCHAR *filename)
{
HANDLE hfile = INVALID_HANDLE_VALUE;
DWORD file_size;
BOOL r;
DWORD bytesRead;
DWORD bytesToRead;
RAWTAPE header;
HRESULT hr = E_FAIL;
int c;
bit32 *buffer = NULL;

	ClearError();
	do
	{
		hfile = CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		if (hfile == INVALID_HANDLE_VALUE)
		{
			hr = SetError(E_FAIL,TEXT("Could not open tape file %s."),filename);
			break;
		}

		file_size = GetFileSize(hfile, 0);
		if (INVALID_FILE_SIZE == file_size || file_size > 5000000L || file_size < sizeof(RAWTAPE) + sizeof(bit32))
		{
			hr = SetError(E_FAIL,TEXT("%s is not a valid raw tape file."), filename);
			break;
		}

		ZeroMemory(&header, sizeof(header));
		bytesToRead = sizeof(header) - sizeof(header.data);
		r = ReadFile(hfile, &header, bytesToRead, &bytesRead, NULL);
		if (r==0)
		{
			hr = SetError(E_FAIL,TEXT("Could not read from tape file %s."), filename);
			break;
		}

		if (_strnicmp((char *)&header.Signature[0],"C64-TAPE-RAW", sizeof(header.Signature))!=0)
		{
			hr = SetError(E_FAIL,TEXT("%s is not a valid raw tape file."), filename);
			break;
		}

		if (header.Version !=0 && header.Version !=1)
		{		
			hr = SetError(E_FAIL,TEXT("%s is in version %d format.\nOnly versions 0 and 1 are supported.")
				, filename ,(int)header.Version);
			break;
		}

		LARGE_INTEGER spos_zero;
		LARGE_INTEGER spos;
		ULARGE_INTEGER spos_dummy;
		spos_zero.QuadPart = 0;

		hr = S_OK;
		if (FAILED(hr))
			break;

		hr = ReadTapeData(hfile, header.Version, NULL, 0, &c);
		if (FAILED(hr))
		{
			break;
		}
		if (c==0)
		{
			hr = E_FAIL;
			break;
		}

		buffer = (bit32 *) GlobalAlloc(GMEM_FIXED, c * sizeof(bit32));
		if (buffer == NULL)
		{
			hr = SetError(E_FAIL,TEXT("Could not allocate memory for tape file %s."), filename);
			break;
		}
		spos.QuadPart = sizeof(RAWTAPE) - 1;
		if (FileStream::SetFilePointerEx(hfile, spos, (PLARGE_INTEGER) &spos_dummy, FILE_BEGIN) == 0)
		{
			hr = this->SetErrorFromGetLastError();
			break;
		}
		hr = ReadTapeData(hfile, header.Version, buffer, c, NULL);
		if (FAILED(hr))
		{
			break;
		}
		hr = S_OK;
	}
	while (false);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hfile);
		hfile = INVALID_HANDLE_VALUE;
	}
	if (FAILED(hr))
	{
		if (buffer)
		{
			GlobalFree(buffer);
			buffer=NULL;
		}
	}
	else
	{
		UnloadTAP();
		this->tape_max_counter = c;
		this->TapeHeader = header;
		this->pData = buffer;
		buffer = NULL;
	}
	return hr;
}

HRESULT TAP64::ReadTapeData(HANDLE hfile, int version, bit32 *buffer, int bufferlen, int *count)
{
BOOL r;
DWORD bytesRead;
DWORD byteToRead;
HRESULT hr = E_FAIL;

	int c = 0;
	bool eof = false;
	for (c=0; !eof; c++)
	{
		bit8 v;
		bit32 w;
		byteToRead = 1;
		r = ReadFile(hfile, &v, byteToRead, &bytesRead, NULL);
		if (r==0 && GetLastError() != ERROR_HANDLE_EOF)
		{
			hr = SetErrorFromGetLastError();
			break;
		}
		else if (bytesRead < byteToRead)
		{
			eof = true;
			break;
		}
		if (buffer)
		{
			if (c == bufferlen)
				return E_FAIL;
		}
		if (v == 0)
		{
			if (version == 0)
			{
				if (buffer)
				{
					buffer[c] = DEFAULTDELAY;
				}
			}
			else
			{
				//LITTLE ENDIAN
				w = 0;
				byteToRead = 3;
				r = ReadFile(hfile, &w, byteToRead, &bytesRead, NULL);
				if (r == 0)
				{
					hr = SetErrorFromGetLastError();
					break;
				}
				else if (bytesRead < byteToRead)
				{
					eof = true;
					break;
				}
				if (buffer)
				{
					buffer[c] = w;
				}
			}
		}
		else
		{
			if (buffer)
			{
				if (c == bufferlen)
					return E_FAIL;
				buffer[c] = v * 8;
			}
		}
	}
	if (eof)
	{
		hr = S_OK;
	}
	
	if (count)
	{
		*count = c;
	}
	return hr;
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
	if (pData)
	{
		GlobalFree(pData);
		pData=NULL;
	}
	tape_position=0;
	tape_max_counter=0;
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
	if (tape_max_counter > 0 && pData!=NULL)
	{
		bEOT = false;
	}
	else
		bEOT = true;
}

HRESULT Tape64::InsertTAPFile(TCHAR *filename)
{
HRESULT hr;

	ClearError();
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
	if (!bMotorOn || pData==NULL || !bPlayDown || bEOT)
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
		{
			TapeEvent->Pulse(CurrentClock);
		}
		
		if (tape_position < tape_max_counter)
		{
			tape_pulse_length = (bit32)pData[tape_position];
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
				{
					TapeEvent->EndOfTape(CurrentClock);
				}
				CurrentClock = sysclock;
				return;
			}
		}
	}
	nextTapeTickClock = sysclock + tape_pulse_length;
}

void Tape64::GetState(SsTape &state)
{
	state.CurrentClock = CurrentClock;
	state.tape_max_counter = tape_max_counter;
	state.bRecordDown = 0;
	state.tape_position = tape_position;
	state.tape_pulse_length = tape_pulse_length;
	state.nextTapeTickClock = nextTapeTickClock;
	state.bMotorOn = bMotorOn;
	state.bPlayDown = bPlayDown;
	state.bEOT = bEOT;
	state.bEOD = bEOD;
}

void Tape64::SetState(const SsTape &state)
{
	CurrentClock = state.CurrentClock;
	//state.bRecordDown;
	tape_max_counter = state.tape_max_counter;
	tape_position = state.tape_position;
	tape_pulse_length = state.tape_pulse_length;
	nextTapeTickClock = state.nextTapeTickClock;
	bMotorOn = state.bMotorOn != 0;
	bPlayDown = state.bPlayDown != 0;
	bEOT = state.bEOT != 0;
	bEOD = state.bEOD != 0;
}

