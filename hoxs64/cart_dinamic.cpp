#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
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

CartDinamic::CartDinamic(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

bit8 CartDinamic::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address >= 0xDE00 || address < 0xDF00)
	{
		m_iSelectedBank = (bit8)(address & 0x3F);
		ConfigureMemoryMap();
	}
	return 0;
}

void CartDinamic::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

void CartDinamic::UpdateIO()
{
	if (m_bIsCartIOActive)
	{
		GAME = m_crtHeader.GAME;
		EXROM = m_crtHeader.EXROM;
	}
	else
	{
		GAME = 1;
		EXROM = 1;
	}
	BankRom();
}
