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

const int Cart::RAMRESERVEDSIZE = 64 * 1024 + 8 * 1024;//Assume 64K cart RAM + 8K zero byte bank
const int Cart::ZEROBANKOFFSET = 64 * 1024;

CartCommon::CartCommon(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory)
	: m_crtHeader(pCart->m_crtHeader), m_lstBank(pCart->m_lstBank)
{
	this->m_pCart = pCart;
	this->m_pCpu = pCpu;
	this->m_pC64RamMemory = pC64RamMemory;

	this->m_pCartData = pCart->m_pCartData;
	this->m_pZeroBankData = pCart->m_pZeroBankData;

	this->m_ipROML_8000 = pCart->m_pZeroBankData - 0x8000;
	this->m_ipROMH_A000 = pCart->m_pZeroBankData - 0xA000;
	this->m_ipROMH_E000 = pCart->m_pZeroBankData - 0xE000;
	m_bIsCartAttached = false;
	InitReset(pCpu->Get6510CurrentClock());
}

CartCommon::~CartCommon()
{
}

void CartCommon::ConfigureMemoryMap()
{
	if (this->IsCartAttached())
	{
		UpdateIO();
	}
	else
	{
		GAME = 1;
		EXROM = 1;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();
	}
	m_pCpu->ConfigureMemoryMap();
}

bit8 *CartCommon::Get_RomH()
{
	return this->m_ipROMH_E000 + 0xE000;
}

void CartCommon::PreventClockOverflow()
{
}

ICLK CartCommon::GetCurrentClock()
{
	return CurrentClock;
}

void CartCommon::SetCurrentClock(ICLK sysclock)
{
	this->CurrentClock = sysclock;
}

void CartCommon::CheckForCartFreeze()
{
}

void CartCommon::ExecuteCycle(ICLK sysclock)
{
	CurrentClock = sysclock;
}

bit8 CartCommon::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void CartCommon::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

bit8 CartCommon::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	m_bEffects = false;
	bit8 r = ReadRegister(address, sysclock);
	m_bEffects = true;
	return r;
}

void CartCommon::BankRom()
{
	m_ipROML_8000 = m_pZeroBankData - 0x8000;
	m_ipROMH_A000 = m_pZeroBankData - 0xA000;
	m_ipROMH_E000 = m_pZeroBankData - 0xE000;
	SIZE_T i = m_iSelectedBank;
	if (i < m_lstBank.size())
	{
		Sp_CrtBank p = m_lstBank[i];
		if (p)
		{
			CrtChipAndData* pL = &p->chipAndDataLow;
			CrtChipAndData* pH = &p->chipAndDataHigh;
			if (pL->chip.ROMImageSize != 0)
			{
				if (pL->chip.ROMImageSize <= 0x2000)
				{
					m_ipROML_8000 = pL->pData - 0x8000;
				}
				else if (pL->chip.ROMImageSize <= 0x4000)
				{
					m_ipROML_8000 = pL->pData - 0x8000;
					m_ipROMH_A000 = &pL->pData[0x2000] - 0xA000;
					m_ipROMH_E000 = &pL->pData[0x2000] - 0xE000;
				}
			}
			if (pH->chip.ROMImageSize != 0)
			{
				m_ipROMH_E000 = pH->pData - 0xE000;
				m_ipROMH_A000 = pH->pData - 0xA000;
			}
		}
	}
}

void CartCommon::CartFreeze()
{
}

void CartCommon::InitReset(ICLK sysclock)
{
	CurrentClock = sysclock;
	m_bEffects = true;
	m_iSelectedBank = 0;
	m_bFreezePending = false;
	m_bFreezeDone = false;
	m_bREUcompatible = false;
	m_bAllowBank = false;
	m_iRamBankOffsetIO = 0;
	m_iRamBankOffsetRomL = 0;
	m_bEnableRAM = false;
	m_bIsCartIOActive = true;
	m_bIsCartRegActive = true;
	reg1 = 0;
	reg2 = 0;
	m_pCpu->Clear_CRT_IRQ();
	m_pCpu->Clear_CRT_NMI();
}

void CartCommon::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	ConfigureMemoryMap();
}

void CartCommon::CartReset()
{
	if (m_bIsCartAttached)
	{
		this->Reset(m_pCpu->Get6510CurrentClock());
		m_pCpu->Reset6510(m_pCpu->Get6510CurrentClock());
	}
}


