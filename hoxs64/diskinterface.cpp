#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
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

#undef DEBUG_DISKPULSES

#define DISK16CELLSPERCLOCK (16)
#define DISKHEADFILTERWIDTH (40)
#define DISKHEADSTEPWAITTIME (5000UL)
#define DISKMOTORSLOWTIME (140000)

DiskInterface::DiskInterface()
{
	firstboot = true;
	P64ImageCreate(&this->m_P64Image);
	m_currentHeadIndex=0;
	m_currentTrackNumber = 0;
	m_previousTrackNumber = 0;
	m_lastHeadStepDir = 0;
	m_lastHeadStepPosition = 0;
	m_shifterWriter_UD3 = 0;
	m_shifterReader_UD2 = 0;	
	m_busByteReadyPreviousData = 0;
	m_busByteReadySignal = 1;
	m_busDataUpdateClock = 0;
	m_frameCounter_UC3 = 0;
	m_debugFrameCounter = 0;
	m_clockDivider1_UE7 = 0;
	m_clockDivider2_UF4 = 0;
	m_bDiskMotorOn = false;
	m_bDriveWriteWasOn = false;
	m_bDriveLedOn = false;
	m_driveWriteChangeClock = 0;
	m_writeStream = 0;
	m_diskLoaded = 0;
	m_totalclocks_UE7 = 0;
	m_lastPulseTime = 0;
	m_lastWeakPulseTime = 0;
	m_bPulseState = false;
	m_bLastPulseState = false;
	m_bPendingPulse = false;
	m_counterStartPulseFilter = DISKHEADFILTERWIDTH;
	m_nextP64PulsePosition = -1;

	m_d64_soe_enable = 1;
	m_d64_write_enable = 0;
	m_d64_serialbus = 0;
	m_d64_dipswitch = 0;
	m_d64_sync = 0;
	m_d64_forcesync = 0;
	m_d64_diskwritebyte = 0;
	m_d64_diskchange_counter = 0;
	m_diskChangeCounter = 0;

	m_c64_serialbus_diskview = 0;
	m_c64_serialbus_diskview_next = 0;
	m_changing_c64_serialbus_diskview_diskclock = 0;

	m_pD1541_ram = NULL;
	m_pD1541_rom = NULL;
	m_pIndexedD1541_rom = NULL;
	CurrentPALClock = 0;
	CurrentClock = 0;

	m_d64TrackCount = 0;

	m_motorOffClock = 0;
	m_headStepClock = 0;
	m_headStepClockUp = 0;

	mThreadId = 0;
	mhThread = 0;
	mbDiskThreadCommandQuit = false;
	mbDiskThreadPause = false;
	mbDiskThreadHasQuit = false;
	mevtDiskReady = 0;
	mevtDiskCommand = 0;
}

DiskInterface::~DiskInterface()
{
	Cleanup();
}

void DiskInterface::InitReset(ICLK sysclock, bool poweronreset)
{
	ThreadSignalPause();
	WaitThreadReady();

	m_diskd64clk_xf = -Disk64clk_dy2 / 2;

	m_currentHeadIndex = 0;
	m_shifterWriter_UD3 = 0;
	m_shifterReader_UD2 = 0;
	m_busByteReadyPreviousData = 0;
	m_busByteReadySignal = 1;
	m_busDataUpdateClock = sysclock;
	m_frameCounter_UC3 = 0;
	m_debugFrameCounter = 0;
	m_clockDivider1_UE7 = 0;
	m_clockDivider2_UF4 = 0;
	m_bDiskMotorOn = false;
	m_bDriveWriteWasOn = false;
	m_bDriveLedOn = false;
	m_driveWriteChangeClock = sysclock;
	m_writeStream = 0;
	m_totalclocks_UE7 = 0;
	m_lastPulseTime = 0;
	m_lastWeakPulseTime = 0;
	m_bPulseState = false;
	m_bLastPulseState = false;
	m_bPendingPulse = false;
	m_counterStartPulseFilter = DISKHEADFILTERWIDTH;
	m_nextP64PulsePosition = -1;

	m_d64_soe_enable = 1;
	m_d64_write_enable = 0;
	m_d64_serialbus = 0;
	m_d64_dipswitch = 0;
	m_d64_sync = 0;
	m_d64_forcesync = 0;
	m_d64_diskwritebyte = 0;
	m_d64_diskchange_counter = 0;
	m_diskChangeCounter = 0;
	m_changing_c64_serialbus_diskview_diskclock = sysclock;

	m_motorOffClock = 0;
	m_headStepClock = 0;
	m_headStepClockUp =0;

	if (poweronreset || firstboot)
	{
		SetRamPattern();
	}
	if (firstboot)
	{
		m_currentTrackNumber = 0x2;
		m_lastHeadStepDir = 0;
		m_lastHeadStepPosition = 2;
		firstboot = false;
	}
	else
	{
		if (m_currentTrackNumber >= HOST_MAX_TRACKS)
		{
			m_currentTrackNumber = HOST_MAX_TRACKS - 1;
		}
	}
	m_previousTrackNumber = m_currentTrackNumber;
}

void DiskInterface::Reset(ICLK sysclock, bool poweronreset)
{
	ThreadSignalPause();
	WaitThreadReady();
	CurrentClock = sysclock;
	CurrentPALClock = sysclock;
	m_pendingclocks = 0;
	//CurrentPALClock must be set before calling ThreadSignalCommandResetClock
	ThreadSignalCommandResetClock();
	WaitThreadReady();
	InitReset(sysclock, poweronreset);
	via1.Reset(sysclock, poweronreset);
	via2.Reset(sysclock, poweronreset);
	cpu.Reset(sysclock, poweronreset);
	PrepareP64Head(m_currentTrackNumber);
}

void DiskInterface::SetRamPattern()
{
int i;
	for (i=0 ; i<=0x07ff ; i++)
	{
		if ((i & 64)==0)
			m_pD1541_ram[i] = 0;
		else
			m_pD1541_ram[i] = 255;
	}
}

