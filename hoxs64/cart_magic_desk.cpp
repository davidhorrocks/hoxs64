#include "cart.h"

CartMagicDesk::CartMagicDesk(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartMagicDesk::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDE00)
	{
		return reg1;
	}
	return 0;
}

void CartMagicDesk::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartMagicDesk::UpdateIO()
{
	m_iSelectedBank = reg1 & 0x3f;
	if (reg1 & 0x80)
	{
		GAME = 1;
		EXROM = 1;
	}
	else
	{
		GAME = 1;
		EXROM = 0;
	}

	m_bIsCartIOActive = true;
	BankRom();
}
