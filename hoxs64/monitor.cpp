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
#include "cevent.h"
#include "bits.h"
#include "util.h"
#include "register.h"
#include "savestate.h"
#include "c6502.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"
#include "monitor.h"

RadixChangedEventArgs::RadixChangedEventArgs(DBGSYM::MonitorOption::Radix radix)
{
	this->Radix = radix;
}

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
	m_mux = NULL;
	this->radix = DBGSYM::MonitorOption::Hex;
}

Monitor::~Monitor()
{
	if (m_mux)
	{
		CloseHandle(m_mux);
		m_mux = NULL;
	}
}

HRESULT Monitor::Init(IC64Event *pIC64Event, IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk)
{
HRESULT hr = E_FAIL;
	try
	{
		this->m_pIC64Event = pIC64Event;
		this->m_pMonitorMainCpu = pMonitorMainCpu;
		this->m_pMonitorDiskCpu = pMonitorDiskCpu;
		this->m_pMonitorVic = pMonitorVic;
		this->m_pMonitorDisk = pMonitorDisk;

		m_mux = CreateMutex(NULL, FALSE, NULL);
		if (m_mux==NULL)
			throw std::runtime_error("CreateMutex failed in Monitor::Init()");
		hr = S_OK;
	}
	catch(...)
	{
		hr = E_FAIL;
	}
	return hr;
}

void Monitor::MonitorEventsOn()
{
}

void Monitor::MonitorEventsOff()
{
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
TCHAR szNumber[BUFSIZEADDRESSTEXT];
int instruction_size;
TCHAR szAssembly[BUFSIZEMNEMONICTEXT];
bit8 operandByte;
bit16 operandWord;
bit16 absAddress;

	if (pMonitorCpu == NULL)
	{
		return 0;
	}

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
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly, TEXT(" #"));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);
		}

		lstrcat(szAssembly, szNumber);
		break;
	case amZEROPAGE:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly, TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly, TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);
		}

		lstrcat(szAssembly, szNumber);
		break;
	case amZEROPAGEX:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly, TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly, TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);			
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(",X"));
		break;
	case amZEROPAGEY:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly,TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly, TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(",Y"));
		break;
	case amABSOLUTE:
		instruction_size=3;
		operandWord = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly, TEXT(" "));
		operandWord |= ((bit16)pMonitorCpu->MonReadByte(address+2, memorymap) << 8);
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandWord);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex((bit32)operandWord, szNumber, 4);
		}

		lstrcat(szAssembly, szNumber);
		break;
	case amABSOLUTEX:
		instruction_size=3;
		operandWord = pMonitorCpu->MonReadByte(address+1, memorymap);
		operandWord |= ((bit16)pMonitorCpu->MonReadByte(address+2, memorymap) << 8);
		lstrcat(szAssembly,TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandWord);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex((bit32)operandWord, szNumber, 4);
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(",X"));
		break;
	case amABSOLUTEY:
		instruction_size=3;
		operandWord = pMonitorCpu->MonReadByte(address+1, memorymap);
		operandWord |= ((bit16)pMonitorCpu->MonReadByte(address+2, memorymap) << 8);
		lstrcat(szAssembly,TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandWord);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex((bit32)operandWord, szNumber, 4);
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(",Y"));
		break;
	case amINDIRECT:
		instruction_size=3;
		operandWord = pMonitorCpu->MonReadByte(address+1, memorymap);
		operandWord |= ((bit16)pMonitorCpu->MonReadByte(address+2, memorymap) << 8);
		lstrcat(szAssembly,TEXT(" ("));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandWord);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex((bit32)operandWord, szNumber, 4);
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(")"));
		break;
	case amINDIRECTX:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly,TEXT(" ("));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);
		}

		lstrcat(szAssembly, szNumber);
		lstrcat(szAssembly, TEXT(",X)"));
		break;
	case amINDIRECTY:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		lstrcat(szAssembly,TEXT(" ("));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)operandByte);
		}
		else
		{
			lstrcat(szAssembly,TEXT("$"));
			HexConv::long_to_hex(operandByte, szNumber, 2);
		}

		lstrcat(szAssembly,szNumber);
		lstrcat(szAssembly,TEXT("),Y"));
		break;
	case amRELATIVE:
		instruction_size=2;
		operandByte = pMonitorCpu->MonReadByte(address+1, memorymap);
		if (operandByte & 0x80)
		{
			absAddress = ((bit16) address - (bit16) ((bit8)~(operandByte) + (bit8)1) + (bit16) 2);
		}
		else
		{
			absAddress = ((bit16) address + (bit16) operandByte + (bit16) 2);
		}

		lstrcat(szAssembly, TEXT(" "));
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)absAddress);
		}
		else
		{
			lstrcat(szAssembly, TEXT("$"));
			HexConv::long_to_hex((bit32)absAddress, szNumber, 4);
		}

		lstrcat(szAssembly,szNumber);
		break;
	default:		
		instruction_size=1;
		break;
	}

	isUndoc = ii.undoc!=0;
	if (pMnemonicText != NULL && cchMnemonicText > 0)
	{
		_tcsncpy_s(pMnemonicText, cchMnemonicText, szAssembly, _TRUNCATE);
	}

	if (pBytesText != NULL && cchBytesText > 0)
	{
		DisassembleBytes(pMonitorCpu, address, memorymap, instruction_size, pBytesText, cchBytesText);
	}

	if (pAddressText != NULL && cchAddressText > 0)
	{
		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			_sntprintf_s(szAddress, _countof(szNumber), _TRUNCATE, TEXT("%5d"), (unsigned int)address);
		}
		else
		{
			szAddress[0]=TEXT('$');
			HexConv::long_to_hex(address, &szAddress[1], 4);
		}

		_tcsncpy_s(pAddressText, cchAddressText, szAddress, _TRUNCATE);
	}
	
	return instruction_size;
}

