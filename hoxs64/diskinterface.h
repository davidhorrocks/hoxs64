#ifndef __DISKINTERFACE_H__
#define __DISKINTERFACE_H__

#define DISKCHANGE3 30
#define DISKCHANGE2 20
#define DISKCHANGE1 10

class CIA2;

class DiskInterface : public IRegister, public ErrorMsg
{
public:
	DiskInterface();
	~DiskInterface();
	HRESULT Init(CConfig *cfg, CAppStatus *appStatus, IC64Event *pIC64Event, TCHAR *szAppDirectory);
	HRESULT InitDiskThread();
	static DWORD WINAPI DiskThreadProc( LPVOID lpParam );
	DWORD DiskThreadProc();

	void WaitThreadReady();
	void ThreadSignalCommandExecuteClock(ICLK PalClock);
	void ThreadSignalCommandResetClock();
	void ThreadSignalCommandClose();
	void CloseDiskThread();

	void Cleanup();
	void LoadImageBits(GCRDISK *);
	void SaveImageBits(GCRDISK *);
	void SetDiskLoaded();
	void RemoveDisk();

	//IRegister
	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	bit8 GetDisk16(bit8 m_currentTrackNumber, bit32 headIndex);
	void PutDisk16(bit8 trackNumber, bit32 headIndex, bit8 data);
	void MotorDisk16(bit8 m_currentTrackNumber, bit32 *headIndex);
	void MoveHead(bit8 trackNo);
	bool StepHeadIn();
	bool StepHeadOut();
	void StepHeadAuto();
	void D64_serial_write(bit8 c64_serialbus);
	void D64_Attention_Change();
	void D64_DiskProtect(BOOL);

	void C64SerialBusChange(ICLK palclock, bit8 c64_serialbus);
	bit8 GetC64SerialBusDiskView(ICLK diskclock);
	void PreventClockOverflow();

	bit8 GetProtectSensorState();

	void ClockDividerAdd(bit8 clocks, bit8 speed);
	void SpinDisk(ICLK sysclock);

	void ExecutePALClock(ICLK palclock);
	void ExecuteOnePendingDiskCpuClock();
	void AccumulatePendingDiskCpuClocksToPalClock(ICLK palclock);
	void ExecuteAllPendingDiskCpuClocks();

	void SetRamPattern();
	volatile bit8 m_d64_serialbus;
	bit8 m_d64_dipswitch;
	bit8 m_d64_protectOff;
	bit8 m_d64_sync;
	bit8 m_d64_forcesync;
	bit8 m_d64_soe_enable;
	bit8 m_d64_write_enable;
	bit8 mi_d64_diskchange;
	bit8 m_d64TrackCount;
	volatile bit8 m_c64_serialbus_diskview;
	volatile bit8 m_c64_serialbus_diskview_next;
	ICLKS m_diskChangeCounter;
	ICLK CurrentPALClock;
	ICLK m_changing_c64_serialbus_diskview_diskclock;
	ICLK m_driveWriteChangeClock;
	ICLK m_motorOffClock;
	ICLK m_headStepClock;
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
	char m_lastHeadStepDir;
	bit8 m_lastHeadStepPosition;
	bit8 m_shifterWriter; //74LS165 UD3
	bit16 m_shifterReader; //74LS164 UD2 2 flops make the shifter effectively 10bits. Only lower 8 bits are read by the VIA
	bit8 m_frameCounter; //74LS191 UC3
	bit8 m_debugFrameCounter;
	bit8 m_clockDividerReload;
	bit8 m_clockDivider1; //74LS193 UE7 16MHz
	bit8 m_clockDivider2; //74LS193 UF4

	bit8 m_writeStream;
	bit16 m_extraClock;
	bit32 m_lastOne;
	bit32 m_lastGap;

	bit8 *m_rawTrackData[G64_MAX_TRACKS];

	bit8 *m_pD1541_ram;
	bit8 *m_pD1541_rom;
	bit8 *m_pIndexedD1541_rom;

	CPUDisk cpu;
	VIA1 via1;
	VIA2 via2;

	IC64Event *pIC64Event;

	CConfig *cfg;
	CAppStatus *appStatus;

	static const long DISKCLOCKSPERSECOND = 1000000;
	static const long DISKCHANGECLOCKRELOAD = 20000;
	//long Disk64clk_dy1 = 2*30789;
	//long Disk64clk_dy2 = 2*31250;
	static const __int64 Disk64clk_dy1 = 2*985248;
	static const __int64 Disk64clk_dy2 = 2*1000000;
	__int64 m_diskd64clk_xf;
private:
	TCHAR m_szAppDirectory[MAX_PATH+1];
	HANDLE mhThread;
	DWORD mThreadId;
	HANDLE mevtDiskClocksDone;
	HANDLE mevtDiskCommand;
	volatile bool mbDiskThreadCommandQuit;
	volatile bool mbDiskThreadCommandResetClock;
	volatile bool mbDiskThreadHasQuit;
	CRITICAL_SECTION mcrtDisk;
};


#endif