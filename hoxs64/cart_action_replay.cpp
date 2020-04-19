#include "cart.h"

CartActionReplay::CartActionReplay(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
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
			return m_pCartData[(addr & 0x1fff) + m_iRamBankOffsetIO];
		}
		else
		{
			addr = address - 0xDF00 + 0x9F00;
			return this->m_ipROML[addr & 0x1fff];
		}
	}
	return 0;
}

void CartActionReplay::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address >= 0xDE00 && address < 0xDF00)
	{
		if (m_bFreezeMode)
		{
			if (data & 0x40)
			{
				m_bFreezePending = false;
				m_bFreezeMode = false;
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
		m_bFreezeMode = false;
	}
}

void CartActionReplay::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeMode = true;
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
	else if (m_bFreezeMode)	
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
		m_ipROMH = m_ipROML;
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
		m_ipROMH = m_ipROML;
	}			
}
