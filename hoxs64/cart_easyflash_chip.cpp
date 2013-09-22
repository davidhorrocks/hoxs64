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

EasyFlashChip::EasyFlashChip()
{
	m_chipNumber = 0;
	m_vecPendingSectorErase.reserve(MAXEASYFLASHBANKS);
}

EasyFlashChip::~EasyFlashChip()
{
	CleanUp();
}

unsigned int EasyFlashChip::GetStateBytes(void *pstate)
{
int totalsize = sizeof(SsState) + sizeof(bit32) + EasyFlashChip::MAXEASYFLASHBANKS;
bit8 *p;

	if (pstate)
	{
		ZeroMemory(pstate, totalsize);
		SsState *s = (SsState *)pstate;
		s->size = totalsize;
		s->m_iLastCommandWriteClock = m_iLastCommandWriteClock;
		s->m_iCommandByte = m_iCommandByte;
		s->m_iCommandCycle = m_iCommandCycle;
		s->m_iStatus = m_iStatus;
		s->m_iByteWritten = m_iByteWritten;
		//s->m_iAddressWrite = m_iAddressWrite;
		//s->m_iSectorWrite = m_iSectorWrite;
		s->m_mode = m_mode;
		
		p = &(((bit8 *)s)[sizeof(SsState)]);

		bit32 n = (bit32)min(this->m_vecPendingSectorErase.size(), EasyFlashChip::MAXEASYFLASHBANKS);
		*((bit32 *)p) = n;
		p++;
		for (unsigned int i=0; i<n; i++, p++)
		{
			*p = m_vecPendingSectorErase[i];
		}

	}
	return totalsize;
}

HRESULT EasyFlashChip::SetStateBytes(void *pstate, unsigned int size)
{
int totalsize = sizeof(SsState) + sizeof(bit32) + EasyFlashChip::MAXEASYFLASHBANKS;
SsState *s;
	if (!pstate || size != totalsize)
		return E_FAIL;

	s = (SsState *)pstate;
	if (s->size != totalsize)
		return E_FAIL;

	m_iLastCommandWriteClock = s->m_iLastCommandWriteClock;
	m_iCommandByte = s->m_iCommandByte;
	m_iCommandCycle = s->m_iCommandCycle;
	m_iStatus = s->m_iStatus;
	m_iByteWritten = s->m_iByteWritten;
	//m_iAddressWrite = s->m_iAddressWrite;
	//m_iSectorWrite = s->m_iSectorWrite;
	m_mode = (EEasyFlashMode)s->m_mode;

	bit8 *p = &(((bit8 *)s)[sizeof(SsState)]);
	bit32 n = (bit32)min(this->m_vecPendingSectorErase.size(), EasyFlashChip::MAXEASYFLASHBANKS);

	m_vecPendingSectorErase.resize(n);
	for (unsigned int i=0; i<n; i++)
	{
		m_vecPendingSectorErase.at(i) = p[i];
	}
	return S_OK;

}

void EasyFlashChip::Detach()
{
	CleanUp();
}

void EasyFlashChip::CleanUp()
{

}

