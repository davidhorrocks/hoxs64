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
	m_bREUcompatible = false;
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

void Cart::Init(IC6510 *pCpu, bit8 *pC64RamMemory)
{
	m_pCpu = pCpu;
	m_pC64RamMemory = pC64RamMemory;
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

			//Read header.
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
			//TEST
			//hdr.HardwareType = 36;
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

				//Read chip header
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
				chip.ChipType = wordswap(chip.ChipType);				
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
				if (chip.ROMImageSize != 0x2000 && chip.ROMImageSize != 0x4000)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				switch (chip.ChipType)
				{
					case 0://ROM
					case 1://RAM
					case 2://Flash ROM
						break;
					default:
						hr = SetError(E_FAIL, S_READFAILED, filename);
						ok = false;
						break;
				}
				if (!ok)
					break;
				
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
			SIZE_T lenAlloc = GetTotalCartMemoryRequirement(lstChipAndData);
			pCartData = (bit8 *)GlobalAlloc(GPTR, lenAlloc);
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
				if ((*it)->chip.ChipType == 1)
					continue;
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
			m_crtHeader = hdr;
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
	m_bIsCartAttached = false;
	m_lstChipAndData.clear();
	if (m_pCartData)
	{
		GlobalFree(m_pCartData);
		m_pCartData = 0;
	}
}

void Cart::InitReset(ICLK sysclock)
{
	CurrentClock = sysclock;
	reg1 = 0;
	reg2 = 0;
	m_bFreezePending = false;
	m_bFreezeDone= false;
	m_bDE01WriteDone = false;
	UpdateIO();
}

void Cart::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	m_pCpu->Clear_CRT_IRQ();
	m_pCpu->Clear_CRT_NMI();
	ConfigureMemoryMap();
}

void Cart::DetachCart()
{
	if (m_bIsCartAttached)
	{
		m_bFreezePending = false;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();
		m_bIsCartAttached = false;
		ConfigureMemoryMap();
		CleanUp();
	}
}

bool Cart::IsCartAttached()
{
	return m_bIsCartAttached;
}

bool Cart::IsUltimax()
{
	return GAME == 0 && EXROM != 0;
}

void Cart::ExecuteCycle(ICLK sysclock)
{
	CurrentClock = sysclock;
}

void Cart::UpdateIO()
{
//TODO CartType
	if (m_bIsCartAttached)
	{
		switch (m_crtHeader.HardwareType)
		{
		case CartType::Retro_Replay:
			if (m_bFreezePending)
			{
				m_bAllowBank = false;
				m_iSelectedBank = 0;
				m_iRamBankOffset = 0;
				GAME = 1;
				EXROM = 1;
				m_bEnableRAM = false;
				m_bIsCartIOActive = false;
			}
			else if (m_bFreezeDone)
			{
				m_bREUcompatible = (reg2 & 0x40) != 0;
				m_bAllowBank = (reg2 & 0x2) != 0;
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_iRamBankOffset = 0;
				if (m_bAllowBank)
				{
					m_iRamBankOffset = (bit16)(((int)m_iSelectedBank) & 3 << 13); 
				}
				//Ultimax
				GAME = 0;
				EXROM = 1;
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_bIsCartIOActive = true;
			}
			else
			{
				m_bREUcompatible = (reg2 & 0x40) != 0;
				m_bAllowBank = (reg2 & 0x2) != 0;
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_iRamBankOffset = 0;
				if (m_bAllowBank)
				{
					m_iRamBankOffset = (bit16)(((int)m_iSelectedBank) & 3 << 13); 
				}
				GAME = (~reg1 & 1);
				EXROM = (reg1 >> 1) & 1;
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;			
			}
			break;
		case CartType::Action_Replay://AR5 + AR6
			if (m_bFreezePending)
			{
				m_bAllowBank = false;
				m_iSelectedBank = 0;
				m_iRamBankOffset = 0;
				GAME = 1;
				EXROM = 1;
				m_bEnableRAM = false;
				m_bIsCartIOActive = false;
			}
			else if (m_bFreezeDone)	
			{
				m_bREUcompatible = false;
				m_bAllowBank = false;
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_iRamBankOffset = 0;
				//Ultimax
				GAME = 0;
				EXROM = 1;
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_bIsCartIOActive = true;
			}
			else
			{
				m_bREUcompatible = false;
				m_bAllowBank = false;
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_iRamBankOffset = 0;
				GAME = (~reg1 & 1);
				EXROM = (reg1 >> 1) & 1;
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;
			}
			break;
		case CartType::Ocean_1:
			m_iSelectedBank = reg1 & 0x3f;
			GAME = m_crtHeader.GAME;
			EXROM = m_crtHeader.EXROM;
			m_bIsCartIOActive = true;
			break;
		case CartType::Magic_Desk:
			m_bREUcompatible = false;
			m_bAllowBank = false;
			m_iSelectedBank = reg1 & 0x3f;
			if (reg1 == 0x80)
			{
				GAME = 1;
				EXROM = 1;
				m_bEnableRAM = true;
			}
			else
			{
				GAME = m_crtHeader.GAME;
				EXROM = m_crtHeader.EXROM;
				m_bEnableRAM = false;
			}
			m_bIsCartIOActive = true;
			break;
		default: 
			m_bREUcompatible = false;
			m_bAllowBank = false;
			m_iSelectedBank = 0;
			m_iRamBankOffset = 0;
			GAME = m_crtHeader.GAME;
			EXROM = m_crtHeader.EXROM;
			m_bEnableRAM = false;
			m_bIsCartIOActive = false;			
			break;
		}
	}
	else
	{
		GAME = 1;
		EXROM = 1;
		m_bIsCartIOActive = false;			
	}
}

