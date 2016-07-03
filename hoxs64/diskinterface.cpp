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
#define DISKHEADFILTERWIDTH (39)

DiskInterface::DiskInterface()
{
int i;
	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		m_rawTrackData[i] = 0;
	}

	m_currentHeadIndex=0;
	m_currentTrackNumber = 0;
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

	m_bPulseState = false;
	m_bLastPulseState = false;
	m_bPendingPulse = false;
	m_counterStartPulseFilter = DISKHEADFILTERWIDTH;

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

	mThreadId = 0;
	mhThread = 0;
	mbDiskThreadCommandQuit = 0;
	mevtDiskClocksDone = 0;
	mevtDiskCommand = 0;
	mbDiskThreadCommandQuit = false;
	mbDiskThreadHasQuit = false;
}

DiskInterface::~DiskInterface()
{
	Cleanup();
}

void DiskInterface::InitReset(ICLK sysclock)
{
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

	m_bPulseState = false;
	m_bLastPulseState = false;
	m_bPendingPulse = false;
	m_counterStartPulseFilter = DISKHEADFILTERWIDTH;

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

	SetRamPattern();

	m_currentTrackNumber = 0x2;
	m_lastHeadStepDir = 0;
	m_lastHeadStepPosition = 2;
}

void DiskInterface::Reset(ICLK sysclock)
{
	WaitThreadReady();

	CurrentClock = sysclock;
	CurrentPALClock = sysclock;
	m_pendingclocks = 0;
	//CurrentPALClock must be set before calling ThreadSignalCommandResetClock
	ThreadSignalCommandResetClock();
	WaitThreadReady();

	InitReset(sysclock);

	via1.Reset(sysclock);
	via2.Reset(sysclock);
	cpu.Reset(sysclock);

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
int i;
HANDLE hfile=0;
BOOL r;
DWORD bytes_read;
TCHAR szRomPath[MAX_PATH+1];
HRESULT hr;

	ClearError();
	Cleanup();

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

	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		m_rawTrackData[i] = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, DISK_RAW_TRACK_SIZE);
		if (m_rawTrackData[i] == 0)
		{
			Cleanup();
			return SetError(E_OUTOFMEMORY, TEXT("Could not read 0x4000 bytes from C1541.rom"));
		}
	}
	
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

	mevtDiskClocksDone = CreateEvent(0, TRUE, FALSE, NULL);
	if (mevtDiskClocksDone == 0)
	{
		return E_FAIL;
	}
	mevtDiskCommand = CreateEvent(0, TRUE, FALSE, NULL);
	if (mevtDiskCommand == 0)
	{
		return E_FAIL;
	}
	else
	{
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
	}

	mhThread = CreateThread(NULL, 0, DiskInterface::DiskThreadProc, this, 0, &mThreadId);
	if (mhThread == 0)
	{
		return E_FAIL;
	}

	WaitThreadReady();
	
	return S_OK;
}

void DiskInterface::WaitThreadReady()
{
	while (1)
	{
		DWORD r;
		r = WaitForSingleObject(mevtDiskClocksDone, 0);
		if (r == WAIT_OBJECT_0)
			return ;
		else if (r == WAIT_TIMEOUT)
			continue;
		else
			return ;
	}
}

void DiskInterface::ThreadSignalCommandResetClock()
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		mbDiskThreadCommandResetClock = TRUE;
		ResetEvent(mevtDiskClocksDone);
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
		ResetEvent(mevtDiskClocksDone);
		SetEvent(mevtDiskCommand);
	}
	LeaveCriticalSection(&mcrtDisk);	
}

void DiskInterface::ThreadSignalCommandClose()
{
	EnterCriticalSection(&mcrtDisk);
	if (!mbDiskThreadHasQuit)
	{
		mbDiskThreadCommandQuit = true;
		ResetEvent(mevtDiskClocksDone);
		SetEvent(mevtDiskCommand);
	}
	LeaveCriticalSection(&mcrtDisk);
}

