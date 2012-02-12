#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "bits.h"
#include "register.h"
#include "errormsg.h"
#include "C6502.h"
#include "hexconv.h"
#include "monitor.h"

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
	m_pMonitorMainCpu = NULL;
	m_pMonitorDiskCpu = NULL;
	m_pMonitorVic = NULL;
	m_pMonitorDisk = NULL;
}

HRESULT Monitor::Init(int iCpuId, IMonitorCpu *pMonitorMainCpu, IMonitorCpu *pMonitorDiskCpu, IMonitorVic *pMonitorVic, IMonitorDisk *pMonitorDisk)
{
	m_iCpuId = iCpuId;
	this->m_pMonitorMainCpu = pMonitorMainCpu;
	this->m_pMonitorDiskCpu = pMonitorDiskCpu;
	this->m_pMonitorVic = pMonitorVic;
	this->m_pMonitorDisk = pMonitorDisk;
	return S_OK;
}

void Monitor::GetVicRegisters(TCHAR *pLine_Text, int cchLine_Text, TCHAR *pCycle_Text, int cchCycle_Text)
{
TCHAR szWord[5];

	bit16 line = m_pMonitorVic->GetRasterLine();
	bit8 cycle = m_pMonitorVic->GetRasterCycle();

	if (pLine_Text != NULL && cchLine_Text > 0)
	{
		HexConv::long_to_hex(line, &szWord[0], 3);
		_tcsncpy_s(pLine_Text, cchLine_Text, szWord, _TRUNCATE);
	}

	if (pCycle_Text != NULL && cchCycle_Text > 0)
	{
		_stprintf_s(pCycle_Text, cchCycle_Text,TEXT("%d"), cycle);

		//HexConv::long_to_hex(cycle, &szByte[0], 2);
		//_tcsncpy_s(pCycle_Text, cchCycle_Text, szByte, _TRUNCATE);
	}
}

void Monitor::GetCpuRegisters(TCHAR *pPC_Text, int cchPC_Text, TCHAR *pA_Text, int cchA_Text, TCHAR *pX_Text, int cchX_Text, TCHAR *pY_Text, int cchY_Text, TCHAR *pSR_Text, int cchSR_Text, TCHAR *pSP_Text, int cchSP_Text, TCHAR *pDDR_Text, int cchDDR_Text, TCHAR *pData_Text, int cchData_Text)
{
TCHAR szWord[5];
TCHAR szByte[5];
TCHAR szBitByte[10];
CPUState cpu;

	this->GetCpu()->GetCpuState(cpu);

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

int Monitor::GetCpuId()
{
	return m_iCpuId;
}

IMonitorCpu *Monitor::GetCpu()
{
	if (this->m_iCpuId == CPUID_MAIN)
	{
		return this->m_pMonitorMainCpu;
	}
	else if (this->m_iCpuId == CPUID_DISK)
	{
		return this->m_pMonitorDiskCpu;
	}
	else
	{
		return NULL;
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

int Monitor::DisassembleOneInstruction(bit16 address, int memorymap, TCHAR *pAddressText, int cchAddressText, TCHAR *pBytesText, int cchBytesText, TCHAR *pMnemonicText, int cchMnemonicText, bool &isUndoc)
{
TCHAR szAddress[BUFSIZEADDRESSTEXT];
TCHAR szHex[BUFSIZEADDRESSTEXT];
int instruction_size;
TCHAR szAssembly[BUFSIZEMNEMONICTEXT];
bit8 operand1;

	if (this->GetCpu() == NULL)
		return 0;

	szAssembly[0]=0;
	bit8 opcode = this->GetCpu()->MonReadByte(address, memorymap);

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
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amZEROPAGE:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amZEROPAGEX:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X"));
		break;
	case amZEROPAGEY:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",Y"));
		break;
	case amABSOLUTE:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		break;
	case amABSOLUTEX:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X"));
		break;
	case amABSOLUTEY:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" $"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",Y"));
		break;
	case amINDIRECT:
		instruction_size=3;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+2, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(")"));
		break;
	case amINDIRECTX:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT(",X)"));
		break;
	case amINDIRECTY:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" ($"));
		HexConv::long_to_hex(this->GetCpu()->MonReadByte(address+1, memorymap), szHex, 2);
		lstrcat(szAssembly,szHex);
		lstrcat(szAssembly,TEXT("),Y"));
		break;
	case amRELATIVE:
		instruction_size=2;
		lstrcat(szAssembly,TEXT(" $"));
		
		operand1=this->GetCpu()->MonReadByte(address+1, memorymap);
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
		DisassembleBytes(address, memorymap, instruction_size, pBytesText, cchMnemonicText);
	}

	if (pAddressText != NULL && cchAddressText > 0)
	{
		szAddress[0]=TEXT('$');
		HexConv::long_to_hex(address, &szAddress[1], 4);
		_tcsncpy_s(pAddressText, cchAddressText, szAddress, _TRUNCATE);
	}
	
	return instruction_size;
}

int Monitor::AssembleMnemonic(bit8 address, int memorymap, const TCHAR *ppMnemonicText)
{
	return 0;
}

int Monitor::AssembleBytes(bit8 address, int memorymap, const TCHAR *ppBytesText)
{
	return 0;
}


int Monitor::DisassembleBytes(unsigned short address, int memorymap, int count, TCHAR *pBuffer, int cchBuffer)
{
TCHAR *s;
unsigned char b;
const unsigned int DIGITS = 2;

	if (this->GetCpu() == NULL)
		return 0;
	if (pBuffer == NULL || cchBuffer <=0)
		return 0;
	s=pBuffer;
	*s=0;
	int charsRemaining = cchBuffer - 1;
	int elementsFormatted = 0;
	while (elementsFormatted < count){
		b = this->GetCpu()->MonReadByte(address, memorymap);

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

HRESULT Monitor::AssembleText(LPCTSTR pszText, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
	return E_FAIL;
}