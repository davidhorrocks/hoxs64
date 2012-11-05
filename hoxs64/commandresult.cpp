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


void CommandResult::InitVars()
{
	m_hevtQuit = 0;
	m_hevtLineTaken = 0;
	m_hThread = 0;
	m_hWnd = 0;
	cmd = DBGSYM::CliCommand::Unknown;
	m_status = DBGSYM::CliCommandStatus::Failed;
	line = 0;
	m_mux = 0;
	m_pIMonitor = 0;
	m_pCommandToken = 0;
	m_bIsQuit = false;
	m_pIRunCommand = NULL;
}

CommandResult::CommandResult(IMonitor *pIMonitor, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress)
{
	const char *S_EVENTCREATEERROR = "CreateEvent failed in CommandResult::CommandResult()";
	InitVars();
	try
	{
		m_hevtQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hevtQuit==NULL)
			throw std::runtime_error(S_EVENTCREATEERROR);
		m_hevtLineTaken = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hevtLineTaken==NULL)
			throw std::runtime_error(S_EVENTCREATEERROR);
		m_hevtResultDataReady = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hevtResultDataReady==NULL)
			throw std::runtime_error(S_EVENTCREATEERROR);
		m_hevtResultDataTaken = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hevtResultDataTaken==NULL)
			throw std::runtime_error(S_EVENTCREATEERROR);
		//
		m_mux = CreateMutex(NULL, FALSE, NULL);
		if (m_mux==NULL)
			throw std::runtime_error(S_EVENTCREATEERROR);
		m_pIMonitor = pIMonitor;
		m_cpumode = cpumode;
		m_iDebuggerMmuIndex = iDebuggerMmuIndex;
		m_iDefaultAddress = iDefaultAddress;
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
	Quit();
	if (m_hThread)
	{
		WaitFinished(INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if (m_hevtQuit)
	{
		CloseHandle(m_hevtQuit);
		m_hevtQuit = NULL;
	}
	if (m_hevtLineTaken)
	{
		CloseHandle(m_hevtLineTaken);
		m_hevtLineTaken = NULL;
	}
	if (m_hevtResultDataReady)
	{
		CloseHandle(m_hevtResultDataReady);
		m_hevtResultDataReady = NULL;
	}
	if (m_hevtResultDataTaken)
	{
		CloseHandle(m_hevtResultDataTaken);
		m_hevtResultDataTaken = NULL;
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
	if (m_pCommandToken)
	{
		delete m_pCommandToken;
		m_pCommandToken = 0;
	}
	if (m_pIRunCommand)
	{
		delete m_pIRunCommand;
		m_pIRunCommand = 0;
	}
}

HRESULT CommandResult::Run()
{
	Assembler as;
	HRESULT hr = E_FAIL;
	try
	{
		if (m_pCommandToken)
		{
			delete m_pCommandToken;
			m_pCommandToken = 0;
		}
		if (m_pIRunCommand)
		{
			delete m_pIRunCommand;
			m_pIRunCommand = 0;
		}
		SetStatus(DBGSYM::CliCommandStatus::Running);
		hr = as.CreateCliCommandToken(m_sCommandLine.c_str(), &m_pCommandToken);
		if (SUCCEEDED(hr))
		{
			hr = CreateRunCommand(m_pCommandToken, &m_pIRunCommand);
			if (SUCCEEDED(hr))
			{
				hr = m_pIRunCommand->Run();
			}
		}
		DWORD rm = WaitForSingleObject(m_mux, INFINITE);
		if (rm == WAIT_OBJECT_0)
		{
			if (SUCCEEDED(hr))
			{
				SetStatus(DBGSYM::CliCommandStatus::CompletedOK);
				PostFinished();
			}
			else
			{
				SetStatus(DBGSYM::CliCommandStatus::Failed);
				PostFinished();
			}
			ReleaseMutex(m_mux);
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}
	return hr;
}

HRESULT CommandResult::CreateRunCommand(CommandToken *pCommandToken, IRunCommand **ppRunCommand)
{
	HRESULT hr = E_FAIL;
	IRunCommand *pcr = 0;
	const bit16 DEFAULTDISASSEMBLEBYTES = 0xf;
	const bit16 DEFAULTMEMORYBYTES = 0xff;
	try
	{
		switch(pCommandToken->cmd)
		{
		case DBGSYM::CliCommand::Help:
			pcr = new RunCommandHelp(this);
			break;
		case DBGSYM::CliCommand::MapMemory:
			pcr = new RunCommandMapMemory(this, m_iDebuggerMmuIndex, pCommandToken->mapmemory);
			break;
		case DBGSYM::CliCommand::Disassemble:
			if (pCommandToken->bHasStartAddress && pCommandToken->bHasFinishAddress)
				pcr = new RunCommandDisassembly(this, m_cpumode, m_iDebuggerMmuIndex, pCommandToken->startaddress, pCommandToken->finishaddress);
			else if (pCommandToken->bHasStartAddress)
				pcr = new RunCommandDisassembly(this, m_cpumode, m_iDebuggerMmuIndex, pCommandToken->startaddress, (pCommandToken->startaddress + DEFAULTDISASSEMBLEBYTES) & 0xffff);
			else
				pcr = new RunCommandDisassembly(this, m_cpumode, m_iDebuggerMmuIndex, m_iDefaultAddress, (m_iDefaultAddress + DEFAULTDISASSEMBLEBYTES) & 0xffff);
			break;
		case DBGSYM::CliCommand::ReadMemory:
			if (pCommandToken->bHasStartAddress && pCommandToken->bHasFinishAddress)
				pcr = new RunCommandReadMemory(this, m_cpumode, m_iDebuggerMmuIndex, pCommandToken->startaddress, pCommandToken->finishaddress);
			else if (pCommandToken->bHasStartAddress)
				pcr = new RunCommandReadMemory(this, m_cpumode, m_iDebuggerMmuIndex, pCommandToken->startaddress, (pCommandToken->startaddress + DEFAULTMEMORYBYTES) & 0xffff);
			else
				pcr = new RunCommandReadMemory(this, m_cpumode, m_iDebuggerMmuIndex, m_iDefaultAddress, (m_iDefaultAddress + DEFAULTMEMORYBYTES) & 0xffff);
			break;
		case DBGSYM::CliCommand::Assemble:
			pcr = new RunCommandAssemble(this, m_cpumode, m_iDebuggerMmuIndex, pCommandToken->startaddress, pCommandToken->buffer, pCommandToken->dataLength);
			break;
		case DBGSYM::CliCommand::Error:
			pcr = new RunCommandText(this, pCommandToken->text.c_str());
			break;
		case DBGSYM::CliCommand::ShowCpu:
			pcr = new RunCommandCpuRegisters(this, m_cpumode);
			break;
		case DBGSYM::CliCommand::SelectCpu:
			switch(pCommandToken->cpumode)
			{
			case DBGSYM::CliCpuMode::C64:
				pcr = new RunCommandText(this, NULL);
				break;
			case DBGSYM::CliCpuMode::Disk:
				pcr = new RunCommandText(this, NULL);
				break;
			}
			pcr = new RunCommandText(this, pCommandToken->text.c_str());
			break;
		case DBGSYM::CliCommand::ClearScreen:
			pcr = new RunCommandText(this, NULL);
			break;
		default:
			pcr = new RunCommandText(this, TEXT("Unknown command.\r"));
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


size_t CommandResult::CountUnreadLines()
{
size_t i = 0;
DWORD rm;
	rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		i = a_lines.size() - line; 
		ReleaseMutex(m_mux);
	}
	return i;
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
			{
				a_lines.push_back(s);
				ResetEvent(this->m_hevtLineTaken);
			}
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
HRESULT hr = E_FAIL;
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
				if (line < a_lines.size())
				{
					hr = S_OK;
				}
				else
				{
					SetEvent(m_hevtLineTaken);
					hr = S_FALSE;
				}
			}
			else
			{
				SetEvent(m_hevtLineTaken);
				if (ppszLine)
					*ppszLine = NULL;
				hr = E_FAIL;
			}
		}
		catch(...)
		{
		}
		ReleaseMutex(m_mux);
	}
	return hr;
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


HRESULT CommandResult::Start(HWND hWnd, LPCTSTR pszCommandString, int id)
{
	HRESULT r = E_FAIL;
	Quit();
	this->WaitFinished(INFINITE);
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_bIsQuit = false;
		ResetEvent(m_hevtQuit);
		ResetEvent(m_hevtResultDataReady);
		ResetEvent(m_hevtResultDataTaken);
		try
		{
			m_sCommandLine.clear();
			m_sCommandLine.append(pszCommandString);
			m_hWnd = hWnd;
			m_id = id;
			this->SetStatus(DBGSYM::CliCommandStatus::Running);
			m_hThread = CreateThread(NULL, 0, CommandResult::ThreadProc, this, 0, &m_dwThreadId);
			if (m_hThread)
			{
				r = S_OK;
			}
			else
			{
				this->SetStatus(DBGSYM::CliCommandStatus::Failed);
				r = E_FAIL;
			}
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

void CommandResult::SetHwnd(HWND hWnd)
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_hWnd = hWnd;
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

int CommandResult::GetId()
{
	int s=0;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  m_id;
		ReleaseMutex(m_mux);
	}
	return s;
}

IMonitor *CommandResult::GetMonitor()
{
	IMonitor *s=0;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  m_pIMonitor;
		ReleaseMutex(m_mux);
	}
	return s;
}

CommandToken *CommandResult::GetToken()
{
	CommandToken *s=0;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  this->m_pCommandToken;
		ReleaseMutex(m_mux);
	}
	return s;
}

IRunCommand *CommandResult::GetRunCommand()
{
	IRunCommand *s=0;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s = this->m_pIRunCommand;
		ReleaseMutex(m_mux);
	}
	return s;
}

bool CommandResult::IsQuit()
{
	bool s= false;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		s =  m_bIsQuit;
		ReleaseMutex(m_mux);
	}
	return s;
}

