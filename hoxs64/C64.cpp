#include <windows.h>
#include <commctrl.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
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
#include "diskinterface.h"
#include "t64.h"
#include "c64file.h"
#include "C64.h"


//RAWTAPE *pTAPBase;


C64::C64()
{
	pIC64Event = 0;
	bPendingReset = false;
	bHardResetSystem = false;
	bSoftResetSystem = false;
}

C64::~C64()
{
}

HRESULT C64::Init(CConfig *cfg, CAppStatus *appStatus, IC64Event *pIC64Event, CDX9 *dx, TCHAR *szAppDirectory)
{
	ClearError();

	this->cfg = cfg;
	this->appStatus = appStatus;
	this->pIC64Event = pIC64Event;
	this->dx = dx;
	if (szAppDirectory==NULL)
		m_szAppDirectory[0] = 0;
	else
		_tcscpy_s(m_szAppDirectory, _countof(m_szAppDirectory), szAppDirectory);

	tape64.TapeEvent = (ITapeEvent *)this;

	if (ram.Init(m_szAppDirectory)!=S_OK) return SetError(ram);

	if (cpu.Init(pIC64Event, CPUID_MAIN, static_cast<IRegister *>(&cia1), static_cast<IRegister *>(&cia2), static_cast<IRegister *>(&vic), static_cast<IRegister *>(&sid), &ram, static_cast<ITape *>(&tape64))!=S_OK) return SetError(cpu);

	if (cia1.Init(cfg, appStatus, static_cast<IC64 *>(this), &cpu, &vic, &tape64, dx, static_cast<IAutoLoad *>(this))!=S_OK) return SetError(cia1);
	if (cia2.Init(cfg, appStatus, &cpu, &vic, &diskdrive)!=S_OK) return SetError(cia2);

	if (vic.Init(cfg, appStatus, dx, &ram, &cpu)!=S_OK) return SetError(vic);

	if (sid.Init(cfg, appStatus, dx, cfg->m_fps)!=S_OK) return SetError(sid);

	if (diskdrive.Init(cfg, appStatus, pIC64Event, szAppDirectory)!=S_OK) return SetError(diskdrive);

	return S_OK;
}

void C64::Reset(ICLK sysclock)
{
	diskdrive.WaitThreadReady();

	m_iClockOverflowCheckCounter = 0;
	tape64.CurrentClock = sysclock;
	vic.CurrentClock = sysclock;
	cia1.CurrentClock = sysclock;
	cia2.CurrentClock = sysclock;
	sid.CurrentClock = sysclock;
	cpu.CurrentClock = sysclock;
	diskdrive.CurrentPALClock = sysclock;
	diskdrive.CurrentClock = sysclock;
	diskdrive.cpu.CurrentClock = sysclock;
	diskdrive.via1.CurrentClock = sysclock;
	diskdrive.via2.CurrentClock = sysclock;

	tape64.nextTapeTickClock = sysclock;
	cia1.nextKeyboardScanClock = sysclock;
	cia1.ClockNextWakeUpClock = sysclock;
	cia2.ClockNextWakeUpClock = sysclock;
	vic.ClockNextWakeUpClock = sysclock;

	tape64.PressStop();
	ram.Reset();
	vic.Reset(sysclock);
	cia1.Reset(sysclock);
	cia2.Reset(sysclock);
	sid.Reset(sysclock);
	cpu.Reset(sysclock);
	diskdrive.Reset(sysclock);
	cpu.SetCassetteSense(1);

	pIC64Event->DiskMotorLed(diskdrive.m_bDiskMotorOn);
	pIC64Event->DiskDriveLed(diskdrive.m_bDriveLedOn);
	pIC64Event->DiskWriteLed(diskdrive.m_bDriveWriteWasOn);
	m_bLastPostedDriveWriteLed = diskdrive.m_bDriveWriteWasOn;
}

void C64::PreventClockOverflow()
{
	if (++m_iClockOverflowCheckCounter > 25L)
	{
		m_iClockOverflowCheckCounter=0;
		cpu.PreventClockOverflow();
		cia1.PreventClockOverflow();
		cia2.PreventClockOverflow();
		vic.PreventClockOverflow();

		diskdrive.PreventClockOverflow();
	}

	cpu.CheckPortFade(cpu.CurrentClock);
}

void C64::EnterDebugRun(bool bWithSound)
{
	diskdrive.WaitThreadReady();
	assert(vic.CurrentClock == cpu.CurrentClock);
	assert(vic.CurrentClock == cia1.CurrentClock);
	assert(vic.CurrentClock == cia2.CurrentClock);
	assert(vic.CurrentClock == diskdrive.CurrentPALClock || cfg->m_bD1541_Emulation_Enable==0);

	if (cfg->m_bD1541_Emulation_Enable==0)
		diskdrive.CurrentPALClock = vic.CurrentClock;

	if (bWithSound && appStatus->m_bSoundOK && appStatus->m_bFilterOK)
		sid.LockSoundBuffer();

	cpu.StartDebug();
	diskdrive.cpu.StartDebug();
}

void C64::FinishDebugRun()
{
	sid.UnLockSoundBuffer();
	cpu.StopDebug();
	diskdrive.cpu.StopDebug();

	PreventClockOverflow();
	CheckDriveLedNofication();
}


