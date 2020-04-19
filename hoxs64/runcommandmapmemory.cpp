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

RunCommandMapMemory::RunCommandMapMemory(ICommandResult *pCommandResult, int iDebuggerMmuIndex, DBGSYM::CliMapMemory::CliMapMemory map)
{
	m_iMmuIndex = -1;
	m_pCommandResult  = pCommandResult;
	m_map  = map;
	m_iDebuggerMmuIndex = iDebuggerMmuIndex;
}

HRESULT RunCommandMapMemory::Run()
{
IMonitor *mon = m_pCommandResult->GetMonitor();
	m_bSetDebuggerToFollowC64Mmu = ((m_map & DBGSYM::CliMapMemory::_ALL) == DBGSYM::CliMapMemory::SETCURRENT);
	m_bViewDebuggerC64Mmu = ((m_map & DBGSYM::CliMapMemory::_ALL) == DBGSYM::CliMapMemory::VIEWCURRENT);

	if (m_bViewDebuggerC64Mmu)
	{
		if (m_iDebuggerMmuIndex < 0)
		{
			//Debugger follows C64
			m_iMmuIndex = m_pCommandResult->GetMonitor()->GetMainCpu()->GetCurrentCpuMmuMemoryMap();
			m_pCommandResult->AddLine(TEXT("C64 memory map - follow C64\r"));
		}
		else
		{
			//Debugger using own map
			m_iMmuIndex = m_iDebuggerMmuIndex;
			m_pCommandResult->AddLine(TEXT("C64 memory map - Debugger fixed\r"));
		}
	}
	else if (m_bSetDebuggerToFollowC64Mmu)
	{
		//Debugger to follow C64
		m_iMmuIndex = m_pCommandResult->GetMonitor()->GetMainCpu()->GetCurrentCpuMmuMemoryMap();
		m_pCommandResult->AddLine(TEXT("C64 memory map - follow C64\r"));
	}
	else
	{
		//Debugger to use own fixed map
		m_iMmuIndex = GetMmuIndexFromMap(m_map);
		m_pCommandResult->AddLine(TEXT("C64 memory map - Debugger fixed\r"));
	}
	switch(m_iMmuIndex & 0xf)
	{
	case 0xf:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: Kernal ROM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("C000 - CFFF: RAM\r"));
		m_pCommandResult->AddLine(TEXT("A000 - BFFF: Basic ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - 9FFF: RAM\r"));
		break;
	case 0xa:
	case 0xb:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: RAM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - CFFF: RAM\r"));
		break;
	case 0x8:
		if (m_iMmuIndex & 0x10)
		{
			m_pCommandResult->AddLine(TEXT("E000 - FFFF: RAM\r"));
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
			m_pCommandResult->AddLine(TEXT("0000 - CFFF: RAM\r"));
		}
		else
		{
			m_pCommandResult->AddLine(TEXT("0000 - FFFF: RAM\r"));
		}
		break;
	case 0x7:
	case 0x6:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: Kernal ROM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - CFFF: RAM\r"));
		break;
	case 0x0:
	case 0x2:
	case 0x3:
		m_pCommandResult->AddLine(TEXT("0000 - FFFF: RAM\r"));
		break;
	case 0xe: 
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: Kernal ROM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("C000 - CFFF: RAM\r"));
		m_pCommandResult->AddLine(TEXT("A000 - BFFF: Basic ROM\r"));
		m_pCommandResult->AddLine(TEXT("8000 - 9FFF: Low ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - 7FFF: RAM\r"));
		break;
	case 0x4:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: Kernal ROM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("C000 - CFFF: RAM\r"));
		m_pCommandResult->AddLine(TEXT("A000 - BFFF: High ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - 9FFF: RAM\r"));
		break;
	case 0xc:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: Kernal ROM\r"));
		if (m_iMmuIndex & 0x10)
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		else
			m_pCommandResult->AddLine(TEXT("D000 - DFFF: Char ROM\r"));
		m_pCommandResult->AddLine(TEXT("C000 - CFFF: RAM\r"));
		m_pCommandResult->AddLine(TEXT("A000 - BFFF: High ROM\r"));
		m_pCommandResult->AddLine(TEXT("8000 - 9FFF: Low ROM\r"));
		m_pCommandResult->AddLine(TEXT("0000 - 7FFF: RAM\r"));
		break;
	default:
		m_pCommandResult->AddLine(TEXT("E000 - FFFF: High ROM\r"));
		m_pCommandResult->AddLine(TEXT("D000 - DFFF: IO\r"));
		m_pCommandResult->AddLine(TEXT("A000 - CFFF: Open\r"));
		m_pCommandResult->AddLine(TEXT("8000 - 9FFF: Low ROM\r"));
		m_pCommandResult->AddLine(TEXT("1000 - 7FFF: Open\r"));
		m_pCommandResult->AddLine(TEXT("0000 - 0FFF: RAM\r"));
		break;
	}
	return S_OK;
}