HRESULT DiskInterface::Init(CAppStatus *appStatus, IC64Event *pIC64Event, IBreakpointManager *pIBreakpointManager,TCHAR *szAppDirectory)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read;
TCHAR szRomPath[MAX_PATH+1];
HRESULT hr;

	ClearError();
	Cleanup();
	P64ImageCreate(&this->m_P64Image);
	hr = InitDiskThread();
	if (FAILED(hr))
	{
		Cleanup();
		return SetError(hr, TEXT("InitDiskThread failed"));
	}

	if (szAppDirectory==NULL)
		m_szAppDirectory[0] = 0;
	else
		_tcscpy_s(m_szAppDirectory, _countof(m_szAppDirectory), szAppDirectory);

	this->appStatus = appStatus;
	this->pIC64Event = pIC64Event;

	m_pD1541_ram=(bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,0x0800);
	if (m_pD1541_ram==NULL)
	{
		Cleanup();
		return SetError(E_OUTOFMEMORY, TEXT("Memory allocation failed"));
	}
	m_pD1541_rom=(bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,0x4000);
	if (m_pD1541_ram==NULL)
	{
		Cleanup();
		return SetError(E_OUTOFMEMORY, TEXT("Memory allocation failed"));
	}

	hfile=INVALID_HANDLE_VALUE;
	szRomPath[0]=0;
	if (_tmakepath_s(szRomPath, _countof(szRomPath), 0, m_szAppDirectory, TEXT("C1541.rom"), 0) == 0)
		hfile=CreateFile(szRomPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
		hfile=CreateFile(TEXT("C1541.rom"),GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		Cleanup();
		return SetError(E_FAIL, TEXT("Could not open C1541.rom"));
	}
	r=ReadFile(hfile,m_pD1541_rom,0x4000,&bytes_read,NULL);
	CloseHandle(hfile);
	if (r==0)
	{
		Cleanup();
		return SetError(E_FAIL, TEXT("Could not read from C1541.rom"));
	}
	if (bytes_read!=0x4000)
	{
		Cleanup();
		return SetError(E_FAIL, TEXT("Could not read 0x4000 bytes from C1541.rom"));
	}

	m_pIndexedD1541_rom = m_pD1541_rom-0xC000;	
	via1.Init(1, appStatus, &cpu, this);
	via2.Init(2, appStatus, &cpu, this);
	cpu.Init(pIC64Event, CPUID_DISK, &via1, &via2, this, m_pD1541_ram, m_pIndexedD1541_rom, pIBreakpointManager);
	m_d64_protectOff=1;//1=no protect;0=protected;
	return S_OK;
}

typedef BOOL (WINAPI *LPInitializeCriticalSectionAndSpinCount)(LPCRITICAL_SECTION riticalSection, DWORD dwSpinCount );

HRESULT DiskInterface::InitDiskThread()
{

LPInitializeCriticalSectionAndSpinCount pInitializeCriticalSectionAndSpinCount = NULL;
	CloseDiskThread();

	mbDiskThreadCommandQuit = false;
	mbDiskThreadHasQuit = false;
	mbDiskThreadPause = false;

	mevtDiskReady = CreateEvent(0, TRUE, FALSE, NULL);
	if (mevtDiskReady == 0)
	{
		return E_FAIL;
	}
	mevtDiskCommand = CreateEvent(0, FALSE, FALSE, NULL);
	if (mevtDiskCommand == 0)
	{
		return E_FAIL;
	}
	mevtDiskQuit = CreateEvent(0, TRUE, FALSE, NULL);
	if (mevtDiskQuit == 0)
	{
		return E_FAIL;
	}	
	mevtDiskPause = CreateEvent(0, FALSE, FALSE, NULL);
	if (mevtDiskPause == 0)
	{
		return E_FAIL;
	}
	this->m_waitCommand[0] = this->mevtDiskQuit;
	this->m_waitCommand[1] = this->mevtDiskPause;
	this->m_waitCommand[2] = this->mevtDiskCommand;
	this->m_waitReady[0] = this->mevtDiskQuit;
	this->m_waitReady[1] = this->mevtDiskReady;
	if (G::IsWinVerSupportInitializeCriticalSectionAndSpinCount())
	{
		HMODULE hKernel32 = GetModuleHandle(TEXT("KERNEL32"));
		if (hKernel32) 
		{
			pInitializeCriticalSectionAndSpinCount = (LPInitializeCriticalSectionAndSpinCount) (GetProcAddress(hKernel32, "InitializeCriticalSectionAndSpinCount") );
		}
	}

	if (pInitializeCriticalSectionAndSpinCount!=0)
	{
		pInitializeCriticalSectionAndSpinCount(&mcrtDisk, 0x4000);
	}
	else
	{
		InitializeCriticalSection(&mcrtDisk);
	}

	mhThread = CreateThread(NULL, 0, DiskInterface::DiskThreadProc, this, 0, &mThreadId);
	if (mhThread == 0)
	{
		return E_FAIL;
	}

	WaitThreadReady();
	
	return S_OK;
}

bool DiskInterface::WaitThreadReady()
{
	DWORD r = WaitForMultipleObjects(2, &m_waitReady[0], FALSE, INFINITE);
	if (r == WAIT_OBJECT_0 + 1)
	{
		return true;
	}
	return false;
}

void DiskInterface::ThreadSignalCommandResetClock()
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		mbDiskThreadCommandResetClock = TRUE;
		ResetEvent(mevtDiskReady);
		SetEvent(mevtDiskCommand);
	}
	LeaveCriticalSection(&mcrtDisk);	
}

void DiskInterface::ThreadSignalCommandExecuteClock(ICLK PalClock)
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		m_DiskThreadCommandedPALClock = PalClock;
		ResetEvent(mevtDiskReady);
		SetEvent(mevtDiskCommand);
	}
	LeaveCriticalSection(&mcrtDisk);	
}

void DiskInterface::ThreadSignalExecute()
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		ResetEvent(mevtDiskReady);
		SetEvent(mevtDiskCommand);
	}
	LeaveCriticalSection(&mcrtDisk);
}

void DiskInterface::ThreadSignalPause()
{	
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		ResetEvent(mevtDiskReady);
		SetEvent(mevtDiskPause);
		mbDiskThreadPause = true;
	}
	LeaveCriticalSection(&mcrtDisk);
}

void DiskInterface::ThreadSignalQuit()
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		ResetEvent(mevtDiskReady);
		SetEvent(mevtDiskQuit);
		mbDiskThreadCommandQuit = true;
	}
	LeaveCriticalSection(&mcrtDisk);
}

void DiskInterface::CloseDiskThread()
{
	if (mhThread)
	{
		ThreadSignalQuit();
		WaitForMultipleObjects(1, &mhThread, TRUE, INFINITE);
		CloseHandle(mhThread);		
		mhThread = 0;
	}

	if (mevtDiskReady)
	{
		CloseHandle(mevtDiskReady);
		mevtDiskReady = 0;
	}
	
	if (mevtDiskCommand)
	{
		CloseHandle(mevtDiskCommand);
		DeleteCriticalSection(&mcrtDisk);
		mevtDiskCommand = 0;
	}

	if (mevtDiskQuit)
	{
		CloseHandle(mevtDiskQuit);
		mevtDiskQuit = 0;
	}	
}

DWORD WINAPI  DiskInterface::DiskThreadProc( LPVOID lpParam )
{
DiskInterface *pDisk;
DWORD r;

	pDisk = (DiskInterface *)lpParam;
	r = pDisk->DiskThreadProc();
	SetEvent(pDisk->mevtDiskReady);
	pDisk->mbDiskThreadHasQuit = true;
	return r;
}

