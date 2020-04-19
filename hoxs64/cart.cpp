#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <algorithm>
#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "Huff.h"
#include "cart.h"

CartCommon::CartCommon(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
{
	m_crtHeader = crtHeader;
	m_plstBank = NULL;
	m_pCartData = NULL;
	m_pZeroBankData = NULL;
	this->m_pCpu = pCpu;
	this->m_pC64RamMemory = pC64RamMemory;
	m_bIsCartAttached = false;	
	m_state0 = 0;
	m_state1 = 0;
	m_state2 = 0;
	m_state3 = 0;
}

CartCommon::~CartCommon()
{
	CleanUp();
}

void CartCommon::CleanUp()
{
	m_bIsCartAttached = false;
	if (m_plstBank)
	{
		m_plstBank->clear();
		delete m_plstBank;
		m_plstBank = NULL;
	}
	if (m_pCartData)
	{
		GlobalFree(m_pCartData);
		m_pCartData = 0;
		m_pZeroBankData = 0;
	}
}

HRESULT CartCommon::InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData)
{
	CleanUp();
	m_plstBank = plstBank;
	m_pCartData = pCartData;
	m_pZeroBankData = pZeroBankData;

	this->m_ipROML = pZeroBankData;
	this->m_ipROMH = pZeroBankData;

	InitReset(m_pCpu->Get6510CurrentClock(), true);
	return S_OK;
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
	return this->m_ipROMH;
}

void CartCommon::PreventClockOverflow()
{
}

unsigned int CartCommon::GetStateBytes(void *pstate)
{
	SsCartCommon *p = (SsCartCommon *)pstate;
	if (pstate)
	{	
		p->size = sizeof(SsCartCommon);
		p->reg1 = reg1;
		p->reg2 = reg2;

		p->GAME = GAME;
		p->EXROM = EXROM;
		p->m_bIsCartAttached = m_bIsCartAttached;
		p->m_bIsCartIOActive = m_bIsCartIOActive;
		p->m_bIsCartRegActive = m_bIsCartRegActive;
		p->m_iSelectedBank = m_iSelectedBank;

		p->m_bEnableRAM = m_bEnableRAM;
		p->m_bAllowBank = m_bAllowBank;
		p->m_bREUcompatible = m_bREUcompatible;
		p->m_bFreezePending = m_bFreezePending;
		p->m_bFreezeMode = m_bFreezeMode;
		p->m_state0 = m_state0;
		p->m_state1 = m_state1;
		p->m_state2 = m_state2;
		p->m_state3 = m_state3;
	}
	return sizeof(SsCartCommon);
}

HRESULT CartCommon::SetStateBytes(void *pstate, unsigned int size)
{
SsCartCommon *p;
	if (!pstate || size != sizeof(SsCartCommon))
		return E_FAIL;

	p = (SsCartCommon *)pstate;
	if (p->size != sizeof(SsCartCommon))
		return E_FAIL;

	reg1 = p->reg1;
	reg2 = p->reg2;
	GAME = p->GAME;
	EXROM = p->EXROM;
	m_bIsCartIOActive = p->m_bIsCartIOActive != 0;
	m_bIsCartRegActive = p->m_bIsCartRegActive != 0;
	m_iSelectedBank = p->m_iSelectedBank;
	m_bEnableRAM = p->m_bEnableRAM != 0;
	m_bAllowBank = p->m_bAllowBank != 0;
	m_bREUcompatible = p->m_bREUcompatible != 0;
	m_bFreezePending = p->m_bFreezePending;
	m_bFreezeMode = p->m_bFreezeMode;
	m_state0 = p->m_state0;
	m_state1 = p->m_state1;
	m_state2 = p->m_state2;
	m_state3 = p->m_state3;
	return S_OK;
}

