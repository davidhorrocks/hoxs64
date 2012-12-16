#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <list>
#include <algorithm>

#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "errormsg.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "cart.h"

const int Cart::RAMRESERVEDSIZE = 64 * 1024;//Assume 64K cart RAM

Cart::Cart()
{
	m_crtHeader.EXROM = 1;
	m_crtHeader.GAME = 1;
	m_crtHeader.HardwareType = 0;
	reg1 = 0;
	m_pCartData = 0;
	m_bIsCartAttached = false;
	m_bIsCartIOActive = false;
	m_bSelectedBank = 0;
	GAME = 1;
	EXROM = 1;
	m_pCpu = NULL;
	m_ClockCartReset = 0;
	m_bEnableRAM = false;
}

Cart::~Cart()
{
	CleanUp();
}

void Cart::Init(IC6502 *pCpu)
{
	this->m_pCpu = pCpu;
}

HRESULT Cart::LoadCrtFile(LPCTSTR filename)
{
HRESULT hr = E_FAIL;
HANDLE hFile = NULL;
DWORD nBytesRead;
BOOL br;
LPCTSTR S_READFAILED = TEXT("Could not read file %s.");
LPCTSTR S_OUTOFMEMORY = TEXT("Out of memory.");
CrtHeader hdr;
__int64 pos = 0;
__int64 spos = 0;
bit8 S_SIGHEADER[] = "C64 CARTRIDGE";
bit8 S_SIGCHIP[] = "CHIP";
CrtChipAndDataList lstChipAndData;
bit8 *pCartData = NULL;
__int64 filesize=0;
const int MAXBANKS = 256;
__int64 iFileIndex = 0;

	ClearError();
	
	try
	{
		bool ok = true;
		do
		{
			lstChipAndData.reserve(MAXBANKS);
			hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				hr =  SetError(E_FAIL, TEXT("Could not open crt file %s."), filename);
				ok = false;
				break;
			}
			filesize = G::FileSize(hFile);
			if (filesize < 0)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			br = ReadFile(hFile, &hdr, sizeof(hdr), &nBytesRead, NULL);
			if (!br)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			if (_strnicmp((char *)&hdr.Signature, (char *)&S_SIGHEADER[0], _countof(S_SIGHEADER) - 1) != 0)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			hdr.HardwareType = wordswap(hdr.HardwareType);
			hdr.FileHeaderLength = dwordswap(hdr.FileHeaderLength);
			hdr.Version = wordswap(hdr.Version);
			if (hdr.FileHeaderLength != 0 && hdr.FileHeaderLength != sizeof(hdr))
			{
				pos = G::FileSeek(hFile, (__int64)hdr.FileHeaderLength, FILE_BEGIN);
				if (pos < 0  || pos != hdr.FileHeaderLength)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
			}

			int nChipCount = 0;
			do
			{
				CrtChip chip;
				spos = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (spos < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (filesize - spos < sizeof(chip))
				{
					//ok
					break;
				}
				br = ReadFile(hFile, &chip, sizeof(chip), &nBytesRead, NULL);
				if (!br)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}

				chip.BankLocation = wordswap(chip.BankLocation);
				chip.LoadAddressRange = wordswap(chip.LoadAddressRange);
				chip.ROMImageSize = wordswap(chip.ROMImageSize);
				chip.TotalPacketLength = dwordswap(chip.TotalPacketLength);

				if (_strnicmp((char *)&chip.Signature, (char *)&S_SIGCHIP[0], _countof(S_SIGCHIP) - 1) != 0)
				{
					//ok
					break;
				}
				if (nChipCount > MAXBANKS)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (chip.ROMImageSize ==0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				
				Sp_CrtChipAndData sp(new CrtChipAndData(chip, NULL));
				if (sp == 0)
					throw std::bad_alloc();
				iFileIndex = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (iFileIndex < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				sp->iFileIndex = iFileIndex;
				lstChipAndData.push_back(sp);
				nChipCount++;
				pos = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (pos < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (chip.TotalPacketLength != pos - spos)
				{
					__int64 nextpos = spos + (__int64)chip.TotalPacketLength;
					if (nextpos >= filesize)
					{
						//ok
						break;
					}
					pos = G::FileSeek(hFile, nextpos, FILE_BEGIN);
					if (pos < 0)
					{
						hr = SetError(E_FAIL, S_READFAILED, filename);
						ok = false;
						break;
					}
				}
			} while (ok);
			if (!ok)
				break;
		} while (false);
		do
		{
			if (!ok)
				break;
			pCartData = (bit8 *)GlobalAlloc(GPTR, this->GetTotalCartMemoryRequirement(lstChipAndData));
			if (!pCartData)
			{
				ok = false;
				hr = SetError(E_OUTOFMEMORY, S_OUTOFMEMORY);
				break;
			}
			bit8 *p = pCartData + (INT_PTR)RAMRESERVEDSIZE;
			for (CrtChipAndDataIter it = lstChipAndData.begin(); it!=lstChipAndData.end(); it++)
			{
				(*it)->pData = p;
				iFileIndex = G::FileSeek(hFile, (*it)->iFileIndex, FILE_BEGIN);
				if (iFileIndex < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				br = ReadFile(hFile, p, (*it)->chip.ROMImageSize, &nBytesRead, NULL);
				if (!br)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				p = p + (INT_PTR)((*it)->chip.ROMImageSize);
			}
			if (!ok)
				break;
		} while (false);
		if (ok)
		{
			hr = S_OK;
		}
	}
	catch (std::exception&)
	{
		hr = SetError(E_FAIL, S_READFAILED, filename);
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

	if (SUCCEEDED(hr))
	{
		if (lstChipAndData.size() == 0)
		{
			hr = SetError(E_FAIL, S_READFAILED, filename);
		}
		else
		{
			std::sort(lstChipAndData.begin(), lstChipAndData.end(), LessChipAndDataBank());
			m_lstChipAndData.clear();
			if (m_pCartData)
			{
				GlobalFree(m_pCartData);
				m_pCartData = NULL;
			}
			m_lstChipAndData = lstChipAndData;
			m_pCartData = pCartData;
			pCartData = NULL;
		}
	}
	if (pCartData)
	{
		GlobalFree(pCartData);
		pCartData = NULL;
	}	
	return hr;
}

void Cart::CleanUp()
{
	m_lstChipAndData.clear();
	if (m_pCartData)
	{
		GlobalFree(m_pCartData);
		m_pCartData = 0;
	}
}


void Cart::Reset(ICLK sysclock)
{
	CurrentClock = sysclock;
	reg1 = 0;
	if (this->m_bIsCartAttached)
	{
		m_state = CartReset;

		m_ClockCartReset = sysclock + 100;
	}
	ConfigureMemoryMap();
	SetWakeUpClock();
}

void Cart::ExecuteCycle(ICLK sysclock)
{
	if (m_state == Normal)
	{
		CurrentClock = sysclock;
	}
	else 
	{
		CurrentClock = sysclock;
		if ((ICLKS) (m_ClockCartReset - CurrentClock) <= 0)
		{
			m_ClockCartReset = 0;
			m_state = Normal;
			this->ConfigureMemoryMap();
		}		
	}
	SetWakeUpClock();
}

void Cart::ConfigureMemoryMap()
{
	if (this->m_bIsCartAttached)
	{
		if (this->m_state == Normal)
		{
			this->m_bSelectedBank = (reg1 >> 3) & 3;
			this->GAME = (~reg1 & 1);
			this->EXROM = (reg1 >> 1) & 1;
			this->m_bEnableRAM = (reg1 & 0x20) != 0;
		}
		this->m_bIsCartIOActive = (reg1 & 0x4)==0;
	}
	else
	{
		this->m_state = Normal;
		this->GAME = 1;
		this->EXROM = 1;
	}
	m_pCpu->ConfigureMemoryMap();
}

void Cart::SetWakeUpClock()
{
	switch (m_state)
	{
	case Normal:
		ClockNextWakeUpClock = CurrentClock + 0x10000000;
		break;
	case CartReset:
	case CartFreeze:
		ClockNextWakeUpClock = m_ClockCartReset;
		break;
	}
}

void Cart::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = 0x4000;
	const ICLKS CLOCKSYNCBAND_FAR = 0x40000000;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;

	if ((ICLKS)(CurrentClock - m_ClockCartReset) >= CLOCKSYNCBAND_FAR)
		m_ClockCartReset = ClockBehindNear;
}

bit8 Cart::ReadRegister(bit16 address, ICLK sysclock)
{
	this->ExecuteCycle(sysclock);
	if (address >= 0xDE00 && address < 0xDF00)
	{
		if (this->m_bIsCartAttached)
		{
			if (this->m_bIsCartIOActive)
			{
				return reg1;
			}
		}
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (this->m_bIsCartAttached)
		{
			if (this->m_bIsCartIOActive)
			{
				if (this->m_bEnableRAM)
				{
					return this->m_pCartData[address - 0xDF00 + 0x9F00];
				}
			}
		}
	}
	return 0;
}

void Cart::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	this->ExecuteCycle(sysclock);
	if (address >= 0xDE00 && address < 0xDF00)
	{
		if (this->m_bIsCartIOActive)
		{
			this->reg1 = data;
			this->ConfigureMemoryMap();
		}
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (this->m_bIsCartAttached)
		{
			if (this->m_bIsCartIOActive)
			{
				if (this->m_bEnableRAM)
				{
					this->m_pCartData[address - 0xDF00 + 0x9F00] = data;
				}
			}
		}
	}

	SetWakeUpClock();
}

bit8 Cart::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	return 0;
}

bool Cart::IsCartRegister(bit16 address)
{
	return this->m_bIsCartAttached && this->m_bIsCartIOActive && address >= 0xDE00 && address < 0xE000;
}

int Cart::GetTotalCartMemoryRequirement()
{
	return GetTotalCartMemoryRequirement(m_lstChipAndData);
}

int Cart::GetTotalCartMemoryRequirement(CrtChipAndDataList lstChip)
{
	int i = RAMRESERVEDSIZE;
	for (CrtChipAndDataConstIter it = lstChip.cbegin(); it!=lstChip.cend(); it++)
	{
		i = i + (int)((*it)->chip.ROMImageSize);
	}
	return i;}

bool LessChipAndDataBank::operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const
{
	return x->chip.BankLocation < y->chip.BankLocation;
}

CrtChipAndData::~CrtChipAndData()
{
}

CrtChipAndData::CrtChipAndData(CrtChip &chip, bit8 *pData)
{
	this->chip = chip;
	this->pData = pData;
}
