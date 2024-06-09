#include "cart.h"

CartRetroReplay::CartRetroReplay(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory),
	m_bDE01WriteDone(m_statebool[0]),
	m_bIsFreezeButtonPressed(m_statebool[0]),
	m_freezeButtonPressedClock(m_stateuint[0])
{
	m_bDE01WriteDone = false;
}

void CartRetroReplay::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	m_bDE01WriteDone = false;
	ConfigureMemoryMap();
}

bit8 CartRetroReplay::ReadRegister(bit16 address, ICLK sysclock)
{
	bit16 addr;

	if (!IsCartAttached())
	{
		return 0;
	}

	if ((address == 0xDE00 || address == 0xDE01))
	{
		return (reg1 & 0xB8) | (reg2 & 0x42) | ((m_bIsFreezeButtonPressed != false) << 2);
	}
	else if (address >= 0xDE00 && address < 0xDF00 && m_bREUcompatible)
	{
		if (m_bEnableRAM)
		{
			addr = address - 0xDE00 + 0x1E00;
			return m_pCartData[addr + m_iRamBankOffsetIO];
		}
		else
		{
			addr = address - 0xDE00 + 0x1E00;
			return this->m_ipROML[addr & 0x1fff];
		}
	}
	else if (address >= 0xDF00 && address < 0xE000 && !m_bREUcompatible)
	{
		if (m_bEnableRAM)
		{
			addr = address - 0xDF00 + 0x1F00;
			return m_pCartData[addr + m_iRamBankOffsetIO];
		}
		else
		{
			addr = address - 0xDF00 + 0x1F00;
			return this->m_ipROML[addr & 0x1fff];
		}
	}
	return 0;
}

void CartRetroReplay::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (!IsCartAttached())
	{
		return;
	}

	if (address == 0xDE00)
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
	else if (address == 0xDE01)
	{
		if (m_bDE01WriteDone)
		{
			reg2 = (reg2 & 0x46) | (data & ~0x46);
		}
		else
		{
			reg2 = data;
			m_bDE01WriteDone = true;
		}
		reg1 = (reg1 & 0x63) | (data & 0x98);
		ConfigureMemoryMap();
	}
	else if (m_bREUcompatible && address >= 0xDE00 && address < 0xDF00)
	{
		if (m_bEnableRAM)
		{
			m_pCartData[address - 0xDE00 + 0x1E00 + m_iRamBankOffsetIO] = data;
		}
	}
	else if (!m_bREUcompatible && address >= 0xDF00 && address < 0xE000)
	{
		if (m_bEnableRAM)
		{
			m_pCartData[address - 0xDF00 + 0x1F00 + m_iRamBankOffsetIO] = data;
		}
	}
}

bit8 CartRetroReplay::ReadROML(bit16 address)
{
	if (m_bEnableRAM)
	{
		return m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL];
	}
	else
	{
		return m_ipROML[address & 0x1fff];
	}
}

bit8 CartRetroReplay::ReadROMH(bit16 address)
{
	return m_ipROMH[address & 0x1fff];
}

bit8 CartRetroReplay::ReadUltimaxROML(bit16 address)
{
	return ReadROML(address);
}

bit8 CartRetroReplay::ReadUltimaxROMH(bit16 address)
{
	return ReadROMH(address);
}

void CartRetroReplay::WriteROML(bit16 address, bit8 data)
{
	if (m_bEnableRAM)
	{
		m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL] = data;
	}

	m_pC64RamMemory[address] = data;
}

void CartRetroReplay::WriteROMH(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
}

void CartRetroReplay::WriteUltimaxROML(bit16 address, bit8 data)
{
	if (m_bEnableRAM)
	{
		m_pCartData[(address & 0x1fff) + m_iRamBankOffsetRomL] = data;
	}
}

void CartRetroReplay::WriteUltimaxROMH(bit16 address, bit8 data)
{

}

bool CartRetroReplay::IsCartIOActive(bit16 address, bool isWriting)
{
	return m_bIsCartIOActive && (((address & 0xFF00) == 0xDE00) || (!m_bREUcompatible && (address & 0xFF00) == 0xDF00));
}

void CartRetroReplay::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		bool bWasPressed = m_bIsFreezeButtonPressed;
		m_bIsFreezeButtonPressed = true;
		m_freezeButtonPressedClock = m_pCpu->Get6510CurrentClock();
		if (!bWasPressed && !m_bFreezePending && (reg2 & 0x04) == 0)
		{
			m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
			m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
			m_bFreezePending = true;
			m_bFreezeMode = false;
		}
	}
}

void CartRetroReplay::CheckForCartFreeze()
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

void CartRetroReplay::UpdateIO()
{
	if (m_bFreezePending)
	{
		BankRom();
	}
	else if (m_bFreezeMode)
	{
		m_bREUcompatible = (reg2 & 0x40) != 0;
		m_bAllowBank = (reg2 & 0x2) != 0;
		m_iSelectedBank = ((reg1 & 0x18) >> 3) | ((reg1 & 0x80) >> 5);// | ((reg1 & 0x20) >> 2);
		m_bEnableRAM = (reg1 & 0x20) != 0;
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = (bit16)((((int)m_iSelectedBank) & 3) << 13);//Maximum of 64K RAM available in non flash mode.
		if (m_bAllowBank)
		{
			m_iRamBankOffsetIO = m_iRamBankOffsetRomL;
		}
		//Ultimax
		GAME = 0;
		EXROM = 1;
		m_bIsCartIOActive = true;
		BankRom();
		m_ipROMH = m_ipROML;
	}
	else
	{
		m_bREUcompatible = (reg2 & 0x40) != 0;
		m_bAllowBank = (reg2 & 0x2) != 0;
		m_iSelectedBank = ((reg1 >> 3) & 3) | ((reg1 >> 5) & 4);
		m_bEnableRAM = (reg1 & 0x20) != 0;
		m_iRamBankOffsetIO = 0;
		m_iRamBankOffsetRomL = (bit16)((((int)m_iSelectedBank) & 3) << 13); 
		if (m_bAllowBank)
		{
			m_iRamBankOffsetIO = m_iRamBankOffsetRomL;
		}
		GAME = (~reg1 & 1);
		EXROM = (reg1 >> 1) & 1;
		m_bIsCartIOActive = (reg1 & 0x4) == 0;
		BankRom();
		m_ipROMH = m_ipROML;
	}
}


void CartRetroReplay::ExecuteCycle(ICLK sysclock)
{
	if (m_bIsFreezeButtonPressed)
	{
		if ((m_pCpu->Get6510CurrentClock() - m_freezeButtonPressedClock) > 500000)
		{
			m_bIsFreezeButtonPressed = false;
		}
	}
}