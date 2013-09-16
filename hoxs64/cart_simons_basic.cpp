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

CartSimonsBasic::CartSimonsBasic(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

void CartSimonsBasic::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	m_bSimonsBasic16K = false;
	ConfigureMemoryMap();
}

bit8 CartSimonsBasic::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address == 0xDE00)
	{
		if (m_bEffects)
		{
			m_bSimonsBasic16K = false;
			ConfigureMemoryMap();
		}
		return 0;
	}
	return 0;
}

void CartSimonsBasic::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{
		m_bSimonsBasic16K = true;
		ConfigureMemoryMap();
	}
}

void CartSimonsBasic::UpdateIO()
{
	m_iSelectedBank = 0;
	if (m_bSimonsBasic16K)
	{
		GAME = 0;
		EXROM = 0;
	}
	else
	{
		GAME = m_crtHeader.GAME;
		EXROM = m_crtHeader.EXROM;
	}
	m_bIsCartIOActive = true;
	BankRom();
}
