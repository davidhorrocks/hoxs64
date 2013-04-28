#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <list>
#include <algorithm>

#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "errormsg.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "cart.h"

CartNormalCartridge::CartNormalCartridge(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(pCart, pCpu, pC64RamMemory)
{
}

bit8 CartNormalCartridge::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void CartNormalCartridge::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

//bit8 CartNormalCartridge::ReadROML(bit16 address)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//bit8 CartNormalCartridge::ReadROMH(bit16 address)
//{
//	assert(address >= 0xA000 && address < 0xC000);
//}
//
//bit8 CartNormalCartridge::ReadUltimaxROML(bit16 address)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//bit8 CartNormalCartridge::ReadUltimaxROMH(bit16 address)
//{
//	assert(address >= 0xE000);
//}
//
//void CartNormalCartridge::WriteROML(bit16 address, bit8 data)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//void CartNormalCartridge::WriteROMH(bit16 address, bit8 data)
//{
//	assert(address >= 0xA000 && address < 0xC000);
//}
//
//void CartNormalCartridge::WriteUltimaxROML(bit16 address, bit8 data)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//void CartNormalCartridge::WriteUltimaxROMH(bit16 address, bit8 data)
//{
//	assert(address >= 0xE000);
//}
//
//bit8 CartNormalCartridge::MonReadROML(bit16 address)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//bit8 CartNormalCartridge::MonReadROMH(bit16 address)
//{
//	assert(address >= 0xA000 && address < 0xC000);
//}
//
//bit8 CartNormalCartridge::MonReadUltimaxROML(bit16 address)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//bit8 CartNormalCartridge::MonReadUltimaxROMH(bit16 address)
//{
//	assert(address >= 0xE000);
//}
//
//void CartNormalCartridge::MonWriteROML(bit16 address, bit8 data)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//void CartNormalCartridge::MonWriteROMH(bit16 address, bit8 data)
//{
//	assert(address >= 0xA000 && address < 0xC000);
//}
//
//void CartNormalCartridge::MonWriteUltimaxROML(bit16 address, bit8 data)
//{
//	assert(address >= 0x8000 && address < 0xA000);
//}
//
//void CartNormalCartridge::MonWriteUltimaxROMH(bit16 address, bit8 data)
//{
//	assert(address >= 0xE000);
//}

//void CartNormalCartridge::CartFreeze()
//{
//}
//
//void CartNormalCartridge::CheckForCartFreeze()
//{
//}

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
