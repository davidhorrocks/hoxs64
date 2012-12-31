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

#define ACTIONREPLAYMK2DISABLEROMCOUNT (20)
#define ACTIONREPLAYMK2DISABLEROMTRIGGER (200)

#define ACTIONREPLAYMK2ENABLEROMCOUNT (20)
#define ACTIONREPLAYMK2ENABLEROMTRIGGER (12)

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
	m_bIsCartRegActive = false;
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
	m_ipROML_8000 = NULL;
	m_ipROMH_A000 = NULL;
	m_ipROMH_E000 = NULL;
	m_bEffects = true;
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
			br = ReadFile(hFile, &hdr, sizeof(hdr), &nBytesRead, NULL);
			if (!br)
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
				if (nChipCount > 2*MAXBANKS)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				//if ((chip.LoadAddressRange != 0x8000 || (chip.ROMImageSize != 0x2000 && chip.ROMImageSize != 0x4000)) && (chip.LoadAddressRange != 0xA000 || (chip.ROMImageSize != 0x2000)) && (chip.LoadAddressRange != 0xE000 || (chip.ROMImageSize != 0x2000)))
				//{
				//	hr = SetError(E_FAIL, TEXT("Unsupported chip in bank $%x address $%0.4x with size $%0.4x. 16K chip sizes must start at $8000 and 8K chip sizes must start at $8000 or $A000 or $E000."), (int)chip.BankLocation, (int)chip.LoadAddressRange, (int)chip.ROMImageSize);
				//	ok = false;
				//	break;
				//}
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
						if (pChipAndData->chip.ChipType == 0 || pChipAndData->chip.ChipType == 2)//If ROM or EPROM then read from file.
						{
							iFileIndex = G::FileSeek(hFile, pChipAndData->iFileIndex, FILE_BEGIN);
							if (iFileIndex < 0)
							{
								hr = SetError(E_FAIL, S_READFAILED, filename);
								ok = false;
								break;
							}
							DWORD nBytesToRead = pChipAndData->chip.ROMImageSize;
							if ((DWORD)pChipAndData->romOffset + (DWORD)pChipAndData->chip.ROMImageSize > (DWORD)pChipAndData->allocatedSize)
							{
								nBytesToRead = (DWORD)pChipAndData->allocatedSize - (DWORD)pChipAndData->romOffset;
							}
							if (nBytesToRead > 0)
							{
								br = ReadFile(hFile, &p[pChipAndData->romOffset], nBytesToRead, &nBytesRead, NULL);
								if (!br)
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
#if (_MSC_VER < 1600)
				this->OnReadROML = boost::bind(&Cart::ReadROML_Zaxxon, this, _1);
				this->OnReadUltimaxROML = boost::bind(&Cart::ReadUltimaxROML_Zaxxon, this, _1);
#else
				this->OnReadROML = std::bind(&Cart::ReadROML_Zaxxon, this, std::placeholders::_1);
				this->OnReadUltimaxROML = std::bind(&Cart::ReadUltimaxROML_Zaxxon, this, std::placeholders::_1);
#endif
				if (lstBank[0] && lstBank[0]->chipAndDataLow.allocatedSize == 0x2000 && lstBank[0]->chipAndDataLow.pData!=0)
				{
					CopyMemory(&lstBank[0]->chipAndDataLow.pData[0x1000], &lstBank[0]->chipAndDataLow.pData[0], 0x1000);
				}
				
				if (lstBank[1])
				{
					lstBank[1]->chipAndDataLow = lstBank[0]->chipAndDataLow;
				}
				break;
			default:
#if (_MSC_VER < 1600)
				this->OnReadROML = boost::bind(&Cart::ReadROML, this, _1);
				this->OnReadUltimaxROML = boost::bind(&Cart::ReadUltimaxROML, this, _1);
#else
				this->OnReadROML = std::bind(&Cart::ReadROML, this, std::placeholders::_1);
				this->OnReadUltimaxROML = std::bind(&Cart::ReadUltimaxROML, this, std::placeholders::_1);
#endif
				break;
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
			m_ipROML_8000 = m_pZeroBankData - 0x8000;
			m_ipROMH_A000 = m_pZeroBankData - 0xA000;
			m_ipROMH_E000 = m_pZeroBankData - 0xE000;
			pCartData = NULL;

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

void Cart::CleanUp()
{
	m_bIsCartAttached = false;
	m_lstBank.clear();
	if (m_pCartData)
	{
		GlobalFree(m_pCartData);
		m_pCartData = 0;
		m_pZeroBankData = 0;
	}
}

void Cart::InitReset(ICLK sysclock)
{
	CurrentClock = sysclock;
	reg1 = 0;
	reg2 = 0;
	m_iSelectedBank = 0;
	m_bSimonsBasic16K = false;
	m_bFreezePending = false;
	m_bFreezeDone = false;
	m_bDE01WriteDone = false;
	m_bIsCartIOActive = true;
	m_bREUcompatible = false;
	m_bAllowBank = false;
	m_iRamBankOffset = 0;
	m_bEnableRAM = false;

	if (this->m_bIsCartAttached)
	{
		GAME = m_crtHeader.GAME;
		EXROM = m_crtHeader.EXROM;
	}
	else
	{
		GAME = 1;
		EXROM = 1;
	}

	m_bActionReplayMk2Rom = true;
	m_iActionReplayMk2EnableRomCounter=0;
	m_iActionReplayMk2DisableRomCounter=0;
	m_clockLastDE00Write = sysclock;
	m_clockLastDF40Read = sysclock;

	switch(this->m_crtHeader.HardwareType)
	{
	case CartType::EasyFlash:
		reg1 = 0;
		reg2 = 5;
		break;
	}
	//UpdateIO called here to allow the CPU to see the correct reset vector since cpu.Reset() is called before cart.Reset()
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
				BankRom();
			}
			else if (m_bFreezeDone)
			{
				m_bREUcompatible = (reg2 & 0x40) != 0;
				m_bAllowBank = (reg2 & 0x2) != 0;
				m_iSelectedBank = ((reg1 & 0x18) >> 3) | ((reg1 & 0x80) >> 5);// | ((reg1 & 0x20) >> 2);
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_iRamBankOffset = 0;
				if (m_bAllowBank)
				{
					m_iRamBankOffset = (bit16)(((int)m_iSelectedBank) & 3 << 13);//Maximum of 64K RAM available in non flash mode.
				}
				//Ultimax
				GAME = 0;
				EXROM = 1;
				m_bIsCartIOActive = true;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
			}
			else
			{
				m_bREUcompatible = (reg2 & 0x40) != 0;
				m_bAllowBank = (reg2 & 0x2) != 0;
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_iRamBankOffset = 0;
				if (m_bAllowBank)
				{
					m_iRamBankOffset = (bit16)(((int)m_iSelectedBank) & 3 << 13); 
				}
				GAME = (~reg1 & 1);
				EXROM = (reg1 >> 1) & 1;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
			}
			break;
		case CartType::Action_Replay://AR5 + AR6
			if (m_bFreezePending)
			{
				BankRom();
			}
			else if (m_bFreezeDone)	
			{
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_iRamBankOffset = 0;
				//Ultimax
				GAME = 0;
				EXROM = 1;
				m_bIsCartIOActive = true;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
			}
			else
			{
				m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
				m_bEnableRAM = (reg1 & 0x20) != 0;
				m_iRamBankOffset = 0;
				GAME = (~reg1 & 1);
				EXROM = (reg1 >> 1) & 1;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
			}			
			break;
		case CartType::Action_Replay_4:
			if (m_bFreezePending)
			{
				BankRom();
			}
			else
			{
				m_iSelectedBank = ((reg1) & 1) | ((reg1 >> 3) & 2);
				m_bEnableRAM = false;
				m_iRamBankOffset = 0;
				GAME = (reg1 >> 1) & 1;
				EXROM = (~reg1 >> 3) & 1;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;
				BankRom();
				switch(((reg1 & 2) >> 1) | ((reg1 & 8) >> 2))
				{
				case 0://GAME=0 EXROM=1 Ultimax
					//E000 mirrors 8000
					m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
					m_ipROML_8000 = m_pZeroBankData - 0x8000;
					break;
				case 1://GAME=1 EXROM=1
					break;
				case 2://GAME=0 EXROM=0
					//A000 mirrors 8000
					m_ipROMH_A000 = m_ipROML_8000 + 0x8000 - 0xA000;
					break;
				case 3://GAME=1 EXROM=0
					m_ipROMH_A000 = m_ipROML_8000 + 0x8000 - 0xA000;
					break;
				}
			}			
			break;
		case CartType::Action_Replay_3:
			if (m_bFreezePending)
			{
				BankRom();
			}
			else
			{
				m_iSelectedBank = ((reg1) & 1);
				m_bEnableRAM = false;
				m_iRamBankOffset = 0;
				GAME = (reg1 >> 1) & 1;//0;
				EXROM = (~reg1 >> 3) & 1;
				m_bIsCartIOActive = (reg1 & 0x4) == 0;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
				m_ipROMH_A000 = m_ipROML_8000 + 0x8000 - 0xA000;
			}			
			break;
		case CartType::Action_Replay_2:
			m_bEnableRAM = false;
			m_iRamBankOffset = 0;
			if (m_bFreezePending)
			{
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
				m_ipROMH_A000 = m_ipROML_8000 + 0x8000 - 0xA000;
			}
			else
			{
				if (m_bActionReplayMk2Rom)
				{
					if (m_iSelectedBank == 0)
					{
						GAME = 0;
						EXROM = 1;
					}
					else
					{
						GAME = 1;
						EXROM = 0;
					}
				}
				else
				{
					GAME = 1;
					EXROM = 1;
				}
				m_bIsCartIOActive = true;
				BankRom();
				m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
				m_ipROMH_A000 = m_ipROML_8000 + 0x8000 - 0xA000;
			}			
			break;
		case CartType::Final_Cartridge_III:
			if (m_bFreezePending)
			{
				BankRom();
			}
			else
			{
				m_iSelectedBank = reg1 & 3;
				GAME = (reg1 & 0x20) != 0;
				EXROM = (reg1 & 0x10) != 0;
				if (reg1 & 0x40)
					m_pCpu->Clear_CRT_NMI();
				else
					m_pCpu->Set_CRT_NMI(m_pCpu->GetCurrentClock());
				m_bIsCartIOActive = true;
				BankRom();
			}
			break;
		case CartType::EasyFlash:
			m_iSelectedBank = reg1 & 0x3f;
			GAME = (reg2 & 0x1) == 0;
			EXROM = (reg2 & 0x2) == 0;
			m_bIsCartIOActive = true;
			BankRom();
			break;
		case CartType::Ocean_1:
			m_iSelectedBank = reg1 & 0x3f;
			GAME = m_crtHeader.GAME;
			EXROM = m_crtHeader.EXROM;
			m_bIsCartIOActive = true;
			BankRom();
			break;
		case CartType::Magic_Desk:
			m_iSelectedBank = reg1 & 0x3f;
			if (reg1 & 0x80)
			{
				GAME = 1;
				EXROM = 1;
			}
			else
			{
				GAME = m_crtHeader.GAME;
				EXROM = m_crtHeader.EXROM;
			}
			m_bIsCartIOActive = true;
			BankRom();
			break;
		case CartType::System_3:
		case CartType::Dinamic:
			if (m_bIsCartIOActive)
			{
				GAME = m_crtHeader.GAME;
				EXROM = m_crtHeader.EXROM;
			}
			else
			{
				GAME = 1;
				EXROM = 1;
			}
			BankRom();
			break;
		case CartType::Fun_Play:
			//m_iSelectedBank = (bit8)(((reg1 & 0x38) >> 3) | ((reg1 & 0x1) << 3));
			m_iSelectedBank = reg1;
			if (reg1 == 0x86)
			{
				m_bIsCartIOActive = false;
				GAME = 1;
				EXROM = 1;
			}
			else
			{
				GAME = m_crtHeader.GAME;
				EXROM = m_crtHeader.EXROM;
			}
			BankRom();
			break;
		case CartType::Super_Games:
			m_iSelectedBank = reg1 & 3;
			GAME = (reg1 & 4) != 0;
			EXROM = (GAME!=0) && (reg1 & 8) != 0;
			BankRom();
			break;
		case CartType::Simons_Basic:
			m_iSelectedBank = 0;
			if (m_bSimonsBasic16K)
			{
				GAME = 0;
				EXROM = 0;
			}
			else
			{
				GAME = m_crtHeader.GAME;
				EXROM = m_crtHeader.EXROM;
			}
			m_bIsCartIOActive = true;
			BankRom();
			break;
		case CartType::Zaxxon:
			m_bIsCartIOActive = true;
			GAME = m_crtHeader.GAME;
			EXROM = m_crtHeader.EXROM;
			BankRom();
			break;
		default: 
			m_bREUcompatible = false;
			m_bAllowBank = false;
			m_iSelectedBank = 0;
			m_iRamBankOffset = 0;
			m_bEnableRAM = false;
			m_bIsCartIOActive = false;
			GAME = m_crtHeader.GAME;
			EXROM = m_crtHeader.EXROM;
			BankRom();
			break;
		}

	}
	else
	{
		m_bEnableRAM = false;
		m_iRamBankOffset = 0;
		GAME = 1;
		EXROM = 1;
		m_bIsCartIOActive = false;
	}
}

