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

#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"
#include "monitor.h"

CommandResultDisassembly::CommandResultDisassembly(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit16 finishaddress)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
	this->m_startaddress = startaddress;
	this->m_finishaddress = finishaddress;
	this->m_address = startaddress;
	this->m_sLineBuffer.reserve(50);
}

HRESULT CommandResultDisassembly::Run()
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
	bit16 currentAddress = m_startaddress;
	while (true)
	{
		m_sLineBuffer.erase();
		int instructionSize = pMon->DisassembleOneInstruction(pCpu, currentAddress, -1, AddressText, _countof(AddressText), BytesText, _countof(BytesText), MnemonicText, _countof(MnemonicText), bIsUndoc);
		if (instructionSize<=0)
			break;
		m_sLineBuffer.append(TEXT(".A "));
		m_sLineBuffer.append(AddressText);
		m_sLineBuffer.append(TEXT(" "));
		m_sLineBuffer.append(BytesText);
		for (int k=0; k<(8-_tcslen(BytesText)); k++)
			m_sLineBuffer.append(TEXT(" "));
		
		m_sLineBuffer.append(TEXT(" "));
		m_sLineBuffer.append(MnemonicText);
		m_sLineBuffer.append(TEXT("\r"));
		m_pCommandResult->AddLine(m_sLineBuffer.data());

		for (int i = 1; i<instructionSize && currentAddress != m_finishaddress; i++)
		{
			currentAddress = (currentAddress + 1) & 0xffff;
		}
		if (currentAddress == m_finishaddress)
			break;
		currentAddress++;
	}
	//m_pCommandResult->AddLine(TEXT(""));
	return S_OK;
}