DWORD DiskInterface::DiskThreadProc()
{
DWORD r;
ICLK clk;
bool bQuit;
bool bPause;
bool bCommand;

	r = WAIT_OBJECT_0;
	bCommand = false;
	bQuit = false;
	SetEvent(mevtDiskReady);
	while (!bQuit)
	{		
		r = WaitForMultipleObjects(3, &m_waitCommand[0], FALSE, INFINITE);
		if ( r == WAIT_OBJECT_0 + 2)//Command
		{
			bPause = false;
			while (!bQuit || !bPause)
			{
				EnterCriticalSection(&mcrtDisk);

				if (mbDiskThreadCommandQuit)
				{
					bQuit = true;
					LeaveCriticalSection(&mcrtDisk);
					break;
				}

				if (mbDiskThreadPause)
				{
					mbDiskThreadPause = false;
					bPause = true;
					LeaveCriticalSection(&mcrtDisk);
					break;
				}

				if (mbDiskThreadCommandResetClock)
				{
					mbDiskThreadCommandResetClock = false;
					m_DiskThreadCommandedPALClock = CurrentPALClock;
					m_DiskThreadCurrentPALClock = CurrentPALClock; 
				}

				clk = m_DiskThreadCommandedPALClock;
				m_DiskThreadCurrentPALClock = CurrentPALClock;

				if ((ICLKS)(m_DiskThreadCurrentPALClock - clk) >= 0)
				{
					SetEvent(mevtDiskReady);
					LeaveCriticalSection(&mcrtDisk);
					break;
				}
				LeaveCriticalSection(&mcrtDisk);

				ExecuteAllPendingDiskCpuClocks();
				ExecutePALClock(clk);
			}
		}
		else if ( r == WAIT_OBJECT_0 + 1)//Pause
		{
			EnterCriticalSection(&mcrtDisk);
			mbDiskThreadPause = false;
			SetEvent(mevtDiskReady);
			LeaveCriticalSection(&mcrtDisk);
			continue;
		}
		else if ( r == WAIT_OBJECT_0)//Quit
		{
			return 0;
		}
		else if (r == WAIT_ABANDONED)
		{			
			return 0;
		}
		else if (r == WAIT_TIMEOUT)
		{
			continue;
		}
		else //WAIT_FAILED
		{
			return 1;
		}
	}
	return 0;
}

void DiskInterface::Cleanup()
{

	CloseDiskThread();
	if (m_pD1541_ram)
	{
		GlobalFree(m_pD1541_ram);
		m_pD1541_ram=NULL;
	}
	if (m_pD1541_rom)
	{
		GlobalFree(m_pD1541_rom);
		m_pD1541_rom=NULL;
	}
	m_pIndexedD1541_rom=NULL;
	P64ImageDestroy(&m_P64Image);
}

void DiskInterface::D64_DiskProtect(bool bOn)
{
	if (bOn)
	{
		m_d64_protectOff = 0;
	}
	else
	{
		m_d64_protectOff = 1;
	}
}

bit8 DiskInterface::GetProtectSensorState()
{
	if (m_d64_diskchange_counter == 0)
	{
		if (m_diskLoaded)
			return m_d64_protectOff;
		else
			return 1;
	}
	else 
	{
		if (m_d64_diskchange_counter>DISKCHANGE2)
			return 0;
		else if (m_d64_diskchange_counter>DISKCHANGE1)
			return 1;
		else
			return 0;
	}
}

void DiskInterface::D64_serial_write(bit8 c64_serialbus)
{
	m_c64_serialbus_diskview = c64_serialbus;
	D64_Attention_Change();
}

void DiskInterface::D64_Attention_Change()
{
bit8 t,autoATN,portOut;
bit8 ATN;

	portOut = via1.PortBOutput();

	ATN = (~m_c64_serialbus_diskview <<2) & portOut & 0x80;


	autoATN = portOut & 0x10;
	t=~portOut;
	if (autoATN)//auto attention
	{
		m_d64_serialbus = ((t << 6) & (~m_c64_serialbus_diskview << 2) ) & 0x80  //DATA OUT
			|((t & 0x8)<< 3);//CLOCK OUT
	}
	else
	{
		m_d64_serialbus = ((t << 6) & (m_c64_serialbus_diskview << 2)) & 0x80  //DATA OUT
			|((t & 0x8)<< 3);//CLOCK OUT
	}

	via1.SetCA1Input(ATN!=0, 1);//1
}


void DiskInterface::MoveHead(bit8 trackNo)
{
	m_currentTrackNumber = trackNo;
}

bool DiskInterface::StepHeadIn()
{
  	if (m_currentTrackNumber < (HOST_MAX_TRACKS-1))
	{
		m_previousTrackNumber = m_currentTrackNumber++;
		m_lastHeadStepDir = 1;
		m_lastHeadStepPosition = (m_lastHeadStepPosition + 1) & 3;
		m_headStepClock = DISKHEADSTEPWAITTIME;
		m_headStepClockUp = 0;
		m_nextP64PulsePosition = -1;
		return true;
	}
	else
	{
		m_lastHeadStepDir = -1;
		return false;
	}
}

bool DiskInterface::StepHeadOut()
{
	if (m_currentTrackNumber > 0)
	{
		m_previousTrackNumber = m_currentTrackNumber--;
		m_lastHeadStepDir = -1;
		m_lastHeadStepPosition = (m_lastHeadStepPosition - 1) & 3;
		m_headStepClock = DISKHEADSTEPWAITTIME;
		m_headStepClockUp = 0;
		m_nextP64PulsePosition = -1;
		return true;
	}
	else
	{
		m_lastHeadStepDir = 1;
		return false;
	}
}

void DiskInterface::StepHeadAuto()
{
	if (m_lastHeadStepDir == 1)
	{
		if (m_lastHeadStepPosition == 1 || m_lastHeadStepPosition == 3)
		{
			if (StepHeadOut())
				StepHeadOut();
		}
	}
	else if (m_lastHeadStepDir == -1)
	{
		if (m_lastHeadStepPosition == 0 || m_lastHeadStepPosition == 2)
		{
			if (StepHeadIn())
				StepHeadIn();
		}
	}
}

bit8 DiskInterface::GetBusDataByte()
{
	if ((ICLKS)(CurrentClock - m_busDataUpdateClock) >= 0)
	{
		return (bit8)m_shifterReader_UD2;
	}
	else
	{
		return (bit8)m_busByteReadyPreviousData;
	}
}