void C64::SynchroniseDevicesWithVIC()
{
ICLK sysclock;
	sysclock = vic.CurrentClock;
	diskdrive.WaitThreadReady();
	cpu.StartDebug();
	diskdrive.cpu.StartDebug();
	if (diskdrive.CurrentPALClock != sysclock)
	{
		sysclock = sysclock;
	}
	cpu.ExecuteCycle(sysclock);
	cia1.ExecuteCycle(sysclock);
	cia2.ExecuteCycle(sysclock);
	if (cfg->m_bSID_Emulation_Enable)
	{
		sid.ExecuteCycle(sysclock);
	}
	if (cfg->m_bD1541_Emulation_Enable)
	{
		diskdrive.ExecutePALClock(sysclock);
	}
	cpu.StopDebug();
	diskdrive.cpu.StopDebug();
}

void C64::ExecuteDiskClock()
{
ICLK sysclock;

	if (!cfg->m_bD1541_Emulation_Enable)
		return;

	EnterDebugRun(false);
	sysclock = diskdrive.CurrentPALClock;
	
	do
	{
		if (diskdrive.m_pendingclocks == 0)
			sysclock++;

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock-1);
			if (diskdrive.m_pendingclocks>1)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
				break;
			}
		}

		vic.ExecuteCycle(sysclock);
		cia1.ExecuteCycle(sysclock);
		cia2.ExecuteCycle(sysclock);
		cpu.ExecuteCycle(sysclock); 

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock);
			if (diskdrive.m_pendingclocks>0)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
			}
		}
		if (cfg->m_bSID_Emulation_Enable)
		{
			sid.ExecuteCycle(sysclock);
		}
	} while (false);
	FinishDebugRun();
}

void C64::ExecuteDiskInstruction()
{
ICLK sysclock;
bool bBreak;

	if (!cfg->m_bD1541_Emulation_Enable)
		return;

	EnterDebugRun(false);
	sysclock = diskdrive.CurrentPALClock;
	
	bBreak = false;
	while(!bBreak)
	{
		if (diskdrive.m_pendingclocks == 0)
			sysclock++;

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock-1);
			while (diskdrive.m_pendingclocks>1)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
				if (diskdrive.cpu.IsOpcodeFetch())// && diskdrive.cpu.PROCESSOR_INTERRUPT == 0)
				{
					bBreak = true;
					break;
				}
			}
			if (bBreak)
				break;
		}

		vic.ExecuteCycle(sysclock);
		cia1.ExecuteCycle(sysclock);
		cia2.ExecuteCycle(sysclock);
		cpu.ExecuteCycle(sysclock); 

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock);
			while (diskdrive.m_pendingclocks>0)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
				if (diskdrive.cpu.IsOpcodeFetch())// && diskdrive.cpu.PROCESSOR_INTERRUPT == 0)
				{
					bBreak = true;
					break;
				}
			}
		}
		if (cfg->m_bSID_Emulation_Enable)
		{
			sid.ExecuteCycle(sysclock);
		}
		if (bBreak || diskdrive.cpu.m_cpu_sequence==HLT_IMPLIED)
			break;

	}
	FinishDebugRun();
}

void C64::ExecuteC64Clock()
{
ICLK sysclock;

	EnterDebugRun(false);
	sysclock = vic.CurrentClock;
	
	do
	{
		sysclock++;

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock-1);
			while (diskdrive.m_pendingclocks>1)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
			}
		}

		vic.ExecuteCycle(sysclock);
		cia1.ExecuteCycle(sysclock);
		cia2.ExecuteCycle(sysclock);
		cpu.ExecuteCycle(sysclock); 

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock);
			while (diskdrive.m_pendingclocks>0)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
			}
		}
		if (cfg->m_bSID_Emulation_Enable)
		{
			sid.ExecuteCycle(sysclock);
		}
	} while (false);
	FinishDebugRun();
}

void C64::ExecuteC64Instruction()
{
ICLK sysclock;
bool bBreak;

	EnterDebugRun(false);
	sysclock = vic.CurrentClock;
	
	bBreak = false;
	while(!bBreak)
	{
		sysclock++;

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock-1);
			while (diskdrive.m_pendingclocks>1)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
			}
		}

		bool bWasC64CpuOpCodeFetch = cpu.IsOpcodeFetch();

		vic.ExecuteCycle(sysclock);
		cia1.ExecuteCycle(sysclock);
		cia2.ExecuteCycle(sysclock);
		cpu.ExecuteCycle(sysclock); 

		if (cpu.IsOpcodeFetch() && !bWasC64CpuOpCodeFetch)// && cpu.PROCESSOR_INTERRUPT == 0
		{
			bBreak = true;
		}

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock);
			while (diskdrive.m_pendingclocks>0)
			{
				diskdrive.ExecuteOnePendingDiskCpuClock();
			}
		}
		if (cfg->m_bSID_Emulation_Enable)
		{
			sid.ExecuteCycle(sysclock);
		}
		if (bBreak || cpu.m_cpu_sequence==HLT_IMPLIED)
			break;

	}
	FinishDebugRun();
}

