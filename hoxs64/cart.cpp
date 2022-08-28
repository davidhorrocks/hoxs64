#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <algorithm>
#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "wfs.h"
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

CartCommon::CartCommon(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	:reg1(registerArray[0]), reg2(registerArray[1])
{
	m_crtHeader = crtHeader;
	m_pCpu = pCpu;
	m_pVic = pVic;
	m_pC64RamMemory = pC64RamMemory;
	m_plstBank = NULL;
	m_pCartData = NULL;
	m_pZeroBankData = NULL;
	m_amountOfExtraRAM = 0;
	m_bIsCartAttached = false;
	m_bEffects = true;
	unsigned int i;
	for (i = 0; i < _countof(registerArray); i++)
	{
		registerArray[i] = 0;
	}

	for (i = 0; i < _countof(m_statebyte); i++)
	{
		m_statebyte[i] = 0;
	}

	for (i = 0; i < _countof(m_stateushort); i++)
	{
		m_stateushort[i] = 0;
	}

	for (i = 0; i < _countof(m_stateuint); i++)
	{
		m_stateuint[i] = 0;
	}

	for (i = 0; i < _countof(m_statebool); i++)
	{
		m_statebool[i] = false;
	}
}

CartCommon::~CartCommon()
{
	CleanUp();
}

void CartCommon::CleanUp() noexcept
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
		m_amountOfExtraRAM = 0;
	}
}

HRESULT CartCommon::InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData)
{
	CleanUp();
	m_plstBank = plstBank;
	m_pCartData = pCartData;
	m_pZeroBankData = pZeroBankData;
	m_amountOfExtraRAM = amountOfExtraRAM;
	m_ipROML = pZeroBankData;
	m_ipROMH = pZeroBankData;
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
		DMA = 1;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();
		m_pCpu->SetCartridgeRdyHigh(CurrentClock);
	}

	m_pCpu->ConfigureMemoryMap();
}

bool CartCommon::SnoopWrite(bit16 address, bit8 data)
{
	return true;
}

void CartCommon::SnoopRead(bit16 address, bit8 data)
{
}

bit8 *CartCommon::Get_RomH()
{
	return this->m_ipROMH;
}

bit8* CartCommon::Get_RomL()
{
	return this->m_ipROML;
}

void CartCommon::PreventClockOverflow()
{
}

unsigned int CartCommon::GetStateBytes(int version, void *pstate)
{	
	// Save the emulated state to a structure pointed to by pstate
	// Return the size of the required structure.
	if (version == 0)
	{
		// Dont save. We do not need to downgrade back to version 0
		return sizeof(SsCartCommonV0);
	}
	else if (version == 1)
	{		
		int i;
		// Save if the version is the latest version.
		SsCartCommonV1* p = (SsCartCommonV1*)pstate;
		if (pstate)
		{
			p->size = sizeof(SsCartCommonV1);
			memcpy(&p->registerArray[0], &registerArray[0], sizeof(SsCartCommonV1::registerArray));
			p->GAME = GAME;
			p->EXROM = EXROM;
			p->DMA = DMA;
			p->m_bIsCartAttached = m_bIsCartAttached;
			p->m_bIsCartIOActive = m_bIsCartIOActive;
			p->m_bIsCartRegActive = m_bIsCartRegActive;
			p->m_iSelectedBank = m_iSelectedBank;
			p->m_bEnableRAM = m_bEnableRAM;
			p->m_bAllowBank = m_bAllowBank;
			p->m_bREUcompatible = m_bREUcompatible;
			p->m_bFreezePending = m_bFreezePending;
			p->m_bFreezeMode = m_bFreezeMode;

			for (i = 0; i < _countof(m_statebyte); i++)
			{
				p->m_statebyte[i] = m_statebyte[i];
			}

			for (i = 0; i < _countof(m_stateushort); i++)
			{
				p->m_stateushort[i] = m_stateushort[i];
			}

			for (i = 0; i < _countof(m_stateuint); i++)
			{
				p->m_stateuint[i] = m_stateuint[i];
			}

			for (i = 0; i < _countof(m_statebool); i++)
			{
				p->m_statebool[i] = m_statebool[i];
			}
		}

		return sizeof(SsCartCommonV1);
	}
	else
	{ 
		return 0;
	}
}


