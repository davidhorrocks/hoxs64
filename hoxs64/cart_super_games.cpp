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
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
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
	m_iSelectedBank = reg1 & 3;
	GAME = (reg1 & 4) != 0;
	EXROM = (GAME!=0) && (reg1 & 8) != 0;
	BankRom();
}
