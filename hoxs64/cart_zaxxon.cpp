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
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "cart.h"

CartZaxxon::CartZaxxon(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{
}

bit8 CartZaxxon::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void CartZaxxon::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}


bit8 CartZaxxon::ReadROML(bit16 address)
{
	if (m_bEffects)
	{
		if (address < 0x9000 && m_iSelectedBank != 0)
		{
			m_iSelectedBank = 0;
			ConfigureMemoryMap();
		}
		else if (address >= 0x9000 && m_iSelectedBank != 1)
		{
			m_iSelectedBank = 1;
			ConfigureMemoryMap();
		}
	}
	return CartCommon::ReadROML(address);
}

bit8 CartZaxxon::ReadUltimaxROML(bit16 address)
{
	if (m_bEffects)
	{
		if (address < 0x9000 && m_iSelectedBank != 0)
		{
			m_iSelectedBank = 0;
			ConfigureMemoryMap();
		}
		else if (address >= 0x9000 && m_iSelectedBank != 1)
		{
			m_iSelectedBank = 1;
			ConfigureMemoryMap();
		}
	}
	return CartCommon::ReadUltimaxROML(address);
}


void CartZaxxon::UpdateIO()
{
	m_bIsCartIOActive = true;
	GAME = 0;
	EXROM = 0;
	BankRom();
}