void CartCommon::AttachCart()
{
	if (!m_bIsCartAttached)
	{
		m_bIsCartAttached = true;
		this->Reset(m_pCpu->Get6510CurrentClock());
		if (m_bFreezePending)
		{
			CartFreeze();
		}
		ConfigureMemoryMap();
	}
}

void CartCommon::DetachCart()
{
	if (m_bIsCartAttached)
	{
		m_bIsCartAttached = false;
		ConfigureMemoryMap();
	}
}

bit8 CartCommon::Get_GAME()
{
	return this->GAME;
}

bit8 CartCommon::Get_EXROM()
{
	return this->EXROM;
}

bool CartCommon::IsCartIOActive()
{
	return this->m_bIsCartAttached && this->m_bIsCartIOActive;
}

bool CartCommon::IsCartAttached()
{
	return this->m_bIsCartAttached;
}

bool CartCommon::IsUltimax()
{
	return GAME == 0 && EXROM != 0;
}


bit8 CartCommon::ReadROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		bit16 addr = address - 0x8000;
		return m_pCartData[addr + m_iRamBankOffsetRomL];
	}
	else
	{
		return m_ipROML_8000[address];
	}
}

void CartCommon::WriteROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		m_pCartData[address - 0x8000 + m_iRamBankOffsetRomL] = data;
	}
	m_pC64RamMemory[address] = data;
}

bit8 CartCommon::ReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xC000);
	return m_ipROMH_A000[address];
}

void CartCommon::WriteROMH(bit16 address, bit8 data)
{
	assert(address >= 0xA000 && address < 0xC000);
	m_pC64RamMemory[address] = data;
}

bit8 CartCommon::ReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		bit16 addr = address - 0x8000;
		return m_pCartData[addr + m_iRamBankOffsetRomL];
	}
	else
	{
		return m_ipROML_8000[address];
	}
}

void CartCommon::WriteUltimaxROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		m_pCartData[address - 0x8000 + m_iRamBankOffsetRomL] = data;
	}
}

bit8 CartCommon::ReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	return m_ipROMH_E000[address];
}

void CartCommon::WriteUltimaxROMH(bit16 address, bit8 data)
{
	assert(address >= 0xE000);
}

bit8 CartCommon::MonReadROML(bit16 address)
{
	m_bEffects = false;
	bit8 r = ReadROML(address);
	m_bEffects = true;
	return r;
}

bit8 CartCommon::MonReadROMH(bit16 address)
{
	m_bEffects = false;
	bit8 r = ReadROMH(address);
	m_bEffects = true;
	return r;
}

bit8 CartCommon::MonReadUltimaxROML(bit16 address)
{
	m_bEffects = false;
	bit8 r = ReadUltimaxROML(address);
	m_bEffects = true;
	return r;
}

bit8 CartCommon::MonReadUltimaxROMH(bit16 address)
{
	m_bEffects = false;
	bit8 r = ReadUltimaxROMH(address);
	m_bEffects = true;
	return r;
}

void CartCommon::MonWriteROML(bit16 address, bit8 data)
{
	m_bEffects = false;
	WriteROML(address, data);
	m_bEffects = true;
}

void CartCommon::MonWriteROMH(bit16 address, bit8 data)
{
	m_bEffects = false;
	WriteROMH(address, data);
	m_bEffects = true;
}

void CartCommon::MonWriteUltimaxROML(bit16 address, bit8 data)
{
	m_bEffects = false;
	WriteUltimaxROML(address, data);
	m_bEffects = true;
}

void CartCommon::MonWriteUltimaxROMH(bit16 address, bit8 data)
{
	m_bEffects = false;
	WriteUltimaxROMH(address, data);
	m_bEffects = true;
}

Cart::Cart()
{
	m_crtHeader.EXROM = 1;
	m_crtHeader.GAME = 1;
	m_crtHeader.HardwareType = 0;
	m_pCartData = 0;
	m_pCpu = NULL;
	m_bIsCartDataLoaded = false;
}

Cart::~Cart()
{
	CleanUp();
}

void Cart::Init(IC6510 *pCpu, bit8 *pC64RamMemory)
{
	this->m_pCpu = pCpu;
	this->m_pC64RamMemory = pC64RamMemory;
}

void Cart::CleanUp()
{
	m_bIsCartDataLoaded = false;
	m_lstBank.clear();
	if (m_pCartData)
	{
		GlobalFree(m_pCartData);
		m_pCartData = 0;
		m_pZeroBankData = 0;
	}
}

