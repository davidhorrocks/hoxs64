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

RunCommandCpuRegisters::RunCommandCpuRegisters(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode)
{
	m_pCommandResult  = pCommandResult;
	m_cpumode = cpumode;
	m_sLineBuffer.reserve(50);
}

HRESULT RunCommandCpuRegisters::Run()
{
TCHAR addressText[Monitor::BUFSIZEADDRESSTEXT];
TCHAR byteText[4];
IMonitor *mon = m_pCommandResult->GetMonitor();
IMonitorCpu *cpu;

	m_sLineBuffer.clear();
	switch (m_cpumode)
	{
	case DBGSYM::CliCpuMode::C64:
		cpu = mon->GetMainCpu();
		m_pCommandResult->AddLine(TEXT("CPU C64\r"));
		break;
	case DBGSYM::CliCpuMode::Disk:
		cpu = mon->GetDiskCpu();
		m_pCommandResult->AddLine(TEXT("CPU Disk\r"));
		break;
	default:
		return E_FAIL;
	}

	CPUState reg;
	cpu->GetCpuState(reg);
	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		m_pCommandResult->AddLine(TEXT("   PC   A   X   Y NV-BDIZC  SP\r"));
	}
	else
	{
		m_pCommandResult->AddLine(TEXT("  PC  A  X  Y NV-BDIZC SP\r"));
	}

	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		_sntprintf_s(addressText, _countof(addressText), _TRUNCATE, TEXT("%5d"), (unsigned int)reg.PC_CurrentOpcode);
	}
	else
	{
		HexConv<TCHAR>::long_to_hex(reg.PC_CurrentOpcode, addressText, 4);
	}

	m_sLineBuffer.append(addressText);
	m_sLineBuffer.append(TEXT(" "));
	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		_sntprintf_s(byteText, _countof(byteText), _TRUNCATE, TEXT("%3d"), (unsigned int)reg.A);
	}
	else
	{
		HexConv<TCHAR>::long_to_hex(reg.A, byteText, 2);
	}

	m_sLineBuffer.append(byteText);
	m_sLineBuffer.append(TEXT(" "));
	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		_sntprintf_s(byteText, _countof(byteText), _TRUNCATE, TEXT("%3d"), (unsigned int)reg.X);
	}
	else
	{
		HexConv<TCHAR>::long_to_hex(reg.X, byteText, 2);
	}

	m_sLineBuffer.append(byteText);
	m_sLineBuffer.append(TEXT(" "));
	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		_sntprintf_s(byteText, _countof(byteText), _TRUNCATE, TEXT("%3d"), (unsigned int)reg.Y);
	}
	else
	{
		HexConv<TCHAR>::long_to_hex(reg.Y, byteText, 2);
	}

	m_sLineBuffer.append(byteText);
	m_sLineBuffer.append(TEXT(" "));
	bit8 b = reg.Flags;
	for (int i=0; i<8; i++, b=(b<<1) & 0xff)
	{
		if (b & 0x80)
		{
			m_sLineBuffer.append(TEXT("1"));
		}
		else
		{
			m_sLineBuffer.append(TEXT("0"));
		}
	}

	m_sLineBuffer.append(TEXT(" "));
	if (mon->Get_Radix() == DBGSYM::MonitorOption::Dec)
	{
		_sntprintf_s(byteText, _countof(byteText), _TRUNCATE, TEXT("%3d"), (unsigned int)reg.SP);
	}
	else
	{
		HexConv<TCHAR>::long_to_hex(reg.SP, byteText, 2);
	}

	m_sLineBuffer.append(byteText);
	m_sLineBuffer.append(TEXT("\r"));
	m_pCommandResult->AddLine(m_sLineBuffer.c_str());
	return S_OK;
}
