#include "cart.h"

CartFunPlay::CartFunPlay(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartFunPlay::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDE00)
	{
		return reg1;
	}
	return 0;
}

void CartFunPlay::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{			
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartFunPlay::UpdateIO()
{
	//m_iSelectedBank = (bit8)(((reg1 & 0x38) >> 3) | ((reg1 & 0x1) << 3));
	m_iSelectedBank = reg1;
	if (reg1 == 0x86)
	{
		m_bIsCartIOActive = false;
		GAME = 1;
		EXROM = 1;
	}
	else
	{
		GAME = 1;
		EXROM = 0;
	}
	BankRom();
}
