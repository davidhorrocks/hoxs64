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

CartEasyFlash::CartEasyFlash(IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(pCpu, pC64RamMemory)
{
	m_EasyFlashChipROML.Init(this, 0);
	m_EasyFlashChipROMH.Init(this, 1);
}

CartEasyFlash::~CartEasyFlash()
{
	m_EasyFlashChipROML.Detach();
	m_EasyFlashChipROMH.Detach();
}

void CartEasyFlash::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	reg1 = 0;
	reg2 = 5;
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
	GAME = (reg2 & 0x1) == 0;
	EXROM = (reg2 & 0x2) == 0;
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

