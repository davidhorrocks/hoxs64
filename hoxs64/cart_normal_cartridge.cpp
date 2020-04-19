#include "cart.h"

CartNormalCartridge::CartNormalCartridge(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

bit8 CartNormalCartridge::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void CartNormalCartridge::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

void CartNormalCartridge::UpdateIO()
{
	m_bREUcompatible = false;
	m_bAllowBank = false;
	m_iSelectedBank = 0;
	m_iRamBankOffsetIO = 0;
	m_iRamBankOffsetRomL = 0;
	m_bEnableRAM = false;
	m_bIsCartIOActive = false;
	GAME = m_crtHeader.GAME;
	EXROM = m_crtHeader.EXROM;
	BankRom();
}
