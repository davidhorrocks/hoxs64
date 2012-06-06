#include <windows.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "errormsg.h"
#include "hexconv.h"
#include "c64.h"


VicCursorMoveEventArgs::VicCursorMoveEventArgs(int cycle, int line)
{
	this->Cycle = cycle;
	this->Line = line;
}

BreakpointC64ExecuteChangedEventArgs::BreakpointC64ExecuteChangedEventArgs(MEM_TYPE memorymap, bit16 address, int count)
{
	this->Memorymap = memorymap;
	this->Address = address;
	this->Count = count;
}

BreakpointDiskExecuteChangedEventArgs::BreakpointDiskExecuteChangedEventArgs(bit16 address, int count)
{
	this->Address = address;
	this->Count = count;
}

Monitor::Monitor()
{
	m_bMonitorEvents = true;
	m_pMonitorMainCpu = NULL;
	m_pMonitorDiskCpu = NULL;
	m_pMonitorVic = NULL;
	m_pMonitorDisk = NULL;
}

HRESULT Monitor::Init(IC64Event *pIC64Event, IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk)
{
	this->m_pIC64Event = pIC64Event;
	this->m_pMonitorMainCpu = pMonitorMainCpu;
	this->m_pMonitorDiskCpu = pMonitorDiskCpu;
	this->m_pMonitorVic = pMonitorVic;
	this->m_pMonitorDisk = pMonitorDisk;
	return S_OK;
}

void Monitor::GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text)
{
TCHAR szWord[5];

	bit16 line = m_pMonitorVic->GetNextRasterLine();
	bit8 cycle = m_pMonitorVic->GetNextRasterCycle();

	if (pLine_Text != NULL && cchLine_Text > 0)
	{
		HexConv::long_to_hex(line, &szWord[0], 3);
		_tcsncpy_s(pLine_Text, cchLine_Text, szWord, _TRUNCATE);
	}

	if (pCycle_Text != NULL && cchCycle_Text > 0)
	{
		_stprintf_s(pCycle_Text, cchCycle_Text,TEXT("%d"), cycle);
	}
}

void Monitor::GetCpuRegisters(IMonitorCpu *pMonitorCpu, TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDDR_Text, int cchDDR_Text, TCHAR *pData_Text, int cchData_Text)
{
TCHAR szWord[5];
TCHAR szByte[5];
TCHAR szBitByte[10];
CPUState cpu;

	pMonitorCpu->GetCpuState(cpu);

	if (pPC_Text != NULL && cchPC_Text > 0)
	{
		HexConv::long_to_hex(cpu.PC_CurrentOpcode, &szWord[0], 4);
		_tcsncpy_s(pPC_Text, cchPC_Text, szWord, _TRUNCATE);
	}

	if (pA_Text != NULL && cchA_Text > 0)
	{
		HexConv::long_to_hex(cpu.A, &szByte[0], 2);
		_tcsncpy_s(pA_Text, cchA_Text, szByte, _TRUNCATE);
	}

	if (pX_Text != NULL && cchX_Text > 0)
	{
		HexConv::long_to_hex(cpu.X, &szByte[0], 2);
		_tcsncpy_s(pX_Text, cchX_Text, szByte, _TRUNCATE);
	}

	if (pY_Text != NULL && cchY_Text > 0)
	{
		HexConv::long_to_hex(cpu.Y, &szByte[0], 2);
		_tcsncpy_s(pY_Text, cchY_Text, szByte, _TRUNCATE);
	}

	if (pSP_Text != NULL && cchSP_Text > 0)
	{
		HexConv::long_to_hex(cpu.SP, &szByte[0], 2);
		_tcsncpy_s(pSP_Text, cchSP_Text, szByte, _TRUNCATE);
	}

	int i = 0;
	if (pSR_Text != NULL && cchSR_Text > 0)
	{
		pSR_Text[0] = 0;
		ZeroMemory(szBitByte, _countof(szBitByte) * sizeof(TCHAR));
		for (i=0; i < _countof(szBitByte) && i < 8; i++)
		{
			int k = (((unsigned int)cpu.Flags & (1UL << (7-i))) != 0);
			szBitByte[i] = TEXT('0') + k;
		}
		_tcsncpy_s(pSR_Text, cchSR_Text, szBitByte, _TRUNCATE);
	}

	if (pDDR_Text != NULL && cchDDR_Text > 0)
	{
		HexConv::long_to_hex(cpu.PortDdr, &szByte[0], 2);
		_tcsncpy_s(pDDR_Text, cchDDR_Text, szByte, _TRUNCATE);
	}

	if (pData_Text != NULL && cchData_Text > 0)
	{
		HexConv::long_to_hex(cpu.PortDataStored, &szByte[0], 2);
		_tcsncpy_s(pData_Text, cchData_Text, szByte, _TRUNCATE);
	}
}

