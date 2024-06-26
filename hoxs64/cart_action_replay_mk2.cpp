#include "cart.h"

#define ACTIONREPLAYMK2DISABLEROMCOUNT (20)
#define ACTIONREPLAYMK2DISABLEROMTRIGGER (200)

#define ACTIONREPLAYMK2ENABLEROMCOUNT (20)
#define ACTIONREPLAYMK2ENABLEROMTRIGGER (12)

CartActionReplayMk2::CartActionReplayMk2(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartActionReplay(crtHeader, pCpu, pVic, pC64RamMemory)
{
	m_bActionReplayMk2Rom = true;
	m_iActionReplayMk2EnableRomCounter = 0;
	m_iActionReplayMk2DisableRomCounter = 0;
	m_clockLastDE00Write = 0;
	m_clockLastDF40Read = 0;
}

void CartActionReplayMk2::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	m_bActionReplayMk2Rom = true;
	m_iActionReplayMk2EnableRomCounter=0;
	m_iActionReplayMk2DisableRomCounter=0;
	m_clockLastDE00Write = sysclock;
	m_clockLastDF40Read = sysclock;
	ConfigureMemoryMap();
}

HRESULT CartActionReplayMk2::LoadState(IStream* pfs, const CrtHeader& crtHeader, int version)
{
	HRESULT hr = CartCommon::LoadState(pfs, crtHeader, version);
	if (SUCCEEDED(hr))
	{
		m_bActionReplayMk2Rom = m_stateuint[0] != 0;
		m_iActionReplayMk2EnableRomCounter = m_stateuint[1];
		m_iActionReplayMk2DisableRomCounter = m_stateuint[2];
		m_clockLastDE00Write = m_stateuint[3];
		m_clockLastDF40Read = m_stateuint[4];
	}

	return hr;
}

HRESULT CartActionReplayMk2::SaveState(IStream* pfs)
{
	m_stateuint[0] = m_bActionReplayMk2Rom ? 1 : 0;
	m_stateuint[1] = m_iActionReplayMk2EnableRomCounter;
	m_stateuint[2] = m_iActionReplayMk2DisableRomCounter;
	m_stateuint[3] = m_clockLastDE00Write;
	m_stateuint[4] = m_clockLastDF40Read;
	return CartCommon::SaveState(pfs);
}


bit8 CartActionReplayMk2::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (m_bEffects)
	{
		if (((ICLKS)(sysclock - m_clockLastDF40Read)) > ACTIONREPLAYMK2DISABLEROMCOUNT)
			m_iActionReplayMk2DisableRomCounter = 0;
	}
	if (address >= 0xDE00 && address < 0xDF00)
	{
		return 0;
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		if (m_bEffects)
		{
			if (address == 0xDF40)
			{
				if (((ICLKS)(sysclock - m_clockLastDF40Read)) <= ACTIONREPLAYMK2DISABLEROMCOUNT)
				{
					m_iActionReplayMk2DisableRomCounter++;
					if (m_iActionReplayMk2DisableRomCounter > ACTIONREPLAYMK2DISABLEROMTRIGGER)
					{
						m_iActionReplayMk2DisableRomCounter = 0;
						m_bActionReplayMk2Rom = false;
						ConfigureMemoryMap();
					}
				}
				m_clockLastDF40Read = sysclock;
			}
		}
		addr = address - 0xDF00 + 0x9F00;
		return this->m_ipROML[addr & 0x1fff];
	}
	return 0;
}

void CartActionReplayMk2::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
bit8 t;
	if (address >= 0xDE00 && address < 0xE000)
	{
		m_iActionReplayMk2DisableRomCounter = 0;
		if (((ICLKS)(sysclock - m_clockLastDE00Write)) <= ACTIONREPLAYMK2ENABLEROMCOUNT)
		{
			m_iActionReplayMk2EnableRomCounter++;
			if (m_iActionReplayMk2EnableRomCounter > ACTIONREPLAYMK2ENABLEROMTRIGGER)
			{
				m_iActionReplayMk2EnableRomCounter = 0;
				m_bActionReplayMk2Rom = true;
				t = (address >> 8) & 0xff;
				//CHECKME always select bank1?
				if (t == 0xDE)
					m_iSelectedBank = 1;
				else if (t == 0xDF)
					m_iSelectedBank = 1;
				ConfigureMemoryMap();
			}
		}
		else
		{
			m_iActionReplayMk2EnableRomCounter = 0;
		}
		m_clockLastDE00Write = sysclock;
	}
}

void CartActionReplayMk2::CartFreeze()
{
	if (m_bIsCartAttached)
	{
		m_pCpu->Set_CRT_IRQ(m_pCpu->Get6510CurrentClock());
		m_pCpu->Set_CRT_NMI(m_pCpu->Get6510CurrentClock());
		m_bFreezePending = true;
		m_bFreezeMode = false;
	}
}

void CartActionReplayMk2::CheckForCartFreeze()
{
	if (m_bFreezePending)
	{
		m_iSelectedBank = 0;
		m_bFreezePending = false;
		m_bFreezeMode = false;
		m_pCpu->Clear_CRT_IRQ();
		m_pCpu->Clear_CRT_NMI();			
		ConfigureMemoryMap();
	}
}

void CartActionReplayMk2::UpdateIO()
{
	m_bEnableRAM = false;
	m_iRamBankOffsetIO = 0;
	m_iRamBankOffsetRomL = 0;
	if (m_bFreezePending)
	{
		BankRom();
		m_ipROMH = m_ipROML;
		m_ipROMH = m_ipROML;
	}
	else
	{
		if (m_bActionReplayMk2Rom)
		{
			if (m_iSelectedBank == 0)
			{
				GAME = 0;
				EXROM = 1;
			}
			else
			{
				GAME = 1;
				EXROM = 0;
			}
		}
		else
		{
			GAME = 1;
			EXROM = 1;
		}
		m_bIsCartIOActive = true;
		BankRom();
		m_ipROMH = m_ipROML;
	}			
}
