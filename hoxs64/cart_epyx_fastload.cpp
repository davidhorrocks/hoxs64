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

#define CAPACITORCHARGETHRESHOLD (512)
#define CAPACITORCHARGELIMIT (1024)
#define CAPACITORDISCHARGERATE (50)

CartEpyxFastLoad::CartEpyxFastLoad(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pC64RamMemory)
{
}

void CartEpyxFastLoad::ExecuteCycle(ICLK sysclock)
{
	if (m_bCapacitorCharged)
	{
		CurrentClock = sysclock;	
		return;
	}
	
	int clocks = (int)(sysclock - CurrentClock);
	if (clocks > 0)
	{
		CurrentClock = sysclock;
		int old = m_iCapacitorCharge;
		int next = m_iCapacitorCharge + clocks;
		if (next > CAPACITORCHARGELIMIT)
		{
			next = CAPACITORCHARGELIMIT;
			m_bCapacitorCharged = true;
		}
		m_iCapacitorCharge = next;
		if ( old <= CAPACITORCHARGETHRESHOLD && next > CAPACITORCHARGETHRESHOLD)
		{
			ConfigureMemoryMap();
		}
	}
	return;
}

void CartEpyxFastLoad::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	m_bCapacitorCharged = false;
	m_iCapacitorCharge = 0;
	ConfigureMemoryMap();
}

void CartEpyxFastLoad::DischargeCapacitor()
{
	m_bCapacitorCharged = false;
	int old = m_iCapacitorCharge;
	int next = old - CAPACITORDISCHARGERATE;
	if (next < 0)
		next = 0;
	m_iCapacitorCharge = next;
		
	if (next <= CAPACITORCHARGETHRESHOLD && old > CAPACITORCHARGETHRESHOLD)
	{
		ConfigureMemoryMap();
	}
}

bit8 CartEpyxFastLoad::ReadRegister(bit16 address, ICLK sysclock)
{
	if (!m_bCapacitorCharged)
		this->ExecuteCycle(sysclock);
	if (address >= 0xDE00 && address < 0xDF00)
	{
		if (this->m_bEffects)
		{
			DischargeCapacitor();
		}
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		return CartCommon::ReadROML(address - 0x4000);
	}
	return 0;
}

bit8 CartEpyxFastLoad::ReadROML(bit16 address)
{
	if (!m_bCapacitorCharged)
		this->ExecuteCycle(this->m_pCpu->Get6510CurrentClock());
	if (EXROM == 0)
	{
		if (this->m_bEffects)
		{
			DischargeCapacitor();
		}
		return CartCommon::ReadROML(address);
	}
	else
	{
		return this->m_pC64RamMemory[address];
	}
}

bit8 CartEpyxFastLoad::ReadUltimaxROML(bit16 address)
{
	return ReadROML(address);
}

bit8 CartEpyxFastLoad::Get_EXROM()
{
	if (!m_bCapacitorCharged)
		this->ExecuteCycle(this->m_pCpu->Get6510CurrentClock());
	return EXROM;
}

void CartEpyxFastLoad::UpdateIO()
{
	if (m_iCapacitorCharge > CAPACITORCHARGETHRESHOLD)
	{
		GAME = 1;
		EXROM = 1;
	}
	else
	{
		GAME = 1;
		EXROM = 0;
	}
	BankRom();
}
