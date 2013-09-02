#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "hexconv.h"
#include "cia6526.h"
#include "vic6569.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "d64.h"
#include "d1541.h"
#include "diskinterface.h"
#include "tap.h"
#include "cia6526.h"
#include "cia2.h"


#define NO_CHANGE_TO_IDLE 4
#define MINIMUM_STAY_IDLE 3
#define MINIMUM_GO_IDLE_TIME 5

CIA2::CIA2()
{
	cfg = 0L;
	appStatus = 0L;
	cpu = 0L;
	vic = 0L;
	disk = 0L;
	c64_serialbus = 0;
}

HRESULT CIA2::Init(CConfig *cfg, CAppStatus *appStatus, CPU6510 *cpu, VIC6569 *vic, DiskInterface *disk)
{
	ClearError();
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->cpu = cpu;
	this->vic = vic;
	this->disk = disk;
	return S_OK;
}

void CIA2::ExecuteDevices(ICLK sysclock)
{
}
//pragma optimize( "ag", off )
//pragma optimize( "ag", on )

bit8 CIA2::ReadPortA()
{
bit8 t;
	if (cfg->m_bD1541_Emulation_Enable)
	{
		//The c64 sees the disk affect the serial bus one cycle behind
		if (cfg->m_bD1541_Thread_Enable && !appStatus->m_bSerialTooBusyForSeparateThread)
		{
			appStatus->m_bSerialTooBusyForSeparateThread = true;
			disk->ThreadSignalCommandExecuteClock(CurrentClock-1);
			disk->WaitThreadReady();
		}
		else
		{
			disk->ExecutePALClock(CurrentClock-1);
		}

		t=((c64_serialbus & disk->m_d64_serialbus) & 0xC0) | 4    | 0x3f;
		return (pra_out & ddra & t) | (~ddra & t);
	}
	else
		return pra_out & ddra | ~ddra;
}

bit8 CIA2::ReadPortB()
{
bit8 t;
	t = prb_out & ddrb | ~ddrb;
	t = (t & ~bPB67TimerMode) | (bPB67TimerOut & bPB67TimerMode);
	return t;
}

void CIA2::WritePortA()
{
bit8 t;

	t = PortAOutput_Strong0s();
	c64_serialbus = ((~t <<2) & 0x20) //ATN
		| ((~t <<2) & t & 0x40) // CLK
		| ((~t <<2) & t & 0x80); //DATA
	if (cfg->m_bD1541_Emulation_Enable)
	{
		//The disk sees the c64 affect the serial bus one cycle behind
		if (cfg->m_bD1541_Thread_Enable && !appStatus->m_bSerialTooBusyForSeparateThread)
		{
			appStatus->m_bSerialTooBusyForSeparateThread = true;
			disk->ThreadSignalCommandExecuteClock(CurrentClock-1);
			disk->WaitThreadReady();
		}
		else
		{
			disk->ExecutePALClock(CurrentClock-1);
		}

		disk->C64SerialBusChange(CurrentClock, c64_serialbus);
	}
	
	bit8 newbank = (t & 3) ^ 3;
	bit8 oldbank = m_commandedVicBankIndex;

	
	if ((newbank == 1 && oldbank == 2) || (newbank == 2 && oldbank == 1))
	{
		vic->SetMMU(3);
		vic->m_bVicBankChanging = true;
		vic->vicBankChangeByte = newbank;
		m_commandedVicBankIndex = newbank;
	}
	else if (newbank != oldbank)
	{
		vic->SetMMU(newbank);
		m_commandedVicBankIndex = newbank;
	}
}

void CIA2::WritePortB()
{
}

void CIA2::InitReset(ICLK sysclock)
{
	CIA::InitReset(sysclock);
	m_commandedVicBankIndex = 3;
}

void CIA2::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	CIA::Reset(sysclock);
	bit8 t = PortAOutput_Strong0s();
	m_commandedVicBankIndex = (t & 3) ^ 3;
}

void CIA2::SetSystemInterrupt()
{
	cpu->Set_CIA_NMI(CurrentClock);
}

void CIA2::ClearSystemInterrupt()
{
	cpu->Clear_CIA_NMI();
}

void CIA2::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CIA::SetCurrentClock(sysclock);
	DevicesClock+=v;
}

void CIA2::GetState(SsCia2 &state)
{
	ZeroMemory(&state, sizeof(state));
	CIA::GetState(state.cia);
	
	state.c64_serialbus = c64_serialbus;
	state.m_commandedVicBankIndex = m_commandedVicBankIndex;
}

void CIA2::SetState(const SsCia2 &state)
{
	CIA::SetState(state.cia);
	c64_serialbus = state.c64_serialbus;
	m_commandedVicBankIndex = state.m_commandedVicBankIndex;
}
