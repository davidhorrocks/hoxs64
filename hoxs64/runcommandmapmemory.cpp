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
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"

#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"

RunCommandMapMemory::RunCommandMapMemory(ICommandResult *pCommandResult)
{
	this->m_pCommandResult  = pCommandResult;
}

HRESULT RunCommandMapMemory::Run()
{
//std::basic_string<TCHAR> s;
	
	//DBGSYM::CliMapMemory::CliMapMemory mm = m_pCommandResult->GetToken()->mapmemory;
	//if ((int)mm & DBGSYM::CliMapMemory::k
	//s.append(TEXT("Kernal"));
	//this->m_pCommandResult->AddLine(s);
	return S_OK;
}

int GetMmuIndexFromMap(DBGSYM::CliMapMemory::CliMapMemory map)
{
	if ((map & DBGSYM::CliMapMemory::_ALL) == DBGSYM::CliMapMemory::CURRENT)
	{
		return -1;//current
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