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

class IMonitorCommand
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
	virtual HWND ShowDevelopment() = 0;
	virtual bool IsRunning()=0;
	virtual void SoundOff()=0;
	virtual void SoundOn()=0;
	virtual void SetBreakpointC64Execute(MEM_TYPE memorymap, int address, int count)=0;
	virtual void SetBreakpointDiskExecute(int address, int count)=0;


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

	EventSource<BreakpointC64ExecuteChangedEventArgs> EsBreakpointC64ExecuteChanged;
	EventSource<BreakpointDiskExecuteChangedEventArgs> EsBreakpointDiskExecuteChanged;

};

class Monitor
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
	HRESULT Init(IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk);
	int DisassembleOneInstruction(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc);
	int DisassembleBytes(IMonitorCpu *pMonitorCpu, unsigned short address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer);
	void GetCpuRegisters(IMonitorCpu *pMonitorCpu, TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDdr_Text, int cchDdr_Text, TCHAR *pData_Text, int cchData_Text);
	void GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text);
	IMonitorCpu *GetMainCpu();
	IMonitorCpu *GetDiskCpu();
	IMonitorVic *GetVic();
	IMonitorDisk *GetDisk();
private:
	IMonitorCpu *m_pMonitorMainCpu;
	IMonitorCpu *m_pMonitorDiskCpu;
	IMonitorVic *m_pMonitorVic;
	IMonitorDisk *m_pMonitorDisk;

};
#endif
