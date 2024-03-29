#include "cart.h"

CartActionReplayMk3::CartActionReplayMk3(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartActionReplay(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartActionReplayMk3::ReadRegister(bit16 address, ICLK sysclock)
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

void CartActionReplayMk3::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address >= 0xDE00 && address < 0xDF00)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartActionReplayMk3::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeMode = false;
	}
}

void CartActionReplayMk3::CheckForCartFreeze()
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

void CartActionReplayMk3::UpdateIO()
{
	if (m_bFreezePending)
	{
		BankRom();
	}
	else
	{
		m_iSelectedBank = ((reg1) & 1);
		m_bEnableRAM = false;
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = 0;
		GAME = (reg1 >> 1) & 1;//0;
		EXROM = (~reg1 >> 3) & 1;
		m_bIsCartIOActive = (reg1 & 0x4) == 0;
		BankRom();
		m_ipROMH = m_ipROML;
	}			
}