bool Cart::IsSupported()
{
	return IsSupported((CartType::ECartType)this->m_crtHeader.HardwareType);
}

bool Cart::IsSupported(CartType::ECartType hardwareType)
{
	switch(hardwareType)
	{
	case CartType::Normal_Cartridge:
	case CartType::Action_Replay:
	case CartType::Final_Cartridge_III:
	case CartType::Simons_Basic:
	case CartType::Ocean_1:
	case CartType::Fun_Play:
	case CartType::Super_Games:
	case CartType::Epyx_FastLoad:
	case CartType::System_3:
	case CartType::Dinamic:
	case CartType::Zaxxon:
	case CartType::Magic_Desk:
	case CartType::Action_Replay_4:
	case CartType::EasyFlash:
	case CartType::Action_Replay_3:
	case CartType::Retro_Replay:
	case CartType::Action_Replay_2:
		return true;
	}
	return false;
}

int Cart::GetTotalCartMemoryRequirement()
{
	return GetTotalCartMemoryRequirement(m_lstBank);
}

int Cart::GetTotalCartMemoryRequirement(CrtBankList lstBank)
{
	int i = RAMRESERVEDSIZE;
	for (CrtBankListIter it = lstBank.begin(); it!=lstBank.end(); it++)
	{
		Sp_CrtBank sp = *it;
		if (!sp)
			continue;
		i = i + (int)(sp->chipAndDataLow.allocatedSize);
		i = i + (int)(sp->chipAndDataHigh.allocatedSize);
	}
	return i;
}