void C64::ExecuteDebugFrame()
{
ICLK cycles,sysclock;
bool bBreakC64, bBreakDisk;

	if (bPendingReset)
	{
		ProcessReset();
	}
	if (vic.vic_raster_line==PAL_MAX_LINE && vic.vic_raster_cycle==63)
		cycles = (PAL_MAX_LINE+1)*63;
	else if (vic.vic_check_irq_in_cycle2)
		cycles = (PAL_MAX_LINE+1)*63 -1;
	else
		cycles = (PAL_MAX_LINE+1-vic.vic_raster_line)*63 - (vic.vic_raster_cycle);

	sysclock = vic.CurrentClock;
	
	EnterDebugRun(true);
	bBreakC64 = false;
	bBreakDisk = false;
	while(cycles > 0)
	{
		cycles--;
		sysclock++;

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock-1);
			while (diskdrive.m_pendingclocks > 1)
			{
				bool bWasDiskCpuOnInt = diskdrive.cpu.IsInterruptInstruction();
				diskdrive.ExecuteOnePendingDiskCpuClock();
				if (diskdrive.cpu.IsOpcodeFetch())
				{
					if (diskdrive.cpu.PROCESSOR_INTERRUPT == 0)
					{
						if (diskdrive.cpu.CheckExecute(diskdrive.cpu.mPC.word) >= 0)
						{
							bBreakDisk = true;
							break;
						}
					}
				}
				if (diskdrive.cpu.GetBreakOnInterruptTaken())
				{
					if (diskdrive.cpu.IsInterruptInstruction() && (diskdrive.cpu.IsOpcodeFetch() || !bWasDiskCpuOnInt))
						bBreakDisk = true;
				}
			}
 			if (bBreakDisk)
				break;
		}

		bool bWasC64CpuOpCodeFetch = cpu.IsOpcodeFetch();
		bool bWasC64CpuOnInt = cpu.IsInterruptInstruction();

		vic.ExecuteCycle(sysclock);
		cia1.ExecuteCycle(sysclock);
		cia2.ExecuteCycle(sysclock);
		cpu.ExecuteCycle(sysclock); 

		if (cpu.IsOpcodeFetch() && !bWasC64CpuOpCodeFetch)
		{			
			if (cpu.PROCESSOR_INTERRUPT == 0)
			{
				if (cpu.CheckExecute(cpu.mPC.word) >= 0)
				{
					bBreakC64 = true;
				}
			}
		}
		if (cpu.GetBreakOnInterruptTaken())
		{
			if (cpu.IsInterruptInstruction() && (cpu.IsOpcodeFetch() || !bWasC64CpuOnInt))
				bBreakC64 = true;
		}

		if (cfg->m_bD1541_Emulation_Enable)
		{
			diskdrive.AccumulatePendingDiskCpuClocksToPalClock(sysclock);
			while (diskdrive.m_pendingclocks>0)
			{
				bool bWasDiskCpuOnInt = diskdrive.cpu.IsInterruptInstruction();
				diskdrive.ExecuteOnePendingDiskCpuClock();
				if (diskdrive.cpu.IsOpcodeFetch())
				{
					if (diskdrive.cpu.PROCESSOR_INTERRUPT == 0)
					{
						if (diskdrive.cpu.CheckExecute(diskdrive.cpu.mPC.word) >= 0)
						{
							bBreakDisk = true;
							break;
						}
					}
				}
				if (diskdrive.cpu.GetBreakOnInterruptTaken())
				{
					if (diskdrive.cpu.IsInterruptInstruction() && (diskdrive.cpu.IsOpcodeFetch() || !bWasDiskCpuOnInt))
						bBreakDisk = true;
				}
			}
		}
		if (cfg->m_bSID_Emulation_Enable)
		{
			sid.ExecuteCycle(sysclock);
		}
 		if (bBreakC64 || bBreakDisk)
			break;

	}
	FinishDebugRun();
	if (pIC64Event)
	{
		if (bBreakC64)
		{
			pIC64Event->BreakExecuteCpu64();
		}
		if (bBreakDisk)
		{
			pIC64Event->BreakExecuteCpuDisk();
		}
	}
}

void C64::ExecuteFrame()
{
ICLK cycles,sysclock;

	if (bPendingReset)
	{
		ProcessReset();
	}
	if (vic.vic_raster_line==PAL_MAX_LINE && vic.vic_raster_cycle==63)
		cycles = (PAL_MAX_LINE+1)*63;
	else if (vic.vic_check_irq_in_cycle2)
		cycles = (PAL_MAX_LINE+1)*63 -1;
	else
		cycles = (PAL_MAX_LINE+1-vic.vic_raster_line)*63 - (vic.vic_raster_cycle);

	sysclock = vic.CurrentClock + cycles;
	
	if (appStatus->m_bSoundOK && appStatus->m_bFilterOK)
		sid.LockSoundBuffer();

	BOOL bIsDiskEnabled = cfg->m_bD1541_Emulation_Enable;
	BOOL bIsDiskThreadEnabled = cfg->m_bD1541_Thread_Enable;
	if (bIsDiskEnabled && !bIsDiskThreadEnabled)
	{
		diskdrive.ExecuteAllPendingDiskCpuClocks();
	}

	cpu.ExecuteCycle(sysclock);
	vic.ExecuteCycle(sysclock);
	cia1.ExecuteCycle(sysclock);
	cia2.ExecuteCycle(sysclock);
	if (bIsDiskEnabled)
	{
		if (bIsDiskThreadEnabled && !appStatus->m_bSerialTooBusyForSeparateThread)
		{
			diskdrive.ThreadSignalCommandExecuteClock(sysclock);
		}
		else
		{
			diskdrive.ExecutePALClock(sysclock);
			appStatus->m_bSerialTooBusyForSeparateThread = false;
		}
	}
	if (cfg->m_bSID_Emulation_Enable)
	{
		sid.ExecuteCycle(sysclock);
	}
	sid.UnLockSoundBuffer();
	CheckDriveLedNofication();
}