void DiskInterface::ClockDividerAdd(bit8 clocks, bit8 speed, bool bStartWithPulse)
{
bit8 clockDivider1_UE7;
bit8 clockDivider2_UF4;
bit8 prevClockDivider2;
bit8 byteReady;
bit8 writeClock;
bool bUF4_QB_rising;
bit8 bandpos;
#define WEAKBITIME1 (320)
#define WEAKBITIME2 (130)
	if (clocks==0)
	{
		return;	
	}
	clockDivider1_UE7 = m_clockDivider1_UE7;
	clockDivider2_UF4 = m_clockDivider2_UF4;
	bandpos = m_totalclocks_UE7 & 0xf;
	writeClock = bandpos;
	m_lastPulseTime +=clocks;
	m_lastWeakPulseTime +=clocks;
	m_totalclocks_UE7+=clocks;

	if (bStartWithPulse)
	{
		//Reset the clockdividers because a disk pulse has occurred.
		m_lastPulseTime = 0;
		m_lastWeakPulseTime = 0;
		clockDivider1_UE7 = speed;
		clockDivider2_UF4 = 0;

		byteReady=1;
		if ((m_frameCounter_UC3 & 7) == 7)
		{
			if (m_d64_soe_enable!=0 && (clockDivider2_UF4 & 2) == 0)
			{
				byteReady = 0;
			}
			if (m_busByteReadySignal != byteReady)
			{
				m_busByteReadySignal = byteReady;
				if (writeClock < 7)//7
				{
					via2.ExecuteCycle(CurrentClock - 1);//
					via2.SetCA1Input(byteReady, 0);//0
				}
				else
				{
					via2.ExecuteCycle(CurrentClock - 1);//
					via2.SetCA1Input(byteReady, 1);//1
				}
			}
		}
	}

	if (m_lastPulseTime > WEAKBITIME1 && m_d64_write_enable == 0)
	{
		if ((m_bDiskMotorOn || m_motorOffClock> 0) && m_d64_diskchange_counter==0 && m_lastWeakPulseTime > WEAKBITIME2 && rand() < 1000)
		{
			clockDivider1_UE7 = speed;
			clockDivider2_UF4 = 0;
			m_lastWeakPulseTime = 0;
		}
	}

	m_writeStream = 0;
	clockDivider1_UE7 += clocks;

	for (int c=0 ; c==0 || clockDivider1_UE7 >= 16 ; c++)
	{
		if (clockDivider1_UE7 >= 16)
		{
			if (c==0)
			{
				writeClock = (bandpos + 15 + clocks - clockDivider1_UE7) & 0xf;
			}
			else
			{
				writeClock = (writeClock - speed) & 0xf;
			}

			clockDivider1_UE7 = clockDivider1_UE7 + speed - 16;
			prevClockDivider2 = clockDivider2_UF4;
			clockDivider2_UF4 =  (clockDivider2_UF4 + 1) & 0xf;
	
			bUF4_QB_rising =  ((clockDivider2_UF4 & 2) != 0 && (prevClockDivider2 & 2) == 0);
			if (bUF4_QB_rising)
			{
				//rising QB
				if ((via2.delay & (VIACA1Trans0 | VIACA1Trans1)))//Make sure pending transitions of the byte ready signal to via2.ca1 have been seen before updating the bus byte.
				{
					via2.ExecuteCycle(CurrentClock - 1);
				}
				m_busByteReadyPreviousData = m_shifterReader_UD2;				
				if (writeClock < 14)//test
				{
					m_busDataUpdateClock = CurrentClock;
				}
				else
				{
					m_busDataUpdateClock = CurrentClock + 1;
				}

				if ((signed char)m_shifterWriter_UD3 < 0)
				{
					//writing a 1
					m_writeStream = writeClock + 1;
				}
				m_shifterWriter_UD3 <<= 1;
				
				m_shifterReader_UD2 <<= 1;
				if ((clockDivider2_UF4 & 0xc) == 0)
				{
					//reading a 1
					m_shifterReader_UD2 |= 1;
				}
				if (!m_d64_sync)
				{
					m_frameCounter_UC3 =  (m_frameCounter_UC3 + 1) & 0xf;
				}
				m_debugFrameCounter = (m_debugFrameCounter + 1) & 0xf;
				bit8 oldSync = m_d64_sync;
				m_d64_sync = ((m_d64_write_enable==0) && ((m_shifterReader_UD2 & 0x3ff) == 0x3ff)) || (m_d64_forcesync!=0);
				if (m_d64_sync)
				{
					m_frameCounter_UC3 = 0;
				}
				
	#ifdef DEBUG_DISKPULSES
				if (!m_d64_sync && oldSync!=0)
				{
					m_debugFrameCounter = 0;
				}
	#endif
			}
		}

		byteReady=1;
		if ((m_frameCounter_UC3 & 7) == 7)
		{
			if ((clockDivider2_UF4 & 3) == 3)
			{
				m_shifterWriter_UD3 = m_d64_diskwritebyte;
			}
			if ((m_d64_soe_enable != 0) && (clockDivider2_UF4 & 2) == 0)
			{
				byteReady = 0;
			}
		}

		#ifdef DEBUG_DISKPULSES
		static bool allowCapture = true;
		static bool printDebug = false;
		if ((m_debugFrameCounter & 7) == 7 )
		{
			if ((clockDivider2_UF4 & 2)==0 && allowCapture)
			{
				allowCapture = false;
				TCHAR sDebug[50];
				static int linePrintCount = 0;
				int dataByte = (int)(m_shifterReader_UD2 & 0xff);
				_stprintf_s(sDebug, _countof(sDebug), TEXT("%2X "), dataByte);		
				if (printDebug)
				{
					OutputDebugString(sDebug);
					linePrintCount ++;
					if (linePrintCount>=16)
					{
						linePrintCount = 0;
						OutputDebugString(TEXT("\n"));
					}
				}
			}
		}
		else
			allowCapture = true;
		#endif
		if (m_busByteReadySignal != byteReady)
		{
			m_busByteReadySignal = byteReady;
			if (writeClock < 7)//7
			{
				via2.ExecuteCycle(CurrentClock - 1);//
				via2.SetCA1Input(byteReady, 0);//0
			}
			else
			{
				via2.ExecuteCycle(CurrentClock - 1);//
				via2.SetCA1Input(byteReady, 1);//1
			}
		}	
	} 
	m_clockDivider1_UE7 = clockDivider1_UE7;
	m_clockDivider2_UF4 = clockDivider2_UF4;
}