HRESULT CartCommon::LoadState(IStream *pfs)
{
ULONG bytesRead;
ULONG bytesToRead;
bool eof = false;
void *pstate = NULL;
CrtBankList *plstBank = 0;
bit8 *pCartData = NULL;
bit8 *pZeroBankData = NULL;
bit32 dwordCount;
unsigned int cartstatesize, cartfullstatesize;

	HRESULT hr;
	hr = S_OK;
	try
	{
		do
		{
			pCartData = (bit8 *)GlobalAlloc(GPTR, Cart::RAMRESERVEDSIZE);
			if (!pCartData)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			pZeroBankData = &pCartData[Cart::ZEROBANKOFFSET];
			plstBank = new CrtBankList();
			plstBank->resize(Cart::MAXBANKS);
			cartstatesize = this->GetStateBytes(NULL);
			bytesToRead = sizeof(bit32);
			hr = pfs->Read(&cartfullstatesize, bytesToRead, &bytesRead);
			if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
			{
				break;
			}
			else if (bytesRead < bytesToRead)
			{
				eof = true;
				hr = E_FAIL;
			}

			if (FAILED(hr))
			{
				break;
			}

			LARGE_INTEGER pos_in;
			ULARGE_INTEGER pos_out;
			pos_in.QuadPart = 0;
			pos_out.QuadPart = 0;
			hr = pfs->Seek(pos_in, STREAM_SEEK_CUR, &pos_out);
			if (FAILED(hr))
			{
				break;
			}
			
			pstate = malloc(cartstatesize);
			if (!pstate)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			memset(pstate, 0, cartstatesize);
			bytesToRead = cartstatesize;
			hr = pfs->Read(pstate, bytesToRead, &bytesRead);
			if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
			{
				break;
			}
			else if (bytesRead < bytesToRead)
			{
				eof = true;			
				hr = E_FAIL;
			}

			if (FAILED(hr))
			{
				break;
			}

			if (cartfullstatesize > cartstatesize + sizeof(bit32))
			{
				pos_in.QuadPart = cartfullstatesize - cartstatesize - sizeof(bit32);
				hr = pfs->Seek(pos_in, STREAM_SEEK_CUR, &pos_out);
				if (FAILED(hr))
				{
					break;
				}
			}
		
			SsCartMemoryHeader memoryheader;
			bytesToRead = sizeof(memoryheader);
			hr = pfs->Read(&memoryheader, bytesToRead, &bytesRead);
			if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
			{
				break;
			}
			else if (bytesRead < bytesToRead)
			{
				eof = true;
				hr = E_FAIL;
			}

			if (FAILED(hr))
			{
				break;
			}

			if (memoryheader.banks > Cart::MAXBANKS)
			{
				hr = E_FAIL;
				break;
			}

			if (memoryheader.ramsize > Cart::CARTRAMSIZE)
			{
				hr = E_FAIL;
				break;
			}

			if (memoryheader.ramsize > 0)
			{
				bytesToRead = memoryheader.ramsize;
				hr = pfs->Read(pCartData, bytesToRead, &bytesRead);
				if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
				{
					break;
				}
				else if (bytesRead < bytesToRead)
				{
					eof = true;
					hr = E_FAIL;
				}
				if (FAILED(hr))
					break;
			}

			HuffDecompression hw;

			for (int i = 0; i < (int)memoryheader.banks; i++)
			{
				SsCartBank bank;

				bytesToRead = sizeof(bank);
				hr = pfs->Read(&bank, bytesToRead, &bytesRead);
				if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
				{
					break;
				}
				else if (bytesRead < bytesToRead)
				{
					eof = true;
					hr = E_FAIL;
				}
				if (FAILED(hr))
					break;

				if (bank.bank > Cart::MAXBANKS)
				{
					hr = E_FAIL;
					break;
				}

				if (bank.roml.allocatedSize != 0)
				{
					if (bank.bank != bank.roml.chip.BankLocation)
					{
						hr = E_FAIL;
						break;
					}
					if (bank.roml.allocatedSize != 0x2000 && bank.roml.allocatedSize != 0x4000)
					{
						hr = E_FAIL;
						break;
					}
				}
				if (bank.romh.allocatedSize != 0)
				{
					if (bank.bank != bank.romh.chip.BankLocation)
					{
						hr = E_FAIL;
						break;
					}
					if (bank.romh.allocatedSize != 0x2000 && bank.romh.allocatedSize != 0x4000)
					{
						hr = E_FAIL;
						break;
					}
				}

				if (bank.roml.chip.ROMImageSize > bank.roml.allocatedSize || bank.romh.chip.ROMImageSize > bank.romh.allocatedSize)
				{
					hr = E_FAIL;
					break;
				}

				Sp_CrtBank sp = plstBank->at(bank.bank);
				if (!sp)
				{
					sp = Sp_CrtBank(new CrtBank());
					if (sp == 0)
						throw std::bad_alloc();
					plstBank->at(bank.bank) = sp;
				}

				sp->bank = (bit16)bank.bank;
				sp->chipAndDataLow.allocatedSize = bank.roml.allocatedSize;
				sp->chipAndDataLow.chip = bank.roml.chip;
				sp->chipAndDataHigh.allocatedSize = bank.romh.allocatedSize;
				sp->chipAndDataHigh.chip = bank.romh.chip;
			
				SsDataChunkHeader dataheader;

				CrtChipAndData *pcd;

				pcd = &sp->chipAndDataLow;

				CrtChipAndData *cds[2] = {&sp->chipAndDataLow, &sp->chipAndDataHigh};
				for (int n=0; n < _countof(cds); n++)
				{
					pcd = cds[n];
					bytesToRead = sizeof(SsDataChunkHeader);
					hr = pfs->Read(&dataheader, bytesToRead, &bytesRead);
					if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
					{
						break;
					}
					else if (bytesRead < bytesToRead)
					{
						eof = true;
						hr = E_FAIL;
					}
					if (FAILED(hr))
						break;

					if (dataheader.byteCount != pcd->allocatedSize)
					{
						hr = E_FAIL;
						break;
					}

					if (dataheader.byteCount > 0)
					{
						if (dataheader.compressionType == Cart::NOCOMPRESSION)
						{
							pcd->pData = (bit8 *)GlobalAlloc(GPTR, pcd->allocatedSize);
							if (!pcd->pData)
							{
								hr = E_FAIL;
								break;
							}
							pcd->ownData = true;
							bytesToRead = dataheader.byteCount;
							hr = pfs->Read(pcd->pData, bytesToRead, &bytesRead);
							if (FAILED(hr))
								break;
						}
						else
						{
							dwordCount = dataheader.byteCount / sizeof(bit32);

							bytesToRead = sizeof(dwordCount);
							hr = pfs->Read(&dwordCount, bytesToRead, &bytesRead);
							if (FAILED(hr))
								break;

							if (dwordCount * sizeof(bit32) != dataheader.byteCount)
							{
								hr = E_FAIL;
								break;
							}

							if (dwordCount > 0)
							{
								hr = hw.Decompress(dwordCount, (bit32 **)&pcd->pData);
								if (FAILED(hr))
									break;
								pcd->ownData = true;
							}
						}
					}
				}
				if (FAILED(hr))
					break;

			}
		} while (false);
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		hr = this->InitCart(plstBank, pCartData, pZeroBankData);
		if (SUCCEEDED(hr))
		{
			plstBank = NULL;
			pCartData = NULL;
			pZeroBankData = NULL;
			this->SetStateBytes(pstate, cartstatesize);
		}
	}

	if (pCartData)
	{
		GlobalFree(pCartData);
		pCartData = 0;
		pZeroBankData = 0;
	}
	if (plstBank)
	{
		plstBank->clear();
		delete plstBank;
		plstBank = NULL;
	}
	if (pstate)
	{
		free(pstate);
		pstate = NULL;
	}
	return hr;
}