void C64::CheckDriveLedNofication()
{
	if (pIC64Event)
	{
		if ((diskdrive.m_d64_write_enable == 0) && (diskdrive.CurrentClock - diskdrive.m_driveWriteChangeClock) > 200000L) 
		{
			diskdrive.m_bDriveWriteWasOn = false;
		}
		if (m_bLastPostedDriveWriteLed != diskdrive.m_bDriveWriteWasOn)
		{
			m_bLastPostedDriveWriteLed = diskdrive.m_bDriveWriteWasOn;
			pIC64Event->DiskWriteLed(diskdrive.m_bDriveWriteWasOn);
		}

		pIC64Event->DiskMotorLed(diskdrive.m_bDiskMotorOn);
		pIC64Event->DiskDriveLed(diskdrive.m_bDriveLedOn);
	}
}

void C64::ResetKeyboard()
{
	cia1.ResetKeyboard();
}


void C64::SetBasicProgramEndAddress(bit16 last_byte)
{
	//last_byte=start+code_size-1;
	ram.mMemory[0x2d]=(bit8)(last_byte+1);
	ram.mMemory[0x2e]=(bit8)((last_byte+1)>>8);
	ram.mMemory[0x2f]=(bit8)(last_byte+1);
	ram.mMemory[0x30]=(bit8)((last_byte+1)>>8);
	ram.mMemory[0x31]=(bit8)(last_byte+1);
	ram.mMemory[0x32]=(bit8)((last_byte+1)>>8);
	ram.mMemory[0xae]=(bit8)(last_byte+1);
	ram.mMemory[0xaf]=(bit8)((last_byte+1)>>8);
}


#define ClocksFromFrame(x) (PALCLOCKSPERSECOND*x)
void C64::WriteSysCallToScreen(bit16 startaddress)
{
char szAddress[7];
const char szSysCall[] = {19,25,19,32,0};
	CopyMemory(&ram.miMemory[SCREENWRITELOCATION], szSysCall, strlen(szSysCall));
	szAddress[0] = 0;
	sprintf_s(szAddress, _countof(szAddress), "%ld", startaddress);
	unsigned int i;
	for (i=0 ; i < strlen(szAddress) ; i++)
		ram.miMemory[SCREENWRITELOCATION + 4 + i] = szAddress[i];
}

