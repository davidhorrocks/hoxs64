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

RunCommandStep::RunCommandStep(ICommandResult* pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit64s stepClocks)
{
    this->m_pCommandResult = pCommandResult;
    this->m_cpumode = cpumode;
    this->stepClocks = stepClocks;
}

HRESULT RunCommandStep::Run()
{
	IMonitor* pMon = m_pCommandResult->GetMonitor();
	IMonitorCpu* pCpu;
	TraceStepInfo stepInfo;
	if (m_cpumode == DBGSYM::CliCpuMode::C64)
	{
		pCpu = pMon->GetMainCpu();
	}
	else
	{
		pCpu = pMon->GetDiskCpu();
	}

	stepInfo.Enable = true;
	stepInfo.CpuMode = this->m_cpumode;
	stepInfo.StepCount = this->stepClocks;
	m_pCommandResult->SetTraceStepCount(stepInfo);
    return S_OK;
}
