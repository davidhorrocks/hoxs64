#pragma once
#include <windows.h>
#include <tchar.h>
#include "boost2005.h"
#include "util.h"
#include "bpenum.h"

typedef enum tagMemoryType
{
	MT_DEFAULT,
	MT_RAM,
	MT_IO,
	MT_CHARGEN,
	MT_BASIC,
	MT_KERNAL,
	MT_ROML,
	MT_ROMH,
	MT_ROML_ULTIMAX,
	MT_ROMH_ULTIMAX,
	MT_EXRAM,
	MT_NOTCONNECTED
} MEM_TYPE;

class IRegister
{
public:
	IRegister() noexcept : CurrentClock(0) {};
	virtual ~IRegister() noexcept {};
	IRegister(const IRegister&) = delete;
	IRegister& operator=(const IRegister&) = delete;
	IRegister(IRegister&&) = delete;
	IRegister& operator=(IRegister&&) = delete;

	virtual void Reset(ICLK sysclock, bool poweronreset) = 0;
	virtual void ExecuteCycle(ICLK sysclock) = 0;
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock) = 0;
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data) = 0;
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) = 0;
	virtual ICLK GetCurrentClock() = 0;
	virtual void SetCurrentClock(ICLK sysclock) = 0;
	ICLK CurrentClock;
};

class IC6502 : public virtual IRegister
{
public:
	virtual bit16 GetPC() = 0;
	virtual void AddClockDelay() = 0;
	virtual void SetRdyLow(ICLK sysclock) = 0;
	virtual void SetRdyHigh(ICLK sysclock) = 0;
	virtual bool IsDebug() = 0;
};

class IC6510 : public virtual IC6502
{
public:
	virtual void Reset6510(ICLK sysclock, bool poweronreset) = 0;
	virtual ICLK Get6510CurrentClock() = 0;
	virtual void Set6510CurrentClock(ICLK sysclock) = 0;
	virtual void Set_VIC_IRQ(ICLK sysclock) = 0;
	virtual void Clear_VIC_IRQ() = 0;
	virtual void Set_CIA_IRQ(ICLK sysclock) = 0;
	virtual void Clear_CIA_IRQ() = 0;
	virtual void Set_CRT_IRQ(ICLK sysclock) = 0;
	virtual void Clear_CRT_IRQ() = 0;
	virtual void Set_CIA_NMI(ICLK sysclock) = 0;
	virtual void Clear_CIA_NMI() = 0;
	virtual void Set_CRT_NMI(ICLK sysclock) = 0;
	virtual void Clear_CRT_NMI() = 0;
	virtual void SetVicRdyHigh(ICLK sysclock) = 0;
	virtual void SetVicRdyLow(ICLK sysclock) = 0;
	virtual void SetCartridgeRdyHigh(ICLK sysclock) = 0;
	virtual void SetCartridgeRdyLow(ICLK sysclock) = 0;
	virtual bit8 Get_IRQ_VIC() = 0;
	virtual bit8 Get_IRQ_CIA() = 0;
	virtual bit8 Get_IRQ_CRT() = 0;
	virtual bit8 Get_NMI_CIA() = 0;
	virtual bit8 Get_NMI_CRT() = 0;
	virtual void ConfigureMemoryMap() = 0;
	virtual bool IsVicNotifyCpuWriteCycle() = 0;
	virtual bool ReadByte(bit16 address, bit8& data) = 0;
	virtual void WriteByte(bit16 address, bit8 data) = 0;
	virtual bit8 ReadDmaByte(bit16 address) = 0;
	virtual void WriteDmaByte(bit16 address, bit8 data) = 0;
	virtual bit8 MonReadByte(bit16 address, int memorymap) = 0;
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap) = 0;
	virtual MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap) = 0;
	virtual MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap) = 0;
	virtual MEM_TYPE GetCurrentCpuMmuReadMemoryType(bit16 address) = 0;
	virtual MEM_TYPE GetCurrentCpuMmuWriteMemoryType(bit16 address) = 0;
};

class ISid64 : public IRegister
{
public:
	int NumberOfExtraSidChips;
	bit16 AddressOfSecondSID;
	bit16 AddressOfThirdSID;
	bit16 AddressOfFourthSID;
	bit16 AddressOfFifthSID;
	bit16 AddressOfSixthSID;
	bit16 AddressOfSeventhSID;
	bit16 AddressOfEighthSID;
	bool SidAtDE00toDF00;
};

