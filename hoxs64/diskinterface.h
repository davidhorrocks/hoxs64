#pragma once
#include <string>
#include "errormsg.h"
#define DISKCHANGE3 30
#define DISKCHANGE2 20
#define DISKCHANGE1 10

class CIA2;

class DiskInterface : public IRegister, public IMonitorDisk, public ErrorMsg
{
public:
	DiskInterface();
	~DiskInterface();
	DiskInterface(const DiskInterface&) = delete;
	DiskInterface& operator=(const DiskInterface&) = delete;
	DiskInterface(DiskInterface&&) = delete;
	DiskInterface& operator=(DiskInterface&&) = delete;

	static DWORD WINAPI DiskThreadProc(LPVOID lpParam);
	HRESULT Init(CAppStatus *appStatus, IC64 *pIC64, IC64Event *pIC64Event, IBreakpointManager *pIBreakpointManager, const wchar_t *pszAppDirectory);
	HRESULT InitDiskThread();
	DWORD DiskThreadProc();
	bool WaitThreadReady();
	void ThreadSignalCommandExecuteClock(ICLK PalClock);
	void ThreadSignalExecute();
	void ThreadSignalCommandResetClock();
	void ThreadSignalQuit();
	void ThreadSignalPause();
	void CloseDiskThread() noexcept;
	void Cleanup() noexcept;
	void LoadMoveP64Image(GCRDISK *);
	void SaveCopyP64Image(GCRDISK *);
	void SetDiskLoaded(bool immediately);
	void RemoveDisk();
	void InitReset(ICLK sysclock, bool poweronreset);

	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	void GetState(SsDiskInterfaceV2 &state);
	void SetState(const SsDiskInterfaceV2 &state);
	void PrepareP64Head(unsigned int trackNumber);
	static void UpgradeStateV0ToV1(const SsDiskInterfaceV0 &in, SsDiskInterfaceV1 &out);
	static void UpgradeStateV1ToV2(const SsDiskInterfaceV1 &in, SsDiskInterfaceV2 &out);

	//IMonitorDisk
	virtual bit8 GetHalfTrackIndex();

	void MotorDisk16(bit8 m_currentTrackNumber, bit32 *headIndex);
	void MoveHead(bit8 trackNo);
	bool StepHeadIn();
	bool StepHeadOut();
	void StepHeadAuto();
	void D64_serial_write(bit8 c64_serialbus);
	bool Get_ATN(bit8& outputPortB);
	void D64_Attention_Change();
	void D64_DiskProtect(bool bOn);

