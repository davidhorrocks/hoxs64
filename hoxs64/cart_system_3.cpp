#include "cart.h"

CartSystem3::CartSystem3(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartSystem3::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address >= 0xDE00 && address < 0xDF00)
	{
		m_iSelectedBank = (bit8)(address & 0x3F);
		ConfigureMemoryMap();
	}

	return 0;
}

void CartSystem3::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address >= 0xDE00 && address < 0xDF00)
	{
		m_iSelectedBank = (bit8)(address & 0x3F);
		ConfigureMemoryMap();
	}
}

void CartSystem3::UpdateIO()
{
	if (m_bIsCartIOActive)
	{
		GAME = 1;
		EXROM = 0;
	}
	else
	{
		GAME = 1;
		EXROM = 1;
	}
	BankRom();
}