void C64::AutoLoadHandler(ICLK sysclock)
{
const int resettime=120;
const char szRun[] = {18,21,14,0};
const char szLoadDisk[] = {12,15,1,4,34,42,34,44,56,44,49,0};
const char szSysCall[] = {19,25,19,32,0};
const int LOADPREFIXLENGTH = 5;
const int LOADPOSTFIXINDEX = 6;
const int LOADPOSTFIXLENGTH = 5;
HRESULT hr;
ICLK period;
bit16 loadSize;
int directoryIndex;
char szAddress[7];

	if (autoLoadCommand.sequence == AUTOSEQ_RESET)
	{
		appStatus->m_bAutoload = TRUE;
		autoLoadCommand.sequence = C64::AUTOSEQ_LOAD;
	}

	period = sysclock / (PALCLOCKSPERSECOND / 50);
	if (period < resettime)
	{
		return;	
	}
	switch (autoLoadCommand.type)
	{
	case C64::AUTOLOAD_TAP_FILE:
		if (period < (resettime + 3))
		{
		}
		else if (period < (resettime + 6))
		{
			cia1.SetKeyMatrixDown(1, 7);//lshift
		}
		else if (period  < (resettime + 9))
		{
			cia1.SetKeyMatrixDown(1, 7);//lshift
			cia1.SetKeyMatrixDown(7, 7);//stop
		}
		else if (period < (resettime + 12))
		{
			break;
		}
		else 
		{
			TapePressPlay();
			autoLoadCommand.CleanUp();
			appStatus->m_bAutoload = FALSE;
		}
		break;
	case C64::AUTOLOAD_PRG_FILE:		
	case C64::AUTOLOAD_T64_FILE:	
		if (autoLoadCommand.sequence == C64::AUTOSEQ_LOAD)
		{
			autoLoadCommand.sequence = C64::AUTOSEQ_RUN;
			if (autoLoadCommand.type == AUTOLOAD_PRG_FILE)
			{
				hr = LoadImageFile(&autoLoadCommand.filename[0], &autoLoadCommand.startaddress, &autoLoadCommand.imageSize);
			}
			else
			{
				directoryIndex  = (autoLoadCommand.directoryIndex < 0) ? 0 : autoLoadCommand.directoryIndex;
				hr = LoadT64ImageFile(&autoLoadCommand.filename[0], directoryIndex, &autoLoadCommand.startaddress, &autoLoadCommand.imageSize);
			}
			if (SUCCEEDED(hr))
			{
				if (autoLoadCommand.startaddress <= BASICSTARTADDRESS)
				{
					SetBasicProgramEndAddress(autoLoadCommand.startaddress + autoLoadCommand.imageSize - 1);
				}
				else
				{
					CopyMemory(&ram.miMemory[SCREENWRITELOCATION], szSysCall, strlen(szSysCall));
					szAddress[0] = 0;
					sprintf_s(szAddress, _countof(szAddress), "%ld", autoLoadCommand.startaddress);
					unsigned int i;
					for (i=0 ; i < strlen(szAddress) ; i++)
						ram.miMemory[SCREENWRITELOCATION + 4 + i] = szAddress[i];
				}
			}
			else
			{
				//FIXME Desirable not to show messages in the C64 class
				DisplayError(NULL, errorText);
				autoLoadCommand.CleanUp();
				appStatus->m_bAutoload = FALSE;
				break;
			}
		}
		if (autoLoadCommand.startaddress <= BASICSTARTADDRESS)
		{
			if (period < resettime + 3)	
			{
				cia1.SetKeyMatrixDown(2, 1);//r
			}
			else if (period < resettime + 6)
			{
			}
			else if (period < resettime + 9)	
			{
				cia1.SetKeyMatrixDown(3, 6);//u
			}
			else if (period < resettime + 12)
			{
			}
			else if (period < resettime + 15)	
			{
				cia1.SetKeyMatrixDown(4, 7);//n
			}
			else if (period < resettime + 18)
			{
			}
			else if (period < resettime + 21)
			{
				cia1.SetKeyMatrixDown(0, 1);//return
			}
			else
			{
				autoLoadCommand.CleanUp();
				appStatus->m_bAutoload = FALSE;
			}
		}
		else
		{
			if (period < resettime + 3)	
			{
				cia1.SetKeyMatrixDown(1, 5);//s
			}
			else if (period < resettime + 6)
			{
			}
			else if (period < resettime + 9)
			{
				cia1.SetKeyMatrixDown(0, 1);//return
			}
			else
			{
				autoLoadCommand.CleanUp();
				appStatus->m_bAutoload = FALSE;
			}
		}
		break;
	case C64::AUTOLOAD_SID_FILE:		
		if (autoLoadCommand.sequence == C64::AUTOSEQ_LOAD)
		{
			autoLoadCommand.sequence = C64::AUTOSEQ_RUN;
			if (autoLoadCommand.pSidFile)
			{
				SIDLoader &sl = *autoLoadCommand.pSidFile;
				if (autoLoadCommand.directoryIndex < 0)
					hr = sl.LoadSID(ram.miMemory, &autoLoadCommand.filename[0], true, 0);
				else
					hr = sl.LoadSID(ram.miMemory, &autoLoadCommand.filename[0], false, autoLoadCommand.directoryIndex + 1);
				if (SUCCEEDED(hr))
				{
					if (autoLoadCommand.pSidFile->RSID_BASIC)
					{
						SetBasicProgramEndAddress((WORD)(sl.startSID+sl.lenSID-1));
						autoLoadCommand.type = C64::AUTOLOAD_PRG_FILE;
						autoLoadCommand.sequence = C64::AUTOSEQ_RUN;
						autoLoadCommand.startaddress = BASICSTARTADDRESS;
						break;
					}
					else
					{
						cpu.mPC.word = sl.driverLoadAddress;
						cpu.m_cpu_sequence = C_FETCH_OPCODE;
					}
				}
				else
					//FIXME Desirable not to show messages in the C64 class
					sl.DisplayError(NULL, sl.errorText);
			}
			autoLoadCommand.CleanUp();
			appStatus->m_bAutoload = FALSE;
		}
		break;
	case C64::AUTOLOAD_DISK_FILE:
		if (autoLoadCommand.sequence == C64::AUTOSEQ_LOAD)
		{
			autoLoadCommand.sequence = C64::AUTOSEQ_RUN;
			if (autoLoadCommand.bQuickLoad)
			{
				autoLoadCommand.type = C64::AUTOLOAD_PRG_FILE;
				if (autoLoadCommand.pImageData!=0 && autoLoadCommand.imageSize > 2)
				{
					autoLoadCommand.startaddress = autoLoadCommand.pImageData[0] + autoLoadCommand.pImageData[1] * 0x100;

					loadSize = autoLoadCommand.imageSize - 2;
					if ((bit32)autoLoadCommand.startaddress + (bit32)loadSize -1 > 0xffffL)
					{
						loadSize = 0xffff - autoLoadCommand.startaddress + 1;
					}
					if (loadSize > 0)
					{
						memcpy_s(&ram.miMemory[autoLoadCommand.startaddress], 0x10000 - autoLoadCommand.startaddress, &autoLoadCommand.pImageData[2], loadSize);					
						if (autoLoadCommand.startaddress <= BASICSTARTADDRESS)
							SetBasicProgramEndAddress(autoLoadCommand.startaddress + loadSize - 1);
						else
						{
							CopyMemory(&ram.miMemory[SCREENWRITELOCATION], szSysCall, strlen(szSysCall));
							szAddress[0] = 0;
							sprintf_s(szAddress, _countof(szAddress), "%ld", autoLoadCommand.startaddress);
							unsigned int i;
							for (i=0 ; i < strlen(szAddress) ; i++)
								ram.miMemory[SCREENWRITELOCATION + 4 + i] = szAddress[i];
						}
					}
				}
				else
				{
					autoLoadCommand.startaddress = BASICSTARTADDRESS;
				}
			}
			else
			{
				int filenamelen = C64File::GetC64FileNameLength(autoLoadCommand.c64filename, sizeof(autoLoadCommand.c64filename));
				if (filenamelen == 0)
				{
					memcpy_s(&ram.miMemory[SCREENWRITELOCATION], 40, szLoadDisk, strlen(szLoadDisk));
				}
				else
				{
					bit8 screencodebuffer[sizeof(autoLoadCommand.c64filename)];
					memset(screencodebuffer, 0, sizeof(screencodebuffer));
					for (int i=0 ; i < sizeof(screencodebuffer) ; i++)
					{
						screencodebuffer[i]  = C64File::ConvertPetAsciiToScreenCode(autoLoadCommand.c64filename[i]);
					}
					//ConvertPetAsciiToScreenCode
					memcpy_s(&ram.miMemory[SCREENWRITELOCATION], 40, szLoadDisk, LOADPREFIXLENGTH);
					memcpy_s(&ram.miMemory[SCREENWRITELOCATION + LOADPREFIXLENGTH], C64Directory::D64FILENAMELENGTH, screencodebuffer, filenamelen); 
					memcpy_s(&ram.miMemory[SCREENWRITELOCATION + LOADPREFIXLENGTH + filenamelen], 40, &szLoadDisk[LOADPOSTFIXINDEX], LOADPOSTFIXLENGTH);
				}
			}
		}
		if (period < resettime + 3)
		{
		}
		else if (period < resettime + 6)
		{
			cia1.SetKeyMatrixDown(1, 7);//lshift
		}
		else if (period < resettime + 9)
		{
			cia1.SetKeyMatrixDown(1, 7);//lshift
			cia1.SetKeyMatrixDown(7, 7);//stop
		}
		else
		{
			autoLoadCommand.CleanUp();
			appStatus->m_bAutoload = FALSE;
		}
		break;
	default:
		autoLoadCommand.CleanUp();
		appStatus->m_bAutoload = FALSE;

	}
}

