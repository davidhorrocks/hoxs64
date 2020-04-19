#include "cart.h"

CartOcean1::CartOcean1(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

bit8 CartOcean1::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDE00)
	{
		return reg1;
	}
	return 0;
}

void CartOcean1::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
}

void CartOcean1::UpdateIO()
{
	m_iSelectedBank = reg1 & 0x3f;
	GAME = m_crtHeader.GAME;
	EXROM = m_crtHeader.EXROM;
	m_bIsCartIOActive = true;
	BankRom();
}