HRESULT Cart::LoadCrtFile(LPCTSTR filename)
{
HRESULT hr = E_FAIL;
HANDLE hFile = NULL;
DWORD nBytesRead;
DWORD nBytesToRead;
BOOL br;
LPCTSTR S_READFAILED = TEXT("Could not read file %s.");
LPCTSTR S_OUTOFMEMORY = TEXT("Out of memory.");
CrtHeader hdr;
__int64 pos = 0;
__int64 spos = 0;
bit8 S_SIGHEADER[] = "C64 CARTRIDGE";
bit8 S_SIGCHIP[] = "CHIP";
CrtBankList lstBank;
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
			nBytesToRead = sizeof(hdr);
			br = ReadFile(hFile, &hdr, nBytesToRead, &nBytesRead, NULL);
			if (!br || nBytesRead < nBytesToRead)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			if (_strnicmp((char *)&hdr.Signature, (char *)&S_SIGHEADER[0], _countof(S_SIGHEADER) - 1) != 0)
			{
				hr = SetError(E_FAIL, TEXT("Cartridge signature not found."));
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
			
			lstBank.resize(MAXBANKS);
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
				nBytesToRead = sizeof(chip);
				br = ReadFile(hFile, &chip, nBytesToRead, &nBytesRead, NULL);
				if (!br || nBytesRead < nBytesToRead)
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
				if (nChipCount > 2*MAXBANKS)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}

				switch (chip.ChipType)
				{
				case ChipType::ROM:
				case ChipType::RAM:
				case ChipType::EPROM:
					break;
				default:
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (!ok)
					break;
				
				iFileIndex = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (iFileIndex < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (chip.BankLocation >= lstBank.size())
				{
					lstBank.resize(chip.BankLocation + 1);
				}

				Sp_CrtBank spBank = lstBank[chip.BankLocation];
				if (!spBank)
				{
					spBank = Sp_CrtBank(new CrtBank());
					if (spBank == 0)
						throw std::bad_alloc();
					spBank->bank = chip.BankLocation;
					lstBank[chip.BankLocation] = spBank;
				}
				CrtChipAndData *pChipAndData;
				bit16 romOffset = 0;
				if (chip.LoadAddressRange >= 0x8000 && chip.LoadAddressRange < 0xA000 && chip.ROMImageSize > 0 && chip.ROMImageSize <= 0x4000)
				{
					pChipAndData = &spBank->chipAndDataLow;
					romOffset = chip.LoadAddressRange - 0x8000;
				}
				else if (chip.LoadAddressRange >= 0xA000  && chip.LoadAddressRange < 0xC000 && chip.ROMImageSize > 0 && chip.ROMImageSize <= 0x2000)
				{
					pChipAndData = &spBank->chipAndDataHigh;
					romOffset = chip.LoadAddressRange - 0xA000;
				}
				else if (chip.LoadAddressRange >= 0xE000 && chip.ROMImageSize > 0 && chip.ROMImageSize <= 0x2000)
				{
					pChipAndData = &spBank->chipAndDataHigh;
					romOffset = chip.LoadAddressRange - 0xE000;
				}
				else
				{
					hr = SetError(E_FAIL, TEXT("Unsupported chip in bank $%x address $%0.4x with size $%0.4x. 16K chip sizes must reside at $8000-9FFF and 8K chip sizes must reside at $8000-9FFF or $A000-BFFF or $E000-FFFF."), (int)chip.BankLocation, (int)chip.LoadAddressRange, (int)chip.ROMImageSize);
					ok = false;
					break;
				}
				pChipAndData->chip = chip;
				pChipAndData->iFileIndex = iFileIndex;
				pChipAndData->romOffset = romOffset;
				if (chip.ROMImageSize <= 0x2000)
					pChipAndData->allocatedSize = 0x2000;
				else
					pChipAndData->allocatedSize = 0x4000;

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
			SIZE_T lenAlloc = GetTotalCartMemoryRequirement(lstBank);
			pCartData = (bit8 *)GlobalAlloc(GPTR, lenAlloc);
			if (!pCartData)
			{
				ok = false;
				hr = SetError(E_OUTOFMEMORY, S_OUTOFMEMORY);
				break;
			}
			bit8 *p = pCartData + (INT_PTR)RAMRESERVEDSIZE;
			for (CrtBankListIter it = lstBank.begin(); it!=lstBank.end(); it++)
			{
				Sp_CrtBank sp = *it;
				if (!sp)
					continue;
				for (int i = 0; i < 2; i++)
				{
					CrtChipAndData* pChipAndData;
					if (i==0)
						pChipAndData = &sp->chipAndDataLow;
					else
						pChipAndData = &sp->chipAndDataHigh;
					if (pChipAndData->chip.ROMImageSize > 0)
					{
						pChipAndData->pData = p;
						if (pChipAndData->chip.ChipType == ChipType::ROM || pChipAndData->chip.ChipType == ChipType::EPROM)//If ROM or EPROM then read from file.
						{
							iFileIndex = G::FileSeek(hFile, pChipAndData->iFileIndex, FILE_BEGIN);
							if (iFileIndex < 0)
							{
								hr = SetError(E_FAIL, S_READFAILED, filename);
								ok = false;
								break;
							}
							nBytesToRead = pChipAndData->chip.ROMImageSize;
							if ((DWORD)pChipAndData->romOffset + (DWORD)pChipAndData->chip.ROMImageSize > (DWORD)pChipAndData->allocatedSize)
							{
								nBytesToRead = (DWORD)pChipAndData->allocatedSize - (DWORD)pChipAndData->romOffset;
							}
							if (nBytesToRead > 0)
							{
								br = ReadFile(hFile, &p[pChipAndData->romOffset], nBytesToRead, &nBytesRead, NULL);
								if (!br || nBytesRead < nBytesToRead)
								{
									hr = SetError(E_FAIL, S_READFAILED, filename);
									ok = false;
									break;
								}
							}
						}
					}
					p = p + (INT_PTR)(pChipAndData->allocatedSize);				
				}
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
		if (lstBank.size() == 0)
		{
			hr = SetError(E_FAIL, S_READFAILED, filename);
		}
		else
		{
			switch(hdr.HardwareType)
			{
			case CartType::Zaxxon:
				if (lstBank[0] && lstBank[0]->chipAndDataLow.allocatedSize == 0x2000 && lstBank[0]->chipAndDataLow.pData!=0)
				{
					CopyMemory(&lstBank[0]->chipAndDataLow.pData[0x1000], &lstBank[0]->chipAndDataLow.pData[0], 0x1000);
				}
				
				if (lstBank[1])
				{
					lstBank[1]->chipAndDataLow = lstBank[0]->chipAndDataLow;
					lstBank[1]->chipAndDataLow.chip.BankLocation = 1;
				}
				break;
			}
			if (this->m_spCurrentCart)
			{
				m_spCurrentCart->DetachCart();
				m_spCurrentCart = 0;
			}
			m_lstBank.clear();
			if (m_pCartData)
			{
				GlobalFree(m_pCartData);
				m_pCartData = NULL;
				m_pZeroBankData = 0;
			}
			m_crtHeader = hdr;
			m_lstBank = lstBank;
			m_pCartData = pCartData;
			m_pZeroBankData = &pCartData[ZEROBANKOFFSET];
			pCartData = NULL;
			m_bIsCartDataLoaded = true;

			if (!IsSupported())
				hr = SetError(APPWARN_UNKNOWNCARTTYPE, TEXT("The hardware type for this cartridge is not supported. The emulator will attempt to run the ROM images with generic hardware. The cartridge software may not run correctly."));
		}
	}
	if (pCartData)
	{
		GlobalFree(pCartData);
		pCartData = NULL;		
	}
	
	return hr;
}

shared_ptr<ICartInterface> Cart::CreateCartInterface(IC6510 *pCpu, bit8 *pC64RamMemory)
{
shared_ptr<ICartInterface> p;
	try
	{
		switch(this->m_crtHeader.HardwareType)
		{
		case CartType::Normal_Cartridge:
			p = shared_ptr<ICartInterface>(new CartNormalCartridge(this, pCpu, pC64RamMemory));
			break;
		case CartType::Action_Replay:
			p = shared_ptr<ICartInterface>(new CartActionReplay(this, pCpu, pC64RamMemory));
			break;
		case CartType::Final_Cartridge_III:
			p = shared_ptr<ICartInterface>(new CartFinalCartridgeIII(this, pCpu, pC64RamMemory));
			break;
		case CartType::Simons_Basic:
			p = shared_ptr<ICartInterface>(new CartSimonsBasic(this, pCpu, pC64RamMemory));
			break;
		case CartType::Ocean_1:
			p = shared_ptr<ICartInterface>(new CartOcean1(this, pCpu, pC64RamMemory));
			break;
		case CartType::Fun_Play:
			p = shared_ptr<ICartInterface>(new CartFunPlay(this, pCpu, pC64RamMemory));
			break;
		case CartType::Super_Games:
			p = shared_ptr<ICartInterface>(new CartSuperGames(this, pCpu, pC64RamMemory));
			break;
		case CartType::System_3:
			p = shared_ptr<ICartInterface>(new CartSystem3(this, pCpu, pC64RamMemory));
			break;
		case CartType::Dinamic:
			p = shared_ptr<ICartInterface>(new CartDinamic(this, pCpu, pC64RamMemory));
			break;
		case CartType::Zaxxon:
			p = shared_ptr<ICartInterface>(new CartZaxxon(this, pCpu, pC64RamMemory));
			break;
		case CartType::Magic_Desk:
			p = shared_ptr<ICartInterface>(new CartMagicDesk(this, pCpu, pC64RamMemory));
			break;
		case CartType::Action_Replay_4:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk4(this, pCpu, pC64RamMemory));
			break;
		case CartType::EasyFlash:
			p = shared_ptr<ICartInterface>(new CartEasyFlash(this, pCpu, pC64RamMemory));
			break;
		case CartType::Action_Replay_3:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk3(this, pCpu, pC64RamMemory));
			break;
		case CartType::Retro_Replay:
			p = shared_ptr<ICartInterface>(new CartRetroReplay(this, pCpu, pC64RamMemory));
			break;
		case CartType::Action_Replay_2:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk2(this, pCpu, pC64RamMemory));			
			break;
		case CartType::Epyx_FastLoad:
			p = shared_ptr<ICartInterface>(new CartEpyxFastLoad(this, pCpu, pC64RamMemory));
			break;
		}
	}
	catch(std::bad_alloc&)
	{
		SetError(E_OUTOFMEMORY, ErrorMsg::ERR_OUTOFMEMORY);
		p = 0;
	}
	return p;
}

