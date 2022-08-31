#include "dx_version.h"
#include "utils.h"
#include "register.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "register.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"
#include "filter.h"
#include "sid.h"
#include "sidfile.h"
#include "p64.h"
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "t64.h"
#include "tap.h"
#include "diskinterface.h"

HRESULT VIA1::Init(int ID, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk)
{
	this->ID = ID;
	this->appStatus = appStatus;
	this->cpu = cpu;
	this->disk = disk;
	port_a_floating = 0xFF;
	port_a_floating_clock = 0;
	return S_OK;
}

void VIA1::InitReset(ICLK sysclock, bool poweronreset)
{
	VIA::InitReset(sysclock, poweronreset);
}

void VIA1::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	VIA::Reset(sysclock, poweronreset);
	port_a_floating = 0xFF;
	port_a_floating_clock = sysclock;
}

void VIA1::ExecuteDevices(ICLK sysclock)
{
}

bit8 VIA1::ReadPinsPortA()
{
	bit8 strong0;
	switch (appStatus->m_TrackZeroSensorStyle)
	{
		case HCFG::TZSSPullHigh:
			strong0 = 0xFF;
			break;
		case HCFG::TZSSPullLow:
			strong0 = 0xFE;
			break;
		case HCFG::TZSSPositiveHigh:
			if (disk->m_currentTrackNumber == 0)
			{
				strong0 = 0xFF;
			}
			else
			{
				strong0 = 0xFE;
			}

			break;
		case HCFG::TZSSPositiveLow:
			if (disk->m_currentTrackNumber == 0)
			{
				strong0 = 0xFF;
			}
			else
			{
				strong0 = 0xFE;
			}

			break;
		default:
			strong0 = 0xFF;
			break;
	}

	if (((ICLKS)(CurrentClock - port_a_floating_clock)) > 0x1000)
	{
		port_a_floating = 0xFF;
	}

	return port_a_floating & strong0;
}

void VIA1::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;
	if (port_a_floating_clock > CLOCKSYNCBAND_FAR)
	{
		port_a_floating_clock = ClockBehindNear;
	}
}

bit8 VIA1::ReadPinsPortB()
{
bit8 t;
	t=(disk->m_d64_serialbus & disk->m_c64_serialbus_diskview);
	t=(t >> 7) | ((t & 0x40) >> 4) | ((disk->m_c64_serialbus_diskview & 0x20) <<2);
	t|=0x1a;
	return ((t^0x85) | disk->m_d64_dipswitch);	
}

void VIA1::SetCA2Output(bool value)
{
}

void VIA1::SetCB1Output(bool value)
{
}

void VIA1::SetCB2Output(bool value)
{
}

void VIA1::SetPinsPortA(bit8 newPin)
{
	port_a_floating = newPin;
	port_a_floating_clock = this->CurrentClock;
}

void VIA1::SetPinsPortB(bit8 newPin)
{
	disk->D64_Attention_Change();
}

void VIA1::SetSystemInterrupt()
{
	cpu->Set_VIA1_IRQ(CurrentClock);
}

void VIA1::ClearSystemInterrupt()
{
	cpu->Clear_VIA1_IRQ();
}

//DATA=7 CLOCK=6 ATN=5

void VIA1::GetState(SsVia1 &state)
{
	ZeroMemory(&state, sizeof(state));
	VIA::GetState(state.via);
}

void VIA1::SetState(const SsVia1 &state)
{
	VIA::SetState(state.via);
}

