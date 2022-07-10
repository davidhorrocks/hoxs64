#include "cart.h"

CartSimonsBasic::CartSimonsBasic(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory),
	m_bSimonsBasic16K(m_statebool[0])
{
}

void CartSimonsBasic::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	m_bSimonsBasic16K = false;
	ConfigureMemoryMap();
}

bit8 CartSimonsBasic::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDE00)
	{
		if (m_bEffects)
		{
			m_bSimonsBasic16K = false;
			ConfigureMemoryMap();
		}
		return 0;
	}
	return 0;
}

void CartSimonsBasic::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{
		m_bSimonsBasic16K = true;
		ConfigureMemoryMap();
	}
}

void CartSimonsBasic::UpdateIO()
{
	m_iSelectedBank = 0;
	if (m_bSimonsBasic16K)
	{
		GAME = 0;
		EXROM = 0;
	}
	else
	{
		GAME = 1;
		EXROM = 0;
	}
	m_bIsCartIOActive = true;
	BankRom();
}