HRESULT EasyFlashChip::Init(CartEasyFlash *pCartEasyFlash, int chipNumber)
{
/*
	Easy Flash has two 512 KB chips that provide a total of 1 MB.
	Each 512 KB chip as 64 banks and each bank provides 8 KB.
*/
int i;
const int BANKSIZE = 0x2000;
HRESULT hr = E_FAIL;

	try
	{
		m_plstBank = pCartEasyFlash->m_plstBank;
		m_pCartEasyFlash = pCartEasyFlash;
		m_chipNumber = chipNumber;

		CrtBankList *plstBank = m_pCartEasyFlash->m_plstBank;
		
		i = 0;
		for (CrtBankListIter it = plstBank->begin(); it != plstBank->end() && i < MAXEASYFLASHBANKS; it++,i++)
		{
			CrtChipAndData *pcdLo;
			CrtChipAndData *pcdHi;
			Sp_CrtBank spBank;
			if (*it == NULL)
			{
				spBank = Sp_CrtBank(new CrtBank());
				if (!spBank)
					throw std::bad_alloc();
				*it = spBank;
				spBank->bank = i;
			}
			else
			{
				spBank = *it;
			}
			pcdLo = &spBank.get()->chipAndDataLow;
			pcdHi = &spBank.get()->chipAndDataHigh;
			if (chipNumber == 0)
			{
				if (pcdLo->pData == NULL)
				{
					if (!(pcdLo->pData = (bit8 *)GlobalAlloc(GPTR, BANKSIZE)))
						throw std::bad_alloc();
					pcdLo->ownData = true;
					pcdLo->allocatedSize = BANKSIZE;
					pcdLo->chip.BankLocation = i;
					pcdLo->chip.ChipType = Cart::ChipType::EPROM;
					pcdLo->chip.ROMImageSize = BANKSIZE;
					pcdLo->chip.LoadAddressRange = 0x8000;
				}
			}
			else
			{
				if (pcdHi->pData == NULL)
				{
					if (pcdLo->allocatedSize == 0x4000 && pcdLo->pData != NULL)
					{
						pcdHi->pData = &pcdLo->pData[0x2000];
						pcdHi->ownData = false;
						pcdHi->allocatedSize = BANKSIZE;
						pcdHi->chip.BankLocation = i;
						pcdHi->chip.ChipType = Cart::ChipType::EPROM;
						pcdHi->chip.ROMImageSize = BANKSIZE;
						pcdHi->chip.LoadAddressRange = 0xA000;
					}
					else
					{
						if (!(pcdHi->pData = (bit8 *)GlobalAlloc(GPTR, BANKSIZE)))
							throw std::bad_alloc();
						pcdHi->ownData = true;
						pcdHi->allocatedSize = BANKSIZE;
						pcdHi->chip.BankLocation = i;
						pcdHi->chip.ChipType = Cart::ChipType::EPROM;
						pcdHi->chip.ROMImageSize = BANKSIZE;
						pcdHi->chip.LoadAddressRange = 0xA000;
					}
				}
			}
		}
		hr = S_OK;
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}
	return hr;
}

void EasyFlashChip::Reset(ICLK sysclock)
{
	m_iLastCommandWriteClock = sysclock;
	m_iCommandByte = 0;
	m_iCommandCycle = 0;
	m_iStatus = 0;
	m_iByteWritten = 0;

	m_mode = Read;
	//m_iAddressWrite = 0;
	//m_iSectorWrite = 0;
	m_iStatus = 0;
	m_vecPendingSectorErase.clear();
}

void EasyFlashChip::MonWriteByte(bit16 address, bit8 data)
{
int k;
	address = address & 0x1fff;
	ICLK clock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();
	CheckForPendingWrite(clock);
	k = (m_pCartEasyFlash->m_iSelectedBank) & 0x3f;
	if ((unsigned int)k < m_plstBank->size())
	{
		CrtChipAndData *pc;
		CrtBank *pBank = m_plstBank->at(k).get();
		if (m_chipNumber == 0)
			pc = &pBank->chipAndDataLow;
		else
			pc = &pBank->chipAndDataHigh;
		if (pc->pData && address < pc->chip.ROMImageSize)
			pc->pData[address] &= data;
	}
}

