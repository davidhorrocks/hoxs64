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

RunCommandAssemble::RunCommandAssemble(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit8 *data, int dataLength)
{
	this->m_pCommandResult  = pCommandResult;
	this->m_cpumode = cpumode;
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
TCHAR addressText[5];
std::basic_string<TCHAR> s;

	s.reserve(20);
	IMonitor *pMon = m_pCommandResult->GetMonitor();
	IMonitorCpu *pCpu;
	if (m_cpumode == DBGSYM::CliCpuMode::C64)
		pCpu = pMon->GetMainCpu();
	else
		pCpu = pMon->GetDiskCpu();
	if (dataLength > 0)
	{
		for(int i = 0; i<dataLength; i++)
		{
			pCpu->MonWriteByte((bit16)(startaddress + i), buffer[i], -1);
		}
		s.append(TEXT("A $"));
		HexConv::long_to_hex((bit16)(startaddress + dataLength), addressText, 4);
		s.append(addressText);
		s.append(TEXT(" "));
		m_pCommandResult->AddLine(s.c_str());
	}
}