void DiskInterface::SpinDisk(ICLK sysclock)
{
bit8 bitTime;
bit8 bitTimeFirstInCell;
bit8 bitTimeDelayed;
ICLKS clocks;
bit8 speed;
p64_uint32_t position;
bool nextPulseState;
int i;

	PP64PulseStream track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + m_currentTrackNumber];
	speed = m_clockDivider1_UE7_Reload;
	clocks = (ICLKS)(sysclock - CurrentClock);
	while (clocks-- > 0)
	{
		CurrentClock++;
		bitTime = 0;
		bitTimeFirstInCell = 0;
		bitTimeDelayed = 0;
		nextPulseState = m_bPulseState;
		bit8 bMotorRun = MotorSlowDownEnv();
		if (m_headStepClock)
		{
			m_headStepClock--;
			position = ((p64_uint32_t)m_currentHeadIndex) << 4;
			if (m_headStepClockUp == 0)
			{
				if (track->CurrentIndex >= 0 && track->Pulses[track->CurrentIndex].Position > position)
				{
					track->CurrentIndex = track->UsedFirst;
				}
			}
			else
			{				
				if (track->CurrentIndex >= 0)
				{
					for (i = 0; track->Pulses[track->CurrentIndex].Next >= 0 && track->Pulses[track->Pulses[track->CurrentIndex].Next].Position < position && i < 20; i++)
					{
						track->CurrentIndex = track->Pulses[track->CurrentIndex].Next;
					}
				}
			}
			m_headStepClockUp++;
			if (m_headStepClock > 0)
			{
				track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + m_previousTrackNumber];
			}
			else
			{
				track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + m_currentTrackNumber];
				m_previousTrackNumber = m_currentTrackNumber;
				m_nextP64PulsePosition = -1;
			}
		}
		m_counterStartPulseFilter += DISK16CELLSPERCLOCK;

		if (bMotorRun && m_d64_diskchange_counter==0)
		{
			//Rotate disk 16 clocks;
			m_currentHeadIndex++;
			if (m_currentHeadIndex >= DISK_RAW_TRACK_SIZE)
			{
				m_currentHeadIndex = 0;
			}
			position = ((p64_uint32_t)m_currentHeadIndex) << 4;
			if (m_nextP64PulsePosition < 0)
			{
				P64PulseStreamGetNextPulse(track, position);
				if (track->CurrentIndex >= 0)
				{
					m_nextP64PulsePosition = (bit32s)track->Pulses[track->CurrentIndex].Position;
				}
				else
				{
					m_nextP64PulsePosition = P64PulseSamplesPerRotation;
				}
			}
			if (m_d64_write_enable==0)
			{				
				if (m_diskLoaded && track->UsedFirst >= 0)
				{
					if (m_nextP64PulsePosition >= 0 && m_nextP64PulsePosition < P64PulseSamplesPerRotation && track->CurrentIndex >= 0)
					{
						p64_uint32_t positionDelta = 0;
						bool first = true;
						if ((p64_uint32_t)m_nextP64PulsePosition >= position)
						{
							positionDelta = m_nextP64PulsePosition - position;
						}
						else
						{
							positionDelta = P64PulseSamplesPerRotation + m_nextP64PulsePosition - position;
						}
						for(i = 0; positionDelta < DISK16CELLSPERCLOCK && i < 16; i++)
						{
							if (track->Pulses[track->CurrentIndex].Strength >= 0x80000000)
							{
								bitTime = positionDelta + 1;
								if (first)
								{
									first = false;
									bitTimeFirstInCell = bitTime;
								}
								else
								{
									nextPulseState = !nextPulseState;
								}
							}
							track->CurrentIndex = track->Pulses[track->CurrentIndex].Next;
							if (track->CurrentIndex < 0)
							{
								track->CurrentIndex = track->UsedFirst;
							}
							if (track->CurrentIndex >= 0)
							{
								m_nextP64PulsePosition = track->Pulses[track->CurrentIndex].Position;										
							}
							else
							{
								m_nextP64PulsePosition = P64PulseSamplesPerRotation;
								break;
							}
							if ((p64_uint32_t)m_nextP64PulsePosition >= position)
							{
								positionDelta = m_nextP64PulsePosition - position;
							}
							else
							{
								positionDelta = P64PulseSamplesPerRotation + m_nextP64PulsePosition - position;
							}
						}
					}
				}

				if (m_bPendingPulse && (m_counterStartPulseFilter) > DISKHEADFILTERWIDTH)
				{
					m_bPendingPulse = false;
					//A pending pulse may be allowed through the DISKHEADFILTERWIDTH (40 clock) delay filter during this 16 disk clock band provide a new pulse is not received with in DISKHEADFILTERWIDTH (40 clock) period.
					//m_counterStartPulseFilter here represent the end if the current 16 disk clock band.
					if (m_counterStartPulseFilter - DISKHEADFILTERWIDTH <= DISK16CELLSPERCLOCK && m_bPulseState != m_bLastPulseState)
					{
						bitTimeDelayed = (bit8)(DISK16CELLSPERCLOCK + DISKHEADFILTERWIDTH - m_counterStartPulseFilter + 1);
						if ((bitTimeFirstInCell == 0 || bitTimeDelayed < bitTimeFirstInCell))
						{
							m_bLastPulseState = m_bPulseState;
						}
						else
						{
							//Another new pulse (in bitTimeFirstInCell) was received before the previous older pulse in (bitTimeDelayed) could pass through the 40 disk clock filter.
							bitTimeDelayed = 0;
						}
					}
				}

				if (bitTime != 0)
				{
					//A new pulse is received before the filter.
					m_bPendingPulse = true;
					m_bPulseState = !nextPulseState;
					//Start the delay filter timer in m_counterStartPulseFilter.
					m_counterStartPulseFilter = DISK16CELLSPERCLOCK - bitTime + 1;
				}

				if (bitTimeDelayed != 0)
				{
					//A pulse passed through the filter.
					bitTimeDelayed--;
					ClockDividerAdd(bitTimeDelayed, speed, false);

					ClockDividerAdd(DISK16CELLSPERCLOCK-bitTimeDelayed, speed, true);
				}
				else
				{
					ClockDividerAdd(DISK16CELLSPERCLOCK, speed, false);
				}
			}
			else
			{
				m_nextP64PulsePosition = -1;
				ClockDividerAdd(DISK16CELLSPERCLOCK, speed, false);
				if (m_d64_protectOff!=0 && m_diskLoaded)
				{
					P64PulseStreamRemovePulses(track, position, 40);
					if (m_writeStream > 0)
					{
						P64PulseStreamAddPulse(track, position + m_writeStream - 1, 0xffffffff);
					}
				}
			}
		}
		else
		{
			ClockDividerAdd(DISK16CELLSPERCLOCK, speed, false);
		}
	}
}

void DiskInterface::MotorDisk16(bit8 trackNumber, bit32 *headIndex)
{
	if (++*headIndex >= (DISK_RAW_TRACK_SIZE))
		*headIndex = 0;
}

