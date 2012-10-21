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

RunCommandHelp::RunCommandHelp(ICommandResult *pCommandResult)
{
	this->m_pCommandResult  = pCommandResult;
}


HRESULT RunCommandHelp::Run()
{
	this->m_pCommandResult->AddLine(TEXT("Command Help\r"));
	this->m_pCommandResult->AddLine(TEXT("a\t- Assemble.\r"));
	this->m_pCommandResult->AddLine(TEXT("cls\t- Clear screen.\r"));
	this->m_pCommandResult->AddLine(TEXT("cpu\t- Select CPU.\r"));
	this->m_pCommandResult->AddLine(TEXT("d\t- Disassemble memory.\r"));
	this->m_pCommandResult->AddLine(TEXT("m\t- Read memory.\r"));
	return S_OK;
}