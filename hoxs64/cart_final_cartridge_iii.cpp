#include "cart.h"

CartFinalCartridgeIII::CartFinalCartridgeIII(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartFinalCartridgeIII::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (address >= 0xDE00 && address < 0xE000)
	{
		addr = address - 0xDE00 + 0x9E00;
		return this->m_ipROML[addr & 0x1fff];
	}
	return 0;
}

void CartFinalCartridgeIII::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDFFF && (reg1 & 0x80)==0)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartFinalCartridgeIII::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeMode = false;
	}
}

void CartFinalCartridgeIII::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_bFreezePending = false;
		m_bFreezeMode = false;
		reg1 = (reg1 & 0x1C) | 0x10;
		//Clear cart IRQ but leave cart NMI active.
		m_pCpu->Clear_CRT_IRQ();
		ConfigureMemoryMap();
	}
}

void CartFinalCartridgeIII::UpdateIO()
{
	if (m_bFreezePending)
	{
		BankRom();
	}
	else
	{
		m_iSelectedBank = reg1 & 3;
		GAME = (reg1 & 0x20) != 0;
		EXROM = (reg1 & 0x10) != 0;
		if (reg1 & 0x40)
			m_pCpu->Clear_CRT_NMI();
		else
			m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bIsCartIOActive = true;
		BankRom();
	}
}