void DiskInterface::C64SerialBusChange(ICLK palclock, bit8 c64_serialbus)
{
ICLK palcycles, cycles;

	palcycles = palclock - CurrentPALClock;
	if ((ICLKS) palcycles <= 0)
	{
		this->D64_serial_write(m_c64_serialbus_diskview_next);
	}
	else
	{
		__int64 div = ((Disk64clk_dy2  * (__int64)palcycles) + m_diskd64clk_xf) / Disk64clk_dy1;
		cycles = (ICLK)div;
		m_changing_c64_serialbus_diskview_diskclock = this->CurrentClock + cycles;
		m_c64_serialbus_diskview_next = c64_serialbus;
	}

}

bit8 DiskInterface::GetC64SerialBusDiskView(ICLK diskclock)
{
	if ((ICLKS)(this->CurrentClock - m_changing_c64_serialbus_diskview_diskclock) >=0)
	{
		return m_c64_serialbus_diskview;
	}
	else
	{
		return m_c64_serialbus_diskview_next;
	}
}

void DiskInterface::AccumulatePendingDiskCpuClocksToPalClock(ICLK palclock)
{
ICLK palcycles, cycles;

	palcycles = palclock - CurrentPALClock;
	if ((ICLKS) palcycles <= 0)
		return;
	__int64 div = ((Disk64clk_dy2  * (__int64)palcycles) + m_diskd64clk_xf) / Disk64clk_dy1;
	m_diskd64clk_xf = ((Disk64clk_dy2 * (__int64)palcycles) + m_diskd64clk_xf) % Disk64clk_dy1;
	cycles = (ICLK)div;
	m_pendingclocks += cycles;
	CurrentPALClock = palclock;
}

void DiskInterface::ExecuteOnePendingDiskCpuClock()
{
	if (m_pendingclocks > 0)
	{
		ExecuteCycle(cpu.CurrentClock + 1);
		m_pendingclocks--;
	}
}

void DiskInterface::ExecuteAllPendingDiskCpuClocks()
{
	if (m_pendingclocks > 0)
	{
		ExecuteCycle(cpu.CurrentClock + m_pendingclocks);
		m_pendingclocks = 0;
	}
}

//985248  = 2 x 2 x 2 x 2 x 2 x 3 x 3 x 11 x 311
//1000000 = 2 x 2 x 2 x 2 x 2 x 2 x 5 x  5 x   5 x 5 x 5 x 5
//985248 PAL
//1022727 NTSC
void DiskInterface::ExecutePALClock(ICLK palclock)
{
ICLK palcycles,cycles,sysclock;

	palcycles = palclock - CurrentPALClock;
	if ((ICLKS) palcycles <= 0)
		return;

	__int64 div = ((Disk64clk_dy2  * (__int64)palcycles) + m_diskd64clk_xf) / Disk64clk_dy1;
	m_diskd64clk_xf = ((Disk64clk_dy2 * (__int64)palcycles) + m_diskd64clk_xf) % Disk64clk_dy1;

	cycles = (ICLK)div;
	m_pendingclocks = 0;
	sysclock = cpu.CurrentClock + cycles;
	ExecuteCycle(sysclock);
	CurrentPALClock = palclock;
}

void DiskInterface::ExecuteCycle(ICLK sysclock)
{
ICLKS cycles;

	cycles = (ICLKS)(sysclock - cpu.CurrentClock);
	if (cycles>0)
	{
		m_diskChangeCounter -= cycles;
		if (m_diskChangeCounter < 0)
		{
			m_diskChangeCounter += DISKCHANGECLOCKRELOAD;
			if (m_d64_diskchange_counter)
				m_d64_diskchange_counter--;
		}
		ICLK clock_change =  m_changing_c64_serialbus_diskview_diskclock;
		ICLKS cycles_to_serialbuschange = (ICLKS)(clock_change - cpu.CurrentClock);
		if (cycles_to_serialbuschange < 0 || cycles < cycles_to_serialbuschange)
		{
			cpu.ExecuteCycle(sysclock);
			SpinDisk(sysclock);
			via1.ExecuteCycle(sysclock);
			via2.ExecuteCycle(sysclock);
		}
		else
		{
			if (cycles_to_serialbuschange > 0)
			{
				cpu.ExecuteCycle(clock_change);
				SpinDisk(clock_change);
				via1.ExecuteCycle(clock_change);
				via2.ExecuteCycle(clock_change);
				cycles -= cycles_to_serialbuschange;
			}

			this->D64_serial_write(m_c64_serialbus_diskview_next);

			if (cycles > 0)
			{
				cpu.ExecuteCycle(sysclock);
				SpinDisk(sysclock);
				via1.ExecuteCycle(sysclock);
				via2.ExecuteCycle(sysclock);
			}
		}
	}
}

ICLK DiskInterface::GetCurrentClock()
{
	return CurrentClock;
}

void DiskInterface::SetCurrentClock(ICLK sysclock)
{
ICLK w = sysclock - CurrentPALClock;
ICLK c = cpu.CurrentClock;
ICLK v = c - CurrentClock;
	CurrentPALClock += w;
	CurrentClock += v;
	m_changing_c64_serialbus_diskview_diskclock += v;
	m_driveWriteChangeClock += v;
	m_busDataUpdateClock += v;
	cpu.SetCurrentClock(sysclock);
	via1.SetCurrentClock(sysclock);
	via2.SetCurrentClock(sysclock);
}

void DiskInterface::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;

	if ((ICLKS)(CurrentClock - m_changing_c64_serialbus_diskview_diskclock) >= CLOCKSYNCBAND_FAR)
	{
		m_changing_c64_serialbus_diskview_diskclock = ClockBehindNear;
	}
	if ((ICLKS)(CurrentClock - m_driveWriteChangeClock) >= CLOCKSYNCBAND_FAR)
	{
		m_driveWriteChangeClock = ClockBehindNear;
	}
	if ((ICLKS)(CurrentClock - m_busDataUpdateClock) >= CLOCKSYNCBAND_FAR)
	{
		m_busDataUpdateClock = ClockBehindNear;
	}
	if (m_counterStartPulseFilter > CLOCKSYNCBAND_FAR)
	{
		m_counterStartPulseFilter = DISKHEADFILTERWIDTH + DISK16CELLSPERCLOCK + 1;
	}
	if (m_lastPulseTime > CLOCKSYNCBAND_FAR)
	{
		m_lastPulseTime = ClockBehindNear;
	}
	if (m_lastWeakPulseTime > CLOCKSYNCBAND_FAR)
	{
		m_lastWeakPulseTime = ClockBehindNear;
	}	
	cpu.PreventClockOverflow();
}

void DiskInterface::LoadMoveP64Image(GCRDISK *dsk)
{
int halfTrack;	
	WaitThreadReady();
	m_d64TrackCount = dsk->m_d64TrackCount;
	this->m_P64Image = dsk->m_P64Image;
	for (halfTrack=0; halfTrack < _countof(dsk->m_P64Image.PulseStreams); halfTrack++)
	{
		dsk->m_P64Image.PulseStreams[halfTrack].Pulses = 0;
	}
	P64ImageClear(&dsk->m_P64Image);
}