void EasyFlashChip::WriteByte(bit16 address, bit8 data)
{
int k;

	//TODO check for 80us command write timeout.
	address = address & 0x1fff;
	ICLK clock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();
	CheckForPendingWrite(clock);
	switch(m_mode)
	{
	case Read:
		if ((ICLKS)(clock - m_iLastCommandWriteClock) > 200)
		{
			m_iCommandCycle = 0;
		}
		break;
	case AutoSelect:
		if (data == 0xF0)
		{
			m_mode = Read;
			m_iCommandCycle = 0;
		}
		break;
	case ByteProgram:
		k = (m_pCartEasyFlash->m_iSelectedBank) & 0x3f;
		if ((unsigned int)k < m_plstBank->size())
		{
			CrtChipAndData *pc;
			CrtBank *pBank = m_plstBank->at(k).get();
			if (m_chipNumber == 0)
				pc = &pBank->chipAndDataLow;
			else
				pc = &pBank->chipAndDataHigh;
			if (pc->pData && address < pc->chip.ROMImageSize)
				pc->pData[address] &= data;
		}
		m_mode = Read;
		m_iCommandCycle = 0;
		break;
	case ChipErase:
		if (data == 0xB0)
		{
			m_mode = ChipEraseSuspend;
		}
		else
		{
			m_mode = Read;
			m_iCommandCycle = 0;
		}
		break;
	case SectorErase:
		if (data == 0xB0)
		{
			m_mode = SectorEraseSuspend;
		}
		else if (address == 0)
		{
			if (m_vecPendingSectorErase.capacity() > m_vecPendingSectorErase.size())
				m_vecPendingSectorErase.push_back(m_pCartEasyFlash->m_iSelectedBank);
		}
		else if (address != 0)
		{
			address=address;
		}
		else
		{
			m_mode = Read;
			m_iCommandCycle = 0;
		}
		break;
	case SectorEraseSuspend:
		if (data == 0x30)
		{
			m_mode = SectorErase;
		}
		break;
	case ChipEraseSuspend:
		if (data == 0x30)
		{
			m_mode = ChipErase;
		}
		break;
	}
	m_iLastCommandWriteClock = clock;

	switch (m_iCommandCycle)
	{
	case 0:
		if (address == 0x555 && data == 0xAA)
		{
			m_iCommandCycle = 1;
		}
		break;
	case 1:
		if (address == 0x2AA && data == 0x55)
		{
			m_iCommandCycle = 2;
		}
		else if (address == 0x555 && data == 0xAA)
		{
			m_iCommandCycle = 1;
		}
		else
		{
			m_iCommandCycle = 0;
		}
		break;
	case 2:
		if (address == 0x555)
		{
			switch (data)
			{
			case 0xF0://Read/Reset
				m_mode = Read;
				m_iCommandCycle = 0;
				break;
			case 0x90://Autoselect
				m_mode = AutoSelect;
				m_iCommandCycle = 0;
				break;
			case 0xA0://Byte Program
				m_mode = ByteProgram;
				m_iCommandCycle = 6;
				break;
			case 0x80://Erase
				m_iCommandByte = data;
				m_iCommandCycle = 3;
				break;
			case 0xAA:
				m_iCommandCycle = 1;
				break;
			default:
				m_iCommandCycle = 0;
				break;
			}
		}
		else
		{
			m_iCommandCycle = 0;
		}
		break;
	case 3:
		if (address == 0x555 && data == 0xAA)
		{
			m_iCommandCycle = 4;
		}
		else
		{
			m_iCommandCycle = 0;
		}
		break;
	case 4:
		if (address == 0x2AA && data == 0x55)
		{
			m_iCommandCycle = 5;
		}
		else if (address == 0x555 && data == 0xAA)
		{
			m_iCommandCycle = 1;
		}
		else
		{
			m_iCommandCycle = 0;
		}
		break;
	case 5:
		if (address == 0x555 && data == 0x10)
		{
			m_mode = ChipErase;
			m_iCommandCycle = 6;
			m_iStatus = 0;
		}
		else if (address == 0x555 && data == 0xAA)
		{
			m_iCommandCycle = 1;
		}
		else if (data == 0x30)
		{
			m_mode = SectorErase;
			m_iCommandCycle = 6;
			m_iStatus = 0;
			m_vecPendingSectorErase.clear();
			m_vecPendingSectorErase.push_back(m_pCartEasyFlash->m_iSelectedBank);
		}
		else
		{
			m_iCommandCycle = 0;
		}
		break;
	}
}

bit8 EasyFlashChip::MonReadByte(bit16 address)
{
int k;
	address = address & 0x1fff;
	ICLK clock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();
	CheckForPendingWrite(clock);
	k = (m_pCartEasyFlash->m_iSelectedBank)  & 0x3f;
	if ((unsigned int)k < m_plstBank->size())
	{
		CrtChipAndData *pc;
		CrtBank *pBank = m_plstBank->at(k).get();
		if (m_chipNumber == 0)
			pc = &pBank->chipAndDataLow;
		else
			pc = &pBank->chipAndDataHigh;
		if (pc->pData && address < pc->chip.ROMImageSize)
			return pc->pData[address];
	}
	return 0;
}

