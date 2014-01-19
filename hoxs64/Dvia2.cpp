#include <windows.h>
#include <commctrl.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
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
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "t64.h"
#include "tap.h"
#include "diskinterface.h"

VIA2::VIA2()
{
	appStatus=0;
	cpu=0;
	disk=0;
	oldDiskControl=0;
}
HRESULT VIA2::Init(int ID, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk)
{
	this->ID = ID;
	this->appStatus = appStatus;
	this->cpu = cpu;
	this->disk = disk;
	return S_OK;
}

void VIA2::InitReset(ICLK sysclock)
{
	VIA::InitReset(sysclock);
	oldDiskControl = 0x3;
}

void VIA2::Reset(ICLK sysclock)
{
	VIA::Reset(sysclock);
	oldDiskControl = PortBOutput();
}

void VIA2::ExecuteDevices(ICLK sysclock)
{
	disk->SpinDisk(sysclock);
}

void VIA2::SetCA2Output(bit8 value)
{
	disk->SpinDisk(CurrentClock);
	disk->m_d64_soe_enable = value;
}

void VIA2::SetCB2Output(bit8 value)
{
bit8 oldWE;

	disk->SpinDisk(CurrentClock);

	oldWE = disk->m_d64_write_enable;
	disk->m_d64_write_enable = !value;
	if (oldWE != disk->m_d64_write_enable)
	{
		disk->m_driveWriteChangeClock = disk->CurrentClock;

		if (value)
		{
			//Read mode
			disk->m_d64_sync = ((disk->m_d64_write_enable==0) && ((disk->m_shifterReader_UD2 & 0x3ff) == 0x3ff)) || (disk->m_d64_forcesync!=0);
		}
		else
		{
			//Write mode
			disk->m_d64_sync = 0;
			disk->m_bDriveWriteWasOn = true;
		}
	}
}

void VIA2::OnTransitionCA1Low()
{
	int a = 0;
	if (!cpu->SOTrigger)
	{
		cpu->SOTrigger = true;
		cpu->SOTriggerClock = CurrentClock;
	}
}

bit8 VIA2::ReadPinsPortA()
{
	disk->SpinDisk(CurrentClock);
	if (disk->m_d64_write_enable)
		return 0;
	else
	{
		return disk->GetBusDataByte();
	}
}

bit8 VIA2::ReadPinsPortB()
{
bit8 sync;
	disk->SpinDisk(CurrentClock);
	if (disk->m_d64_sync)
		sync = 0x0;
	else
		sync = 0x80;
	return (disk->GetProtectSensorState() << 4) | sync | 0x6f;
}

void VIA2::SetPinsPortA(bit8 newPin)
{
	disk->SpinDisk(CurrentClock);
	disk->m_d64_diskwritebyte = newPin;
}

//Does the motor need to be on for head stepping?
void VIA2::SetPinsPortB(bit8 newPin)
{
bit8 t;
bool bMotor;
bool bOldMotor;
bit8 oldPin = oldDiskControl;

	if (oldPin == newPin)
		return;

	disk->SpinDisk(CurrentClock);


	bMotor = ((newPin & 4)>>2) != 0;
	bOldMotor = ((oldPin & 4)>>2) != 0;

	if (((oldPin ^ newPin) & 0x6c) !=0)
	{
		disk->m_bDiskMotorOn = bMotor;
		disk->m_clockDivider1_UE7_Reload = (newPin >> 5) & 3;
		disk->m_bDriveLedOn = ((newPin & 8) != 0);
	}

	if (((oldPin ^ newPin) & 0x80) !=0)
	{
		if ((newPin & 0x80) == 0)
		{
			disk->m_d64_forcesync = 1;
			disk->m_frameCounter_UC3 = 0;
		}
		else
		{
			disk->m_d64_forcesync = 0;
		}
	}

	if (!bMotor && bOldMotor)
	{
		//Allow motor to run for a short time after it is turned off.
		disk->m_motorOffClock = 135000;
	}
	else if (bMotor)
		disk->m_motorOffClock = 0;

	t = disk->m_lastHeadStepPosition & 3;	
	t = ((newPin - t) & 3);
	if ((t != 0) && bMotor)
	{
		//Cafe Odd wants heads to step when the motor is turned on.
		if (t == 1)
		{
			disk->StepHeadIn();
		}
		else if (t == 3)
		{
			disk->StepHeadOut();
		}
		else if (t == 2)
		{
			disk->StepHeadAuto();
			//Fix for Primitive 7 Sins" by Albion Crew
		}
	}

	oldDiskControl = newPin;
}

void VIA2::SetSystemInterrupt()
{
	cpu->Set_VIA2_IRQ(CurrentClock);
}

void VIA2::ClearSystemInterrupt()
{
	cpu->Clear_VIA2_IRQ();
}

//DATA=7 CLOCK=6 ATN=5

void VIA2::GetState(SsVia2 &state)
{
	ZeroMemory(&state, sizeof(state));
	VIA::GetState(state.via);
	state.oldDiskControl = oldDiskControl;
}

void VIA2::SetState(const SsVia2 &state)
{
	VIA::SetState(state.via);
	oldDiskControl = state.oldDiskControl;
}
