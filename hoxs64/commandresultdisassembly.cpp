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
#include "commandresult.h"

CommandResultDisassembly::CommandResultDisassembly(CommandResult *pCommandResult, bit16 startaddress, bit16 finishaddress)
{
	this->m_pCommandResult  = pCommandResult;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;
	this->address = startaddress;
}

HRESULT CommandResultDisassembly::Run()
{
	m_pCommandResult->AddLine(TEXT("DBGSYM::CliCommand::Disassemble\r"));
	return S_OK;
}
