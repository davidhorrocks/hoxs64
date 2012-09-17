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

CommandResultHelp::CommandResultHelp(ICommandResult *pCommandResult)
{
	this->m_pCommandResult  = pCommandResult;
}


HRESULT CommandResultHelp::Run()
{
	this->m_pCommandResult->AddLine(TEXT("Command Help\r"));
	this->m_pCommandResult->AddLine(TEXT("d\t- Disassemble memory.\r"));
	return S_OK;
}