IMonitorCpu *Monitor::GetMainCpu()
{
	return this->m_pMonitorMainCpu;
}

IMonitorCpu *Monitor::GetDiskCpu()
{
	return this->m_pMonitorDiskCpu;
}

IMonitorVic *Monitor::GetVic()
{
	return this->m_pMonitorVic;
}

IMonitorDisk *Monitor::GetDisk()
{
	return this->m_pMonitorDisk;
}

int Monitor::DisassembleOneInstruction(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc)
{
TCHAR szAddress[BUFSIZEADDRESSTEXT];
TCHAR szHex[BUFSIZEADDRESSTEXT];
int instruction_size;
TCHAR szAssembly[BUFSIZEMNEMONICTEXT];
bit8 operand1;

	if (pMonitorCpu == NULL)
		return 0;

	szAssembly[0]=0;
	bit8 opcode = pMonitorCpu->MonReadByte(address, memorymap);

	const InstructionInfo& ii = CPU6502::AssemblyData[opcode];

	lstrcpy(szAssembly, ii.mnemonic);

	switch (ii.address_mode)
	{
	case amIMPLIED:
		instruction_size=1;
		break;
	case amIMMEDIATE:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" #$"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amZEROPAGE:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amZEROPAGEX:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X"));
		break;
	case amZEROPAGEY:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",Y"));
		break;
	case amABSOLUTE:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amABSOLUTEX:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X"));
		break;
	case amABSOLUTEY:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",Y"));
		break;
	case amINDIRECT:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(")"));
		break;
	case amINDIRECTX:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X)"));
		break;
	case amINDIRECTY:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(pMonitorCpu->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT("),Y"));
		break;
	case amRELATIVE:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		
		operand1=pMonitorCpu->MonReadByte(address+1, memorymap);
		if (operand1 & 0x80)
			HexConv::long_to_hex(((bit16) address - (bit16) ((bit8)~(operand1) + (bit8)1) + (bit16) 2), szHex, 4);
		else
			HexConv::long_to_hex(((bit16) address + (bit16) operand1 + (bit16) 2), szHex, 4);
		lstrcat(szAssembly,szHex);
		break;
	default:		
		instruction_size=1;
		break;
	}

	isUndoc = ii.undoc!=0;

	if (pMnemonicText != NULL && cchMnemonicText > 0)
		_tcsncpy_s(pMnemonicText, cchMnemonicText, szAssembly, _TRUNCATE);

	if (pBytesText != NULL && cchBytesText > 0)
	{
		DisassembleBytes(pMonitorCpu, address, memorymap, instruction_size, pBytesText, cchMnemonicText);
	}

	if (pAddressText != NULL && cchAddressText > 0)
	{
		szAddress[0]=TEXT('$');
		HexConv::long_to_hex(address, &szAddress[1], 4);
		_tcsncpy_s(pAddressText, cchAddressText, szAddress, _TRUNCATE);
	}
	
	return instruction_size;
}

int Monitor::DisassembleBytes(IMonitorCpu *pMonitorCpu, unsigned short address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer)
{
TCHAR *s;
unsigned char b;
const unsigned int DIGITS = 2;

	if (pMonitorCpu == NULL)
		return 0;
	if (pBuffer == NULL || cchBuffer <=0)
		return 0;
	s=pBuffer;
	*s=0;
	int charsRemaining = cchBuffer - 1;
	int elementsFormatted = 0;
	while (elementsFormatted < count){
		b = pMonitorCpu->MonReadByte(address, memorymap);

		if (elementsFormatted > 0)
		{
			if (charsRemaining < 1)
				break;
			s[0]=TEXT(' ');
			s=s+1;
		}

		if (charsRemaining < DIGITS)
			break;
		
		HexConv::long_to_hex(b, s, DIGITS);
		s=s+DIGITS;

		elementsFormatted++;
		address++;
	}
	*s=0;
	return elementsFormatted;
}


