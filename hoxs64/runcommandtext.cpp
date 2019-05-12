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
#include "savestate.h"
#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"

RunCommandText::RunCommandText(ICommandResult *pCommandResult, LPCTSTR pText)
{
	this->m_pCommandResult  = pCommandResult;	
	if (pText)
	{
		this->m_pCommandResult->AddLine(pText);
	}
}

HRESULT RunCommandText::Run()
{
	return S_OK;
}