void CartCommon::UpgradeVersion0ToVersion1(const SsCartCommonV0& in, SsCartCommonV1& out)
{
	ZeroMemory(&out, sizeof(SsCartCommonV1));
	memset(out.registerArray, 0, sizeof(out.registerArray));
	out.registerArray[0] = in.reg1;
	out.registerArray[1] = in.reg2;
	out.GAME = in.GAME;
	out.EXROM = in.EXROM;
	out.DMA = 1;
	out.m_bIsCartIOActive = in.m_bIsCartIOActive != 0;
	out.m_bIsCartRegActive = in.m_bIsCartRegActive != 0;
	out.m_iSelectedBank = in.m_iSelectedBank;
	out.m_bEnableRAM = in.m_bEnableRAM != 0;
	out.m_bAllowBank = in.m_bAllowBank != 0;
	out.m_bREUcompatible = in.m_bREUcompatible != 0;
	out.m_bFreezePending = in.m_bFreezePending;
	out.m_bFreezeMode = in.m_bFreezeMode;

	int i;
	for (i = 0; i < _countof(out.m_statebyte); i++)
	{
		out.m_statebyte[i] = 0;
	}

	for (i = 0; i < _countof(out.m_stateushort); i++)
	{
		out.m_stateushort[i] = 0;
	}

	for (i = 0; i < _countof(out.m_stateuint); i++)
	{
		out.m_stateuint[i] = 0;
	}

	for (i = 0; i < _countof(out.m_statebool); i++)
	{
		out.m_statebool[i] = false;
	}

	out.m_stateuint[0] = in.m_state0;
	out.m_stateuint[1] = in.m_state1;
	out.m_stateuint[2] = in.m_state2;
	out.m_stateuint[3] = in.m_state3;
}

void CartCommon::LoadStateFromStructure(const SsCartCommonV1 state)
{
	size_t test = sizeof(registerArray);
	CurrentClock = this->CurrentClock;
	memset(registerArray, 0, sizeof(registerArray));
	GAME = state.GAME;
	EXROM = state.EXROM;
	DMA = state.DMA;
	m_bIsCartIOActive = state.m_bIsCartIOActive != 0;
	m_bIsCartRegActive = state.m_bIsCartRegActive != 0;
	m_iSelectedBank = state.m_iSelectedBank;
	m_bEnableRAM = state.m_bEnableRAM != 0;
	m_bAllowBank = state.m_bAllowBank != 0;
	m_bREUcompatible = state.m_bREUcompatible != 0;
	m_bFreezePending = state.m_bFreezePending;
	m_bFreezeMode = state.m_bFreezeMode;
	int i;

	for (i = 0; i < _countof(registerArray); i++)
	{
		this->registerArray[i] = state.registerArray[i];
	}

	for (i = 0; i < _countof(m_statebyte); i++)
	{
		m_statebyte[i] = state.m_statebyte[i];
	}

	for (i = 0; i < _countof(m_stateushort); i++)
	{
		m_stateushort[i] = state.m_stateushort[i];
	}

	for (i = 0; i < _countof(m_stateuint); i++)
	{
		m_stateuint[i] = state.m_stateuint[i];
	}

	for (i = 0; i < _countof(m_statebool); i++)
	{
		m_statebool[i] = state.m_statebool[i];
	}
}

void CartCommon::SaveStateToStructure(SsCartCommonV1 state)
{
	state.size = sizeof(SsCartCommonV1);
	state.CurrentClock = this->CurrentClock;
	memcpy(&state.registerArray[0], &registerArray[0], sizeof(SsCartCommonV1::registerArray));
	state.GAME = GAME;
	state.EXROM = EXROM;
	state.DMA = DMA;
	state.m_bIsCartAttached = m_bIsCartAttached;
	state.m_bIsCartIOActive = m_bIsCartIOActive;
	state.m_bIsCartRegActive = m_bIsCartRegActive;
	state.m_iSelectedBank = m_iSelectedBank;
	state.m_bEnableRAM = m_bEnableRAM;
	state.m_bAllowBank = m_bAllowBank;
	state.m_bREUcompatible = m_bREUcompatible;
	state.m_bFreezePending = m_bFreezePending;
	state.m_bFreezeMode = m_bFreezeMode;

	int i;
	for (i = 0; i < _countof(registerArray); i++)
	{
		state.registerArray[i] = this->registerArray[i];
	}

	for (i = 0; i < _countof(m_statebyte); i++)
	{
		state.m_statebyte[i] = m_statebyte[i];
	}

	for (i = 0; i < _countof(m_stateushort); i++)
	{
		state.m_stateushort[i] = m_stateushort[i];
	}

	for (i = 0; i < _countof(m_stateuint); i++)
	{
		state.m_stateuint[i] = m_stateuint[i];
	}

	for (i = 0; i < _countof(m_statebool); i++)
	{
		state.m_statebool[i] = m_statebool[i];
	}
}