void C64::TapePressPlay()
{
	cpu.SetCassetteSense(0);
	tape64.PressPlay();
	cia1.SetWakeUpClock();
}

void C64::TapePressStop()
{
	cia1.flag_change = !cia1.f_flag_in;
	cia1.f_flag_in=1;
	cpu.SetCassetteSense(1);
	tape64.PressStop();
	cia1.SetWakeUpClock();
}

void C64::TapePressEject()
{
	cia1.flag_change = !cia1.f_flag_in;
	cia1.f_flag_in=1;
	cpu.SetCassetteSense(1);
	tape64.Eject();
	cia1.SetWakeUpClock();
}

void C64::TapeRewind()
{
	TapePressStop();
	tape64.Rewind();
	cia1.SetWakeUpClock();
}

//ITapeEvent
void C64::Pulse(ICLK sysclock)
{
	cia1.Pulse(sysclock);
}
//ITapeEvent
void C64::EndOfTape(ICLK sysclock)
{
	TapePressStop();
}

HRESULT C64::AutoLoad(TCHAR *filename, int directoryIndex, bool bIndexOnlyPrgFiles, const bit8 c64FileName[C64DISKFILENAMELENGTH], bool bQuickLoad, bool bAlignD64Tracks)
{
HRESULT hr = S_OK;
TCHAR *p;
C64File c64file;

	ClearError();
	autoLoadCommand.CleanUp();

	hr = c64file.Init();
	if (FAILED(hr))
		return SetError(hr, TEXT("Could initialise autoload."));

	autoLoadCommand.type = C64::AUTOLOAD_NONE;
	autoLoadCommand.sequence = C64::AUTOSEQ_RESET;
	autoLoadCommand.directoryIndex = directoryIndex;
	autoLoadCommand.bQuickLoad = bQuickLoad;
	autoLoadCommand.bAlignD64Tracks =  bAlignD64Tracks;
	autoLoadCommand.bIndexOnlyPrgFiles =bIndexOnlyPrgFiles;
	if (c64FileName == NULL)
	{
		memset(autoLoadCommand.c64filename, 0xa0, sizeof(autoLoadCommand.c64filename));
		if (directoryIndex >= 0)
		{
			int numItems;
			hr = c64file.LoadDirectory(filename, C64Directory::D64MAXDIRECTORYITEMCOUNT, numItems, bIndexOnlyPrgFiles, NULL);
			if (SUCCEEDED(hr))
				c64file.GetDirectoryItemName(directoryIndex, autoLoadCommand.c64filename, sizeof(autoLoadCommand.c64filename));
		}
	}
	else
		memcpy_s(autoLoadCommand.c64filename, sizeof(autoLoadCommand.c64filename), c64FileName, C64DISKFILENAMELENGTH);
	appStatus->m_bAutoload = FALSE;

	hr = _tcscpy_s(&autoLoadCommand.filename[0], _countof(autoLoadCommand.filename), filename);
	if (FAILED(hr))
		return SetError(hr, TEXT("%s too long."), filename);

	if (lstrlen(filename) >= 4)
	{
		p= &(filename[lstrlen(filename) - 4]);
		if (lstrcmpi(p, TEXT(".tap"))==0)
		{
			hr = LoadTAPFile(filename);
			if (SUCCEEDED(hr))
			{
				autoLoadCommand.type = C64::AUTOLOAD_TAP_FILE;
				appStatus->m_bAutoload = TRUE;
			}
		}
		else if (lstrcmpi(p, TEXT(".prg"))==0 || lstrcmpi(p, TEXT(".p00"))==0)
		{
			autoLoadCommand.type = C64::AUTOLOAD_PRG_FILE;
			appStatus->m_bAutoload = TRUE;
		}		
		else if (lstrcmpi(p, TEXT(".t64"))==0)
		{
			autoLoadCommand.type = C64::AUTOLOAD_T64_FILE;
			appStatus->m_bAutoload = TRUE;
		}		
		else if (lstrcmpi(p, TEXT(".d64"))==0 || lstrcmpi(p, TEXT(".g64"))==0 || lstrcmpi(p, TEXT(".fdi"))==0)
		{
			if (!cfg->m_bD1541_Emulation_Enable)
			{
				diskdrive.CurrentPALClock = cpu.CurrentClock;
				cfg->m_bD1541_Emulation_Enable = TRUE;
			}
			
			pIC64Event->SetBusy(true);
			hr = InsertDiskImageFile(filename, bAlignD64Tracks);
			if (SUCCEEDED(hr))
			{
				if (bQuickLoad)
				{
					if (autoLoadCommand.pImageData)
					{
						GlobalFree(autoLoadCommand.pImageData);
						autoLoadCommand.pImageData = 0;
					}
					if (autoLoadCommand.directoryIndex<0)
						hr = c64file.LoadFileImage(filename, NULL, &autoLoadCommand.pImageData, &autoLoadCommand.imageSize);
					else
						hr = c64file.LoadFileImage(filename, autoLoadCommand.c64filename, &autoLoadCommand.pImageData, &autoLoadCommand.imageSize);
					if (FAILED(hr))
						SetError(hr, TEXT("Unable to quick load."));
				}
			}
			pIC64Event->SetBusy(false);
			if (SUCCEEDED(hr))
			{
				autoLoadCommand.type = C64::AUTOLOAD_DISK_FILE;
				appStatus->m_bAutoload = TRUE;
				Reset(0);
				return hr;
			}
			else
			{
				autoLoadCommand.CleanUp();
				appStatus->m_bAutoload = FALSE;
				return hr;
			}
		}
		else if (lstrcmpi(p, TEXT(".sid"))==0)
		{
			autoLoadCommand.pSidFile = new SIDLoader();
			if (autoLoadCommand.pSidFile == 0)
				return SetError(E_FAIL, TEXT("Out of memory."));
			hr = autoLoadCommand.pSidFile->LoadSIDFile(filename);
			if (FAILED(hr))
				return SetError(*(autoLoadCommand.pSidFile));
			autoLoadCommand.type = C64::AUTOLOAD_SID_FILE;
			appStatus->m_bAutoload = TRUE;
		}
		else
		{
			return SetError(E_FAIL, TEXT("Unknown file type."));
		}
	}
	else
	{
		return SetError(E_FAIL,TEXT("Unknown file type."));
	}
	Reset(0);
	return S_OK;
}

