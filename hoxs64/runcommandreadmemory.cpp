#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "hexconv.h"
#include "boost2005.h"
#include "bits.h"
#include "mlist.h"
#include "carray.h"
#include "defines.h"
#include "cevent.h"
#include "bits.h"
#include "util.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
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

RunCommandReadMemory::RunCommandReadMemory(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit16 finishaddress)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
	this->m_iDebuggerMmuIndex = iDebuggerMmuIndex;
	this->m_startaddress = startaddress;
	this->m_finishaddress = finishaddress;
	this->m_currentAddress = startaddress;
	this->m_sLineBuffer.reserve(50);
}

HRESULT RunCommandReadMemory::Run()
{
TCHAR addressText[Monitor::BUFSIZEADDRESSTEXT];
TCHAR byteText[4];

	IMonitor *pMon = m_pCommandResult->GetMonitor();
	IMonitorCpu *pCpu;
	if (m_cpumode == DBGSYM::CliCpuMode::C64)
	{
		pCpu = pMon->GetMainCpu();
	}
	else
	{
		pCpu = pMon->GetDiskCpu();
	}

	m_currentAddress = m_startaddress;
	while (true)
	{
		const int COLS = 16;
		if (m_pCommandResult->IsQuit())
		{
			break;
		}

		m_sLineBuffer.clear();
		int instructionSize = COLS;
		if (pMon->Get_Radix() == DBGSYM::MonitorOption::Dec)
		{
			m_sLineBuffer.append(TEXT("a ."));
			_sntprintf_s(addressText, _countof(addressText), _TRUNCATE, TEXT("%-5d"), (unsigned int)m_currentAddress);
		}
		else
		{
			m_sLineBuffer.append(TEXT("a $"));
			HexConv<TCHAR>::long_to_hex(m_currentAddress, addressText, 4);
		}

		m_sLineBuffer.append(addressText);
		m_sLineBuffer.append(TEXT(" m"));
		bool bHitFinish = false;
		for (int x = 0; x < COLS && !bHitFinish; x++, m_currentAddress = (m_currentAddress + 1) & 0xffff)
		{
			if (m_currentAddress == m_finishaddress)
			{
				bHitFinish = true;
			}

			m_sLineBuffer.append(TEXT(" "));
			bit8 data = pCpu->MonReadByte(m_currentAddress, m_iDebuggerMmuIndex);
			if (pMon->Get_Radix() == DBGSYM::MonitorOption::Dec)
			{
				_sntprintf_s(byteText, _countof(byteText), _TRUNCATE, TEXT("%3d"), (unsigned int)data);
			}
			else
			{
				HexConv<TCHAR>::long_to_hex(data, byteText, 2);
			}
			
			m_sLineBuffer.append(byteText);			
		}

		m_sLineBuffer.append(TEXT("\r"));		
		if (m_pCommandResult->CountUnreadLines() > 0)
		{
			m_pCommandResult->WaitLinesTakenOrQuit(INFINITE);
			if (m_pCommandResult->IsQuit())
			{
				break;
			}
		}

		m_pCommandResult->AddLine(m_sLineBuffer.c_str());		
		if (bHitFinish)
		{
			break;
		}
	}

	return S_OK;
}
