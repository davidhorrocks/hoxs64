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

CartMagicDesk::CartMagicDesk(IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(pCpu, pC64RamMemory)
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
		GAME = m_crtHeader.GAME;
		EXROM = m_crtHeader.EXROM;
	}
	m_bIsCartIOActive = true;
	BankRom();
}
