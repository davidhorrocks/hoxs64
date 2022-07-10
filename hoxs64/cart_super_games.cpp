#include "cart.h"

CartSuperGames::CartSuperGames(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory),
	regDisabled(m_statebool[0])
{
}

bit8 CartSuperGames::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDF00)
	{
		return reg1;
	}
	return 0;
}

void CartSuperGames::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDF00)
	{			
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartSuperGames::UpdateIO()
{
	if (!regDisabled)
	{
		m_iSelectedBank = reg1 & 3;
		if ((reg1 & 4) == 0)
		{
			GAME = 0;
			EXROM = 0;
		}
		else
		{
			GAME = 1;
			EXROM = 1;
		}
		BankRom();
		if (reg1 & 8)
		{
			regDisabled = 1;
		}
	}
}