void Cart::ConfigureMemoryMap()
{
	UpdateIO();
	m_pCpu->ConfigureMemoryMap();
}

void Cart::CheckForCartFreeze()
{
//TODO CartType
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeDone = true;
		reg1 &= 0x20;
		reg2 &= 0x43;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();
		ConfigureMemoryMap();
	}
}

void Cart::CartFreeze()
{
//TODO CartType
	if (!m_bIsCartAttached)
		return;
	switch(m_crtHeader.HardwareType)
	{
		case CartType::Retro_Replay:
		case CartType::Action_Replay://AR5 + AR6
		case CartType::Action_Replay_2:
		case CartType::Action_Replay_4:
		case CartType::Action_Replay_3:
			if ((reg2 & 0x04) == 0)
			{
				m_pCpu->Set_CRT_IRQ(m_pCpu->GetCurrentClock());
				m_pCpu->Set_CRT_NMI(m_pCpu->GetCurrentClock());
				m_bFreezePending = true;
				m_bFreezeDone = false;
			}
			break;
	}
}

void Cart::CartReset()
{
	if (!m_bIsCartAttached)
		return;
	this->Reset(m_pCpu->GetCurrentClock());
	m_pCpu->Reset(m_pCpu->GetCurrentClock());
}

bit8 Cart::ReadRegister(bit16 address, ICLK sysclock)
{
//TODO CartType
	if (!m_bIsCartAttached)
		return 0;
	switch (m_crtHeader.HardwareType)
	{
	case CartType::Retro_Replay:
		if ((address == 0xDE00 || address == 0xDE01) && m_bIsCartIOActive)
		{
			return (reg1 & 0xB8) | (reg2 & 0x42);
		}
		else if (address >= 0xDE00 && address < 0xDF00 && m_bREUcompatible)
		{
			bit16 addr = address - 0xDE00 + 0x1E00;
			if (m_bEnableRAM)
			{
				return m_pCartData[addr + m_iRamBankOffset];
			}
			else
			{
				if (m_iSelectedBank < m_lstChipAndData.size() && addr < m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
					return m_lstChipAndData[m_iSelectedBank]->pData[addr];
			}
		}
		else if (address >= 0xDF00 && address < 0xE000 && !m_bREUcompatible)
		{
			bit16 addr = address - 0xDF00 + 0x1F00;
			if (m_bEnableRAM)
			{
				return m_pCartData[addr + m_iRamBankOffset];
			}
			else
			{
				if (m_iSelectedBank < m_lstChipAndData.size() && addr < m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
					return m_lstChipAndData[m_iSelectedBank]->pData[addr];
			}
		}
		break;
	case CartType::Action_Replay:
		if ((address >= 0xDE00 && address < 0xDF00) && m_bIsCartIOActive)
		{
			return 0;
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			bit16 addr = address - 0xDF00 + 0x1F00;
			if (m_bEnableRAM)
			{
				return m_pCartData[addr];
			}
			else
			{
				if (m_iSelectedBank < m_lstChipAndData.size() && addr < m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
					return m_lstChipAndData[m_iSelectedBank]->pData[addr];
			}
		}
		break;
	case CartType::Ocean_1:
		if (address == 0xDE00)
		{
			return reg1;
		}
		break;
	case CartType::Magic_Desk:
		if (address == 0xDE00)
		{
			return reg1;
		}
		break;
	}
	return 0;
}

void Cart::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
//TODO CartType
	if (!m_bIsCartAttached)
		return;
	switch (m_crtHeader.HardwareType)
	{
	case CartType::Retro_Replay:
		if (address == 0xDE00 && m_bIsCartIOActive)
		{
			if (m_bFreezeDone)
			{
				if (data & 0x40)
				{
					m_bFreezePending = false;
					m_bFreezeDone = false;
				}
			}
			reg1 = data;
			ConfigureMemoryMap();
		}
		else if (address == 0xDE01 && m_bIsCartIOActive)
		{
			if (m_bDE01WriteDone)
			{
				reg2 = (reg2 & 0x86) | (data & ~0x86);
			}
			else
			{
				reg2 = data;
				m_bDE01WriteDone = true;
			}
			reg1 = (reg1 & 0x63) | (data & 0x98);
			ConfigureMemoryMap();
		}
		else if (m_bREUcompatible && address >= 0xDE00 && address < 0xDF00)
		{
			if (m_bEnableRAM)
			{
				m_pCartData[address - 0xDE00 + 0x1E00 + m_iRamBankOffset] = data;
			}
		}
		else if (!m_bREUcompatible && address >= 0xDF00 && address < 0xE000)
		{
			if (m_bEnableRAM)
			{
				m_pCartData[address - 0xDF00 + 0x1F00 + m_iRamBankOffset] = data;
			}
		}
		break;
	case CartType::Action_Replay:
		if (address >= 0xDE00 && address < 0xDF00 && m_bIsCartIOActive)
		{
			if (m_bFreezeDone)
			{
				if (data & 0x40)
				{
					m_bFreezePending = false;
					m_bFreezeDone = false;
				}
			}
			reg1 = data;
			ConfigureMemoryMap();
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			if (m_bEnableRAM)
			{
				m_pCartData[address - 0xDF00 + 0x1F00] = data;
			}
		}
		break;
	case CartType::Ocean_1:
		if (address == 0xDE00)
		{
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
	case CartType::Magic_Desk:
		if (address == 0xDE00)
		{
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
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
	if (m_bEnableRAM)
	{
		return m_pCartData[addr + m_iRamBankOffset];
	}
	else
	{
		if (m_iSelectedBank >= m_lstChipAndData.size() || addr >= m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
			return 0;
		return m_lstChipAndData[m_iSelectedBank]->pData[addr];
	}
}

bit8 Cart::ReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	bit16 addr = address - 0x8000;
	if (m_bEnableRAM)
	{
		return m_pCartData[addr + m_iRamBankOffset];
	}
	else
	{
		if (m_iSelectedBank >= m_lstChipAndData.size() || addr >= m_lstChipAndData[m_iSelectedBank]->chip.ROMImageSize)
			return 0;
		return m_lstChipAndData[m_iSelectedBank]->pData[addr];
	}
}

bit8 Cart::ReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xE000);
	bit16 addr = address - 0xA000;
	if (m_iSelectedBank >= m_lstChipAndData.size())
		return 0;
	Sp_CrtChipAndData p = m_lstChipAndData[m_iSelectedBank];
	if (p->chip.ROMImageSize > 0x2000)
		address += 0x2000;
	if (addr >= p->chip.ROMImageSize)
		return 0;
	return p->pData[addr];
}

bit8 Cart::ReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	bit16 addr = address - 0xE000;
	if (m_iSelectedBank >= m_lstChipAndData.size())
		return 0;
	Sp_CrtChipAndData p = m_lstChipAndData[m_iSelectedBank];
	if (p->chip.ROMImageSize > 0x2000)
		address += 0x2000;
	if (addr >= p->chip.ROMImageSize)
		return 0;
	return p->pData[addr];
}

void Cart::WriteROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		m_pCartData[address - 0x8000 + m_iRamBankOffset] = data;
	}
	m_pC64RamMemory[address] = data;
}

void Cart::WriteUltimaxROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		m_pCartData[address - 0x8000 + m_iRamBankOffset] = data;
	}
}

void Cart::WriteROMH(bit16 address, bit8 data)
{
	assert(address >= 0xA000 && address < 0xE000);
	m_pC64RamMemory[address] = data;
}

void Cart::WriteUltimaxROMH(bit16 address, bit8 data)
{
	assert(address >= 0xE000);
}

bool Cart::IsCartIOActive()
{
	return m_bIsCartAttached && m_bIsCartIOActive;
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
	return i;
}

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
