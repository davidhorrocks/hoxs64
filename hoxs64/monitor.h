#ifndef __MONITOR_H__
#define __MONITOR_H__


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
	virtual void Trace()=0;
	virtual void TraceFrame()=0;
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
	EventSource<BreakpointChangedEventArgs> EsBreakpointChanged;
	EventSource<VicCursorMoveEventArgs> EsVicCursorMove;
	EventSource<EventArgs> EsMemoryChanged;
};

class Monitor : public IMonitor
{
public:
	static const int BUFSIZEADDRESSTEXT = 6;
	static const int BUFSIZEINSTRUCTIONBYTESTEXT = 9;
	static const int BUFSIZEMNEMONICTEXT = 12;

	static const int BUFSIZEBITTEXT = 2;
	static const int BUFSIZEBYTETEXT = 3;
	static const int BUFSIZEWORDTEXT = 5;

	static const int BUFSIZEBITBYTETEXT = 9;

	static const int BUFSIZEMMUTEXT = 20;

	Monitor();
	~Monitor();
	HRESULT Init(IC64Event *pIC64Event, IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk);
	virtual void MonitorEventsOn();
	virtual void MonitorEventsOff();
	virtual int DisassembleOneInstruction(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc);
	virtual int DisassembleBytes(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer);
	virtual void GetCpuRegisters(IMonitorCpu *pMonitorCpu, TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDdr_Text, int cchDdr_Text, TCHAR *pData_Text, int cchData_Text);
	virtual void GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text);
	virtual IMonitorCpu *GetMainCpu();
	virtual IMonitorCpu *GetDiskCpu();
	virtual IMonitorVic *GetVic();
	virtual IMonitorDisk *GetDisk();

	virtual bool BM_SetBreakpoint(Sp_BreakpointItem bp);
	virtual bool BM_GetBreakpoint(Sp_BreakpointKey k, Sp_BreakpointItem &bp);
	virtual void BM_DeleteBreakpoint(Sp_BreakpointKey k);
	virtual void BM_EnableBreakpoint(Sp_BreakpointKey k);
	virtual void BM_DisableBreakpoint(Sp_BreakpointKey k);
	virtual void BM_EnableAllBreakpoints();
	virtual void BM_DisableAllBreakpoints();
	virtual void BM_DeleteAllBreakpoints();
	virtual IEnumBreakpointItem *BM_CreateEnumBreakpointItem();

	virtual HRESULT ExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, LPTSTR *ppszResults);
	virtual HRESULT BeginExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, shared_ptr<ICommandResult> *pICommandResult);
	virtual HRESULT EndExecuteCommandLine(shared_ptr<ICommandResult> pICommandResult);
	virtual void QuitCommands();
	virtual void QuitCommand(shared_ptr<ICommandResult> pICommandResult);
	virtual void RemoveCommand(shared_ptr<ICommandResult> pICommandResult);

private:
	IMonitorCpu *m_pMonitorMainCpu;
	IMonitorCpu *m_pMonitorDiskCpu;
	IMonitorVic *m_pMonitorVic;
	IMonitorDisk *m_pMonitorDisk;

	//Breakpoint container
	BpMap MapBpExecute;
	bool m_bMonitorEvents;
	IC64Event *m_pIC64Event;

	std::list<shared_ptr<ICommandResult>> m_lstCommandResult;
	HANDLE m_mux;
};
#endif
