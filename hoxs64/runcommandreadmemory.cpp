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
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"

#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"
#include "monitor.h"

RunCommandReadMemory::RunCommandReadMemory(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit16 finishaddress)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
	this->m_startaddress = startaddress;
	this->m_finishaddress = finishaddress;
	this->m_address = startaddress;
	this->m_sLineBuffer.reserve(50);
}

HRESULT RunCommandReadMemory::Run()
{
TCHAR addressText[Monitor::BUFSIZEADDRESSTEXT];
TCHAR byteText[3];

	IMonitor *pMon = m_pCommandResult->GetMonitor();
	IMonitorCpu *pCpu;
	if (m_cpumode == DBGSYM::CliCpuMode::C64)
		pCpu = pMon->GetMainCpu();
	else
		pCpu = pMon->GetDiskCpu();
	bit16 currentAddress = m_startaddress;
	while (true)
	{
		const int COLS = 16;
		if (m_pCommandResult->IsQuit())
			break;
		m_sLineBuffer.clear();
		int instructionSize = COLS;
		m_sLineBuffer.append(TEXT(". $"));
		HexConv::long_to_hex(currentAddress, addressText, 4);
		m_sLineBuffer.append(addressText);
		bool bHitFinish = false;
		for (int x = 0; x < COLS; x++, currentAddress = (currentAddress + 1) & 0xffff)
		{
			if (currentAddress == m_finishaddress)
				bHitFinish = true;
			m_sLineBuffer.append(TEXT(" "));
			bit8 data = pCpu->MonReadByte(currentAddress, -1);
			HexConv::long_to_hex(data, byteText, 2);
			m_sLineBuffer.append(byteText);			
		}
		m_sLineBuffer.append(TEXT("\r"));
		
		if (m_pCommandResult->CountUnreadLines() > 0)
		{
			m_pCommandResult->WaitLinesTakenOrQuit(INFINITE);
			if (m_pCommandResult->IsQuit())
				break;
		}
		m_pCommandResult->AddLine(m_sLineBuffer.data());
		
		if (bHitFinish)
			break;
	}
	return S_OK;
}