HRESULT CartCommon::SetStateBytes(int version, void *pstate, unsigned int size)
{	
	if (!pstate)
	{
		return E_FAIL;
	}

	SsCartCommonV1 cartstate = {};
	if (version == 0)
	{
		if (size < sizeof(SsCartCommonV0))
		{
			return E_FAIL;
		}

		SsCartCommonV0* p = (SsCartCommonV0*)pstate;
		UpgradeVersion0ToVersion1(*p, cartstate);
	}
	else if (version == 1)
	{ 
		if (size < sizeof(SsCartCommonV1))
		{
			return E_FAIL;
		}

		SsCartCommonV1* p = (SsCartCommonV1*)pstate;
		cartstate = *p;
	}
	else
	{
		return E_FAIL;
	}

	LoadStateFromStructure(cartstate);
	return S_OK;
}

HRESULT CartCommon::LoadState(IStream *pfs, int version)
{
	ULONG bytesRead;
	ULONG bytesToRead;
	bool eof = false;
	void *pstate = nullptr;
	CrtBankList *plstBank = nullptr;
	bit8 *pCartData = nullptr;
	bit8 *pZeroBankData = nullptr;
	bit32 dwordCount;
	unsigned int cartstatesize, cartfullstatesize;
	unsigned int amountOfExtraRAM = 0;
	HRESULT hr;
	hr = S_OK;
	try
	{
		do
		{
			LARGE_INTEGER pos_in;
			ULARGE_INTEGER pos_out;
			pos_in.QuadPart = 0;
			pos_out.QuadPart = 0;
			hr = pfs->Seek(pos_in, STREAM_SEEK_CUR, &pos_out);
			if (FAILED(hr))
			{
				break;
			}

			ULARGE_INTEGER position_start_of_state = pos_out;
			plstBank = new CrtBankList();
			plstBank->resize(Cart::MAXBANKS);
			cartstatesize = this->GetStateBytes(version, NULL);
			if (cartstatesize == 0)
			{
				hr = E_FAIL;
				break;
			}

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

			if (cartfullstatesize < sizeof(bit32))
			{
				hr = E_FAIL;
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
			if (cartfullstatesize - sizeof(bit32) < cartstatesize )
			{
				bytesToRead = cartfullstatesize - sizeof(bit32);
			}

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

			pos_in.QuadPart = position_start_of_state.QuadPart;
			pos_in.QuadPart += cartfullstatesize;
			hr = pfs->Seek(pos_in, STREAM_SEEK_SET, &pos_out);
			if (FAILED(hr))
			{
				break;
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

			if (memoryheader.ramsize > Cart::MAX_CART_EXTRA_RAM)
			{
				hr = E_FAIL;
				break;
			}

			if (memoryheader.ramsize < Cart::MIN_CART_EXTRA_RAM)
			{
				amountOfExtraRAM = Cart::MIN_CART_EXTRA_RAM;
			}
			else
			{
				amountOfExtraRAM = memoryheader.ramsize;
			}

			pCartData = (bit8*)GlobalAlloc(GPTR, amountOfExtraRAM + Cart::DEFAULT_CHIP_EXTRA_MEMORY);
			if (!pCartData)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			pZeroBankData = &pCartData[amountOfExtraRAM];
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
							{
								break;
							}
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
								hr = hw.DecompressGlobalAlloc(dwordCount, (bit32 **)&pcd->pData);
								if (FAILED(hr))
								{
									break;
								}

								pcd->ownData = true;
							}
						}
					}
				}

				if (FAILED(hr))
				{
					break;
				}
			}
		} while (false);
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		hr = this->InitCart(amountOfExtraRAM, pCartData, plstBank, pZeroBankData);
		if (SUCCEEDED(hr))
		{
			plstBank = NULL;
			pCartData = NULL;
			pZeroBankData = NULL;
			amountOfExtraRAM = 0;
			this->SetStateBytes(version, pstate, cartstatesize);
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

	cartstatesize = GetStateBytes(LatestVersion, NULL);
	if (cartstatesize == 0)
	{
		return E_FAIL;
	}

	pstate = (bit8*)malloc(cartstatesize);
	if (pstate == nullptr)
	{
		return E_OUTOFMEMORY;
	}

	cartstatesize = GetStateBytes(LatestVersion, pstate);
	if (cartstatesize == 0)
	{
		return E_FAIL;
	}

	hr = S_OK;
	do
	{
		cartfullstatesize = sizeof(bit32) + cartstatesize;
		ZeroMemory(&hdr, sizeof(hdr));
		hdr.size = sizeof(hdr) + sizeof(this->m_crtHeader) + cartfullstatesize;
		bytesToWrite = sizeof(hdr);
		hr = pfs->Write(&hdr, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		bytesToWrite = sizeof(this->m_crtHeader);
		hr = pfs->Write(&this->m_crtHeader, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		bytesToWrite = sizeof(bit32);
		hr = pfs->Write(&cartfullstatesize, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		bytesToWrite = cartstatesize;
		hr = pfs->Write(pstate, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		hr = this->SaveMemoryState(pfs);
		if (FAILED(hr))
		{
			break;
		}
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
		{
			break;
		}

		if (m_amountOfExtraRAM > Cart::MAX_CART_EXTRA_RAM)
		{
			hr = E_FAIL;
			break;
		}

		ZeroMemory(&memoryheader, sizeof(memoryheader));
		memoryheader.size = sizeof(memoryheader) + 0;
		memoryheader.ramsize = m_amountOfExtraRAM; 
		bytesToWrite = sizeof(memoryheader);
		hr = pfs->Write(&memoryheader, bytesToWrite, &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		if (m_amountOfExtraRAM > 0)
		{
			bytesToWrite = memoryheader.ramsize;
			hr = pfs->Write(m_pCartData, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
			{
				break;
			}
		}

		HuffCompression hw;
		hr = hw.Init();
		if (FAILED(hr))
		{
			break;
		}

		hr = hw.SetFile(pfs);
		if (FAILED(hr))
		{
			break;
		}

		int banks = 0;
		for (CrtBankListConstIter it = m_plstBank->cbegin(); it != m_plstBank->cend(); it++)
		{
			Sp_CrtBank sp = *it;
			if (!sp)
			{
				continue;
			}

			SsCartBank bank;
			bank.bank = sp->bank;
			bank.roml.allocatedSize = sp->chipAndDataLow.allocatedSize;
			bank.roml.chip =  sp->chipAndDataLow.chip;
			bank.romh.allocatedSize = sp->chipAndDataHigh.allocatedSize;
			bank.romh.chip =  sp->chipAndDataHigh.chip;
			bytesToWrite = sizeof(bank);
			hr = pfs->Write(&bank, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
			{
				break;
			}

			banks++;
			dataheader.byteCount = sp->chipAndDataLow.allocatedSize;
			dataheader.compressionType = Cart::NOCOMPRESSION;
			bytesToWrite = sizeof(dataheader);
			hr = pfs->Write(&dataheader, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
			{
				break;
			}

			if (dataheader.byteCount > 0)
			{
				if (dataheader.compressionType == Cart::NOCOMPRESSION)
				{
					bytesToWrite = dataheader.byteCount;
					hr = pfs->Write(sp->chipAndDataLow.pData, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
					{
						break;
					}
				}
				else
				{
					dwordCount = dataheader.byteCount / sizeof(bit32);

					bytesToWrite = sizeof(dwordCount);
					hr = pfs->Write(&dwordCount, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
					{
						break;
					}

					if (dwordCount > 0)
					{
						hr = hw.Compress((bit32 *)sp->chipAndDataLow.pData, dwordCount, &compressedSize);
						if (FAILED(hr))
						{
							break;
						}
					}
				}
			}

			dataheader.byteCount = sp->chipAndDataHigh.allocatedSize;
			dataheader.compressionType = Cart::NOCOMPRESSION;
			bytesToWrite = sizeof(dataheader);
			hr = pfs->Write(&dataheader, bytesToWrite, &bytesWritten);
			if (FAILED(hr))
			{
				break;
			}

			if (dataheader.byteCount > 0)
			{
				if (dataheader.compressionType == Cart::NOCOMPRESSION)
				{
					bytesToWrite = dataheader.byteCount;
					hr = pfs->Write(sp->chipAndDataHigh.pData, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
					{
						break;
					}
				}
				else
				{
					dwordCount = dataheader.byteCount / sizeof(bit32);

					bytesToWrite = sizeof(dwordCount);
					hr = pfs->Write(&dwordCount, bytesToWrite, &bytesWritten);
					if (FAILED(hr))
					{
						break;
					}

					if (dwordCount > 0)
					{
						hr = hw.Compress((bit32 *)sp->chipAndDataHigh.pData, dwordCount, &compressedSize);
						if (FAILED(hr))
						{
							break;
						}
					}
				}
			}
		}

		hr = pfs->Seek(spos_zero, STREAM_SEEK_CUR, &pos_next_section_header);
		if (FAILED(hr))
		{
			break;
		}

		spos_next.QuadPart = pos_current_section_header.QuadPart;
		hr = pfs->Seek(spos_next, STREAM_SEEK_SET, &pos_dummy);
		if (FAILED(hr))
		{
			break;
		}

		memoryheader.size = (bit32)(pos_next_section_header.QuadPart - pos_current_section_header.QuadPart);
		memoryheader.banks = banks;
		hr = pfs->Write(&memoryheader, sizeof(memoryheader), &bytesWritten);
		if (FAILED(hr))
		{
			break;
		}

		spos_next.QuadPart = pos_next_section_header.QuadPart;
		hr = pfs->Seek(spos_next, STREAM_SEEK_SET, &pos_dummy);
		if (FAILED(hr))
		{
			break;
		}
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
	m_ipROML = 0;
	m_ipROMH = 0;
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
				}
			}

			if (pH->chip.ROMImageSize != 0)
			{
				m_ipROMH = pH->pData;
			}

			if (m_ipROMH == nullptr)
			{
				m_ipROMH = m_ipROML;
			}
		}
	}

	if (m_ipROML == nullptr)
	{
		m_ipROML = m_pZeroBankData;
	}

	if (m_ipROMH == nullptr)
	{
		m_ipROMH = m_pZeroBankData;
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
	m_bEffects = true;
	memset(&this->registerArray[0], 0, _countof(registerArray));
	m_pCpu->Clear_CRT_IRQ();
	m_pCpu->Clear_CRT_NMI();
	m_pCpu->SetCartridgeRdyHigh(sysclock);
}

void CartCommon::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ConfigureMemoryMap();
	if (m_amountOfExtraRAM > 0 && m_pCartData != nullptr)
	{
		ZeroMemory(&m_pCartData[0x0], m_amountOfExtraRAM);
	}
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

bit8 CartCommon::Get_DMA()
{
	return this->m_bIsCartAttached && this->DMA;
}

bool CartCommon::IsCartIOActive(bit16 address, bool isWriting)
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

bool CartCommon::IsREU()
{
	return m_bIsREU;
}

bool CartCommon::IsUltimax()
{
	return GAME == 0 && EXROM != 0;
}

bit8 CartCommon::ReadROML(bit16 address)
{
	return m_ipROML[address & 0x1fff];
}

void CartCommon::WriteROML(bit16 address, bit8 data)
{
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
	return m_ipROML[address & 0x1fff];
}

void CartCommon::WriteUltimaxROML(bit16 address, bit8 data)
{
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
	WriteROML(address, data);
}

void CartCommon::MonWriteROMH(bit16 address, bit8 data)
{
	WriteROMH(address, data);
}

void CartCommon::MonWriteUltimaxROML(bit16 address, bit8 data)
{
	WriteUltimaxROML(address, data);
}

void CartCommon::MonWriteUltimaxROMH(bit16 address, bit8 data)
{
	WriteUltimaxROMH(address, data);
}

Cart::Cart() noexcept
{
	m_crtHeader.EXROM = 1;
	m_crtHeader.GAME = 1;
	m_crtHeader.HardwareType = 0;
	m_pCartData = 0;
	m_pCpu = NULL;
	m_bIsCartDataLoaded = false;
}

Cart::~Cart() noexcept
{
	CleanUp();
}

void Cart::Init(IC6510 *pCpu, IVic*pVic, bit8 *pC64RamMemory)
{
	this->m_pCpu = pCpu;
	this->m_pC64RamMemory = pC64RamMemory;
	this->m_pVic = pVic;
}

void Cart::CleanUp() noexcept
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
		m_amountOfExtraRAM = 0;
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

unsigned int Cart::GetTotalCartMemoryRequirement(unsigned int amountOfExtraRAM, const CrtBankList *plstBank, unsigned int& offsetToDefault8kZeroBank, unsigned int& offsetToFirstChipBank)
{
	unsigned int i = amountOfExtraRAM;
	unsigned int minmumSupportedRamSize = MIN_CART_EXTRA_RAM;
	if (i < minmumSupportedRamSize)
	{
		i = minmumSupportedRamSize;
	}

	offsetToDefault8kZeroBank = i;
	offsetToFirstChipBank = offsetToDefault8kZeroBank + Cart::DEFAULT_CHIP_EXTRA_MEMORY;
	i = offsetToFirstChipBank;
	for (CrtBankListConstIter it = plstBank->cbegin(); it!=plstBank->cend(); it++)
	{
		Sp_CrtBank sp = *it;
		if (!sp)
		{
			continue;
		}

		i = i + (int)(sp->chipAndDataLow.allocatedSize);
		i = i + (int)(sp->chipAndDataHigh.allocatedSize);
	}

	return i;
}

const bit8 Cart::S_SIGHEADER[] = "C64 CARTRIDGE";
const bit8 Cart::S_SIGCHIP[] = "CHIP";
const bit8 Cart::S_CART_NAME_REU1750[] = "1750 REU";

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
CrtBankList* plstBank = NULL;
unsigned int offsetToDefault8kZeroBank;
unsigned int offsetToFirstChipBank;
bit8 *pCartData = NULL;
unsigned int amountOfExtraRAM = 0;
__int64 filesize=0;
__int64 iFileIndex = 0;

	ClearError();
	
	try
	{
		bool ok = true;
		do
		{
			hFile = CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
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
				{
					break;
				}
				
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
				{
					pChipAndData->allocatedSize = 0x2000;
				}
				else
				{
					pChipAndData->allocatedSize = 0x4000;
				}

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
			{
				break;
			}
		} while (false);

		do
		{
			if (!ok)
			{
				break;
			}

			amountOfExtraRAM = MIN_CART_EXTRA_RAM;
			SIZE_T lenAlloc = GetTotalCartMemoryRequirement(amountOfExtraRAM, plstBank, offsetToDefault8kZeroBank, offsetToFirstChipBank);
			pCartData = (bit8 *)GlobalAlloc(GPTR, lenAlloc);
			if (!pCartData)
			{
				ok = false;
				hr = SetError(E_OUTOFMEMORY, S_OUTOFMEMORY);
				break;
			}			

			bit8 *p = pCartData + (UINT_PTR)offsetToFirstChipBank;
			for (CrtBankListIter it = plstBank->begin(); it!=plstBank->end(); it++)
			{
				Sp_CrtBank sp = *it;
				if (!sp)
				{
					continue;
				}

				for (int i = 0; i < 2; i++)
				{
					CrtChipAndData* pChipAndData;
					if (i == 0)
					{
						pChipAndData = &sp->chipAndDataLow;
					}
					else
					{
						pChipAndData = &sp->chipAndDataHigh;
					}

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
			{
				break;
			}
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

	if (pCartData == nullptr)
	{
		hr = SetError(E_FAIL, TEXT("Failed allocate memory for cartridge."));
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
			default:
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
				m_pZeroBankData = nullptr;
				m_amountOfExtraRAM = 0;
			}

			m_crtHeader = hdr;
			m_plstBank = plstBank;
			m_pCartData = pCartData;
			if (pCartData)
			{
				// unneeded conditional check to remove compiler warning.
				m_pZeroBankData = &pCartData[offsetToDefault8kZeroBank];
				m_amountOfExtraRAM = amountOfExtraRAM;
			}

			pCartData = NULL;
			plstBank = NULL;
			m_bIsCartDataLoaded = true;

			if (!IsSupported())
			{
				hr = SetError(APPWARN_UNKNOWNCARTTYPE, TEXT("The hardware type for this cartridge is not supported. The emulator will attempt to run the ROM images with generic hardware. The cartridge software may not run correctly."));
			}
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
		amountOfExtraRAM = 0;
	}
	
	return hr;
}

HRESULT Cart::LoadReu1750()
{
	HRESULT hr = E_FAIL;
	CrtHeader hdr;
	CrtBankList* plstBank = nullptr;
	bit8* pCartData = nullptr;
	bit8* pZeroBankData = nullptr;
	try
	{		
		Cart::MarkHeaderAsReu(hdr);
		assert(hdr.FileHeaderLength == 0x40);
		plstBank = new CrtBankList();
		pCartData = (bit8*)GlobalAlloc(GPTR, Cart::REU_1750_RAM + Cart::DEFAULT_CHIP_EXTRA_MEMORY);
		if (pCartData == nullptr)
		{
			throw std::bad_alloc();
		}

		pZeroBankData = &pCartData[Cart::REU_1750_RAM];

		hr = S_OK;
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		CleanUp();
		m_crtHeader = hdr;
		if (pCartData)
		{
			m_pCartData = pCartData;
			m_amountOfExtraRAM = Cart::REU_1750_RAM;
			m_pZeroBankData = pZeroBankData;
		}

		m_plstBank = plstBank;
		m_bIsCartDataLoaded = true;
		pCartData = nullptr;
		plstBank = nullptr;
		pZeroBankData = nullptr;
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

bool Cart::IsHeaderReu(const CrtHeader& crtHeader) noexcept
{
	// This is an internal fudge employed by the save state file so as reuse the cartridge save state for REUs too.
	// This header is not ever written to an actual CRT file. 
	if (strncmp((char*)&crtHeader.CartridgeName[0], (char*)&Cart::S_CART_NAME_REU1750[0], _countof(crtHeader.CartridgeName) - 1) == 0)
	{
		return (crtHeader.VersionHigh == 0 && crtHeader.VersionLow == 17);
	}
	else
	{ 
		return false;
	}
}

void Cart::MarkHeaderAsReu(CrtHeader& crtHeader) noexcept
{
	memset(&crtHeader, 0, sizeof(CrtHeader));
	crtHeader.FileHeaderLength = sizeof(CrtHeader);
	strcpy((char*)&crtHeader.Signature[0], (char*)&Cart::S_SIGHEADER[0]);
	strcpy((char*)&crtHeader.CartridgeName[0], (char*)&Cart::S_CART_NAME_REU1750[0]);
	crtHeader.HardwareType = Cart::CartType::Normal_Cartridge;
	crtHeader.EXROM = 1;
	crtHeader.GAME = 1;
	crtHeader.VersionHigh = 0;
	crtHeader.VersionLow = 17;
	crtHeader.SubType = 0;
}

shared_ptr<ICartInterface> Cart::CreateCartInterface(const CrtHeader &crtHeader)
{
shared_ptr<ICartInterface> p;
	try
	{
		switch(crtHeader.HardwareType)
		{
		case CartType::Normal_Cartridge:
			if (IsHeaderReu(crtHeader))
			{
				p = shared_ptr<ICartInterface>(new CartReu1750(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			}
			else
			{
				p = shared_ptr<ICartInterface>(new CartNormalCartridge(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			}
			break;
		case CartType::Action_Replay:
			p = shared_ptr<ICartInterface>(new CartActionReplay(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Final_Cartridge_III:
			p = shared_ptr<ICartInterface>(new CartFinalCartridgeIII(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Simons_Basic:
			p = shared_ptr<ICartInterface>(new CartSimonsBasic(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Ocean_1:
			p = shared_ptr<ICartInterface>(new CartOcean1(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Fun_Play:
			p = shared_ptr<ICartInterface>(new CartFunPlay(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Super_Games:
			p = shared_ptr<ICartInterface>(new CartSuperGames(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::System_3:
			p = shared_ptr<ICartInterface>(new CartSystem3(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Dinamic:
			p = shared_ptr<ICartInterface>(new CartDinamic(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Zaxxon:
			p = shared_ptr<ICartInterface>(new CartZaxxon(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Magic_Desk:
			p = shared_ptr<ICartInterface>(new CartMagicDesk(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_4:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk4(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::EasyFlash:
			p = shared_ptr<ICartInterface>(new CartEasyFlash(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_3:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk3(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Retro_Replay:
			p = shared_ptr<ICartInterface>(new CartRetroReplay(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Action_Replay_2:
			p = shared_ptr<ICartInterface>(new CartActionReplayMk2(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::Epyx_FastLoad:
			p = shared_ptr<ICartInterface>(new CartEpyxFastLoad(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		case CartType::KCS_Power:
			p = shared_ptr<ICartInterface>(new CartKcsPower(crtHeader, this->m_pCpu, this->m_pVic, this->m_pC64RamMemory));
			break;
		default:
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

bit8 Cart::Get_DMA()
{
	if (m_spCurrentCart)
	{
		return m_spCurrentCart->Get_DMA();
	}
	else
	{
		return 1;
	}
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

bool Cart::IsREU()
{
	return m_spCurrentCart && m_spCurrentCart->IsREU();
}

bool Cart::IsUltimax()
{
	return m_spCurrentCart && m_spCurrentCart->IsUltimax();
}

bool Cart::IsCartIOActive(bit16 address, bool isWriting)
{
	return m_spCurrentCart && m_spCurrentCart->IsCartIOActive(address, isWriting);
}

bool Cart::IsCartAttached()
{
	return m_spCurrentCart && m_spCurrentCart->IsCartAttached();
}

void Cart::Set_IsCartAttached(bool isAttached)
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->Set_IsCartAttached(isAttached);
	}
}

void Cart::CartFreeze()
{
	if (this->IsCartAttached())
	{
		m_spCurrentCart->CartFreeze();
	}
}

void Cart::CartReset()
{
	if (this->IsCartAttached())
	{
		m_spCurrentCart->CartReset();
	}
}

void Cart::CheckForCartFreeze()
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->CheckForCartFreeze();
	}
}

void Cart::AttachCart(shared_ptr<ICartInterface> spCartInterface)
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->Set_IsCartAttached(false);
	}

	m_spCurrentCart = spCartInterface;
	if (m_spCurrentCart)
	{
		m_spCurrentCart->Set_IsCartAttached(true);
	}
}

void Cart::AttachCart()
{
	if (this->m_bIsCartDataLoaded)
	{
		shared_ptr<ICartInterface> spCartInterface = this->CreateCartInterface(this->m_crtHeader);
		if (spCartInterface)
		{
			HRESULT hr = spCartInterface->InitCart(this->m_amountOfExtraRAM, this->m_pCartData, this->m_plstBank, this->m_pZeroBankData);
			if (SUCCEEDED(hr))
			{
				m_plstBank = NULL;
				m_pCartData = NULL;
				m_pZeroBankData = NULL;
				m_amountOfExtraRAM = 0;
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
	{
		m_spCurrentCart->ConfigureMemoryMap();
	}
}

bool Cart::SnoopWrite(bit16 address, bit8 data)
{
	if (m_spCurrentCart)
	{
		return m_spCurrentCart->SnoopWrite(address, data);
	}
	else
	{
		return true;
	}
}

void Cart::SnoopRead(bit16 address, bit8 data)
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->SnoopRead(address, data);
	}
}

bit8 *Cart::Get_RomH()
{
	if (m_spCurrentCart)
	{
		return m_spCurrentCart->Get_RomH();
	}
	else
	{
		return nullptr;
	}
}

bit8* Cart::Get_RomL()
{
	if (m_spCurrentCart)
	{
		return m_spCurrentCart->Get_RomL();
	}
	else
	{
		return nullptr;
	}
}

void Cart::PreventClockOverflow()
{
	if (m_spCurrentCart)
	{
		m_spCurrentCart->PreventClockOverflow();
	}
}

HRESULT Cart::LoadState(IStream *pfs, int version)
{
	return E_NOTIMPL;
}

HRESULT Cart::LoadCartInterface(IStream *pfs, int version, shared_ptr<ICartInterface> &spCartInterface)
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

		// Read header size
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
		{
			break;
		}

		// Read header remaining body.
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
		{
			break;
		}
		
		shared_ptr<ICartInterface> spLocalCartInterface = this->CreateCartInterface(crtHeader);
		if (!spLocalCartInterface)
		{
			hr = E_FAIL;
			break;
		}

		hr = spLocalCartInterface->LoadState(pfs, version);
		if (FAILED(hr))
		{
			break;
		}

		spCartInterface = spLocalCartInterface;
	} while (false);

	return hr;
}

HRESULT Cart::InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData)
{
	HRESULT hr = E_FAIL;
	if (m_spCurrentCart)
	{
		hr = m_spCurrentCart->InitCart(amountOfExtraRAM, pCartData, plstBank, pZeroBankData);
	}

	return hr;
}

HRESULT Cart::SaveState(IStream *pfs)
{
	if (m_spCurrentCart)
	{
		return m_spCurrentCart->SaveState(pfs);
	}
	else
	{
		return S_FALSE;
	}
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


