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


void CommandResult::InitVars()
{
	m_hevtQuit = 0;
	m_hThread = 0;
	m_hWnd = 0;
	cmd = DBGSYM::CliCommand::Unknown;
	line = 0;
	m_bUseThread = true;
}

CommandResult::CommandResult()
{
	InitVars();
	try
	{
		m_hevtQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hevtQuit==NULL)
			throw std::runtime_error("CreateEvent failed in CommandResult::CommandResult()");
		m_mux = CreateMutex(NULL, FALSE, NULL);
		if (m_mux==NULL)
			throw std::runtime_error("CreateMutex failed in CommandResult::CommandResult()");
	}
	catch(...)
	{
		Cleanup();
		throw;
	}
}

CommandResult::~CommandResult()
{
	Cleanup();
}

void CommandResult::Cleanup()
{
	if (m_hevtQuit)
	{
		CloseHandle(m_hevtQuit);
		m_hevtQuit = NULL;
	}
	if (m_mux)
	{
		CloseHandle(m_mux);
		m_mux = NULL;
	}
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
DWORD r;
	r = WaitForSingleObject(m_mux, INFINITE);
	if (r == WAIT_OBJECT_0)
	{
		try
		{
			LPTSTR s = 0;
			if (pszLine)
				s = _tcsdup(pszLine);
			if (s)
				a_lines.push_back(s);
		}
		catch(...)
		{
		}
		ReleaseMutex(m_mux);
	}
}

HRESULT CommandResult::GetNextLine(LPCTSTR *ppszLine)
{
DWORD r;
	r = WaitForSingleObject(m_mux, INFINITE);
	if (r == WAIT_OBJECT_0)
	{
		try
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
		catch(...)
		{
		}
		ReleaseMutex(m_mux);
	}
	return E_FAIL;
}

void CommandResult::Reset()
{
DWORD r;
	r = WaitForSingleObject(m_mux, INFINITE);
	if (r == WAIT_OBJECT_0)
	{
		try
		{
			line = 0;
		}
		catch(...)
		{
		}
		ReleaseMutex(m_mux);
	}
}


HRESULT CommandResult::Start(HWND hWnd)
{
	if (m_bUseThread)
	{
		Stop();
		m_hWnd = hWnd;
		m_hThread = CreateThread(NULL, 0, CommandResult::ThreadProc, this, 0, &m_dwThreadId);
		if (m_hThread)
			return S_OK;
		else 
			return E_FAIL;
	}
	else
	{
		bool bOk = false;
		try{
			Run();
			bOk = true;
		}
		catch(...)
		{
		}
		return 0;
	}
}

DWORD CommandResult::WaitComplete(DWORD timeout)
{
DWORD r = 0;
	if (m_bUseThread && m_hThread)
	{
		return WaitForSingleObject(m_hThread, timeout);
	}
	else
	{
		return WAIT_OBJECT_0;
	}
}

HRESULT CommandResult::Stop()
{
DWORD r = 0;
	if (m_bUseThread && m_hThread && m_hevtQuit)
	{
		SetEvent(m_hevtQuit);
		r = MsgWaitForMultipleObjects(1, &m_hThread, TRUE, INFINITE, QS_ALLINPUT);
		if (m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		if (r == WAIT_OBJECT_0)
		{
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}

	}
	return S_OK;
}

void CommandResult::PostComplete(int status)
{
	if (m_hWnd)
		PostMessage(m_hWnd, WM_COMMANDRESULT_COMPLETED, status, 0);
}

DWORD WINAPI CommandResult::ThreadProc(LPVOID lpThreadParameter)
{
	CommandResult *p = (CommandResult *)lpThreadParameter;
	try{
		p->Run();
		p->PostComplete(1);
	}
	catch(...)
	{
		p->PostComplete(0);
	}
	return 0;
}