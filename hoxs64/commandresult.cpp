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


void CommandResult::InitVars()
{
	m_hevtQuit = 0;
	m_hThread = 0;
	m_hWnd = 0;
	cmd = DBGSYM::CliCommand::Unknown;
	m_status = DBGSYM::CliCommandStatus::Failed;
	line = 0;
	m_mux = 0;
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

HRESULT CommandResult::Run()
{
	Assembler as;
	CommandToken *pcmdt = 0;
	HRESULT hr = E_FAIL;
	IRunCommand *pcr;
	try
	{
		hr = as.CreateCliCommandToken(m_sCommandLine.data(), &pcmdt);
		if (SUCCEEDED(hr))
		{
			hr = CreateCliCommandResult(pcmdt, &pcr);
			if (SUCCEEDED(hr))
			{
				hr = pcr->Run();
			}
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}
	return hr;
}

HRESULT CommandResult::CreateCliCommandResult(CommandToken *pCommandTToken, IRunCommand **ppRunCommand)
{
	HRESULT hr = E_FAIL;
	IRunCommand *pcr = 0;
	try
	{
		switch(pCommandTToken->cmd)
		{
		case DBGSYM::CliCommand::Help:
			pcr = new CommandResultHelp(this);
			break;
		case DBGSYM::CliCommand::Disassemble:
			pcr = new CommandResultDisassembly(this, pCommandTToken->startaddress, pCommandTToken->finishaddress);
			break;
		case DBGSYM::CliCommand::Error:
			pcr = new CommandResultText(this, pCommandTToken->text.c_str());
			break;
		default:
			pcr = new CommandResultText(this, TEXT("Unknown command."));
			break;
		}
		hr = E_FAIL;
		if (ppRunCommand)
		{
			if (pcr)
				hr = S_OK;
			*ppRunCommand = pcr;
			pcr = NULL;
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}
	if (pcr)
	{
		delete pcr;
		pcr = 0;
	}
	return hr;
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


HRESULT CommandResult::Start(HWND hWnd, LPCTSTR pszCommandString)
{
	HRESULT r = E_FAIL;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		Stop();
		try
		{
			m_sCommandLine.clear();
			m_sCommandLine.append(pszCommandString);
			m_bIsComplete = false;
			m_hWnd = hWnd;
			m_hThread = CreateThread(NULL, 0, CommandResult::ThreadProc, this, 0, &m_dwThreadId);
			if (m_hThread)
				r = S_OK;
			else 
				r = E_FAIL;
		}
		catch(...)
		{
			r = E_FAIL;
		}
		ReleaseMutex(m_mux);
	}
	return r;
}

void CommandResult::SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status)
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_status = status;
		ReleaseMutex(m_mux);
	}
}

DBGSYM::CliCommandStatus::CliCommandStatus CommandResult::GetStatus()
{
	DBGSYM::CliCommandStatus::CliCommandStatus s= DBGSYM::CliCommandStatus::Failed;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  m_status;
		ReleaseMutex(m_mux);
	}
	return s;
}

bool CommandResult::IsComplete()
{
	bool s= false;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  m_bIsComplete;
		ReleaseMutex(m_mux);
	}
	return s;
}

void CommandResult::SetComplete()
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_bIsComplete = true;
		ReleaseMutex(m_mux);
	}
}

DWORD CommandResult::WaitComplete(DWORD timeout)
{
DWORD r = 0;
	if (m_hThread)
	{
		r =  WaitForSingleObject(m_hThread, timeout);
		return r;
	}
	else
	{
		return WAIT_OBJECT_0;
	}
}

HRESULT CommandResult::Stop()
{
DWORD r = 0;
	if (m_hThread)
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
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		if (m_hWnd)
			PostMessage(m_hWnd, WM_COMMANDRESULT_COMPLETED, status, 0);
		ReleaseMutex(m_mux);
	}
}

DWORD WINAPI CommandResult::ThreadProc(LPVOID lpThreadParameter)
{
	CommandResult *p = (CommandResult *)lpThreadParameter;
	HRESULT hr;
	p->SetStatus(DBGSYM::CliCommandStatus::Running);
	hr = p->Run();
	if (SUCCEEDED(hr))
	{
		p->SetStatus(DBGSYM::CliCommandStatus::CompletedOK);
		p->SetComplete();
		p->PostComplete(1);
	}
	else
	{
		p->SetStatus(DBGSYM::CliCommandStatus::Failed);
		p->SetComplete();
		p->PostComplete(0);
	}
	return 0;
}