bool Monitor::BM_SetBreakpoint(Sp_BreakpointItem bp)
{
	if (MapBpExecute.size() >= BREAK_LIST_SIZE)
		return false;
	Sp_BreakpointItem bp_find;
	if (BM_GetBreakpoint(bp, bp_find))
	{
		*bp_find = *bp;
	}
	else
	{
		MapBpExecute[bp] = bp;
	}
	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
	return true;
}

bool Monitor::BM_GetBreakpoint(Sp_BreakpointKey k, Sp_BreakpointItem &bp)
{
	BpMap::iterator it;
	it = MapBpExecute.find(k);
	if (it != MapBpExecute.end())
	{
		bp = it->second;
		return true;
	}
	return false;
}

void Monitor::BM_DeleteBreakpoint(Sp_BreakpointKey k)
{
Sp_BreakpointItem bp;
	if (this->BM_GetBreakpoint(k, bp))
		this->MapBpExecute.erase(k);

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_EnableBreakpoint(Sp_BreakpointKey k)
{
Sp_BreakpointItem bp;
	if (this->BM_GetBreakpoint(k, bp))
	{
		bp->enabled = true;
	}
	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_DisableBreakpoint(Sp_BreakpointKey k)
{
Sp_BreakpointItem bp;
	if (this->BM_GetBreakpoint(k, bp))
	{
		bp->enabled = false;
	}
	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_EnableAllBreakpoints()
{
	for (BpMap::iterator it = MapBpExecute.begin(); it!=MapBpExecute.end(); it++)
		it->second->enabled = true;

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_DisableAllBreakpoints() 
{
	for (BpMap::iterator it = MapBpExecute.begin(); it!=MapBpExecute.end(); it++)
		it->second->enabled = false;

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_DeleteAllBreakpoints()
{
	MapBpExecute.clear();

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

IEnumBreakpointItem *Monitor::BM_CreateEnumBreakpointItem()
{
	BpEnum *r= new BpEnum(&this->MapBpExecute);
	return r;
}

HRESULT Monitor::CreateCliCommandResult(CommandToken *pCommandTToken, CommandResult **ppCommandResult)
{
	HRESULT hr = E_FAIL;
	CommandResult *pcr = 0;
	try
	{
		switch(pCommandTToken->cmd)
		{
		case DBGSYM::CliCommand::Help:
			pcr = new CommandResultHelp();
			break;
		case DBGSYM::CliCommand::Disassemble:
			pcr = new CommandResultDisassembly(pCommandTToken->startaddress, pCommandTToken->finishaddress);
			break;
		case DBGSYM::CliCommand::Error:
			pcr = new CommandResult(pCommandTToken->text.c_str());
			break;
		default:
			pcr = new CommandResult(TEXT("Unknown command."));
			break;
		}
		if (!pcr)
			hr = E_FAIL;
		else
			hr = S_OK;
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

HRESULT Monitor::ExecuteCommandLine(LPCTSTR pszCommandLine, LPTSTR *ppszResults)
{
	Assembler as;
	CommandToken *pcmdt = 0;
	HRESULT hr = E_FAIL;
	LPTSTR ps = 0;
	CommandResult *pcr;
	try
	{
		hr = as.CreateCliCommandToken(pszCommandLine, &pcmdt);
		if (SUCCEEDED(hr))
		{
			hr =  CreateCliCommandResult(pcmdt, &pcr);
			if (SUCCEEDED(hr))
			{
				std::basic_string<TCHAR> s;

				LPCTSTR pline = 0;
				while(pcr->GetNextLine(&pline) == S_OK)
				{
					if (!pline)
						break;
					s.append(pline);
				}
				ps = _tcsdup(s.c_str());
			}
		}
		if (!ps)
			hr = E_FAIL;
		else
			hr = S_FALSE;
		if (ppszResults)
		{
			*ppszResults = ps;
			ps = NULL;
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
	if (pcmdt)
	{
		delete pcmdt;
		pcmdt = 0;
	}
	return hr;
}
