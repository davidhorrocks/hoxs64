#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "bits.h"
#include "mlist.h"
#include "carray.h"
#include "defines.h"
#include "cevent.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"
#include "savestate.h"
#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"
#include "monitor.h"

RunCommandDisassembly::RunCommandDisassembly(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit16 finishaddress)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
	this->m_iDebuggerMmuIndex = iDebuggerMmuIndex;
	this->m_startaddress = startaddress;
	this->m_finishaddress = finishaddress;
	this->m_currentAddress = startaddress;
	this->m_sLineBuffer.reserve(50);
}

HRESULT RunCommandDisassembly::Run()
{
	TCHAR AddressText[Monitor::BUFSIZEADDRESSTEXT];
	TCHAR BytesText[Monitor::BUFSIZEINSTRUCTIONBYTESTEXT];
	TCHAR MnemonicText[Monitor::BUFSIZEMNEMONICTEXT];
	bool bIsUndoc;
	IMonitor *pMon = m_pCommandResult->GetMonitor();
	IMonitorCpu *pCpu;
	if (m_cpumode == DBGSYM::CliCpuMode::C64)
		pCpu = pMon->GetMainCpu();
	else
		pCpu = pMon->GetDiskCpu();
	
	m_currentAddress = m_startaddress;
	bool done=false;
	while (true)
	{
		if (m_pCommandResult->IsQuit())
			break;
		m_sLineBuffer.clear();
		int instructionSize = pMon->DisassembleOneInstruction(pCpu, m_currentAddress, m_iDebuggerMmuIndex, AddressText, _countof(AddressText), BytesText, _countof(BytesText), MnemonicText, _countof(MnemonicText), bIsUndoc);
		if (instructionSize<=0)
			break;
		m_sLineBuffer.append(TEXT("A "));
		m_sLineBuffer.append(AddressText);
		m_sLineBuffer.append(TEXT(" "));
		m_sLineBuffer.append(BytesText);
		for (size_t k=0; k<(8-_tcslen(BytesText)); k++)
			m_sLineBuffer.append(TEXT(" "));
		
		m_sLineBuffer.append(TEXT(" "));
		m_sLineBuffer.append(MnemonicText);
		m_sLineBuffer.append(TEXT("\r"));
		
		if (m_pCommandResult->CountUnreadLines() > 0)
		{
			m_pCommandResult->WaitLinesTakenOrQuit(INFINITE);
			if (m_pCommandResult->IsQuit())
				break;
		}
		m_pCommandResult->AddLine(m_sLineBuffer.c_str());
		
		if (m_currentAddress == m_finishaddress)
			done = true;
		bit16 k = m_currentAddress;
		m_currentAddress = (m_currentAddress + instructionSize) & 0xffff;
		for (int i=1; i<instructionSize; i++)
		{
			k = (k + 1) & 0xffff;
			if (k == m_finishaddress)
				done = true;
		}
		if (done)
			break;
	}
	return S_OK;
}
