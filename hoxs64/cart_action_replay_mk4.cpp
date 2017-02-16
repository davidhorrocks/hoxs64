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

CartActionReplayMk4::CartActionReplayMk4(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

bit8 CartActionReplayMk4::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		return 0;
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		addr = address - 0xDF00 + 0x9F00;
		return this->m_ipROML[addr & 0x1fff];
	}
	return 0;
}

void CartActionReplayMk4::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address >= 0xDE00 && address < 0xDF00)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartActionReplayMk4::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeMode = false;
	}
}

void CartActionReplayMk4::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeMode = false;
		reg1 = 0x00;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();
		ConfigureMemoryMap();
	}
}

void CartActionReplayMk4::UpdateIO()
{
	if (m_bFreezePending)
	{
		BankRom();
	}
	else
	{
		m_iSelectedBank = ((reg1) & 1) | ((reg1 >> 3) & 2);
		m_bEnableRAM = false;
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = 0;
		GAME = (reg1 >> 1) & 1;
		EXROM = (~reg1 >> 3) & 1;
		m_bIsCartIOActive = (reg1 & 0x4) == 0;
		BankRom();
		switch(((reg1 & 2) >> 1) | ((reg1 & 8) >> 2))
		{
		case 0://GAME=0 EXROM=1 Ultimax
			//E000 mirrors 8000
			m_ipROMH = m_ipROML;
			m_ipROML = m_pZeroBankData;
			break;
		case 1://GAME=1 EXROM=1
			break;
		case 2://GAME=0 EXROM=0
			//A000 mirrors 8000
			m_ipROMH = m_ipROML;
			break;
		case 3://GAME=1 EXROM=0
			m_ipROMH = m_ipROML;
			break;
		}
	}			
}
