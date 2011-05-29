#ifndef __MONITOR_H__
#define __MONITOR_H__

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
private:
	IMonitorCpu *m_pMonitorCpu;
	IMonitorVic *m_pMonitorVic;
};
#endif