HRESULT CartCommon::SaveState(IStream *pfs)
{
HRESULT hr;
ULONG bytesToWrite;
ULONG bytesWritten;
SsCartStateHeader hdr;
bit8* pstate;
unsigned int cartstatesize;
unsigned int cartfullstatesize;

	cartstatesize = GetStateBytes(NULL);
	pstate = (bit8*)malloc(cartstatesize);
	cartstatesize = GetStateBytes(pstate);
	hr = S_OK;
	do
	{
		cartfullstatesize = sizeof(bit32) + cartstatesize;

		ZeroMemory(&hdr, sizeof(hdr));
		hdr.size = sizeof(hdr) + sizeof(this->m_crtHeader) + cartfullstatesize;
		bytesToWrite = sizeof(hdr);
		hr = pfs->Write(&hdr, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;
		bytesToWrite = sizeof(this->m_crtHeader);
		hr = pfs->Write(&this->m_crtHeader, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;

		bytesToWrite = sizeof(bit32);
		hr = pfs->Write(&cartfullstatesize, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;

		bytesToWrite = cartstatesize;
		hr = pfs->Write(pstate, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;
		hr = this->SaveMemoryState(pfs);
		if (FAILED(hr))
			return hr;
	}
	while (false);
	if (pstate)
	{
		free(pstate);
		pstate = NULL;
	}
	return hr;
}

HRESULT CartCommon::SaveMemoryState(IStream *pfs)
{
HRESULT hr;
ULONG bytesToWrite;
ULONG bytesWritten;
SsCartMemoryHeader memoryheader;
SsDataChunkHeader dataheader;
bit32 compressedSize;
int dwordCount = 0;

	hr = S_OK;
	do
	{

		LARGE_INTEGER spos_zero;
		LARGE_INTEGER spos_next;
		ULARGE_INTEGER pos_current_section_header;
		ULARGE_INTEGER pos_next_section_header;
		ULARGE_INTEGER pos_dummy;
		spos_zero.QuadPart = 0;
		hr = pfs->Seek(spos_zero, STREAM_SEEK_CUR, &pos_current_section_header);
		if (FAILED(hr))
			break;

		ZeroMemory(&memoryheader, sizeof(memoryheader));
		memoryheader.size = sizeof(memoryheader) + 0;
		memoryheader.ramsize = Cart::CARTRAMSIZE; 
		bytesToWrite = sizeof(memoryheader);
		hr = pfs->Write(&memoryheader, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;

		bytesToWrite = memoryheader.ramsize;
		hr = pfs->Write(m_pCartData, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
			break;

		HuffCompression hw;
		hr = hw.Init();
		if (FAILED(hr))
			break;

		hr = hw.SetFile(pfs);
		if (FAILED(hr))
			break;
		int banks = 0;
		for (CrtBankListIter it = m_plstBank->begin(); it != m_plstBank->end(); it++)
		{
			Sp_CrtBank sp = *it;
			if (!sp)
				continue;
			SsCartBank bank;
			bank.bank = sp->bank;
			bank.roml.allocatedSize = sp->chipAndDataLow.allocatedSize;
			bank.roml.chip =  sp->chipAndDataLow.chip;
			bank.romh.allocatedSize = sp->chipAndDataHigh.allocatedSize;
			bank.romh.chip =  sp->chipAndDataHigh.chip;

			bytesToWrite = sizeof(bank);
			hr = pfs->Write(&bank, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
				break;
			banks++;

			dataheader.byteCount = sp->chipAndDataLow.allocatedSize;
			dataheader.compressionType = Cart::NOCOMPRESSION;

			bytesToWrite = sizeof(dataheader);
			hr = pfs->Write(&dataheader, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
				break;

			if (dataheader.byteCount > 0)
			{
				if (dataheader.compressionType == Cart::NOCOMPRESSION)
				{
					bytesToWrite = dataheader.byteCount;
					hr = pfs->Write(sp->chipAndDataLow.pData, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
						break;
				}
				else
				{
					dwordCount = dataheader.byteCount / sizeof(bit32);

					bytesToWrite = sizeof(dwordCount);
					hr = pfs->Write(&dwordCount, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
						break;
					if (dwordCount > 0)
					{
						hr = hw.Compress((bit32 *)sp->chipAndDataLow.pData, dwordCount, &compressedSize);
						if (FAILED(hr))
							break;
					}
				}
			}

			dataheader.byteCount = sp->chipAndDataHigh.allocatedSize;
			dataheader.compressionType = Cart::NOCOMPRESSION;

			bytesToWrite = sizeof(dataheader);
			hr = pfs->Write(&dataheader, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
				break;

			if (dataheader.byteCount > 0)
			{
				if (dataheader.compressionType == Cart::NOCOMPRESSION)
				{
					bytesToWrite = dataheader.byteCount;
					hr = pfs->Write(sp->chipAndDataHigh.pData, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
						break;
				}
				else
				{
					dwordCount = dataheader.byteCount / sizeof(bit32);

					bytesToWrite = sizeof(dwordCount);
					hr = pfs->Write(&dwordCount, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
						break;

					if (dwordCount > 0)
					{
						hr = hw.Compress((bit32 *)sp->chipAndDataHigh.pData, dwordCount, &compressedSize);
						if (FAILED(hr))
							break;
					}
				}
			}
		}

		hr = pfs->Seek(spos_zero, STREAM_SEEK_CUR, &pos_next_section_header);
		if (FAILED(hr))
			break;

		spos_next.QuadPart = pos_current_section_header.QuadPart;
		hr = pfs->Seek(spos_next, STREAM_SEEK_SET, &pos_dummy);
		if (FAILED(hr))
			break;
		memoryheader.size = (bit32)(pos_next_section_header.QuadPart - pos_current_section_header.QuadPart);
		memoryheader.banks = banks;
		hr = pfs->Write(&memoryheader, sizeof(memoryheader), &bytesWritten);
		if (FAILED(hr))
			break;
		spos_next.QuadPart = pos_next_section_header.QuadPart;
		hr = pfs->Seek(spos_next, STREAM_SEEK_SET, &pos_dummy);
		if (FAILED(hr))
			break;
	}
	while (false);
	return hr;
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
	m_ipROML = m_pZeroBankData;
	m_ipROMH = m_pZeroBankData;
	m_ipROMH = m_pZeroBankData;
	SIZE_T i = m_iSelectedBank;
	if (i < m_plstBank->size())
	{
		Sp_CrtBank p = m_plstBank->at(i);
		if (p)
		{
			CrtChipAndData* pL = &p->chipAndDataLow;
			CrtChipAndData* pH = &p->chipAndDataHigh;
			if (pL->chip.ROMImageSize != 0)
			{
				if (pL->chip.ROMImageSize <= 0x2000)
				{
					m_ipROML = pL->pData;
				}
				else if (pL->chip.ROMImageSize <= 0x4000)
				{
					m_ipROML = pL->pData;
					m_ipROMH = &pL->pData[0x2000];
					m_ipROMH = &pL->pData[0x2000];
				}
			}
			if (pH->chip.ROMImageSize != 0)
			{
				m_ipROMH = pH->pData;
				m_ipROMH = pH->pData;
			}
		}
	}
}

void CartCommon::CartFreeze()
{
}

void CartCommon::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock = sysclock;
	m_bEffects = true;
	m_iSelectedBank = 0;
	m_bFreezePending = false;
	m_bFreezeMode = false;
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

void CartCommon::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ConfigureMemoryMap();
}

void CartCommon::CartReset()
{
	if (m_bIsCartAttached)
	{
		this->Reset(m_pCpu->Get6510CurrentClock(), false);
	}
}

void CartCommon::AttachCart()
{
	if (!m_bIsCartAttached)
	{
		m_bIsCartAttached = true;
		this->Reset(m_pCpu->Get6510CurrentClock(), true);
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

void CartCommon::Set_IsCartAttached(bool isAttached)
{
	m_bIsCartAttached = isAttached;
}

bool CartCommon::IsUltimax()
{
	return GAME == 0 && EXROM != 0;
}


bit8 CartCommon::ReadROML(bit16 address)
{
	if (m_bEnableRAM)
	{
		return m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL];
	}
	else
	{
		return m_ipROML[address & 0x1fff];
	}
}

void CartCommon::WriteROML(bit16 address, bit8 data)
{
	if (m_bEnableRAM)
	{
		m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL] = data;
	}
	m_pC64RamMemory[address] = data;
}

bit8 CartCommon::ReadROMH(bit16 address)
{
	return m_ipROMH[address & 0x1fff];
}

void CartCommon::WriteROMH(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
}

bit8 CartCommon::ReadUltimaxROML(bit16 address)
{
	if (m_bEnableRAM)
	{
		return m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL];
	}
	else
	{
		return m_ipROML[(address & 0x1fff)];
	}
}

void CartCommon::WriteUltimaxROML(bit16 address, bit8 data)
{
	if (m_bEnableRAM)
	{
		m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL] = data;
	}
}

bit8 CartCommon::ReadUltimaxROMH(bit16 address)
{
	return m_ipROMH[address & 0x1fff];
}

void CartCommon::WriteUltimaxROMH(bit16 address, bit8 data)
{
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
	if (m_plstBank)
	{
		m_plstBank->clear();
		delete m_plstBank;
		m_plstBank = NULL;
	}
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
	case CartType::KCS_Power:
		return true;
	}
	return false;
}

int Cart::GetTotalCartMemoryRequirement()
{
	return GetTotalCartMemoryRequirement(m_plstBank);
}

int Cart::GetTotalCartMemoryRequirement(CrtBankList *plstBank)
{
	int i = RAMRESERVEDSIZE;
	for (CrtBankListIter it = plstBank->begin(); it!=plstBank->end(); it++)
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
CrtBankList* plstBank = NULL;
bit8 *pCartData = NULL;
__int64 filesize=0;
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

			plstBank = new CrtBankList();
			if (plstBank == NULL)
			{
				ok = false;
				hr = SetError(E_OUTOFMEMORY, S_OUTOFMEMORY);
				break;
			}
			plstBank->resize(MAXBANKS);
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
				if (nChipCount >= 2*MAXBANKS)
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
				if (chip.BankLocation >= plstBank->size() || chip.BankLocation >= MAXBANKS)
				{
					//plstBank->resize(chip.BankLocation + 1);
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}

				Sp_CrtBank spBank = plstBank->at(chip.BankLocation);
				if (!spBank)
				{
					spBank = Sp_CrtBank(new CrtBank());
					if (spBank == 0)
						throw std::bad_alloc();
					spBank->bank = chip.BankLocation;
					plstBank->at(chip.BankLocation) = spBank;
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
			SIZE_T lenAlloc = GetTotalCartMemoryRequirement(plstBank);
			pCartData = (bit8 *)GlobalAlloc(GPTR, lenAlloc);
			if (!pCartData)
			{
				ok = false;
				hr = SetError(E_OUTOFMEMORY, S_OUTOFMEMORY);
				break;
			}
			bit8 *p = pCartData + (INT_PTR)RAMRESERVEDSIZE;
			for (CrtBankListIter it = plstBank->begin(); it!=plstBank->end(); it++)
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
		if (plstBank->size() == 0)
		{
			hr = SetError(E_FAIL, S_READFAILED, filename);
		}
		else
		{
			switch(hdr.HardwareType)
			{
			case CartType::Zaxxon:
				if (plstBank->at(0) && plstBank->at(0)->chipAndDataLow.allocatedSize == 0x2000 && plstBank->at(0)->chipAndDataLow.pData!=0)
				{
					CopyMemory(&plstBank->at(0)->chipAndDataLow.pData[0x1000], &plstBank->at(0)->chipAndDataLow.pData[0], 0x1000);
				}
				
				if (plstBank->at(1))
				{
					plstBank->at(1)->chipAndDataLow = plstBank->at(0)->chipAndDataLow;
					plstBank->at(1)->chipAndDataLow.chip.BankLocation = 1;
				}
				break;
			}
			if (this->m_spCurrentCart)
			{
				m_spCurrentCart->DetachCart();
				m_spCurrentCart.reset();
			}
			if (m_plstBank)
			{
				m_plstBank->clear();
				delete m_plstBank;
				m_plstBank = NULL;
			}
			if (m_pCartData)
			{
				GlobalFree(m_pCartData);
				m_pCartData = NULL;
				m_pZeroBankData = 0;
			}
			m_crtHeader = hdr;
			m_plstBank = plstBank;
			m_pCartData = pCartData;
			m_pZeroBankData = &pCartData[ZEROBANKOFFSET];
			pCartData = NULL;
			plstBank = NULL;
			m_bIsCartDataLoaded = true;

			if (!IsSupported())
				hr = SetError(APPWARN_UNKNOWNCARTTYPE, TEXT("The hardware type for this cartridge is not supported. The emulator will attempt to run the ROM images with generic hardware. The cartridge software may not run correctly."));
		}
	}
	if (plstBank)
	{
		plstBank->clear();
		delete plstBank;
		plstBank = NULL;
	}
	if (pCartData)
	{
		GlobalFree(pCartData);
		pCartData = NULL;		
	}
	
	return hr;
}

shared_ptr<ICartInterface> Cart::CreateCartInterface(const CrtHeader &crtHeader)
{
shared_ptr<ICartInterface> p;
	try
	{
		switch(crtHeader.HardwareType)
		{
		case CartType::Normal_Cartridge:
			p = shared_ptr<ICartInterface>(new CartNormalCartridge(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay:
			p = shared_ptr<ICartInterface>(new CartActionReplay(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Final_Cartridge_III:
			p = shared_ptr<ICartInterface>(new CartFinalCartridgeIII(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Simons_Basic:
			p = shared_ptr<ICartInterface>(new CartSimonsBasic(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Ocean_1:
			p = shared_ptr<ICartInterface>(new CartOcean1(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Fun_Play:
			p = shared_ptr<ICartInterface>(new CartFunPlay(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Super_Games:
			p = shared_ptr<ICartInterface>(new CartSuperGames(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::System_3:
			p = shared_ptr<ICartInterface>(new CartSystem3(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Dinamic:
			p = shared_ptr<ICartInterface>(new CartDinamic(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Zaxxon:
			p = shared_ptr<ICartInterface>(new CartZaxxon(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Magic_Desk:
			p = shared_ptr<ICartInterface>(new CartMagicDesk(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_4:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk4(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::EasyFlash:
			p = shared_ptr<ICartInterface>(new CartEasyFlash(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_3:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk3(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Retro_Replay:
			p = shared_ptr<ICartInterface>(new CartRetroReplay(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_2:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk2(crtHeader, this->m_pCpu, this->m_pC64RamMemory));			
			break;
		case CartType::Epyx_FastLoad:
			p = shared_ptr<ICartInterface>(new CartEpyxFastLoad(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		case CartType::KCS_Power:
			p = shared_ptr<ICartInterface>(new CartKcsPower(crtHeader, this->m_pCpu, this->m_pC64RamMemory));
			break;
		}
	}
	catch(std::bad_alloc&)
	{
		SetError(E_OUTOFMEMORY, ErrorMsg::ERR_OUTOFMEMORY);
		p.reset();
	}
	return p;
}

void Cart::Reset(ICLK sysclock, bool poweronreset)
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->Reset(sysclock, poweronreset);
	}
}

void Cart::ExecuteCycle(ICLK sysclock)
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->ExecuteCycle(sysclock);
	}
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
	return m_spCurrentCart && m_spCurrentCart->IsUltimax();
}

bool Cart::IsCartIOActive()
{
	return m_spCurrentCart && m_spCurrentCart->IsCartIOActive();
}

bool Cart::IsCartAttached()
{
	return m_spCurrentCart && m_spCurrentCart->IsCartAttached();
}

void Cart::Set_IsCartAttached(bool isAttached)
{
	if (m_spCurrentCart)
		m_spCurrentCart->Set_IsCartAttached(isAttached);
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

void Cart::AttachCart(shared_ptr<ICartInterface> spCartInterface)
{
	if (m_spCurrentCart)
		m_spCurrentCart->Set_IsCartAttached(false);
	m_spCurrentCart = spCartInterface;
	if (m_spCurrentCart)
		m_spCurrentCart->Set_IsCartAttached(true);
}

void Cart::AttachCart()
{
	if (this->m_bIsCartDataLoaded)
	{
		shared_ptr<ICartInterface> spCartInterface = this->CreateCartInterface(this->m_crtHeader);
		if (spCartInterface)
		{
			HRESULT hr = spCartInterface->InitCart(this->m_plstBank, this->m_pCartData, this->m_pZeroBankData);
			if (SUCCEEDED(hr))
			{
				this->m_plstBank = NULL;
				this->m_pCartData = NULL;
				this->m_pZeroBankData = NULL;
				m_bIsCartDataLoaded = false;
				m_spCurrentCart = spCartInterface;
				m_spCurrentCart->AttachCart();			
			}
		}
	}
}

void Cart::DetachCart()
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->DetachCart();
		m_spCurrentCart.reset();
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

HRESULT Cart::LoadState(IStream *pfs)
{
	return E_NOTIMPL;
}

HRESULT Cart::LoadCartInterface(IStream *pfs, shared_ptr<ICartInterface> &spCartInterface)
{
CrtHeader crtHeader;
ULONG bytesRead;
ULONG bytesToRead;
bool eof = false;
SsCartStateHeader hdr;

	HRESULT hr;
	hr = S_OK;
	do
	{
		ZeroMemory(&crtHeader, sizeof(crtHeader));

		bytesToRead = sizeof(hdr);
		hr = pfs->Read(&hdr, bytesToRead, &bytesRead);
		if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
		{
			break;
		}
		else if (bytesRead < bytesToRead)
		{
			eof = true;
			hr = E_FAIL;
		}
		if (FAILED(hr))
			break;

		bytesToRead = sizeof(crtHeader);
		hr = pfs->Read(&crtHeader, bytesToRead, &bytesRead);
		if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
		{
			break;
		}
		else if (bytesRead < bytesToRead)
		{
			eof = true;
			hr = E_FAIL;
		}
		if (FAILED(hr))
			break;
		
		shared_ptr<ICartInterface> spLocalCartInterface = this->CreateCartInterface(crtHeader);
		if (!spLocalCartInterface)
		{
			hr = E_FAIL;
			break;
		}

		hr = spLocalCartInterface->LoadState(pfs);
		if (FAILED(hr))
			break;

		spCartInterface = spLocalCartInterface;
	} while (false);
	return hr;
}

HRESULT Cart::InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData)
{
HRESULT hr=E_FAIL;
	if (m_spCurrentCart)
		hr = m_spCurrentCart->InitCart(plstBank, pCartData, pZeroBankData);
	return hr;
}

HRESULT Cart::SaveState(IStream *pfs)
{
	if (m_spCurrentCart)
		return m_spCurrentCart->SaveState(pfs);
	else
		return S_FALSE;
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
	ownData = false;
	iFileIndex = 0;
	allocatedSize = 0;
	romOffset = 0;
}

CrtChipAndData::~CrtChipAndData()
{
	if (ownData)
	{
		if (pData)
		{
			GlobalFree(pData);
			pData = NULL;
		}
		ownData = false;
	}
}

CrtBank::CrtBank()
{
	bank = 0;
}
CrtBank::~CrtBank()
{
	int test = 0;
	test++;
}