bit8 EasyFlashChip::ReadByte(bit16 address)
{
int k;
const bit8 AmdManufacterId = 1;
const bit8 Am29F040 = 0xA4;

	address = address & 0x1fff;
	ICLK clock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();
	CheckForPendingWrite(clock);
	switch(m_mode)
	{
	case Read:
		k = (m_pCartEasyFlash->m_iSelectedBank)  & 0x3f;
		if ((unsigned int)k < m_plstBank->size())
		{
			CrtChipAndData *pc;
			CrtBank *pBank = m_plstBank->at(k).get();
			if (m_chipNumber == 0)
				pc = &pBank->chipAndDataLow;
			else
				pc = &pBank->chipAndDataHigh;
			if (pc->pData && address < pc->chip.ROMImageSize)
				return pc->pData[address];
		}
		return 0;
	case AutoSelect:
		if (address == 0x0000)
			return AmdManufacterId;
		else if (address == 0x0001)
			return Am29F040;
		return 0;
	case ChipErase:
		m_iStatus = (m_iStatus ^ 0x40);
		return (m_iStatus & 0x40);
	case SectorErase:
		m_iStatus = (m_iStatus ^ 0x40);
		return m_iStatus & 0x40;
	case SectorEraseSuspend:
		return m_iStatus & 0x40 | 0x88;
	case ChipEraseSuspend:
		return m_iStatus & 0x40 | 0x88;
	}
	return 0;
}

void EasyFlashChip::CheckForPendingWrite(ICLK clock)
{
int k;

	//TODO check for 80us command write timeout.
	switch(m_mode)
	{
	case ChipErase:
		if ((ICLKS)(clock - m_iLastCommandWriteClock) > 80)
		{
			m_iCommandCycle = 0;
			for (k = 0; (unsigned int)k < m_plstBank->size(); k++)
			{
				CrtChipAndData *pc;
				CrtBank *pBank = m_plstBank->at(k).get();
				if (m_chipNumber == 0)
					pc = &pBank->chipAndDataLow;
				else
					pc = &pBank->chipAndDataHigh;
				if (pc->pData)
					FillMemory(pc->pData, pc->chip.ROMImageSize, 0xff);
			}
		}
		m_mode = Read;
		m_iCommandCycle = 0;
		break;
	case SectorErase:
		if ((ICLKS)(clock - m_iLastCommandWriteClock) > 80)
		{
			m_iCommandCycle = 0;
			for (std::vector<bit8>::iterator it = m_vecPendingSectorErase.begin(); it!=m_vecPendingSectorErase.end(); it++)
			{
				for (int i = 0; i < 8; i++)
				{
					k = ((*it) +i ) & 0x3f;
					if ((unsigned int)k < m_plstBank->size())
					{
						CrtChipAndData *pc;
						CrtBank *pBank = m_plstBank->at(k).get();
						if (m_chipNumber == 0)
							pc = &pBank->chipAndDataLow;
						else
							pc = &pBank->chipAndDataHigh;
						if (pc->pData)
							FillMemory(pc->pData, pc->chip.ROMImageSize, 0xff);
					}
				}
			}
			m_mode = Read;
			m_iCommandCycle = 0;
		}
		break;
	}
}

void EasyFlashChip::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;

	ICLK clock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();

	ICLK ClockBehindNear = clock - CLOCKSYNCBAND_NEAR;
	
	if ((ICLKS)(clock - m_iLastCommandWriteClock) >= CLOCKSYNCBAND_FAR)
	{
		m_iLastCommandWriteClock = ClockBehindNear;
	}
}

void EasyFlashChip::SetCurrentClock(ICLK sysclock)
{
ICLK CurrentClock = m_pCartEasyFlash->m_pCpu->Get6510CurrentClock();
ICLK v = sysclock - CurrentClock;
	this->m_iLastCommandWriteClock += v;
}