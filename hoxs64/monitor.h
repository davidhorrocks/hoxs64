#pragma once
#include "breakpointresult.h"
#include "register.h"
#include "appstatus.h"

class RadixChangedEventArgs : public EventArgs
{
public:
	RadixChangedEventArgs(DBGSYM::MonitorOption::Radix radix);

	DBGSYM::MonitorOption::Radix Radix;
};

class BreakpointC64ExecuteChangedEventArgs : public EventArgs
{
public:
	MEM_TYPE Memorymap;
	bit16 Address;
	int Count;
	BreakpointC64ExecuteChangedEventArgs(MEM_TYPE memorymap, bit16 address, int count);
};

class BreakpointDiskExecuteChangedEventArgs : public EventArgs
{
public:
	bit16 Address;
	int Count;
	BreakpointDiskExecuteChangedEventArgs(bit16 address, int count);
};

class BreakpointVicChangedEventArgs : public EventArgs
{
};

class BreakpointChangedEventArgs : public EventArgs
{
};


class VicCursorMoveEventArgs : public EventArgs
{
public:
	VicCursorMoveEventArgs(int cycle, int line);
	int Cycle;
	int Line;
};

class IAppCommand
{
public:
	virtual void Resume()=0;
	virtual void Trace(int cpuId)=0;
	virtual void TraceWithTemporaryBreakpoints(int cpuId)=0;
	virtual void TraceFrame(int cpuId)=0;
	virtual void TraceStepOver(int cpuId)=0;
	virtual void TraceStepOut(int cpuId, bool requireRtsRti)=0;
	virtual void ClearAllTemporaryBreakpoints()=0;
	virtual void ExecuteC64Clock()=0;
	virtual void ExecuteDiskClock()=0;
	virtual void ExecuteC64Instruction()=0;
	virtual void ExecuteDiskInstruction()=0;
	virtual void UpdateApplication()=0;
	virtual void UpdateEmulationDisplay() = 0;
	virtual HWND ShowDevelopment() = 0;
	virtual bool IsRunning()=0;
	virtual void SoundOff()=0;
	virtual void SoundOn()=0;
	virtual void ShowCpuDisassembly(int cpuid, DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address)=0;
	virtual HWND GetMainFrameWindow() = 0;
	virtual void DisplayVicCursor(bool bEnabled) = 0;
	virtual void DisplayVicRasterBreakpoints(bool bEnabled) = 0;
	virtual void SetVicCursorPos(int iCycle, int iLine) = 0;
	virtual void GetVicCursorPos(int *piCycle, int *piLine) = 0;
	virtual void SetRadixHexadecimal() = 0;
	virtual void SetRadixDecimal() = 0;
	virtual void GetLastMousePosition(int *x, int *y) = 0;
	virtual void SetLastMousePosition(const int* x, const int* y) = 0;
	virtual bool GetIsMouseOverClientArea() = 0;
	virtual void SetIsMouseOverClientArea(bool isMouseOver) = 0;
	virtual void PostCloseMainWindow() = 0;
	virtual void PostToggleFullscreen() = 0;
	virtual bool PostAutoLoadFile(const wchar_t* pszFilename, int directoryIndex, bool quickload) = 0;
	virtual bool InsertDiskImageFromFile(const wchar_t* pszFilename) = 0;
	virtual bool InsertTapeImageFromFile(const wchar_t* pszFilename) = 0;
	virtual void EnableC64Input(bool enabled) = 0;

