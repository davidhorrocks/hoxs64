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

CartSuperGames::CartSuperGames(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory),
	 regDisabled(m_state0)
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
