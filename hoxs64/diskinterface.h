#ifndef __DISKINTERFACE_H__
#define __DISKINTERFACE_H__

#define DISKCHANGE3 30
#define DISKCHANGE2 20
#define DISKCHANGE1 10

class CIA2;

class DiskInterface : public IRegister, public IMonitorDisk, public ErrorMsg
{
public:
	DiskInterface();
	~DiskInterface();
	HRESULT Init(CAppStatus *appStatus, IC64 *pIC64, IC64Event *pIC64Event, IBreakpointManager *pIBreakpointManager, TCHAR *szAppDirectory);
	HRESULT InitDiskThread();
	static DWORD WINAPI DiskThreadProc( LPVOID lpParam );
	DWORD DiskThreadProc();

	bool WaitThreadReady();
	void ThreadSignalCommandExecuteClock(ICLK PalClock);
	void ThreadSignalExecute();
	void ThreadSignalCommandResetClock();
	void ThreadSignalQuit();
	void ThreadSignalPause();
	void CloseDiskThread();

	void Cleanup();
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

	void PutDisk16(bit8 trackNumber, bit32 headIndex, bit8 data);
	void MotorDisk16(bit8 m_currentTrackNumber, bit32 *headIndex);
	void MoveHead(bit8 trackNo);
	bool StepHeadIn();
	bool StepHeadOut();
	void StepHeadAuto();
	void D64_serial_write(bit8 c64_serialbus);
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
	bool firstboot;
	volatile bit8 m_d64_serialbus;
	bit8 m_d64_dipswitch;
	bit8 m_d64_protectOff;
	bit8 m_d64_sync;
	bit8 m_d64_forcesync;
	bit8 m_d64_soe_enable;
	bit8 m_d64_write_enable;
	bit8 m_d64_diskchange_counter;
	bit8 m_d64TrackCount;
	volatile bit8 m_c64_serialbus_diskview;
	volatile bit8 m_c64_serialbus_diskview_next;
	ICLKS m_diskChangeCounter;
	ICLK CurrentPALClock;
	ICLK m_changing_c64_serialbus_diskview_diskclock;
	ICLK m_driveWriteChangeClock;
	ICLK m_motorOffClock;
	ICLK m_headStepClock;
	ICLK m_headStepClockUp;
	ICLK m_pendingclocks;
	volatile ICLK m_DiskThreadCommandedPALClock;
	volatile ICLK m_DiskThreadCurrentPALClock;
	bit8 m_d64_diskwritebyte;
	bool m_bDiskMotorOn;
	bool m_bDriveLedOn;
	bool m_bDriveWriteWasOn;
	bit8 m_diskLoaded;
	bit32 m_currentHeadIndex;
	bit8 m_currentTrackNumber;
	bit8 m_previousTrackNumber;
	bit8s m_lastHeadStepDir;
	bit8 m_lastHeadStepPosition;
	bit8 m_shifterWriter_UD3; //74LS165 UD3
	bit16 m_shifterReader_UD2; //74LS164 UD2 Two flops make the shifter effectively 10 bits. The lower 8 bits are read by VIA2 port A.
	ICLK m_busDataUpdateClock;
	bit16 m_busByteReadyPreviousData;
	bit8 m_busByteReadySignal;
	bit8 m_frameCounter_UC3; //74LS191 UC3
	bit8 m_debugFrameCounter;
	bit8 m_clockDivider1_UE7_Reload;
	bit8 m_clockDivider1_UE7; //74LS193 UE7 16MHz
	bit8 m_clockDivider2_UF4; //74LS193 UF4
	bit8 m_writeStream;
	bit32 m_totalclocks_UE7;
	bit32 m_lastPulseTime;
	bit32 m_lastWeakPulseTime;
	bit32 m_counterStartPulseFilter;
	bit32s m_nextP64PulsePosition;
	bool m_bPendingPulse;
	bool m_bPulseState;
	bool m_bLastPulseState;
	TP64Image m_P64Image;	
	bit8 *m_pD1541_ram;
	bit8 *m_pD1541_rom;
	bit8 *m_pIndexedD1541_rom;

	CPUDisk cpu;
	VIA1 via1;
	VIA2 via2;

	IC64Event *pIC64Event;
	IC64 *pIC64;

	CAppStatus *appStatus;

	static const long DISKCLOCKSPERSECOND = 1000000;
	static const long DISKCHANGECLOCKRELOAD = 20000;
	static const __int64 Disk64clk_dy1 = 2*985248;
	static const __int64 Disk64clk_dy2 = 2*1000000;
	__int64 m_diskd64clk_xf;
private:
	TCHAR m_szAppDirectory[MAX_PATH+1];
	HANDLE mhThread;
	DWORD mThreadId;
	HANDLE mevtDiskReady;
	HANDLE mevtDiskCommand;
	HANDLE mevtDiskQuit;
	HANDLE mevtDiskPause;
	volatile bool mbDiskThreadCommandQuit;
	volatile bool mbDiskThreadCommandResetClock;
	volatile bool mbDiskThreadHasQuit;
	volatile bool mbDiskThreadPause;
	CRITICAL_SECTION mcrtDisk;
	HANDLE m_waitCommand[3];
	HANDLE m_waitReady[2];
	std::random_device rd;
	std::mt19937 randengine_drive;
	std::uniform_int_distribution<int> dist_motor_slow;
	std::uniform_int_distribution<int> dist_weakbit;
};


#endif