	virtual void FreeDirectX() noexcept = 0;
	virtual void SoundHalt() = 0;
	virtual void SoundResume() = 0;
	virtual const wchar_t* GetAppTitle() = 0;
	virtual const wchar_t* GetAppName() = 0;
	virtual VS_FIXEDFILEINFO* GetVersionInfo() = 0;
	virtual void SetBusyApp(bool isBusy) = 0;
	virtual void TogglePause() = 0;
	virtual void ToggleSoundMute() = 0;
	virtual void ToggleMaxSpeed() = 0;
	virtual void InsertTapeDialog(HWND hWnd) = 0;
	virtual void LoadCrtFileDialog(HWND hWnd) = 0;
	virtual void LoadReu1750(HWND hWnd) = 0;
	virtual void LoadC64ImageDialog(HWND hWnd) = 0;
	virtual void LoadT64Dialog(HWND hWnd) = 0;
	virtual void AutoLoadDialog(HWND hWnd) = 0;
	virtual void InsertDiskImageDialog(HWND hWnd) = 0;
	virtual void SaveD64ImageDialog(HWND hWnd) = 0;
	virtual void SaveFDIImageDialog(HWND hWnd) = 0;
	virtual void SaveP64ImageDialog(HWND hWnd) = 0;
	virtual void SaveC64StateDialog(HWND hWnd) = 0;
	virtual void LoadC64StateDialog(HWND hWnd) = 0;
	virtual void RestoreUserSettings() = 0;
	virtual void RestoreDefaultSettings() = 0;
	virtual void SaveCurrentSetting() = 0;
	virtual void GetUserConfig(CConfig& cfg) = 0;
	virtual void SetUserConfig(const CConfig& newcfg) = 0;
	virtual void ApplyConfig(const CConfig& newcfg) = 0;
	virtual void SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID) = 0;
	virtual void ResetSidChipAddressMap() = 0;
	virtual void UpdateUserConfigFromSid() = 0;	

	EventSource<EventArgs> EsResume;
	EventSource<EventArgs> EsTrace;
	EventSource<EventArgs> EsTraceFrame;
	EventSource<EventArgs> EsExecuteC64Clock;
	EventSource<EventArgs> EsExecuteDiskClock;
	EventSource<EventArgs> EsExecuteC64Instruction;
	EventSource<EventArgs> EsExecuteDiskInstruction;
	EventSource<EventArgs> EsUpdateApplication;
	EventSource<EventArgs> EsShowDevelopment;
	EventSource<EventArgs> EsCpuC64RegPCChanged;
	EventSource<EventArgs> EsCpuDiskRegPCChanged;
	EventSource<EventArgs> EsCpuC64RegAChanged;
	EventSource<EventArgs> EsCpuDiskRegAChanged;
	EventSource<EventArgs> EsCpuC64RegXChanged;
	EventSource<EventArgs> EsCpuDiskRegXChanged;
	EventSource<EventArgs> EsCpuC64RegYChanged;
	EventSource<EventArgs> EsCpuDiskRegYChanged;
	EventSource<EventArgs> EsCpuC64RegSRChanged;
	EventSource<EventArgs> EsCpuDiskRegSRChanged;
	EventSource<EventArgs> EsCpuC64RegSPChanged;
	EventSource<EventArgs> EsCpuDiskRegSPChanged;
	EventSource<EventArgs> EsCpuC64RegDdrChanged;
	EventSource<EventArgs> EsCpuC64RegDataChanged;
	EventSource<BreakpointChangedEventArgs> EsBreakpointChanged;
	EventSource<VicCursorMoveEventArgs> EsVicCursorMove;
	EventSource<EventArgs> EsMemoryChanged;
	EventSource<RadixChangedEventArgs> EsRadixChanged;	
};

class Monitor : public IMonitor
{
public:
	typedef enum tagBuffSize
	{
		BUFSIZEADDRESSTEXT = 30,
		BUFSIZEINSTRUCTIONBYTESTEXT = 30,
		BUFSIZEMNEMONICTEXT = 50,
	} BuffSize;

	Monitor() noexcept;
	Monitor(const Monitor&) = default;
	Monitor& operator=(const Monitor&) = delete;
	Monitor(Monitor&&) = delete;
	Monitor& operator=(Monitor&&) = delete;
	~Monitor() noexcept;
	HRESULT Init(IC64Event *pIC64Event, IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk);
	void MonitorEventsOn() override;
	void MonitorEventsOff() override;
	int DisassembleOneInstruction(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc) override;
	int DisassembleBytes(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer) override;
	void GetCpuRegisters(IMonitorCpu *pMonitorCpu, TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDdr_Text, int cchDdr_Text, TCHAR *pData_Text, int cchData_Text) override;
	void GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text) override;
	IMonitorCpu *GetMainCpu() override;
	IMonitorCpu *GetDiskCpu() override;
	IMonitorVic *GetVic() override;
	IMonitorDisk *GetDisk() override;

	bool BM_SetBreakpoint(const BreakpointItem& bp) override;
	bool BM_GetBreakpoint(const BreakpointItem& key, BreakpointItem& bp) override;
	void BM_DeleteBreakpoint(const BreakpointItem& key) override;
	void BM_EnableBreakpoint(const BreakpointItem& key) override;
	void BM_DisableBreakpoint(const BreakpointItem& key) override;
	void BM_EnableAllBreakpoints() override;
	void BM_DisableAllBreakpoints() override;
	void BM_DeleteAllBreakpoints() override;
	IEnumBreakpointItem *BM_CreateEnumBreakpointItem() override;

	HRESULT ExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, LPTSTR *ppszResults) override;
	HRESULT BeginExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, shared_ptr<ICommandResult> *pICommandResult) override;
	HRESULT EndExecuteCommandLine(shared_ptr<ICommandResult> pICommandResult) override;
	void QuitCommands() override;
	void QuitCommand(shared_ptr<ICommandResult> pICommandResult) override;
	void RemoveCommand(shared_ptr<ICommandResult> pICommandResult) override;
	DBGSYM::MonitorOption::Radix Get_Radix() override;
	void Set_Radix(DBGSYM::MonitorOption::Radix value) override;
	

private:
	IMonitorCpu *m_pMonitorMainCpu;
	IMonitorCpu *m_pMonitorDiskCpu;
	IMonitorVic *m_pMonitorVic;
	IMonitorDisk *m_pMonitorDisk;

	//Breakpoint container
	BpMap MapBpExecute;
	bool m_bMonitorEvents;
	IC64Event *m_pIC64Event;

	list<shared_ptr<ICommandResult>> m_lstCommandResult;
	HANDLE m_mux;

	volatile DBGSYM::MonitorOption::Radix radix;
};