HRESULT C64::LoadImageFile(TCHAR *filename, bit16* pStartAddress, bit16* pSize)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,file_size;
bit32 start,code_size,s;
TCHAR *p;

	ClearError();
	hfile = CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		return SetError(E_FAIL, TEXT("Could not open %s."), filename);
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("Could not open %s."), filename);
	}
	if (file_size > sizeof(ram.tmp_data))
	{
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("%s is too large to be a C64 image."), filename);
	}

	r=ReadFile(hfile,&ram.tmp_data[0],file_size,&bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0)
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
	if (bytes_read!=file_size)
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
	

	if (lstrlen(filename)>=4)
	{
		p= &(filename[lstrlen(filename)-4]);
		if (lstrcmpi(p,TEXT(".p00"))==0)
		{
			start=* ((bit16 *)(&ram.tmp_data[0x1a]));
			code_size=(bit16)file_size - 0x1c;
			s=0x1c;
		}
		else if (lstrcmpi(p,TEXT(".prg"))==0)
		{
			start=* ((bit16 *)(&ram.tmp_data[0x00]));
			code_size=(bit16)file_size - 0x2;
			s=0x2;
		}		
		else
		{
			start=* ((bit16 *)(&ram.tmp_data[0x00]));
			code_size=(bit16)file_size - 0x2;
			s=0x2;
		}
	}
	else
	{
		start=* ((bit16 *)(&ram.tmp_data[0x00]));
		code_size=(bit16)file_size - 0x2;
		s=0x2;
	}

	start &= 0xffff;
	code_size &= 0xffff;
	if ((code_size+start-1)>0xffff)
		code_size = 0x10000 - start;
	memcpy(&ram.mMemory[start],&ram.tmp_data[s], code_size);

	*pStartAddress = (bit16)start;
	*pSize = (bit16)code_size;
	return S_OK;

}