int Monitor::DisassembleBytes(IMonitorCpu *pMonitorCpu, bit16 address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer)
{
TCHAR *s;
unsigned char b;
const unsigned int HEXDIGITS = 2;
const unsigned int DECDIGITS = 3;

	if (pMonitorCpu == NULL)
	{
		return 0;
	}

	if (pBuffer == NULL || cchBuffer <=0)
	{
		return 0;
	}

	s=pBuffer;
	*s=0;
	int charsRemaining = cchBuffer - 1;
	int elementsFormatted = 0;
	int c;
	while (elementsFormatted < count)
	{
		b = pMonitorCpu->MonReadByte(address, memorymap);
		if (elementsFormatted > 0)
		{
			if (charsRemaining < 1)
			{
				break;
			}

			s[0] = TEXT(' ');
			s++;
			charsRemaining--;
		}

		if (this->radix == DBGSYM::MonitorOption::Dec)
		{
			if (charsRemaining < DECDIGITS)
			{
				break;
			}
				
			c = _sntprintf_s(s, charsRemaining + 1, _TRUNCATE, TEXT("%3d"), (unsigned int)b);
			s += c;
			charsRemaining -= c;
		}
		else
		{
			if (charsRemaining < HEXDIGITS)
			{
				break;
			}
		
			HexConv::long_to_hex(b, s, HEXDIGITS);
			s += HEXDIGITS;
			charsRemaining -= HEXDIGITS;
		}

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
	{
		this->MapBpExecute.erase(k);
	}

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
	{
		it->second->enabled = true;
	}

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->BreakpointChanged();
	}
}

void Monitor::BM_DisableAllBreakpoints() 
{
	for (BpMap::iterator it = MapBpExecute.begin(); it!=MapBpExecute.end(); it++)
	{
		it->second->enabled = false;
	}

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

HRESULT Monitor::BeginExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, shared_ptr<ICommandResult> *pICommandResult)
{
	HRESULT hr = E_FAIL;
	shared_ptr<ICommandResult> p;
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		try
		{
			p = shared_ptr<ICommandResult>(new CommandResult(this, cpumode, iDebuggerMmuIndex, iDefaultAddress));
			if (!p)
			{
				throw std::bad_alloc();
			}

			hr = E_FAIL;
			m_lstCommandResult.push_back(p);
			try
			{
				hr = p->Start(hwnd, pszCommandLine, id);
				if (SUCCEEDED(hr))
				{
					*pICommandResult = p;
				}
			}
			catch(...)
			{
			}

			if (FAILED(hr))
			{
				m_lstCommandResult.remove(p);
			}
		}
		catch(...)
		{
			hr = E_FAIL;
			if (p)
			{
				p.reset();
			}
		}

		ReleaseMutex(m_mux);
	}
	return hr;
}