void Cart::BankRom()
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
		switch(m_crtHeader.HardwareType)
		{
		case CartType::Action_Replay:
		case CartType::Retro_Replay:
			m_bFreezePending = false;
			m_bFreezeDone = true;
			reg1 &= 0x20;
			reg2 &= 0x43;
			m_pCpu->Clear_CRT_IRQ();
			m_pCpu->Clear_CRT_NMI();
			ConfigureMemoryMap();
			break;
		case CartType::Action_Replay_4:
		case CartType::Action_Replay_3:
			m_bFreezePending = false;
			m_bFreezeDone = false;
			reg1 = 0x00;
			m_pCpu->Clear_CRT_IRQ();
			m_pCpu->Clear_CRT_NMI();
			ConfigureMemoryMap();
			break;
		case CartType::Action_Replay_2:
			m_iSelectedBank = 0;
			m_bFreezePending = false;
			m_bFreezeDone = false;
			m_pCpu->Clear_CRT_IRQ();
			m_pCpu->Clear_CRT_NMI();			
			ConfigureMemoryMap();
			break;
		case CartType::Final_Cartridge_III:
			m_bFreezePending = false;
			m_bFreezeDone = false;
			reg1 = (reg1 & 0x1C) | 0x10;
			//Clear cart IRQ but leave cart NMI active.
			m_pCpu->Clear_CRT_IRQ();
			ConfigureMemoryMap();
			break;
		}
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
			if ((reg2 & 0x04) == 0)
			{
				m_pCpu->Set_CRT_IRQ(m_pCpu->GetCurrentClock());
				m_pCpu->Set_CRT_NMI(m_pCpu->GetCurrentClock());
				m_bFreezePending = true;
				m_bFreezeDone = false;
			}
			break;
		case CartType::Action_Replay://AR5 + AR6 + AR4.x
		case CartType::Action_Replay_4:
		case CartType::Action_Replay_2:
		case CartType::Action_Replay_3:
		case CartType::Final_Cartridge_III:
			m_pCpu->Set_CRT_IRQ(m_pCpu->GetCurrentClock());
			m_pCpu->Set_CRT_NMI(m_pCpu->GetCurrentClock());
			m_bFreezePending = true;
			m_bFreezeDone = false;
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
bit16 addr;

	if (!m_bIsCartAttached)
		return 0;
	switch (m_crtHeader.HardwareType)
	{
	case CartType::Retro_Replay:
		if ((address == 0xDE00 || address == 0xDE01))
		{
			return (reg1 & 0xB8) | (reg2 & 0x42);
		}
		else if (address >= 0xDE00 && address < 0xDF00 && m_bREUcompatible)
		{
			if (m_bEnableRAM)
			{
				addr = address - 0xDE00 + 0x1E00;
				return m_pCartData[addr + m_iRamBankOffset];
			}
			else
			{
				addr = address - 0xDE00 + 0x9E00;
				return this->m_ipROML_8000[addr];
			}
		}
		else if (address >= 0xDF00 && address < 0xE000 && !m_bREUcompatible)
		{
			if (m_bEnableRAM)
			{
				addr = address - 0xDF00 + 0x1F00;
				return m_pCartData[addr + m_iRamBankOffset];
			}
			else
			{
				addr = address - 0xDF00 + 0x9F00;
				return this->m_ipROML_8000[addr];
			}
		}
		break;
	case CartType::Action_Replay:
		if (address >= 0xDE00 && address < 0xDF00)
		{
			return 0;
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			if (m_bEnableRAM)
			{
				addr = address - 0xDF00 + 0x1F00;
				return m_pCartData[addr + m_iRamBankOffset];
			}
			else
			{
				addr = address - 0xDF00 + 0x9F00;
				return this->m_ipROML_8000[addr];
			}
		}
		break;
	case CartType::Action_Replay_4:
		if (address >= 0xDE00 && address < 0xDF00)
		{
			return 0;
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			addr = address - 0xDF00 + 0x9F00;
			return this->m_ipROML_8000[addr];
		}
		break;
	case CartType::Action_Replay_3:
		if (address >= 0xDE00 && address < 0xDF00)
		{
			return 0;
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			addr = address - 0xDF00 + 0x9F00;
			return this->m_ipROML_8000[addr];
		}
		break;
	case CartType::Action_Replay_2:
		if (m_bEffects)
		{
			if (((ICLKS)(sysclock - m_clockLastDF40Read)) > ACTIONREPLAYMK2DISABLEROMCOUNT)
				m_iActionReplayMk2DisableRomCounter = 0;
		}
		if (address >= 0xDE00 && address < 0xDF00)
		{
			return 0;
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			if (m_bEffects)
			{
				if (address == 0xDF40)
				{
					if (((ICLKS)(sysclock - m_clockLastDF40Read)) <= ACTIONREPLAYMK2DISABLEROMCOUNT)
					{
						m_iActionReplayMk2DisableRomCounter++;
						if (m_iActionReplayMk2DisableRomCounter > ACTIONREPLAYMK2DISABLEROMTRIGGER)
						{
							m_iActionReplayMk2DisableRomCounter = 0;
							m_bActionReplayMk2Rom = false;
							ConfigureMemoryMap();
						}
					}
					m_clockLastDF40Read = sysclock;
				}
			}
			addr = address - 0xDF00 + 0x9F00;
			return this->m_ipROML_8000[addr];
		}
		break;
	case CartType::Final_Cartridge_III:
		if (address >= 0xDE00 && address < 0xE000)
		{
			addr = address - 0xDE00 + 0x9E00;
			return this->m_ipROML_8000[addr];
		}
		break;
	case CartType::EasyFlash:
		if (address >= 0xDF00 && address < 0xE000)
		{
			addr = address - 0xDF00;
			return m_pCartData[addr];
		}
		else if (address == 0xDE00 || address == 0xDE02)
		{
			return 0;
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
	case CartType::System_3:
		break;
	case CartType::Dinamic:
		if (address >= 0xDE00 || address < 0xDF00)
		{
			m_iSelectedBank = (bit8)(address & 0x3F);
			ConfigureMemoryMap();
		}
		break;
	case CartType::Fun_Play:
		if (address == 0xDE00)
		{
			return reg1;
		}
		break;
	case CartType::Super_Games:
		if (address == 0xDF00)
		{
			return reg1;
		}
		break;
	case CartType::Simons_Basic:
		if (address == 0xDE00)
		{
			if (m_bEffects)
			{
				m_bSimonsBasic16K = false;
				ConfigureMemoryMap();
			}
			return 0;
		}
		break;
	}
	return 0;
}

