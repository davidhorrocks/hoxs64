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

bit8 CartKcsPower::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
bit8 v;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		bit8 old1 = reg1;
		addr = address - 0xDE00 + 0x9E00;
		v = this->m_ipROML_8000[addr];
		if (addr & 1)
		{
			reg1 |= 8;
		}
		else
		{
			reg1 &= ((~8) & 0xff);
		}
		reg1 |= 2;
		if (old1 != reg1)
			ConfigureMemoryMap();
		return v;
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		addr = (address - 0xDF00) & 0x7f;
		return m_pCartData[addr];
	}
	return 0;
}

void CartKcsPower::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
bit16 addr;
	if (address >= 0xDE00 && address < 0xDF00)
	{
		bit8 old1 = reg1;
		addr = address - 0xDE00 + 0x9E00;
		if (addr & 1)
		{
			reg1 |= 8;
		}
		else
		{
			reg1 &= ~8;
		}
		reg1 &= ((~2) & 0xff);
		if (old1 != reg1)
			ConfigureMemoryMap();
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		addr = (address - 0xDF00) & 0x7f;
		m_pCartData[addr] = data;
	}
}

void CartKcsPower::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		//m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeDone = false;
	}
}

void CartKcsPower::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeDone = true;
		reg1 = 8;
		m_pCpu->Clear_CRT_IRQ();
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
	GAME = (reg1 & 2) != 0;
	EXROM = (reg1 & 8) != 0;
	BankRom();
	//if (m_bFreezePending)
	//{
	//}
	//else if (m_bFreezeDone)	
	//{
	//}
	//else
	//{
	//}			
}
