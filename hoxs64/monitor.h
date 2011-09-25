#ifndef __MONITOR_H__
#define __MONITOR_H__


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


	EventSource<EventArgs> EsResume;
	EventSource<EventArgs> EsTrace;
	EventSource<EventArgs> EsTraceFrame;
	EventSource<EventArgs> EsExecuteC64Clock;
	EventSource<EventArgs> EsExecuteDiskClock;
	EventSource<EventArgs> EsExecuteC64Instruction;
	EventSource<EventArgs> EsExecuteDiskInstruction;
	EventSource<EventArgs> EsUpdateApplication;
	EventSource<EventArgs> EsShowDevelopment;
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
	HRESULT Init(IMonitorCpu *pMonitorCpu, IMonitorVic *pMonitorVic);
	int DisassembleOneInstruction(bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc);
	int AssembleMnemonic(bit8 address, int memorymap, const TCHAR *ppMnemonicText);
	int AssembleBytes(bit8 address, int memorymap, const TCHAR *ppBytesText);
	int DisassembleBytes(unsigned short address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer);
	void GetCpuRegisters(TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDdr_Text, int cchDdr_Text, TCHAR *pData_Text, int cchData_Text);
	void GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text);

	IMonitorCpu *GetCpu();
	IMonitorVic *GetVic();
private:
	IMonitorCpu *m_pMonitorCpu;
	IMonitorVic *m_pMonitorVic;
};
#endif
