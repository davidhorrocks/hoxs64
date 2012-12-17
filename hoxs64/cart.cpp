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
	reg2 = 0;
	m_pCartData = 0;
	m_bIsCartAttached = false;
	m_bIsCartIOActive = false;
	m_iSelectedBank = 0;
	m_iRamBankOffset = 0;
	m_bAllowBank = false;
	GAME = 1;
	EXROM = 1;
	m_pCpu = NULL;
	m_bEnableRAM = false;
	m_bFreezePending = false;
	m_bFreezeDone= false;
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
	reg2 = 0;
	ConfigureMemoryMap();
}

void Cart::ExecuteCycle(ICLK sysclock)
{
	CurrentClock = sysclock;
}

void Cart::ConfigureMemoryMap()
{
	if (this->m_bIsCartAttached)
	{
		if (m_bFreezePending)
		{
			this->m_bAllowBank = false;
			this->m_iSelectedBank = 0;
			this->m_iRamBankOffset = 0;
			this->GAME = 1;
			this->EXROM = 1;
			this->m_bEnableRAM = false;
			this->m_bIsCartIOActive = false;
		}
		else if (m_bFreezeDone)
		{
			this->m_bAllowBank = (reg2 & 0x2) != 0;
			this->m_iSelectedBank = 0;
			this->m_iRamBankOffset = 0;
			this->GAME = 0;
			this->EXROM = 0;
			this->m_bEnableRAM = false;
			this->m_bIsCartIOActive = true;
		}
		else
		{
			this->m_bAllowBank = (reg2 & 0x2) != 0;
			this->m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
			this->m_iRamBankOffset = 0;
			if (m_bAllowBank)
			{
				this->m_iRamBankOffset = (bit16)(((int)this->m_iSelectedBank) & 3 << 13); 
			}
			this->GAME = (~reg1 & 1);
			this->EXROM = (reg1 >> 1) & 1;
			this->m_bEnableRAM = (reg1 & 0x20) != 0;
			this->m_bIsCartIOActive = (reg1 & 0x4) == 0;
		}
	}
	else
	{
		this->m_bAllowBank = false;
		this->GAME = 1;
		this->EXROM = 1;
		this->m_iSelectedBank = 0;
		this->m_iRamBankOffset = 0;
	}
	m_pCpu->ConfigureMemoryMap();
}

void Cart::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeDone = true;
		m_iSelectedBank = 0;
		m_iRamBankOffset = 0;
		reg1 &= 0x18;
		ConfigureMemoryMap();
	}
}

bit8 Cart::ReadRegister(bit16 address, ICLK sysclock)
{
	if (!this->m_bIsCartAttached)
		return 0;
	if ((address == 0xDE00 || address == 0xDE01) && this->m_bIsCartIOActive)
	{
		return (reg1 & 0xB8) | (reg2 & 2);
	}
	else if (address >= 0xDE00 && address < 0xE000)
	{
		if (this->m_bEnableRAM)
		{
			return this->m_pCartData[address - 0xDE00 + m_iRamBankOffset];
		}
		else
		{
			bit16 addr = address - 0xDE00 + 0x1E00;
			if (m_iSelectedBank < this->m_lstChipAndData.size() && addr < this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
				return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
		}
	}
	else if (address >= 0x8000 && address < 0xA000)
	{
		if (this->m_bEnableRAM)
		{
			return this->m_pCartData[address - 0x8000 + m_iRamBankOffset];
		}
		else
		{
			bit16 addr = address - 0x8000;
			if (m_iSelectedBank < this->m_lstChipAndData.size() && addr < this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
				return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
		}
	}
	else if (address >= 0xA000 || address < 0xE000)
	{
		bit16 addr = address - 0xA000;
		if (m_iSelectedBank < this->m_lstChipAndData.size() && addr < this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
			return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
	}
	else if (address >= 0xE000)
	{
		bit16 addr = address - 0xE000;
		if (m_iSelectedBank < this->m_lstChipAndData.size() && addr < this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
			return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
	}
	return 0;
}

void Cart::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (!this->m_bIsCartAttached)
		return;
	if (address == 0xDE00 && this->m_bIsCartIOActive)
	{
		this->reg1 = data;
		this->ConfigureMemoryMap();
	}
	else if (address == 0xDE01 && this->m_bIsCartIOActive)
	{
		this->reg2 = data;
		this->reg1 = (this->reg1 & 0x63) | (data & 0x98);
		this->ConfigureMemoryMap();
	}
	else if (address >= 0xDE00 && address < 0xE000)
	{
		if (this->m_bEnableRAM)
		{
			this->m_pCartData[address - 0xDE00 + 0x1E00 + m_iRamBankOffset] = data;
		}
	}
	else if (address >= 0x8000 && address < 0xA000)
	{
		if (!this->m_bFreezeDone)
		{
			this->m_pCartData[address - 0x8000 + m_iRamBankOffset] = data;
		}
	}
}

bit8 Cart::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	return ReadRegister(address, sysclock);
}

bit8 Cart::ReadROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	bit16 addr = address - 0x8000;
	if (m_iSelectedBank >= this->m_lstChipAndData.size() || addr >= this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
		return 0;
	return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
}

bit8 Cart::ReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	bit16 addr = address - 0x8000;
	if (m_iSelectedBank >= this->m_lstChipAndData.size() || addr >= this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
		return 0;
	return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
}

bit8 Cart::ReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xE000);
	bit16 addr = address - 0xA000;
	if (m_iSelectedBank >= this->m_lstChipAndData.size() || addr >= this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
		return 0;
	return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
}

bit8 Cart::ReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	bit16 addr = address - 0xE000;
	if (m_iSelectedBank >= this->m_lstChipAndData.size() || addr >= this->m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
		return 0;
	return this->m_lstChipAndData[m_iSelectedBank]->pData[addr];
}

void Cart::WriteROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (!this->m_bFreezeDone)
	{
		this->m_pCartData[address - 0x8000 + m_iRamBankOffset] = data;
	}
}

void Cart::WriteUltimaxROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (!this->m_bFreezeDone)
	{
		this->m_pCartData[address - 0x8000 + m_iRamBankOffset] = data;
	}
}

void Cart::WriteROMH(bit16 address, bit8 data)
{
	assert(address >= 0xA000 && address < 0xE000);
}

void Cart::WriteUltimaxROMH(bit16 address, bit8 data)
{
	assert(address >= 0xE000);
}

bool Cart::IsCartIOActive()
{
	return this->m_bIsCartAttached && this->m_bIsCartIOActive;
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
