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

CartEasyFlash::CartEasyFlash(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{

}

CartEasyFlash::~CartEasyFlash()
{
	m_EasyFlashChipROML.Detach();
	m_EasyFlashChipROMH.Detach();
}

HRESULT CartEasyFlash::InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData)
{
HRESULT hr=E_FAIL;

	try
	{
		do
		{
			m_plstBank = plstBank;
			m_pCartData = pCartData;
			m_pZeroBankData = pZeroBankData;
			if (plstBank->size() < EasyFlashChip::MAXEASYFLASHBANKS)
			{
				plstBank->resize(EasyFlashChip::MAXEASYFLASHBANKS);
			}
			hr = m_EasyFlashChipROML.Init(this, 0);
			if (FAILED(hr))
				break;
			hr = m_EasyFlashChipROMH.Init(this, 1);
			if (FAILED(hr))
				break;
		} while (false);
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}
	if (FAILED(hr))
	{
		m_plstBank = NULL;
		m_pCartData = NULL;
		m_pZeroBankData = NULL;
	}
	return hr;
}

void CartEasyFlash::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	reg1 = 0;
	reg2 = 0;
	m_EasyFlashChipROML.Reset(sysclock);
	m_EasyFlashChipROMH.Reset(sysclock);
	ConfigureMemoryMap();
}

bit8 CartEasyFlash::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (address >= 0xDF00 && address < 0xE000)
	{
		addr = address - 0xDF00;
		return m_pCartData[addr];
	}
	else if (address == 0xDE00 || address == 0xDE02)
	{
		return 0;
	}
	return 0;
}

void CartEasyFlash::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
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
}

bit8 CartEasyFlash::ReadROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	return this->m_EasyFlashChipROML.ReadByte(address - 0x8000);
}

bit8 CartEasyFlash::ReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xC000);
	return this->m_EasyFlashChipROMH.ReadByte(address - 0xA000);
}

bit8 CartEasyFlash::ReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	return this->m_EasyFlashChipROML.ReadByte(address - 0x8000);
}

bit8 CartEasyFlash::ReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	return this->m_EasyFlashChipROMH.ReadByte(address - 0xE000);
}

void CartEasyFlash::WriteROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROML.WriteByte(address - 0x8000, data);
}

void CartEasyFlash::WriteROMH(bit16 address, bit8 data)
{
	assert(address >= 0xA000 && address < 0xC000);
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROMH.WriteByte(address - 0xA000, data);
}

void CartEasyFlash::WriteUltimaxROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	this->m_EasyFlashChipROML.WriteByte(address - 0x8000, data);
}

void CartEasyFlash::WriteUltimaxROMH(bit16 address, bit8 data)
{
	assert(address >= 0xE000);
	this->m_EasyFlashChipROMH.WriteByte(address - 0xE000, data);
}

bit8 CartEasyFlash::MonReadROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	return this->m_EasyFlashChipROML.MonReadByte(address - 0x8000);
}

bit8 CartEasyFlash::MonReadROMH(bit16 address)
{
	assert(address >= 0xA000 && address < 0xC000);
	return this->m_EasyFlashChipROMH.MonReadByte(address - 0xA000);
}

bit8 CartEasyFlash::MonReadUltimaxROML(bit16 address)
{
	assert(address >= 0x8000 && address < 0xA000);
	return this->m_EasyFlashChipROML.MonReadByte(address - 0x8000);
}

bit8 CartEasyFlash::MonReadUltimaxROMH(bit16 address)
{
	assert(address >= 0xE000);
	return this->m_EasyFlashChipROMH.MonReadByte(address - 0xE000);
}

void CartEasyFlash::MonWriteROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROML.MonWriteByte(address - 0x8000, data);
}

void CartEasyFlash::MonWriteROMH(bit16 address, bit8 data)
{
	assert(address >= 0xA000 && address < 0xC000);
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROMH.MonWriteByte(address - 0xA000, data);
}

void CartEasyFlash::MonWriteUltimaxROML(bit16 address, bit8 data)
{
	assert(address >= 0x8000 && address < 0xA000);
	this->m_EasyFlashChipROML.MonWriteByte(address - 0x8000, data);
}

void CartEasyFlash::MonWriteUltimaxROMH(bit16 address, bit8 data)
{
	assert(address >= 0xE000);
	this->m_EasyFlashChipROMH.MonWriteByte(address - 0xE000, data);
}

void CartEasyFlash::UpdateIO()
{
	m_iSelectedBank = reg1 & 0x3f;
	if (reg2 & 4)
	{
		GAME = (reg2 & 0x1) == 0;
		EXROM = (reg2 & 0x2) == 0;
	}
	else
	{
		GAME = 0;
		EXROM = (reg2 & 0x2) == 0;
	}
	m_bIsCartIOActive = true;
	BankRom();
}

void CartEasyFlash::PreventClockOverflow()
{
	m_EasyFlashChipROML.PreventClockOverflow();
	m_EasyFlashChipROMH.PreventClockOverflow();
}

ICLK CartEasyFlash::GetCurrentClock()
{
	return this->m_pCpu->Get6510CurrentClock();
}

void CartEasyFlash::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - this->m_pCpu->Get6510CurrentClock();
	m_EasyFlashChipROML.SetCurrentClock(sysclock);
	m_EasyFlashChipROMH.SetCurrentClock(sysclock);
}

unsigned int CartEasyFlash::GetStateBytes(void *pstate)
{
int k, c;
bit8 *p = (bit8 *)pstate;

	c = 0;
	k = CartCommon::GetStateBytes(p);
	if (p)
		p += k;
	c += k;
	k = m_EasyFlashChipROML.GetStateBytes(p);
	if (p)
		p += k;
	c += k;
	k = m_EasyFlashChipROMH.GetStateBytes(p);
	c += k;
	return c;
}

HRESULT CartEasyFlash::SetStateBytes(void *pstate, unsigned int size)
{
HRESULT hr;
unsigned int k;
	int totalsize = GetStateBytes(NULL);

	if (!pstate || size != totalsize)
		return E_FAIL;

	bit8 *p = (bit8 *)pstate;

	k = CartCommon::GetStateBytes(NULL);
	hr = CartCommon::SetStateBytes(p, k);
	if (FAILED(hr))
		return hr;

	p+=k;
	k = m_EasyFlashChipROML.GetStateBytes(NULL);
	hr = m_EasyFlashChipROML.SetStateBytes(p, k);
	if (FAILED(hr))
		return hr;

	p+=k;
	k = m_EasyFlashChipROMH.GetStateBytes(NULL);
	hr = m_EasyFlashChipROMH.SetStateBytes(p, k);
	if (FAILED(hr))
		return hr;

	return S_OK;
}