DWORD CommandResult::WaitFinished(DWORD timeout)
{
DWORD r = 0;
	if (m_hThread)
	{
		r = WaitForSingleObject(&m_hThread, timeout);
		return r;
	}
	else
	{
		return WAIT_OBJECT_0;
	}
}

DWORD CommandResult::WaitLinesTakenOrQuit(DWORD timeout)
{
DWORD r;
	HANDLE wh[] = {m_hevtLineTaken, m_hevtQuit};
	r = WaitForMultipleObjects(2, &wh[0], FALSE, timeout);
	return r;
}

DWORD CommandResult::WaitResultDataTakenOrQuit(DWORD timeout)
{
DWORD r;
	HANDLE wh[] = {m_hevtResultDataReady, m_hevtQuit};
	r = WaitForMultipleObjects(2, &wh[0], FALSE, timeout);
	return r;
}

void CommandResult::SetDataTaken()
{
	SetEvent(m_hevtResultDataTaken);
}

HRESULT CommandResult::Quit()
{
DWORD r = 0;
HRESULT hr = E_FAIL;
HANDLE hThread = 0;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_bIsQuit = true;
		SetEvent(m_hevtQuit);
		ReleaseMutex(m_mux);
		hr = S_OK;	
	}
	return hr;
}

void CommandResult::PostFinished()
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		if (m_hWnd)
			PostMessage(m_hWnd, WM_COMMANDRESULT_COMPLETED, m_id, (LPARAM)this);
		ReleaseMutex(m_mux);
	}
	SetEvent(m_hevtResultDataReady);
	WaitResultDataTakenOrQuit(INFINITE);
}

DWORD WINAPI CommandResult::ThreadProc(LPVOID lpThreadParameter)
{
	CommandResult *p = (CommandResult *)lpThreadParameter;
	HRESULT hr;
	hr = p->Run();
	return 0;
}