void DiskInterface::CloseDiskThread()
{
	if (mhThread)
	{
		WaitThreadReady();
		ThreadSignalCommandClose();

		WaitForMultipleObjects(1, &mhThread, TRUE, INFINITE);
		CloseHandle(mhThread);
		
		mhThread = 0;
	}

	if (mevtDiskClocksDone)
	{
		CloseHandle(mevtDiskClocksDone);
		mevtDiskClocksDone = 0;
	}
	
	if (mevtDiskCommand)
	{
		CloseHandle(mevtDiskCommand);
		DeleteCriticalSection(&mcrtDisk);
		mevtDiskCommand = 0;
	}
}

DWORD WINAPI  DiskInterface::DiskThreadProc( LPVOID lpParam )
{
DiskInterface *pDisk;
DWORD r;

	pDisk = (DiskInterface *)lpParam;
	r = pDisk->DiskThreadProc();
	SetEvent(pDisk->mevtDiskClocksDone);
	pDisk->mbDiskThreadHasQuit = true;
	return r;
}

DWORD DiskInterface::DiskThreadProc()
{
DWORD r;
ICLK clk;
bool bQuit;
bool bCommand;

	r = WAIT_OBJECT_0;
	bCommand = false;
	bQuit = false;
	SetEvent(mevtDiskClocksDone);
	while (!bQuit)
	{
		r = WaitForSingleObject(mevtDiskCommand, INFINITE);
		if ( r == WAIT_OBJECT_0)
		{
			while (!bQuit)
			{
				EnterCriticalSection(&mcrtDisk);

				if (mbDiskThreadCommandQuit)
				{
					bQuit = true;
					ResetEvent(mevtDiskCommand);
					SetEvent(mevtDiskClocksDone);
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
					ResetEvent(mevtDiskCommand);
					SetEvent(mevtDiskClocksDone);
					LeaveCriticalSection(&mcrtDisk);
					break;
				}
				LeaveCriticalSection(&mcrtDisk);

				ExecuteAllPendingDiskCpuClocks();
				ExecutePALClock(clk);
			}
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
int i;

	CloseDiskThread();

	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		if (m_rawTrackData[i])
			GlobalFree(m_rawTrackData[i]);

		m_rawTrackData[i] = 0;
	}

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
  	if (m_currentTrackNumber < (G64_MAX_TRACKS-1))
	{
		m_currentTrackNumber++;
		m_lastHeadStepDir = 1;
		m_lastHeadStepPosition = (m_lastHeadStepPosition + 1) & 3;
		m_headStepClock = 5000;
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
		m_currentTrackNumber--;
		m_lastHeadStepDir = -1;
		m_lastHeadStepPosition = (m_lastHeadStepPosition - 1) & 3;
		m_headStepClock = 5000;
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

	if (clocks==0)
		return;	
	clockDivider1_UE7 = m_clockDivider1_UE7;
	clockDivider2_UF4 = m_clockDivider2_UF4;
	bandpos = m_totalclocks_UE7 & 0xf;
	writeClock = bandpos;
	m_lastPulseTime +=clocks;
	m_totalclocks_UE7+=clocks;

	if (bStartWithPulse)
	{
		//Reset the clockdividers because a disk pulse has occurred.
		m_lastPulseTime = 0;
		clockDivider1_UE7 = speed;
		clockDivider2_UF4 = 0;

		byteReady=1;
		if ((m_frameCounter_UC3 & 7) == 7)
		{
			if (m_d64_soe_enable!=0)
			{
				byteReady = 0;
			}
			if (m_busByteReadySignal != byteReady)
			{
				m_busByteReadySignal = byteReady;
				if (writeClock < 7 || m_lastPulseTime==clocks)//7
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

	if (m_lastPulseTime > 300 && m_d64_write_enable == 0)
	{
		if (rand() < 5000)
		{
			clockDivider1_UE7 = speed;
			clockDivider2_UF4 = 0;			
		}
	}

	m_writeStream = 0;
	clockDivider1_UE7 += clocks;

	for (int c=0 ; c==0 || clockDivider1_UE7 >= 16 ; c++)
	{
		if (clockDivider1_UE7 >= 16)
		{
			if (c==0)
				writeClock = (bandpos + 15 + clocks - clockDivider1_UE7) & 0xf;
			else
				writeClock = (writeClock - speed) & 0xf;

			clockDivider1_UE7 = clockDivider1_UE7 + speed - 16;
			prevClockDivider2 = clockDivider2_UF4;
			clockDivider2_UF4 =  (clockDivider2_UF4 + 1) & 0xf;
	
			bUF4_QB_rising =  ((clockDivider2_UF4 & 2) != 0 && (prevClockDivider2 & 2) == 0);
			if (bUF4_QB_rising)
			{
				//rising QB
				if ((via2.delay & (VIACA1Trans0 | VIACA1Trans1)))//Make sure pending transitions of the byte ready signal to via2.ca1 have been seen before updating the bus byte.
					via2.ExecuteCycle(CurrentClock - 1);
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
				{//writing a 1
					m_writeStream = writeClock + 1;
				}
				m_shifterWriter_UD3 <<= 1;
				
				m_shifterReader_UD2 <<= 1;
				if ((clockDivider2_UF4 & 0xc) == 0)
				{//reading a 1
					if (m_headStepClock==0)
					{
						m_shifterReader_UD2 |= 1;
					}
				}
				if (!m_d64_sync)
				{
					m_frameCounter_UC3 =  (m_frameCounter_UC3 + 1) & 0xf;
				}

				m_debugFrameCounter = (m_debugFrameCounter + 1) & 0xf;
				bit8 oldSync = m_d64_sync;

				m_d64_sync = ((m_d64_write_enable==0) && ((m_shifterReader_UD2 & 0x3ff) == 0x3ff)) || (m_d64_forcesync!=0);

				if (m_d64_sync)
					m_frameCounter_UC3 = 0;
				
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
			if ((m_d64_soe_enable!=0) && (clockDivider2_UF4 & 2)==0)
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
bit8 bitTimeDelayed;
ICLKS clocks;
bit8 speed;
bit8 bMotorRun = m_bDiskMotorOn;

	speed = m_clockDivider1_UE7_Reload;
	clocks = (ICLKS)(sysclock - CurrentClock);
	while (clocks-- > 0)
	{
		CurrentClock++;
		bitTime = 0;
		bitTimeDelayed = 0;
		if (m_motorOffClock)
		{
			m_motorOffClock--;
			bMotorRun = true;
		}
		if (m_headStepClock)
		{
			m_headStepClock--;
		}
		m_counterStartPulseFilter += DISK16CELLSPERCLOCK;

		if (bMotorRun && m_d64_diskchange_counter==0)
		{
			if (m_d64_write_enable==0)
			{				
				if (m_diskLoaded)
				{
					bitTime = GetDisk16(m_currentTrackNumber, m_currentHeadIndex);					
				}								
			}
			else
			{
				bitTimeDelayed = 0;
				ClockDividerAdd(DISK16CELLSPERCLOCK, speed, false);
				if (m_d64_protectOff!=0 && m_diskLoaded)
				{
					PutDisk16(m_currentTrackNumber, m_currentHeadIndex, m_writeStream);
				}
			}
			MotorDisk16(m_currentTrackNumber, &m_currentHeadIndex);
		}

		if (m_bPendingPulse && m_counterStartPulseFilter > DISKHEADFILTERWIDTH && m_bPulseState != m_bLastPulseState)
		{
			if (m_counterStartPulseFilter - DISKHEADFILTERWIDTH <= DISK16CELLSPERCLOCK)
			{
				bitTimeDelayed = (bit8)(DISK16CELLSPERCLOCK + DISKHEADFILTERWIDTH - m_counterStartPulseFilter + 1);
			}
		}

		if (bitTimeDelayed != 0 && (bitTime == 0 || bitTimeDelayed < bitTime))
		{
			m_bPendingPulse = false;
			m_bLastPulseState = m_bPulseState;
		}
		else
		{
			bitTimeDelayed = 0;
		}

		if (bitTime != 0)
		{
			m_bPendingPulse = true;
			m_bPulseState = !m_bPulseState;
			m_counterStartPulseFilter = DISK16CELLSPERCLOCK - bitTime + 1;
		}

		if (bitTimeDelayed != 0 && m_d64_write_enable == 0)
		{
			bitTimeDelayed--;
			ClockDividerAdd(bitTimeDelayed, speed, false);

			ClockDividerAdd(DISK16CELLSPERCLOCK-bitTimeDelayed, speed, true);
		}
		else
		{
			ClockDividerAdd(DISK16CELLSPERCLOCK, speed, false);
		}
	}
}

bit8 DiskInterface::GetDisk16(bit8 trackNumber, bit32 headIndex)
{
	return m_rawTrackData[trackNumber][headIndex];
}

void DiskInterface::PutDisk16(bit8 trackNumber, bit32 headIndex, bit8 data)
{
	m_rawTrackData[trackNumber][headIndex] = data;
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
		m_changing_c64_serialbus_diskview_diskclock = ClockBehindNear;

	if ((ICLKS)(CurrentClock - m_driveWriteChangeClock) >= CLOCKSYNCBAND_FAR)
		m_driveWriteChangeClock = ClockBehindNear;

	if ((ICLKS)(CurrentClock - m_busDataUpdateClock) >= CLOCKSYNCBAND_FAR)
		m_busDataUpdateClock = ClockBehindNear;
	
	if (m_counterStartPulseFilter > CLOCKSYNCBAND_FAR)
	{
		m_counterStartPulseFilter = DISKHEADFILTERWIDTH + DISK16CELLSPERCLOCK + 1;
	}
	cpu.PreventClockOverflow();
}

void DiskInterface::LoadImageBits(GCRDISK *dsk)
{
int i;
	WaitThreadReady();
	m_d64TrackCount = dsk->m_d64TrackCount;
	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		memcpy_s(m_rawTrackData[i], DISK_RAW_TRACK_SIZE, dsk->m_rawTrackData[i], DISK_RAW_TRACK_SIZE);
	}
}

void DiskInterface::SaveImageBits(GCRDISK *dsk)
{
int i;

	WaitThreadReady();
	dsk->m_d64TrackCount = m_d64TrackCount;

	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		memcpy_s(dsk->m_rawTrackData[i], DISK_RAW_TRACK_SIZE, m_rawTrackData[i], DISK_RAW_TRACK_SIZE);
	}
}

void DiskInterface::SetDiskLoaded()
{
	WaitThreadReady();
	if (m_diskLoaded)
		m_d64_diskchange_counter=DISKCHANGE3;
	else
		m_d64_diskchange_counter=DISKCHANGE1;
	m_diskLoaded = 1;
}

void DiskInterface::RemoveDisk()
{
	WaitThreadReady();
	if (m_diskLoaded)
	{
		m_diskLoaded = 0;
		m_d64_diskchange_counter = DISKCHANGE1;
	}
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

void DiskInterface::GetState(SsDiskInterface &state)
{
	ZeroMemory(&state, sizeof(state));
	state.CurrentClock = CurrentClock;
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
	state.CurrentPALClock = CurrentPALClock;
	state.m_changing_c64_serialbus_diskview_diskclock = m_changing_c64_serialbus_diskview_diskclock;
	state.m_driveWriteChangeClock = m_driveWriteChangeClock;
	state.m_motorOffClock = m_motorOffClock;
	state.m_headStepClock = m_headStepClock;
	state.m_pendingclocks = m_pendingclocks;
	state.m_DiskThreadCommandedPALClock = m_DiskThreadCommandedPALClock;
	state.m_DiskThreadCurrentPALClock = m_DiskThreadCurrentPALClock;
	state.m_d64_diskwritebyte = m_d64_diskwritebyte;
	state.m_bDiskMotorOn = m_bDiskMotorOn;
	state.m_bDriveLedOn = m_bDriveLedOn;
	state.m_bDriveWriteWasOn = m_bDriveWriteWasOn;
	state.m_diskLoaded = m_diskLoaded;
	state.m_currentHeadIndex = m_currentHeadIndex % DISK_RAW_TRACK_SIZE;
	state.m_currentTrackNumber = m_currentTrackNumber % G64_MAX_TRACKS;
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
	state.m_diskd64clk_xf = m_diskd64clk_xf;
	//*m_rawTrackData[G64_MAX_TRACKS];
}

void DiskInterface::SetState(const SsDiskInterface &state)
{
	CurrentClock = state.CurrentClock;
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
	CurrentPALClock = state.CurrentPALClock;
	m_changing_c64_serialbus_diskview_diskclock = state.m_changing_c64_serialbus_diskview_diskclock;
	m_driveWriteChangeClock = state.m_driveWriteChangeClock;
	m_motorOffClock = state.m_motorOffClock;
	m_headStepClock = state.m_headStepClock;
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
	m_diskd64clk_xf = state.m_diskd64clk_xf;
}