void DiskInterface::SaveCopyP64Image(GCRDISK *dsk)
{
int halfTrack;
	WaitThreadReady();
	dsk->m_d64TrackCount = m_d64TrackCount;
	P64ImageClear(&dsk->m_P64Image);
	dsk->m_P64Image = this->m_P64Image;
	for (halfTrack=0; halfTrack < _countof(dsk->m_P64Image.PulseStreams); halfTrack++)
	{
		dsk->m_P64Image.PulseStreams[halfTrack].Pulses = 0;			
	}
	for (halfTrack=0; halfTrack < _countof(dsk->m_P64Image.PulseStreams); halfTrack++)
	{
		if (this->m_P64Image.PulseStreams[halfTrack].Pulses != 0 && this->m_P64Image.PulseStreams[halfTrack].PulsesAllocated > 0)
		{
			PP64Pulses p = (PP64Pulses)p64_malloc(this->m_P64Image.PulseStreams[halfTrack].PulsesAllocated * sizeof(TP64Pulse));
			if (p)
			{
				dsk->m_P64Image.PulseStreams[halfTrack] = this->m_P64Image.PulseStreams[halfTrack];
				dsk->m_P64Image.PulseStreams[halfTrack].Pulses = p;
				memcpy(p, this->m_P64Image.PulseStreams[halfTrack].Pulses, this->m_P64Image.PulseStreams[halfTrack].PulsesAllocated * sizeof(TP64Pulse));
			}
			else
			{
				P64PulseStreamClear(&dsk->m_P64Image.PulseStreams[halfTrack]);
			}
		}
	}
}

void DiskInterface::SetDiskLoaded()
{
	WaitThreadReady();
	if (m_diskLoaded)
	{
		m_d64_diskchange_counter=DISKCHANGE3;
	}
	else
	{
		m_d64_diskchange_counter=DISKCHANGE1;
	}
	m_diskLoaded = 1;
	m_nextP64PulsePosition = -1;
}

void DiskInterface::RemoveDisk()
{
	WaitThreadReady();
	if (m_diskLoaded)
	{
		m_diskLoaded = 0;
		m_d64_diskchange_counter = DISKCHANGE1;
	}
	m_nextP64PulsePosition = -1;
}

bit8 DiskInterface::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void DiskInterface::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

bit8 DiskInterface::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	return 0;
}

bit8 DiskInterface::GetHalfTrackIndex()
{
	return m_currentTrackNumber;
}

void DiskInterface::StartMotor()
{
}

void DiskInterface::StopMotor()
{
	//Allow motor to run for a short time after it is turned off.
	m_motorOffClock = DISKMOTORSLOWTIME + (rand() & 0x1fff);
}

bool DiskInterface::MotorSlowDownEnv()
{	
	if (m_bDiskMotorOn)
	{
		return true;
	}
	else
	{
        if (m_motorOffClock > 0)
        {
            m_motorOffClock--;

            if (m_motorOffClock > (3 * DISKMOTORSLOWTIME / 4))
            {
                return true;
            }
            else if (m_motorOffClock > (1 * DISKMOTORSLOWTIME / 2))
            {
                return (m_motorOffClock & 1) == 0;
            }
            else
            {
                return (m_motorOffClock & 3) == 0;
            }
        }
		else
		{
			return false;
		}
	}
}

void DiskInterface::GetState(SsDiskInterfaceV2 &state)
{
	ZeroMemory(&state, sizeof(state));
	state.CurrentClock = CurrentClock;
	state.CurrentPALClock = CurrentPALClock;
	state.m_diskd64clk_xf = m_diskd64clk_xf;
	state.m_d64_serialbus = m_d64_serialbus;
	state.m_d64_dipswitch = m_d64_dipswitch;
	state.m_d64_protectOff = m_d64_protectOff;
	state.m_d64_sync = m_d64_sync;
	state.m_d64_forcesync = m_d64_forcesync;
	state.m_d64_soe_enable = m_d64_soe_enable;
	state.m_d64_write_enable = m_d64_write_enable;
	state.m_d64_diskchange_counter = m_d64_diskchange_counter;
	state.m_d64TrackCount = m_d64TrackCount;
	state.m_c64_serialbus_diskview = m_c64_serialbus_diskview;
	state.m_c64_serialbus_diskview_next = m_c64_serialbus_diskview_next;
	state.m_diskChangeCounter = m_diskChangeCounter;
	state.m_changing_c64_serialbus_diskview_diskclock = m_changing_c64_serialbus_diskview_diskclock;
	state.m_driveWriteChangeClock = m_driveWriteChangeClock;
	state.m_motorOffClock = m_motorOffClock;
	state.m_headStepClock = m_headStepClock;
	state.m_headStepClockUp = m_headStepClockUp;
	state.m_pendingclocks = m_pendingclocks;
	state.m_DiskThreadCommandedPALClock = m_DiskThreadCommandedPALClock;
	state.m_DiskThreadCurrentPALClock = m_DiskThreadCurrentPALClock;
	state.m_d64_diskwritebyte = m_d64_diskwritebyte;
	state.m_bDiskMotorOn = m_bDiskMotorOn;
	state.m_bDriveLedOn = m_bDriveLedOn;
	state.m_bDriveWriteWasOn = m_bDriveWriteWasOn;
	state.m_diskLoaded = m_diskLoaded;
	state.m_currentHeadIndex = m_currentHeadIndex;
	state.m_currentTrackNumber = m_currentTrackNumber;
	state.m_previousTrackNumber = m_previousTrackNumber;
	state.m_lastHeadStepDir = m_lastHeadStepDir;
	state.m_lastHeadStepPosition = m_lastHeadStepPosition;
	state.m_shifterWriter_UD3 = m_shifterWriter_UD3;
	state.m_shifterReader_UD2 = m_shifterReader_UD2;
	state.m_busDataUpdateClock = m_busDataUpdateClock;
	state.m_busByteReadyPreviousData = m_busByteReadyPreviousData;
	state.m_busByteReadySignal = m_busByteReadySignal;
	state.m_frameCounter_UC3 = m_frameCounter_UC3;
	state.m_debugFrameCounter = m_debugFrameCounter;
	state.m_clockDivider1_UE7_Reload = m_clockDivider1_UE7_Reload;
	state.m_clockDivider1_UE7 = m_clockDivider1_UE7;
	state.m_clockDivider2_UF4 = m_clockDivider2_UF4;
	state.m_writeStream = m_writeStream;
	state.m_totalclocks_UE7 = m_totalclocks_UE7;
	state.m_lastPulseTime = m_lastPulseTime;
	state.m_lastWeakPulseTime = m_lastWeakPulseTime;
	state.m_counterStartPulseFilter = m_counterStartPulseFilter;
	state.m_nextP64PulsePosition = m_nextP64PulsePosition;
	state.m_bPendingPulse = m_bPendingPulse;
	state.m_bPulseState = m_bPulseState;
	state.m_bLastPulseState = m_bLastPulseState;
}

