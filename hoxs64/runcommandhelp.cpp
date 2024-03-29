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
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"
#include "savestate.h"
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
	if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("a")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Assemble.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: a address assembly-code\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: a $c000 lda #$ff\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: a .49152 lda #.255\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: a .49152 m .169 .255\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("cls")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Clear screen.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: cls\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("r")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("CPU registers.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: r\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("cpu")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Select CPU.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: cpu [0|1|c64|disk]\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: cpu 0\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("d")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Disassemble memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: d start-address [- end-address]\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: d start-address [length]\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: d $C000 - $C010\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: d $C000 $10\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("m")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Read memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: m start-address [- end-address]\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: m start-address [length]\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: m $C000 - $C010\r"));
		this->m_pCommandResult->AddLine(TEXT("Example: m $C000 $10\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("map")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Map memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: map [c64] [kernal] [basic] [io | chargen] [ram] [roml] [romh]\r"));
		this->m_pCommandResult->AddLine(TEXT("Examples\r"));
		this->m_pCommandResult->AddLine(TEXT("Tie to C64 map: map c64\r"));
		this->m_pCommandResult->AddLine(TEXT("Select RAM: map ram\r"));
	}
	else if (_tcsicmp(this->m_pCommandResult->GetToken()->text.c_str(), TEXT("t")) == 0)
	{
		this->m_pCommandResult->AddLine(TEXT("Trace atmost a specifed count of CPU clock cycles until a breakpoint. Clocks while the CPU paused with its RDY pin held low are counted too.\r"));
		this->m_pCommandResult->AddLine(TEXT("Usage: t [count]\r"));
		this->m_pCommandResult->AddLine(TEXT("Examples\r"));
		this->m_pCommandResult->AddLine(TEXT("Trace until a breakpoint:\rt\r"));
		this->m_pCommandResult->AddLine(TEXT("Trace atmost 64 clocks until breakpoint:\rt .64\rt $40\r"));
	}
	else
	{	
		this->m_pCommandResult->AddLine(TEXT("Command Summary\r"));
		this->m_pCommandResult->AddLine(TEXT("?[command] - Help.\r"));
		this->m_pCommandResult->AddLine(TEXT("a          - Assemble.\r"));
		this->m_pCommandResult->AddLine(TEXT("cls        - Clear screen.\r"));
		this->m_pCommandResult->AddLine(TEXT("cpu        - Select CPU.\r"));
		this->m_pCommandResult->AddLine(TEXT("d          - Disassemble memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("m          - Read memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("map        - Map memory.\r"));
		this->m_pCommandResult->AddLine(TEXT("r          - CPU registers.\r"));
		this->m_pCommandResult->AddLine(TEXT("t          - Trace atmost a specifed count of CPU clock cycles until a breakpoint.\r"));
	}
	return S_OK;
}