void Cart::Reset(ICLK sysclock)
{
	if (m_spCurrentCart)
		m_spCurrentCart->Reset(sysclock);
}

void Cart::ExecuteCycle(ICLK sysclock)
{
	if (m_spCurrentCart)
		m_spCurrentCart->ExecuteCycle(sysclock);
}

bit8 Cart::ReadRegister(bit16 address, ICLK sysclock)
{
	return m_spCurrentCart->ReadRegister(address, sysclock);
}

void Cart::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	m_spCurrentCart->WriteRegister(address, sysclock, data);
}

bit8 Cart::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	return m_spCurrentCart->ReadRegister_no_affect(address, sysclock);
}

bit8 Cart::Get_GAME()
{
	return m_spCurrentCart->Get_GAME();
}

bit8 Cart::Get_EXROM()
{
	return m_spCurrentCart->Get_EXROM();
}

bit8 Cart::ReadROML(bit16 address)
{
	return m_spCurrentCart->ReadROML(address);
}

bit8 Cart::ReadROMH(bit16 address)
{
	return m_spCurrentCart->ReadROMH(address);
}

bit8 Cart::ReadUltimaxROML(bit16 address)
{
	return m_spCurrentCart->ReadUltimaxROML(address);
}

bit8 Cart::ReadUltimaxROMH(bit16 address)
{
	return m_spCurrentCart->ReadUltimaxROMH(address);
}