class ISid : public IRegister
{
public:
	virtual bit8 Get_PotX() = 0;
	virtual void Set_PotX(ICLK sysclock, bit8 data) = 0;
	virtual bit8 Get_PotY() = 0;
	virtual void Set_PotY(ICLK sysclock, bit8 data) = 0;
};

class ICia1
{
public:
	virtual void EnableInput(bool enabled) = 0;
	virtual bit8 Get_PotAX() = 0;
	virtual bit8 Get_PotAY() = 0;
	virtual bit8 Get_PotBX() = 0;
	virtual bit8 Get_PotBY() = 0;
};

class ITape
{
public:
	virtual void SetMotorWrite(bool motor, bit8 write) = 0;
	virtual void PressPlay() = 0;
	virtual void PressStop() = 0;
	virtual void Rewind() = 0;
	virtual void Eject() = 0;
};

class ITapeEvent
{
public:
	virtual void Pulse(ICLK sysclock) = 0;
	virtual void EndOfTape(ICLK sysclock) = 0;
};

struct TraceStepInfo
{
	bool Enable = false;
	bit64 StepCount = 0;
	DBGSYM::CliCpuMode::CliCpuMode CpuMode = DBGSYM::CliCpuMode::C64;
};

class IBreakpointManager
{
public:
	IBreakpointManager() = default;
	virtual ~IBreakpointManager() noexcept {};
	IBreakpointManager(const IBreakpointManager&) = delete;
	IBreakpointManager& operator=(const IBreakpointManager&) = delete;
	IBreakpointManager(IBreakpointManager&&) = delete;
	IBreakpointManager& operator=(IBreakpointManager&&) = delete;

	virtual bool BM_SetBreakpoint(const BreakpointItem& bp) = 0;
	virtual bool BM_GetBreakpoint(const BreakpointItem& key, BreakpointItem& bp) = 0;
	virtual void BM_DeleteBreakpoint(const BreakpointItem& key) = 0;
	virtual void BM_EnableBreakpoint(const BreakpointItem& key) = 0;
	virtual void BM_DisableBreakpoint(const BreakpointItem& key) = 0;
	virtual void BM_EnableAllBreakpoints() = 0;
	virtual void BM_DisableAllBreakpoints() = 0;
	virtual void BM_DeleteAllBreakpoints() = 0;
	virtual IEnumBreakpointItem* BM_CreateEnumBreakpointItem() = 0;
};

struct CPUState
{
	bit8 A;
	bit8 X;
	bit8 Y;
	bit16 PC;
	bit16 PC_CurrentOpcode;
	bit8 SP;
	bit8 Flags;
	bit8 PortDdr;
	bit8 PortDataStored;
	bit8 processor_interrupt;
	bit16 cpu_sequence;
	bit8 RDY;
	ICLK clock;
	bit8 opcode;
	int cycle;
	bool IsInterruptInstruction;
};

class IMonitorCpu
{
public:
	virtual int GetCpuId() = 0;

	/*
	Function
	MonReadByte(bit16 address, int memorymap)

	Parameters
	[address]
	The 16 bit address as seen by the CPU

	[memorymap]
	The memory map.	A negative number means use the current memory map as seen by the CPU.
	*/
	virtual bit8 MonReadByte(bit16 address, int memorymap) = 0;
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap) = 0;
	virtual int GetCurrentCpuMmuMemoryMap() = 0;
	virtual MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap) = 0;
	virtual MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap) = 0;
	virtual void GetCpuState(CPUState& state) = 0;
	virtual bool IsBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) = 0;
	virtual void DeleteBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) = 0;
	virtual void EnableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) = 0;
	virtual void DisableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) = 0;
	virtual bool SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) = 0;
	virtual bool GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, BreakpointItem& breakpoint) = 0;
	virtual void SetBreakOnInterruptTaken() = 0;
	virtual void ClearBreakOnInterruptTaken() = 0;
	virtual void SetStepCountBreakpoint(bit64 stepCount) = 0;
	virtual void ClearStepCountBreakpoint() = 0;
	virtual void SetStepOverBreakpoint() = 0;
	virtual void ClearStepOverBreakpoint() = 0;
	virtual void SetStepOutWithRtsRtiPlaTsx() = 0;
	virtual void ClearStepOutWithRtsRtiPlaTsx() = 0;
	virtual void SetStepOutWithRtsRti() = 0;
	virtual void ClearStepOutWithRtsRti() = 0;
	virtual void ClearTemporaryBreakpoints() = 0;
	virtual void SetPC(bit16 address) = 0;
	virtual void SetA(bit8 v) = 0;
	virtual void SetX(bit8 v) = 0;
	virtual void SetY(bit8 v) = 0;
	virtual void SetSR(bit8 v) = 0;
	virtual void SetSP(bit8 v) = 0;
	virtual void SetDdr(bit8 v) = 0;
	virtual void SetData(bit8 v) = 0;
};

