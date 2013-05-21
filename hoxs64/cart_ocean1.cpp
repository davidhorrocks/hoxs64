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

CartOcean1::CartOcean1(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(pCart, pCpu, pC64RamMemory)
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