void Cart::WriteROML(bit16 address, bit8 data)
{
	m_spCurrentCart->WriteROML(address, data);
}

void Cart::WriteROMH(bit16 address, bit8 data)
{
	m_spCurrentCart->WriteROMH(address, data);
}

void Cart::WriteUltimaxROML(bit16 address, bit8 data)
{
	m_spCurrentCart->WriteUltimaxROML(address, data);
}

void Cart::WriteUltimaxROMH(bit16 address, bit8 data)
{
	m_spCurrentCart->WriteUltimaxROMH(address, data);
}

bit8 Cart::MonReadROML(bit16 address)
{
	return m_spCurrentCart->MonReadROML(address);
}

bit8 Cart::MonReadROMH(bit16 address)
{
	return m_spCurrentCart->MonReadROMH(address);
}

bit8 Cart::MonReadUltimaxROML(bit16 address)
{
	return m_spCurrentCart->MonReadUltimaxROML(address);
}

bit8 Cart::MonReadUltimaxROMH(bit16 address)
{
	return m_spCurrentCart->MonReadUltimaxROMH(address);
}

void Cart::MonWriteROML(bit16 address, bit8 data)
{
	m_spCurrentCart->MonWriteROML(address, data);
}

void Cart::MonWriteROMH(bit16 address, bit8 data)
{
	m_spCurrentCart->MonWriteROMH(address, data);
}

void Cart::MonWriteUltimaxROML(bit16 address, bit8 data)
{
	m_spCurrentCart->MonWriteUltimaxROML(address, data);
}

void Cart::MonWriteUltimaxROMH(bit16 address, bit8 data)
{
	m_spCurrentCart->MonWriteUltimaxROMH(address, data);
}

bool Cart::IsUltimax()
{
	return m_bIsCartDataLoaded && m_spCurrentCart && m_spCurrentCart->IsUltimax();
}

bool Cart::IsCartIOActive()
{
	return m_bIsCartDataLoaded && m_spCurrentCart && m_spCurrentCart->IsCartIOActive();
}

bool Cart::IsCartAttached()
{
	return m_bIsCartDataLoaded && m_spCurrentCart && m_spCurrentCart->IsCartAttached();
}

void Cart::CartFreeze()
{
	if (this->IsCartAttached())
		m_spCurrentCart->CartFreeze();
}

void Cart::CartReset()
{
	if (this->IsCartAttached())
		m_spCurrentCart->CartReset();
}

void Cart::CheckForCartFreeze()
{
	if (m_spCurrentCart)
		m_spCurrentCart->CheckForCartFreeze();
}

void Cart::AttachCart()
{
	if (this->m_bIsCartDataLoaded)
	{
		m_spCurrentCart = this->CreateCartInterface(this->m_pCpu, this->m_pC64RamMemory);
		if (m_spCurrentCart)
		{
			m_spCurrentCart->AttachCart();
		}
	}
}

void Cart::DetachCart()
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->DetachCart();
		m_spCurrentCart = 0;
	}
	CleanUp();
}

void Cart::ConfigureMemoryMap()
{
	if (m_spCurrentCart)
		m_spCurrentCart->ConfigureMemoryMap();
}

bit8 *Cart::Get_RomH()
{
	if (m_spCurrentCart)
		return m_spCurrentCart->Get_RomH();
	else
		return 0;
}

void Cart::PreventClockOverflow()
{
	if (m_spCurrentCart)
		m_spCurrentCart->PreventClockOverflow();
}

ICLK Cart::GetCurrentClock()
{
	if (m_spCurrentCart)
		return m_spCurrentCart->GetCurrentClock();
	else
		return CurrentClock;
}

void Cart::SetCurrentClock(ICLK sysclock)
{
	CurrentClock = sysclock;
	if (m_spCurrentCart)
		m_spCurrentCart->SetCurrentClock(sysclock);
}

CrtChipAndData::CrtChipAndData()
{
	ZeroMemory(&chip, sizeof(chip));
	pData = NULL;
	iFileIndex = 0;
	allocatedSize = 0;
	romOffset = 0;
}

CrtBank::CrtBank()
{
	bank = 0;
}
CrtBank::~CrtBank()
{
}


