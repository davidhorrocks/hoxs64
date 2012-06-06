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

CommandResult::CommandResult()
{
	cmd = DBGSYM::CliCommand::Unknown;
	line = 0;
}

CommandResult::CommandResult(LPCTSTR pszLine)
{
	cmd = DBGSYM::CliCommand::Unknown;
	line = 0;
	AddLine(pszLine);
}

CommandResult::~CommandResult()
{
	for (std::vector<LPTSTR>::iterator it = a_lines.begin(); it!=a_lines.end(); it++)
	{
		LPTSTR s = *it;
		if(s)
			free(s);
	}
	a_lines.clear();
}

void CommandResult::AddLine(LPCTSTR pszLine)
{
	LPTSTR s = 0;
	if (pszLine)
		s = _tcsdup(pszLine);
	if (s)
		a_lines.push_back(s);
}

HRESULT CommandResult::GetNextLine(LPCTSTR *ppszLine)
{
	if (line < a_lines.size())
	{
		if (ppszLine)
			*ppszLine = a_lines[line];
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

void CommandResult::Reset()
{
	line = 0;
}
