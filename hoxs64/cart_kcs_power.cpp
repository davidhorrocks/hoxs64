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


CartKcsPower::CartKcsPower(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

void CartKcsPower::LatchShift()
{
	reg1 = ((reg1 & ~5) | ((reg1 & 0xA) >> 1)) & 0xff;//Move old EXROM in bit 3 to bit 2. Move old GAME in bit 1 to bit 0
}

void CartKcsPower::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	reg1 = 0;
	reg2 = 0;
	ConfigureMemoryMap();
}

bit8 CartKcsPower::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
bit8 v;
	bit8 old1 = reg1;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		addr = address - 0xDE00 + 0x9E00;
		v = this->m_ipROML_8000[addr];
		if (m_bEffects)
		{
			LatchShift();
			if (addr & 2)
			{
				//Reading with (IO1 == 0 && A1 == 1). 
				reg1 |= 8;//Set EXROM
				reg1 |= 2;//Set GAME
			}
			else
			{
				//Reading with (IO1 == 0 && A1 == 0). 
				reg1 &= ((~8) & 0xff);//Clear EXROM
				reg1 |= 2;//Set GAME
			}
			if (old1 != reg1)
				ConfigureMemoryMap();
		}
		return v;
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (m_bEffects)
		{
			if (m_bFreezeMode && ((address & 0x80) != 0))
			{
				reg1 |= 8;//Set EXROM
				reg1 &= ~2;//Clear GAME
				m_bFreezePending = false;
				m_bFreezeMode = false;
				m_pCpu->Clear_CRT_NMI();
			}
			if (old1 != reg1)
				ConfigureMemoryMap();
		}
		addr = (address - 0xDF00) & 0x7f;
		return m_pCartData[addr];
	}
	return 0;
}

void CartKcsPower::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
bit16 addr;
	bit8 old1 = reg1;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		addr = address - 0xDE00 + 0x9E00;
		LatchShift();
		if (addr & 2)
		{
			//Writing with (A1 == 1 && IO1 == 0).
			//reg1 &= ~8;//Clear EXROM
			//reg1 |= 2;//Set GAME

			//reg1 |= 8;//Set EXROM
			//reg1 &= ~2;//Clear GAME

			reg1 &= ~8;//Clear EXROM
			reg1 &= ~2;//Clear GAME
		}
		else
		{
			//Writing with (A1 == 0 && IO1 == 0). 
			reg1 &= ~8;//Clear EXROM
			reg1 &= ~2;//Clear GAME
		}
		//reg1 &= 0xfd;//Writing with (IO1 == 0). Clear GAME
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (!m_bFreezeMode && ((address & 0x80) != 0))
		{
			reg1 &= ~8;//Clear EXROM
			reg1 &= ~2;//Clear GAME
		}
		addr = (address - 0xDF00) & 0x7f;
		m_pCartData[addr] = data;
	}
	if (old1 != reg1)
		ConfigureMemoryMap();
}

void CartKcsPower::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeMode = false;
	}
}

void CartKcsPower::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeMode = true;
		LatchShift();
		reg1 |= 8;//Set EXROM in bit 3.
		reg1 &= ~2;//Clear GAME in bit 1
		m_pCpu->Clear_CRT_NMI();
		ConfigureMemoryMap();
	}
}

void CartKcsPower::UpdateIO()
{
	m_iSelectedBank = 0;
	m_bEnableRAM = false;
	m_iRamBankOffsetIO = 0;
	m_iRamBankOffsetRomL = 0;
	m_bIsCartIOActive = true;
	if (m_bFreezeMode)
	{
		GAME = 0;
		EXROM= 1;
	}
	else
	{
		GAME = (reg1 & 2) != 0;//bit 1 sets GAME. bit 0 reads GAME on latching.
		EXROM = (reg1 & 8) != 0;//bit 3 sets EXROM. bit 2 reads EXROM on latching.
	}
	BankRom();
}
