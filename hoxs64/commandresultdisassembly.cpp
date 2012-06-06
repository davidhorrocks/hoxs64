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
#include "commandresult.h"

CommandResultDisassembly::CommandResultDisassembly(bit16 startaddress, bit16 finishaddress)
{
	this->cmd  = DBGSYM::CliCommand::Disassemble;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;

	this->address = startaddress;
}

HRESULT CommandResultDisassembly::GetNextLine(LPCTSTR *ppszLine)
{
	if (line == 0)
	{
		if (ppszLine)
			*ppszLine = TEXT("DBGSYM::CliCommand::Disassemble\r");
		line++;
		return S_OK;
	}
	else
	{
		if (ppszLine)
			*ppszLine = NULL;
		return S_FALSE;
	}
}