	void C64SerialBusChange(ICLK palclock, bit8 c64_serialbus);
	bit8 GetC64SerialBusDiskView(ICLK diskclock);
	void PreventClockOverflow();
	bit8 GetProtectSensorState();
	void ClockDividerAdd(bit8 clocks, bit8 speed, bool bStartWithPulse);
	void SpinDisk(ICLK sysclock);
	void StartMotor();
	void StopMotor();
	bool MotorSlowDownEnv();
	void ExecutePALClock(ICLK palclock);
	void ExecuteOnePendingDiskCpuClock();
	void AccumulatePendingDiskCpuClocksToPalClock(ICLK palclock);
	void ExecuteAllPendingDiskCpuClocks();
	void SetRamPattern();
	bit8 GetBusDataByte();
	bool firstboot = false;
	volatile bit8 m_d64_serialbus = 0;
	bit8 m_d64_dipswitch = 0;
	bit8 m_d64_protectOff = 0;
	bit8 m_d64_sync = 0;
	bit8 m_d64_forcesync = 0;
	bit8 m_d64_soe_enable = 0;
	bit8 m_d64_write_enable = 0;
	bit8 m_d64_diskchange_counter = 0;
	bit8 m_d64TrackCount = 0;
	volatile bit8 m_c64_serialbus_diskview = 0;
	volatile bit8 m_c64_serialbus_diskview_next = 0;
	ICLKS m_diskChangeCounter = 0;
	ICLK CurrentPALClock = 0;
	ICLK m_changing_c64_serialbus_diskview_diskclock = 0;
	ICLK m_driveWriteChangeClock = 0;
	ICLK m_motorOffClock = 0;
	ICLK m_headStepClock = 0;
	ICLK m_headStepClockUp = 0;
	ICLK m_pendingclocks = 0;
	volatile ICLK m_DiskThreadCommandedPALClock = 0;
	volatile ICLK m_DiskThreadCurrentPALClock = 0;
	bit8 m_d64_diskwritebyte = 0;
	volatile bool m_bDiskMotorOn = false;
	volatile bool m_bDriveLedOn = false;
	volatile bool m_bDriveWriteWasOn = false;
	bit8 m_diskLoaded = 0;
	bit32 m_currentHeadIndex = 0;
	bit8 m_currentTrackNumber = 0;
	bit8 m_previousTrackNumber = 0;
	bit8s m_lastHeadStepDir = 0;
	bit8 m_lastHeadStepPosition = 0;
	bit8 m_shifterWriter_UD3 = 0; //74LS165 UD3
	bit16 m_shifterReader_UD2 = 0; //74LS164 UD2 Two flops make the shifter effectively 10 bits. The lower 8 bits are read by VIA2 port A.
	ICLK m_busDataUpdateClock = 0;
	bit16 m_busByteReadyPreviousData = 0;
	bool m_busByteReadySignal = false;
	bit8 m_frameCounter_UC3 = 0; //74LS191 UC3
	bit8 m_debugFrameCounter = 0;
	bit8 m_clockDivider1_UE7_Reload = 0;
	bit8 m_clockDivider1_UE7 = 0; //74LS193 UE7 16MHz
	bit8 m_clockDivider2_UF4 = 0; //74LS193 UF4
	bit8 m_writeStream = 0;
	bit32 m_totalclocks_UE7 = 0;
	bit32 m_lastPulseTime = 0;
	bit32 m_lastWeakPulseTime = 0;
	bit32 m_counterStartPulseFilter = 0;
	bit32s m_nextP64PulsePosition = 0;
	bool m_bPendingPulse = false;
	bool m_bPulseState = false;
	bool m_bLastPulseState = false;
	TP64Image m_P64Image;	
	bit8 *m_pD1541_ram = nullptr;
	bit8 *m_pD1541_rom = nullptr;
	bit8 *m_pIndexedD1541_rom = nullptr;
	__int64 m_diskd64clk_xf = 0;

	CPUDisk cpu;
	VIA1 via1;
	VIA2 via2;
	IC64Event *pIC64Event = nullptr;
	IC64 *pIC64 = nullptr;
	CAppStatus *appStatus = nullptr;

	static const long DISKCLOCKSPERSECOND = 1000000;
	static const long DISKCHANGECLOCKRELOAD = 20000;
	static const __int64 Disk64clk_dy1 = 2*985248;
	static const __int64 Disk64clk_dy2 = 2*1000000;
private:
	std::wstring wsAppDirectory;
	HANDLE mhThread = 0;
	DWORD mThreadId = 0;
	HANDLE mevtDiskReady = 0;
	HANDLE mevtDiskCommand = 0;
	HANDLE mevtDiskQuit = 0;
	HANDLE mevtDiskPause = 0;
	volatile bool mbDiskThreadCommandQuit = false;
	volatile bool mbDiskThreadCommandResetClock = false;
	volatile bool mbDiskThreadHasQuit = false;
	volatile bool mbDiskThreadPause = false;
	CRITICAL_SECTION mcrtDisk = {};
	HANDLE m_waitCommand[3] = {};
	HANDLE m_waitReady[2] = {};
	random_device rd;
	mt19937 randengine_drive;
	uniform_int_distribution<int> dist_motor_slow;
	uniform_int_distribution<int> dist_weakbit;
};