int RunCommandMapMemory::GetMmuIndexFromMap(DBGSYM::CliMapMemory::CliMapMemory map)
{
	if ((map & DBGSYM::CliMapMemory::SETCURRENT) == DBGSYM::CliMapMemory::SETCURRENT)
	{
		return m_pCommandResult->GetMonitor()->GetMainCpu()->GetCurrentCpuMmuMemoryMap();
	}
	else if ((map & DBGSYM::CliMapMemory::_ALL) == DBGSYM::CliMapMemory::RAM)
	{
		return 0x03;//RAM
	}
	else if ((map & (DBGSYM::CliMapMemory::KERNAL | DBGSYM::CliMapMemory::BASIC | DBGSYM::CliMapMemory::CHARGEN | DBGSYM::CliMapMemory::IO | DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML))
		== DBGSYM::CliMapMemory::IO)
	{
		return 0x1b;//IO
	}
	else if ((map & (DBGSYM::CliMapMemory::KERNAL | DBGSYM::CliMapMemory::BASIC | DBGSYM::CliMapMemory::CHARGEN | DBGSYM::CliMapMemory::IO | DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML))
		== DBGSYM::CliMapMemory::CHARGEN)
	{
		return 0x0b;//CHARGEN
	}
	else if ((map & (DBGSYM::CliMapMemory::KERNAL | DBGSYM::CliMapMemory::BASIC))
		== DBGSYM::CliMapMemory::KERNAL)
	{
		if ((map & (DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML)) == (DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML))
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x1c;//KERNAL + IO + ROMH + ROML
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x0c;//KERNAL + CHARGEN + ROMH + ROML
			else
				return 0x1c;//KERNAL + IO + ROMH + ROML
		}
		else if ((map & (DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML)) == DBGSYM::CliMapMemory::ROML)
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x1c;//KERNAL + IO + ROMH + ROML
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x0c;//KERNAL + CHARGEN + ROMH + ROML
			else
				return 0x1c;//KERNAL + IO + ROMH + ROML
		}
		else if ((map & (DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML)) == DBGSYM::CliMapMemory::ROMH)
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x14;//KERNAL + IO + ROMH
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x04;//KERNAL + CHARGEN + ROMH
			else
				return 0x14;//KERNAL + IO + ROMH
		}
		else
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x17;//KERNAL + IO
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x07;//KERNAL + CHARGEN
			else
				return 0x17;//KERNAL + IO
		}
	}
	else if ((map & (DBGSYM::CliMapMemory::BASIC))
		== DBGSYM::CliMapMemory::BASIC)
	{
		if ((map & (DBGSYM::CliMapMemory::ROML)) == DBGSYM::CliMapMemory::ROML)
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x1e;//KERNAL + BASIC + IO + ROML
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x0e;//KERNAL + BASIC + CHARGEN + ROML
			else
				return 0x1e;//KERNAL + BASIC + IO + ROML
		}
		else
		{
			if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
				return 0x1f;//KERNAL + BASIC + IO
			else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
				return 0x0f;//KERNAL + BASIC + CHARGEN
			else
				return 0x1f;//KERNAL + BASIC + IO
		}
	}
	else if ((map & (DBGSYM::CliMapMemory::ROMH | DBGSYM::CliMapMemory::ROML)) != (DBGSYM::CliMapMemory::CliMapMemory)0)
	{
		if ((map & (DBGSYM::CliMapMemory::IO)) == DBGSYM::CliMapMemory::IO)
			return 0x1d;//ROMH + IO + ROML
		else if ((map & (DBGSYM::CliMapMemory::CHARGEN)) == DBGSYM::CliMapMemory::CHARGEN)
			return 0x0d;//ROMH + CHARGEN + ROML
		else
			return 0x1d;//ROMH + IO + ROML
	}
	else
		return 0x1f;//KERNAL + BASIC + IO
}