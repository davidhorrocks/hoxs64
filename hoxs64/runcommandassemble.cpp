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

RunCommandAssemble::RunCommandAssemble(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit8 *data, int dataLength)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
	this->m_iDebuggerMmuIndex = iDebuggerMmuIndex;
	this->m_startaddress = startaddress;
	this->m_address = startaddress;
	this->m_data = data;
	this->m_dataLength = dataLength;
	this->m_sLineBuffer.reserve(50);
}

HRESULT RunCommandAssemble::Run()
{
	WriteBytesToMemory(m_startaddress, m_data, m_dataLength);
	return S_OK;
}

void RunCommandAssemble::WriteBytesToMemory(bit16 startaddress, bit8 *buffer, int dataLength)
{
TCHAR addressText[10];
std::basic_string<TCHAR> s;

	s.reserve(20);
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

	if (dataLength > 0)
	{
		for(int i = 0; i<dataLength; i++)
		{
			pCpu->MonWriteByte((bit16)(startaddress + i), buffer[i], m_iDebuggerMmuIndex);
		}
		
		bit16 nextaddress = (bit16)(startaddress + dataLength);
		if (pMon->Get_Radix() == DBGSYM::MonitorOption::Dec)
		{
			s.append(TEXT("A ."));
			_sntprintf_s(addressText, _countof(addressText), _TRUNCATE, TEXT("%-d"), (unsigned int)nextaddress);
		}
		else
		{
			s.append(TEXT("A $"));
			HexConv::long_to_hex(nextaddress, addressText, 4);
		}

		s.append(addressText);
		s.append(TEXT(" "));
		m_pCommandResult->AddLine(s.c_str());
	}
}