bit8 Cart::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	m_bEffects = false;
	bit8 r = ReadRegister(address, sysclock);
	m_bEffects = true;
	return r;
}

void Cart::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
//TODO CartType
bit8 t;
	if (!m_bIsCartAttached)
		return;
	switch (m_crtHeader.HardwareType)
	{
	case CartType::Retro_Replay:
		if (address == 0xDE00)
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
		else if (address == 0xDE01)
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
		if (address >= 0xDE00 && address < 0xDF00)
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
	case CartType::Action_Replay_4:
	case CartType::Action_Replay_3:
		if (address >= 0xDE00 && address < 0xDF00)
		{
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
	case CartType::Action_Replay_2:
		if (address >= 0xDE00 || address < 0xE000)
		{
			m_iActionReplayMk2DisableRomCounter = 0;
			if (((ICLKS)(sysclock - m_clockLastDE00Write)) <= ACTIONREPLAYMK2ENABLEROMCOUNT)
			{
				m_iActionReplayMk2EnableRomCounter++;
				if (m_iActionReplayMk2EnableRomCounter > ACTIONREPLAYMK2ENABLEROMTRIGGER)
				{
					m_iActionReplayMk2EnableRomCounter = 0;
					m_bActionReplayMk2Rom = true;
					t = (address >> 8) & 0xff;
					//CHECKME always select bank1?
					if (t == 0xDE)
						m_iSelectedBank = 1;
					else if (t == 0xDF)
						m_iSelectedBank = 1;
					ConfigureMemoryMap();
				}
			}
			else
			{
				m_iActionReplayMk2EnableRomCounter = 0;
			}
			m_clockLastDE00Write = sysclock;
		}
		break;
	case CartType::Final_Cartridge_III:
		if (address == 0xDFFF && (reg1 & 0x80)==0)
		{
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
	case CartType::EasyFlash:
		if (address == 0xDE00)
		{
			reg1 = data;
			ConfigureMemoryMap();
		}
		else if (address == 0xDE02)
		{
			reg2 = data;
			ConfigureMemoryMap();
		}
		else if (address >= 0xDF00 && address < 0xE000)
		{
			m_pCartData[address - 0xDF00] = data;
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
	case CartType::System_3:
		if (address >= 0xDE00 || address < 0xDF00)
		{
			m_iSelectedBank = (bit8)(address & 0x3F);
			ConfigureMemoryMap();
		}
		break;
	case CartType::Dinamic:
		break;
	case CartType::Fun_Play:
		if (address == 0xDE00)
		{			
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
	case CartType::Super_Games:
		if (address == 0xDF00)
		{			
			reg1 = data;
			ConfigureMemoryMap();
		}
		break;
	case CartType::Simons_Basic:
		if (address == 0xDE00)
		{
			m_bSimonsBasic16K = true;
			ConfigureMemoryMap();
		}
		break;
	}
}

bit8 Cart::ReadROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		bit16 addr = address - 0x8000;
		return m_pCartData[addr + m_iRamBankOffset];
	}
	else
	{
		return m_ipROML_8000[address];
	}
}


bit8 Cart::ReadROML_Zaxxon(bit16 address)
{
	if (m_bEffects)
	{
		if (address < 0x9000 && m_iSelectedBank != 0)
		{
			m_iSelectedBank = 0;
			ConfigureMemoryMap();
		}
		else if (address >= 0x9000 && m_iSelectedBank != 1)
		{
			m_iSelectedBank = 1;
			ConfigureMemoryMap();
		}
	}
	return ReadROML(address);
}

bit8 Cart::ReadUltimaxROML_Zaxxon(bit16 address)
{
	if (m_bEffects)
	{
		if (address < 0x9000 && m_iSelectedBank != 0)
		{
			m_iSelectedBank = 0;
			ConfigureMemoryMap();
		}
		else if (address >= 0x9000 && m_iSelectedBank != 1)
		{
			m_iSelectedBank = 1;
			ConfigureMemoryMap();
		}
	}
	return ReadUltimaxROML(address);
}


bit8 Cart::ReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	if (m_bEnableRAM)
	{
		bit16 addr = address - 0x8000;
		return m_pCartData[addr + m_iRamBankOffset];
	}
	else
	{
		return m_ipROML_8000[address];
	}
}

bit8 Cart::ReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xE000);
	return m_ipROMH_A000[address];
}

bit8 Cart::ReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	return m_ipROMH_E000[address];
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

//bool LessChipAndDataBank::operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const
//{
//	return x->chip.BankLocation < y->chip.BankLocation;
//}

CrtChipAndData::~CrtChipAndData()
{	
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
