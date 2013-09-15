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

CartActionReplay::CartActionReplay(IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(pCpu, pC64RamMemory)
{
}

bit8 CartActionReplay::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		return 0;
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (m_bEnableRAM)
		{
			addr = address - 0xDF00 + 0x1F00;
			return m_pCartData[addr + m_iRamBankOffsetIO];
		}
		else
		{
			addr = address - 0xDF00 + 0x9F00;
			return this->m_ipROML_8000[addr];
		}
	}
	return 0;
}

void CartActionReplay::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
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
}

void CartActionReplay::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeDone = false;
	}
}

void CartActionReplay::CheckForCartFreeze()
{
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

void CartActionReplay::UpdateIO()
{
	if (m_bFreezePending)
	{
		BankRom();
	}
	else if (m_bFreezeDone)	
	{
		m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
		m_bEnableRAM = (reg1 & 0x20) != 0;
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = 0;
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
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = 0;
		GAME = (~reg1 & 1);
		EXROM = (reg1 >> 1) & 1;
		m_bIsCartIOActive = (reg1 & 0x4) == 0;
		BankRom();
		m_ipROMH_E000 = m_ipROML_8000 + 0x8000 - 0xE000;
	}			
}