class IVic : public IRegister
{
public:
	virtual bool Get_BA() = 0;
	virtual ICLK Get_ClockBALow() = 0;
	virtual ICLK Get_CountBALow() = 0;
	virtual ICLK Get_ClockBAHigh() = 0;
	virtual ICLK Get_CountBAHigh() = 0;
	virtual bit8 GetDExxByte(ICLK sysclock) = 0;
	virtual unsigned int GetFrameCounter() = 0;
	virtual bit16 GetCurrentRasterLine() = 0;
	virtual bit8 GetCurrentRasterCycle() = 0;
	virtual bit16 GetNextRasterLine() = 0;
	virtual bit8 GetNextRasterCycle() = 0;
	virtual void SetLPLine(bit8 lineState) = 0;
	virtual void SetLPLineClk(ICLK sysclock, bit8 lineState) = 0;
	virtual bit8 SpriteDMATurningOn() = 0;
};

class IMonitorVic : public IVic
{
public:
	virtual bool GetBreakpointRasterCompare(int line, int cycle, BreakpointItem& breakpoint) = 0;
	virtual bool SetBreakpointRasterCompare(int line, int cycle, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) = 0;
	virtual int CheckBreakpointRasterCompare(int line, int cycle, bool bHitIt) = 0;
};

class IMonitorDisk
{
public:
	virtual bit8 GetHalfTrackIndex() = 0;
};

class IDefaultCpu
{
public:
	virtual ~IDefaultCpu() = default;
	virtual int GetCpuId() = 0;
	virtual IMonitorCpu* GetCpu() = 0;
};

class IC64BreakEvent
{
public:
	virtual void BreakExecuteCpu64() = 0;
	virtual void BreakExecuteCpuDisk() = 0;
	virtual void BreakVicRasterCompare() = 0;
	virtual void BreakpointChanged() = 0;
	virtual void Reset() = 0;
	virtual void MemoryChanged() = 0;
	virtual void RadixChanged(DBGSYM::MonitorOption::Radix value) = 0;
};

class IMonitor;

class CommandToken
{
public:
	CommandToken();
	~CommandToken();

	void SetTokenClearScreen();
	void SetTokenSelectCpu(DBGSYM::CliCpuMode::CliCpuMode cpumode, bool bViewCurrent);
	void SetTokenShowCpu();
	void SetTokenHelp(LPCTSTR name);
	void SetTokenDisassembly();
	void SetTokenDisassembly(bit16 startaddress);
	void SetTokenDisassembly(bit16 startaddress, bit16 finishaddress);
	void SetTokenError(LPCTSTR pszErrortext);
	void SetTokenAssemble(bit16 address, bit8* pData, unsigned int bufferSize);
	void SetTokenStep(bit64 stepCount);
	void SetTokenReadMemory();
	void SetTokenReadMemory(bit16 startaddress);
	void SetTokenReadMemory(bit16 startaddress, bit16 finishaddress);
	void SetTokenWriteMemory(bit16 address, bit8* pData, unsigned int bufferSize);
	void SetTokenMapMemory(DBGSYM::CliMapMemory::CliMapMemory);
	void SetTokenShowCpuRegisters();
	void SetTokenShowCpu64Registers();
	void SetTokenShowCpuDiskRegisters();
	void SetTokenShowVicRegisters();
	void SetTokenShowCia1Registers();
	void SetTokenShowCia2Registers();
	void SetTokenShowSidRegisters();
	void SetTokenShowVia1Registers();
	void SetTokenShowVia2Registers();

