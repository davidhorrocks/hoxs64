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