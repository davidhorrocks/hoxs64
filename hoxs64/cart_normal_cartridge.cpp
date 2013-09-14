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

CartNormalCartridge::CartNormalCartridge(CrtHeader &crtHeader, CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, plstBank, pCartData, pZeroBankData, pCpu, pC64RamMemory)
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