	DBGSYM::CliCommand::CliCommand cmd;

	bit64s stepClocks;
	bit16 startaddress;
	bit16 finishaddress;
	bool bHasStartAddress;
	bool bHasFinishAddress;
	std::basic_string<TCHAR> text;
	DBGSYM::CliCpuMode::CliCpuMode cpumode;
	DBGSYM::CliMapMemory::CliMapMemory mapmemory;
	bit8 buffer[256];
	int dataLength;
	bool bViewCurrent;
};

class IRunCommand;

class ICommandResult
{
public:
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString, int id) = 0;
	virtual HRESULT Quit() = 0;
	virtual bool IsQuit() = 0;
	virtual DWORD WaitLinesTakenOrQuit(DWORD timeout) = 0;
	virtual DWORD WaitAllLinesTakenOrQuit(DWORD timeout) = 0;
	virtual DWORD WaitResultDataTakenOrQuit(DWORD timeout) = 0;
	virtual DWORD WaitDataReady(DWORD timeout) = 0;
	virtual DWORD WaitFinished(DWORD timeout) = 0;
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus() = 0;
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status) = 0;
	virtual void SetHwnd(HWND hWnd) = 0;
	virtual void AddLine(LPCTSTR pszLine) = 0;
	virtual HRESULT GetNextLine(LPCTSTR* ppszLine) = 0;
	virtual void SetDataTaken() = 0;
	virtual void SetAllLinesTaken() = 0;
	virtual size_t CountUnreadLines() = 0;
	virtual void Reset() = 0;
	virtual int GetId() = 0;
	virtual IMonitor* GetMonitor() = 0;
	virtual CommandToken* GetToken() = 0;
	virtual IRunCommand* GetRunCommand() = 0;
	virtual void SetTraceStepCount(const TraceStepInfo& stepInfo) = 0;
	virtual bool GetTraceStepCount(TraceStepInfo& stepInfo) = 0;
};

class IRunCommand
{
public:
	virtual HRESULT Run() = 0;
};

class IMonitor : public IBreakpointManager
{
public:
	virtual void MonitorEventsOn() = 0;
	virtual void MonitorEventsOff() = 0;
	virtual int DisassembleOneInstruction(IMonitorCpu* pMonitorCpu, bit16 address, int memorymap, TCHAR* pAddressText, int cchAddressText, TCHAR* pBytesText, int cchBytesText, TCHAR* pMnemonicText, int cchMnemonicText, bool& isUndoc) = 0;
	virtual int DisassembleBytes(IMonitorCpu* pMonitorCpu, unsigned short address, int memorymap, int count, TCHAR* pBuffer, int cchBuffer) = 0;
	virtual void GetCpuRegisters(IMonitorCpu* pMonitorCpu, TCHAR* pPC_Text, int cchPC_Text, TCHAR* pA_Text, int cchA_Text, TCHAR* pX_Text, int cchX_Text, TCHAR* pY_Text, int cchY_Text, TCHAR* pSR_Text, int cchSR_Text, TCHAR* pSP_Text, int cchSP_Text, TCHAR* pDdr_Text, int cchDdr_Text, TCHAR* pData_Text, int cchData_Text) = 0;
	virtual void GetVicRegisters(TCHAR* pLine_Text, int cchLine_Text, TCHAR* pCycle_Text, int cchCycle_Text) = 0;
	virtual IMonitorCpu* GetMainCpu() = 0;
	virtual IMonitorCpu* GetDiskCpu() = 0;
	virtual IMonitorVic* GetVic() = 0;
	virtual IMonitorDisk* GetDisk() = 0;
	virtual HRESULT ExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, LPTSTR* ppszResults) = 0;
	virtual HRESULT BeginExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, shared_ptr<ICommandResult>* pICommandResult) = 0;
	virtual HRESULT EndExecuteCommandLine(shared_ptr<ICommandResult> pICommandResult) = 0;
	virtual void QuitCommands() = 0;
	virtual void QuitCommand(shared_ptr<ICommandResult> pICommandResult) = 0;
	virtual void RemoveCommand(shared_ptr<ICommandResult> pICommandResult) = 0;
	virtual DBGSYM::MonitorOption::Radix Get_Radix() = 0;
	virtual void Set_Radix(DBGSYM::MonitorOption::Radix value) = 0;
};