HRESULT Monitor::EndExecuteCommandLine(shared_ptr<ICommandResult> pICommandResult)
{
	HRESULT hr = E_FAIL;
	try
	{
		pICommandResult->SetDataTaken();
		pICommandResult->SetAllLinesTaken();
		DWORD r = pICommandResult->WaitFinished(INFINITE);
		if (r == WAIT_OBJECT_0)
		{
			hr = S_OK;
			this->RemoveCommand(pICommandResult);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	return hr;	
}

void Monitor::QuitCommands()
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		for(list<shared_ptr<ICommandResult>>::iterator it = m_lstCommandResult.begin(); it!=m_lstCommandResult.end(); it++)
		{
			(*it)->Quit();
		}

		while (true)
		{
			if(rm == WAIT_OBJECT_0)
			{
				list<shared_ptr<ICommandResult>>::iterator it = m_lstCommandResult.begin();
				if (it == m_lstCommandResult.end())
				{
					ReleaseMutex(m_mux);
					break;
				}

				shared_ptr<ICommandResult> k = *it;
				m_lstCommandResult.erase(it);
				ReleaseMutex(m_mux);
				k->Quit();
				k->WaitFinished(INFINITE);
				k->Reset();
			}
			else
			{
				break;//should not happen
			}

			rm = WaitForSingleObject(m_mux, INFINITE);
		}
	}
}

void Monitor::QuitCommand(shared_ptr<ICommandResult> pICommandResult)
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		RemoveCommand(pICommandResult);
		ReleaseMutex(m_mux);
		pICommandResult->Quit();
		pICommandResult->WaitFinished(INFINITE);
	}
}

void Monitor::RemoveCommand(shared_ptr<ICommandResult> pICommandResult)
{
	DWORD rm = WaitForSingleObject(m_mux, INFINITE);
	if (rm == WAIT_OBJECT_0)
	{
		m_lstCommandResult.remove(pICommandResult);
		ReleaseMutex(m_mux);
	}
}

HRESULT Monitor::ExecuteCommandLine(HWND hwnd, LPCTSTR pszCommandLine, int id, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress, LPTSTR *ppszResults)
{
	CommandToken *pcmdt = 0;
	HRESULT hr = E_FAIL;
	LPTSTR ps = 0;
	shared_ptr<ICommandResult> pcr;
	DWORD dwWaitResult;
	try
	{
		hr = this->BeginExecuteCommandLine(hwnd, pszCommandLine, id, cpumode, iDebuggerMmuIndex, iDefaultAddress, &pcr);
		if (SUCCEEDED(hr))
		{
			dwWaitResult = pcr->WaitDataReady(INFINITE);
			if (dwWaitResult == WAIT_OBJECT_0)
			{
				hr =  EndExecuteCommandLine(pcr);
				if (SUCCEEDED(hr))
				{
					std::basic_string<TCHAR> s;
					LPCTSTR pline = 0;
					while(SUCCEEDED(pcr->GetNextLine(&pline)))
					{
						if (!pline)
						{
							break;
						}

						s.append(pline);
					}

					ps = _tcsdup(s.c_str());
				}
			}
			else
			{
				pcr->Quit();
				EndExecuteCommandLine(pcr);
				ps = _tcsdup(TEXT("Command timeout."));
			}
		}
		hr = E_FAIL;
		if (ppszResults)
		{
			if (ps)
			{
				hr = S_OK;
			}

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
		pcr->Quit();
		//delete pcr;
		//pcr = 0;
		pcr.reset();
	}

	if (pcmdt)
	{
		delete pcmdt;
		pcmdt = 0;
	}
	return hr;
}

DBGSYM::MonitorOption::Radix Monitor::Get_Radix()
{
DBGSYM::MonitorOption::Radix r = DBGSYM::MonitorOption::Hex;
	DWORD index = WaitForSingleObject(m_mux, INFINITE);
	if (index == WAIT_OBJECT_0)
	{
		r = this->radix;
		ReleaseMutex(m_mux);
	}

	return r;
}

void Monitor::Set_Radix(DBGSYM::MonitorOption::Radix radix)
{
	DWORD index = WaitForSingleObject(m_mux, INFINITE);
	if (index == WAIT_OBJECT_0)
	{
		this->radix = radix;
		ReleaseMutex(m_mux);
	}

	if (m_pIC64Event && m_bMonitorEvents)
	{
		m_pIC64Event->RadixChanged(radix);
	}
}
