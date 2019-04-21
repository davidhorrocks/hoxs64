#include <windows.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "savestate.h"
#include "register.h"
#include "bpenum.h"
#include "c6502.h"
#include "breakpoint.h"


BreakpointKey::BreakpointKey()
{
	this->machineident = DBGSYM::MachineIdent::MainCpu;
	this->bptype = DBGSYM::BreakpointType::Execute;
	this->address = 0;
	this->vic_cycle = 1;
	this->vic_line = 0;
}

BreakpointKey::BreakpointKey(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	this->machineident = machineident;
	this->bptype = bptype;
	this->address = address;
	this->vic_cycle = 1;
	this->vic_line = 0;
}

int BreakpointKey::Compare(BreakpointKey y) const
{
const BreakpointKey& x = *this;
	if (x.machineident < y.machineident)
		return -1;
	else if (x.machineident > y.machineident)
		return 1;

	if (x.bptype < y.bptype)
		return -1;
	else if (x.bptype > y.bptype)
		return 1;

	switch (x.bptype)
	{
	case DBGSYM::BreakpointType::Execute:
	case DBGSYM::BreakpointType::Read:
	case DBGSYM::BreakpointType::Write:
		if (x.address < y.address)
			return -1;		
		else if (x.address > y.address)
			return 1;
		return 0;
	case DBGSYM::BreakpointType::VicRasterCompare:
		if (x.vic_line < y.vic_line)
			return -1;		
		else if (x.vic_line > y.vic_line)
			return 1;
		if (x.vic_cycle < y.vic_cycle)
			return -1;		
		else if (x.vic_cycle > y.vic_cycle)
			return 1;
		return 0;
	}
	return 0;
}

bool BreakpointKey::operator==(const BreakpointKey y) const
{
	return Compare(y) == 0;
}

bool BreakpointKey::operator<(const BreakpointKey y) const
{
	return Compare(y) < 0;
}

bool BreakpointKey::operator>(const BreakpointKey y) const
{
	return Compare(y) > 0;
}

bool LessBreakpointKey::operator()(const Sp_BreakpointKey x, const Sp_BreakpointKey y) const
{
	return *x < *y;
}

BreakpointItem::BreakpointItem()
{
	this->enabled = true;
	this->initialSkipOnHitCount = 0;
	this->currentSkipOnHitCount = 0;
}

BreakpointItem::BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
	: BreakpointKey(machineident, bptype, address)
{
	this->enabled = enabled;
	this->initialSkipOnHitCount = initialSkipOnHitCount;
	this->currentSkipOnHitCount = currentSkipOnHitCount;
}

void CPU6502::SetBreakOnInterruptTaken()
{
	m_bBreakOnInterruptTaken = true;
}

void CPU6502::ClearBreakOnInterruptTaken()
{
	m_bBreakOnInterruptTaken = false;
}

bool CPU6502::GetBreakOnInterruptTaken()
{
	return m_bBreakOnInterruptTaken;
}

void CPU6502::SetStepOverBreakpoint()
{
    m_bEnableStepOverBreakpoint = true;
	m_bStepOverGotNextAddress = false;
	m_stepOverAddressBreakpoint = 0;
	m_bStepOverBreakNextInstruction = false;
}

void CPU6502::ClearStepOverBreakpoint()
{
	m_bEnableStepOverBreakpoint = false;
	m_bStepOverGotNextAddress = false;
	m_stepOverAddressBreakpoint = 0;
	m_bStepOverBreakNextInstruction = false;
}

void CPU6502::ClearTemporaryBreakpoints()
{
	this->ClearStepOverBreakpoint();
	this->ClearBreakOnInterruptTaken();
}

bool CPU6502::SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
{
	Sp_BreakpointItem v(new BreakpointItem((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address, enabled, initialSkipOnHitCount, currentSkipOnHitCount));
	if (v == 0)
		return false;
	return this->m_pIBreakpointManager->BM_SetBreakpoint(v);
}

bool CPU6502::GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, Sp_BreakpointItem& breakpoint)
{
	BreakpointKey key((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address);
	Sp_BreakpointKey k(&key, null_deleter());
	return this->m_pIBreakpointManager->BM_GetBreakpoint(k, breakpoint);
}

// Returns: 
// -1: No execute breakpoint is hit.
// > 0: The hit count remaining from a counted breakpoint.
// 0: An execute breakpoint is hit.
int CPU6502::CheckExecute(bit16 address, bool bHitIt)
{
int i = -1;

	if (m_bEnableStepOverBreakpoint)
	{
		if (!m_bStepOverGotNextAddress)
		{			
			if (!this->IsOpcodeFetch())
			{
				if (this->m_op_code == JSR_ABSOLUTE)
				{
					m_stepOverAddressBreakpoint = this->m_CurrentOpcodeAddress.word + (bit16)CPU6502::AssemblyData[this->m_op_code].size;
					m_bStepOverGotNextAddress = true;
				}
				else
				{
					m_bStepOverBreakNextInstruction = true;
					m_bEnableStepOverBreakpoint = false;
				}
			}
		}

		if (m_bStepOverGotNextAddress && address == m_stepOverAddressBreakpoint)
		{
			return 0;
		}
	}

	// Check for opcode fetch cycle
	if (this->IsOpcodeFetch())
	{	
		if (this->PROCESSOR_INTERRUPT == 0)
		{
			// An interrupt sequence is not currently being taken.
			// This is a normal instruction opcode.
			Sp_BreakpointItem bp;
			if (GetBreakpoint(DBGSYM::BreakpointType::Execute, address, bp))
			{
				if (bp->enabled)
				{
					i = bp->currentSkipOnHitCount;
					if (bHitIt && i == 0)
					{
						bp->currentSkipOnHitCount = bp->initialSkipOnHitCount;
					}
				}			
			}
		}

		if (m_bStepOverBreakNextInstruction)
		{
			// We wanted to break on either the next instruction or the next interrupt instruction sequence.
			i = 0;
		}
	}

	return i;
} 


bool CPU6502::IsBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	Sp_BreakpointItem bp;
	return GetBreakpoint(bptype, address, bp);
}

void CPU6502::DeleteBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	Sp_BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
	{
		this->m_pIBreakpointManager->BM_DeleteBreakpoint(bp);
	}
}

void CPU6502::EnableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	Sp_BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
		bp->enabled = true;
}

void CPU6502::DisableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	Sp_BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
		bp->enabled = false;
}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