HRESULT C64::LoadT64ImageFile(TCHAR *filename, int t64Index, bit16* pStartAddress, bit16* pSize)
{
HANDLE hfile=0;
bit16 start,code_size;
T64 t64;
HRESULT hr;
	
	ClearError();
	if (t64Index<0)
		return SetError(E_FAIL,TEXT("Could not open the selected directory item for %s."), filename);
	hr = t64.LoadT64Directory(filename, MAXT64LIST);
	if (FAILED(hr))
		return SetError(t64);
	if (t64.t64Header.maxEntries <= t64Index)
		return SetError(E_FAIL,TEXT("Could not open the selected directory item for %s."), filename);

	if (t64.t64Item[t64Index].mySize > 0xffff || t64.t64Item[t64Index].mySize <= 2)
		return SetError(E_FAIL,TEXT("Could not open the selected directory item for %s."), filename);

	start = t64.t64Item[t64Index].startAddress;
	code_size = (bit16)t64.t64Item[t64Index].mySize;
	
	hr = t64.LoadT64File(filename, t64.t64Item[t64Index].offset, code_size);
	if (FAILED(hr))
		return SetError(t64);

	if (start==0)
		start=* ((bit16 *)(&t64.data[0]));

	if (((bit32)code_size + (bit32)start - 1) > 0xffffL)
		code_size = 0xffff - start + 1;
	memcpy(&ram.mMemory[start],&t64.data[0], code_size);

	*pStartAddress = (bit16)start;
	*pSize = (bit16)code_size;
	return S_OK;
}

HRESULT C64::LoadTAPFile(TCHAR *filename)
{
HRESULT hr;

	ClearError();
	hr = tape64.InsertTAPFile(filename);
	if (FAILED(hr))
	{
		return SetError(hr,tape64.errorText);
	}
	return S_OK;
}


void C64::RemoveDisk()
{
	diskdrive.RemoveDisk();
}

HRESULT C64::InsertDiskImageFile(TCHAR *filename, bool bAlignD64Tracks)
{
TCHAR *p;

	ClearError();
	if (lstrlen(filename) < 4)
		return E_FAIL;

	p = &(filename[lstrlen(filename)-4]);
	
	if (lstrcmpi(p, TEXT(".d64"))==0)
		return LoadD64FromFile(filename, bAlignD64Tracks);
	else if (lstrcmpi(p, TEXT(".g64"))==0)
		return LoadG64FromFile(filename);
	else if (lstrcmpi(p, TEXT(".fdi"))==0)
		return LoadFDIFromFile(filename);
	else
		return E_FAIL;
	return S_OK;
}


HRESULT C64::LoadD64FromFile(TCHAR *filename, bool bAlignD64Tracks)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	hr = dsk.LoadD64FromFile(filename, true, bAlignD64Tracks);

	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	diskdrive.WaitThreadReady();
	diskdrive.LoadImageBits(&dsk);

	diskdrive.SetDiskLoaded();
	return hr;
}

HRESULT C64::LoadG64FromFile(TCHAR *filename)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	hr = dsk.LoadG64FromFile(filename, true);

	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	diskdrive.WaitThreadReady();
	diskdrive.LoadImageBits(&dsk);

	diskdrive.SetDiskLoaded();
	return hr;
}

HRESULT C64::LoadFDIFromFile(TCHAR *filename)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	hr = dsk.LoadFDIFromFile(filename);

	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}
	if (hr == APPERR_BAD_CRC)
	{
		SetError(dsk);
	}

	diskdrive.WaitThreadReady();
	diskdrive.LoadImageBits(&dsk);

	diskdrive.SetDiskLoaded();

	return hr;
}

HRESULT C64::InsertNewDiskImage(TCHAR *diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}

	dsk.InsertNewDiskImage(diskname, id1, id2, bAlignD64Tracks, numberOfTracks);

	diskdrive.WaitThreadReady();
	diskdrive.LoadImageBits(&dsk);
	diskdrive.SetDiskLoaded();
	return S_OK;
}

HRESULT C64::SaveD64ToFile(TCHAR *filename, int numberOfTracks)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}
	diskdrive.WaitThreadReady();
	diskdrive.SaveImageBits(&dsk);	
	hr = dsk.SaveD64ToFile(filename, numberOfTracks);
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}
	return hr;	
}

HRESULT C64::SaveFDIToFile(TCHAR *filename)
{
GCRDISK dsk;
HRESULT hr;

	ClearError();
	hr = dsk.Init();
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}
	diskdrive.WaitThreadReady();
	diskdrive.SaveImageBits(&dsk);	
	hr = dsk.SaveFDIToFile(filename);
	if (FAILED(hr))
	{
		SetError(dsk);
		return hr;
	}
	return S_OK;	
}

void C64::SoftReset(bool bCancelAutoload)
{
	if (bCancelAutoload)
		appStatus->m_bAutoload = 0;
	cpu.Reset(cpu.CurrentClock);
}

void C64::HardReset(bool bCancelAutoload)
{
	if (bCancelAutoload)
		appStatus->m_bAutoload = 0;
	Reset(0);
}

void C64::PostSoftReset(bool bCancelAutoload)
{
	if (bCancelAutoload)
		appStatus->m_bAutoload = 0;
	bPendingReset = true;
	bHardResetSystem = false;
	bSoftResetSystem = true;
}

void C64::PostHardReset(bool bCancelAutoload)
{
	if (bCancelAutoload)
		appStatus->m_bAutoload = 0;
	bPendingReset = true;
	bHardResetSystem = true;
	bSoftResetSystem = false;
}

void C64::ProcessReset()
{
	bPendingReset = false;
	if (bHardResetSystem)
	{
		bHardResetSystem = false;
		bSoftResetSystem = false;
		HardReset(false);
	}
	else if (bSoftResetSystem)
	{	
		bSoftResetSystem = false;
		SoftReset(false);
	}
}