void DiskInterface::SetState(const SsDiskInterfaceV2 &state)
{
	CurrentClock = state.CurrentClock;
	CurrentPALClock = state.CurrentPALClock;
	m_diskd64clk_xf = state.m_diskd64clk_xf;
	m_d64_serialbus = state.m_d64_serialbus;
	m_d64_dipswitch = state.m_d64_dipswitch;
	m_d64_protectOff = state.m_d64_protectOff;
	m_d64_sync = state.m_d64_sync;
	m_d64_forcesync = state.m_d64_forcesync;
	m_d64_soe_enable = state.m_d64_soe_enable;
	m_d64_write_enable = state.m_d64_write_enable;
	m_d64_diskchange_counter = state.m_d64_diskchange_counter;
	m_d64TrackCount = state.m_d64TrackCount;
	m_c64_serialbus_diskview = state.m_c64_serialbus_diskview;
	m_c64_serialbus_diskview_next = state.m_c64_serialbus_diskview_next;
	m_diskChangeCounter = state.m_diskChangeCounter;
	m_changing_c64_serialbus_diskview_diskclock = state.m_changing_c64_serialbus_diskview_diskclock;
	m_driveWriteChangeClock = state.m_driveWriteChangeClock;
	m_motorOffClock = state.m_motorOffClock;
	m_headStepClock = state.m_headStepClock;
	m_headStepClockUp = state.m_headStepClockUp;
	m_pendingclocks = state.m_pendingclocks;
	m_DiskThreadCommandedPALClock = state.m_DiskThreadCommandedPALClock;
	m_DiskThreadCurrentPALClock = state.m_DiskThreadCurrentPALClock;
	m_d64_diskwritebyte = state.m_d64_diskwritebyte;
	m_bDiskMotorOn = state.m_bDiskMotorOn != 0;
	m_bDriveLedOn = state.m_bDriveLedOn != 0;
	m_bDriveWriteWasOn = state.m_bDriveWriteWasOn != 0;
	m_diskLoaded = state.m_diskLoaded;
	m_currentHeadIndex = state.m_currentHeadIndex;
	m_currentTrackNumber = state.m_currentTrackNumber;
	m_previousTrackNumber = state.m_previousTrackNumber;
	m_lastHeadStepDir = state.m_lastHeadStepDir;
	m_lastHeadStepPosition = state.m_lastHeadStepPosition;
	m_shifterWriter_UD3 = state.m_shifterWriter_UD3;
	m_shifterReader_UD2 = state.m_shifterReader_UD2;
	m_busDataUpdateClock = state.m_busDataUpdateClock;
	m_busByteReadyPreviousData = state.m_busByteReadyPreviousData;
	m_busByteReadySignal = state.m_busByteReadySignal;
	m_frameCounter_UC3 = state.m_frameCounter_UC3;
	m_debugFrameCounter = state.m_debugFrameCounter;
	m_clockDivider1_UE7_Reload = state.m_clockDivider1_UE7_Reload;
	m_clockDivider1_UE7 = state.m_clockDivider1_UE7;
	m_clockDivider2_UF4 = state.m_clockDivider2_UF4;
	m_writeStream = state.m_writeStream;
	m_totalclocks_UE7 = state.m_totalclocks_UE7;
	m_lastPulseTime = state.m_lastPulseTime;
	m_lastWeakPulseTime = state.m_lastWeakPulseTime;
	m_counterStartPulseFilter = state.m_counterStartPulseFilter;
	m_nextP64PulsePosition = state.m_nextP64PulsePosition;
	m_bPendingPulse = state.m_bPendingPulse != 0;
	m_bPulseState = state.m_bPulseState != 0;
	m_bLastPulseState = state.m_bLastPulseState != 0;
	PrepareP64Head(m_currentTrackNumber);
}

void DiskInterface::PrepareP64Head(unsigned int trackNumber)
{
	//Restore track head position within the P64Image for the current track and the previous track.
	if (trackNumber >= HOST_MAX_TRACKS)
	{
		m_currentTrackNumber = HOST_MAX_TRACKS - 1;
	}
	else
	{
		m_currentTrackNumber = trackNumber;
	}
	if (m_previousTrackNumber >= HOST_MAX_TRACKS)
	{
		m_previousTrackNumber = HOST_MAX_TRACKS - 1;
	}
	if (abs(m_currentTrackNumber - m_previousTrackNumber) > 1)
	{
		m_previousTrackNumber = m_currentTrackNumber;
	}
	PP64PulseStream track;
	if (this->m_diskLoaded != 0)
	{
		p64_uint32_t position = ((p64_uint32_t)m_currentHeadIndex) << 4;
		track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + m_currentTrackNumber];
		if (track->UsedFirst >= 0 && track->Pulses != NULL && track->CurrentIndex >= 0)
		{
			P64PulseStreamGetNextPulse(track, position);
		}
		track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + m_previousTrackNumber];
		if (track->UsedFirst >= 0 && track->Pulses != NULL && track->CurrentIndex >= 0)
		{
			P64PulseStreamGetNextPulse(track, position);
		}
	}
}

void DiskInterface::UpgradeStateV0ToV1(const SsDiskInterfaceV0 &in, SsDiskInterfaceV1 &out)
{
	ZeroMemory(&out, sizeof(SsDiskInterfaceV1));
	*((SsDiskInterfaceV0 *)&out) = in;
	if (in.m_lastPulseTime <= DISKHEADFILTERWIDTH)
	{
		out.m_bPendingPulse = true;
		out.m_bPulseState = 1;
		out.m_bLastPulseState = 0;
	}
	else
	{
		out.m_bPendingPulse = 0;
		out.m_bPulseState = 0;
		out.m_bLastPulseState = 0;
	}
	out.m_nextP64PulsePosition = -1;
	out.m_headStepClockUp = DISKHEADSTEPWAITTIME - in.m_headStepClock;
}

void DiskInterface::UpgradeStateV1ToV2(const SsDiskInterfaceV1 &in, SsDiskInterfaceV2 &out)
{
	ZeroMemory(&out, sizeof(SsDiskInterfaceV2));
	*((SsDiskInterfaceV1 *)&out) = in;
	out.m_previousTrackNumber = in.m_currentTrackNumber;
	out.m_lastWeakPulseTime = 0;
	out.m_counterStartPulseFilter = 0;

}