class IC64Event : public IC64BreakEvent
{
public:
	virtual void SetBusy(bool isBusy) = 0;
	virtual void DiskMotorLed(bool bOn) = 0;
	virtual void DiskDriveLed(bool bOn) = 0;
	virtual void DiskWriteLed(bool bOn) = 0;
	virtual void ShowErrorBox(LPCTSTR title, LPCTSTR message) = 0;
	virtual void WriteExitCode(int exitCode) = 0;
};

class IC64
{
public:
	virtual bool IsInitOK() = 0;
	virtual void HardReset(bool bCancelAutoload) = 0;
	virtual void SoftReset(bool bCancelAutoload) = 0;
	virtual void CartFreeze(bool bCancelAutoload) = 0;
	virtual void CartReset(bool bCancelAutoload) = 0;
	virtual void PostHardReset(bool bCancelAutoload) = 0;
	virtual void PostSoftReset(bool bCancelAutoload) = 0;
	virtual void PostCartFreeze(bool bCancelAutoload) = 0;
	virtual void ResetKeyboard() = 0;
	virtual void UpdateKeyMap() = 0;
	virtual void TapePressPlay() = 0;
	virtual void TapePressStop() = 0;
	virtual void TapePressRewind() = 0;
	virtual void TapePressEject() = 0;
	virtual HRESULT InsertDiskImageFile(const std::wstring filename, bool alignD64Tracks, bool immediately) = 0;
	virtual HRESULT InsertNewDiskImage(const std::wstring diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks, bool immediately) = 0;
	virtual HRESULT InsertNewDiskImageWithPrgBinary(const std::wstring diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks, bool immediately, const std::wstring prgFileName, bit8* pPrgData, bit32 cbPrgDataLength) = 0;
	virtual HRESULT InsertNewDiskImageWithPrgFile(const std::wstring diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks, bool immediately, const std::wstring prgFileName, const std::wstring prgFileNameFullPath) = 0;
	virtual HRESULT InsertNewDiskImageWithT64File(const std::wstring diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks, bool immediately, const std::wstring prgFileName, const std::wstring t64FileNameFullPath, int t64directoryIndex) = 0;
	virtual void RemoveDisk() = 0;
	virtual void Set_DiskProtect(bool bOn) = 0;
	virtual bool Get_DiskProtect() = 0;
	virtual void DiskReset() = 0;
	virtual void DetachCart() = 0;
	virtual bool IsCartAttached() = 0;
	virtual HRESULT LoadReu1750() = 0;
	virtual bool IsReuAttached() = 0;
	virtual IMonitor* GetMon() = 0;
	virtual void SetupColorTables() = 0;
	virtual HRESULT UpdateBackBuffer() = 0;
	virtual void SynchroniseDevicesWithVIC() = 0;
	virtual bool Get_EnableDebugCart() = 0;
	virtual void Set_EnableDebugCart(bool bEnable) = 0;
	virtual ICLK Get_LimitCycles() = 0;
	virtual void Set_LimitCycles(ICLK cycles) = 0;
	virtual const TCHAR* Get_ExitScreenShot() = 0;
	virtual void Set_ExitScreenShot(const TCHAR* filename) = 0;
	virtual int Get_ExitCode() = 0;
	virtual void Set_ExitCode(int exitCode) = 0;
	virtual int WriteOnceExitCode(int exitCode) = 0;
	virtual bool HasExitCode() = 0;
	virtual void ResetOnceExitCode() = 0;
	virtual HRESULT SavePng(const TCHAR* filename) = 0;
	virtual void ClearAllTemporaryBreakpoints() = 0;
	virtual bool GetDriveLedMotorStatus() = 0;
	virtual bool GetDriveLedDriveStatus() = 0;
	virtual bool GetDriveLedWriteStatus() = 0;
	virtual bit8* GetRomCharPointer() = 0;
};

class DefaultCpu : IDefaultCpu
{
public:
	DefaultCpu(int cpuid, IC64* c64);
	int GetCpuId() override;
	IMonitorCpu* GetCpu() override;
protected:
	int cpuid = 0;
	IC64* c64 = nullptr;
};
