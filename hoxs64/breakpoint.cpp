#include <windows.h>
#include "dx_version.h"
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "savestate.h"
#include "register.h"
#include "bpenum.h"
#include "c6502.h"
#include "breakpoint.h"

BreakpointItem::BreakpointItem() noexcept
{
	this->machineident = DBGSYM::MachineIdent::MainCpu;
	this->bptype = DBGSYM::BreakpointType::Execute;
	this->address = 0;
	this->vic_cycle = 1;
	this->vic_line = 0;
	this->enabled = true;
	this->initialSkipOnHitCount = 0;
	this->currentSkipOnHitCount = 0;
}

BreakpointItem::BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) noexcept
	:BreakpointItem()
{
	this->machineident = machineident;
	this->bptype = bptype;
	this->address = address;
	this->vic_cycle = 1;
	this->vic_line = 0;
}

BreakpointItem::BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) noexcept
	: BreakpointItem(machineident, bptype, address)
{
	this->enabled = enabled;
	this->initialSkipOnHitCount = initialSkipOnHitCount;
	this->currentSkipOnHitCount = currentSkipOnHitCount;
}

int BreakpointItem::Compare(const BreakpointItem& y) const noexcept
{
	const BreakpointItem& x = *this;
	if (x.machineident < y.machineident)
	{
		return -1;
	}
	else if (x.machineident > y.machineident)
	{
		return 1;
	}

	if (x.bptype < y.bptype)
	{
		return -1;
	}
	else if (x.bptype > y.bptype)
	{
		return 1;
	}

	switch (x.bptype)
	{
	case DBGSYM::BreakpointType::Execute:
	case DBGSYM::BreakpointType::Read:
	case DBGSYM::BreakpointType::Write:
		if (x.address < y.address)
		{
			return -1;
		}
		else if (x.address > y.address)
		{
			return 1;
		}

		return 0;
	case DBGSYM::BreakpointType::VicRasterCompare:
		if (x.vic_line < y.vic_line)
		{
			return -1;
		}
		else if (x.vic_line > y.vic_line)
		{
			return 1;
		}

		if (x.vic_cycle < y.vic_cycle)
		{
			return -1;
		}
		else if (x.vic_cycle > y.vic_cycle)
		{
			return 1;
		}

		return 0;
	}

	return 0;
}

bool BreakpointItem::operator==(const BreakpointItem& y) const noexcept
{
	return Compare(y) == 0;
}

bool BreakpointItem::operator<(const BreakpointItem& y) const noexcept
{
	return Compare(y) < 0;
}

bool BreakpointItem::operator>(const BreakpointItem& y) const noexcept
{
	return Compare(y) > 0;
}

bool LessBreakpointItem::operator()(const BreakpointItem& x, const BreakpointItem& y) const noexcept
{
	return x.Compare(y) < 0;
}

void CPU6502::SetBreakOnInterruptTaken() noexcept
{
	m_bBreakOnInterruptTaken = true;
}

void CPU6502::ClearBreakOnInterruptTaken() noexcept
{
	m_bBreakOnInterruptTaken = false;
}

bool CPU6502::GetBreakOnInterruptTaken() noexcept
{
	return m_bBreakOnInterruptTaken;
}

void CPU6502::SetStepCountBreakpoint(bit64 stepCount) noexcept
{
	m_bEnableStepCountBreakpoint = true;
	m_StepCountRemaining = stepCount;
}

void CPU6502::ClearStepCountBreakpoint() noexcept
{
	m_bEnableStepCountBreakpoint = false;
	m_StepCountRemaining = 0;
}

void CPU6502::SetStepOverBreakpoint() noexcept
{
    m_bEnableStepOverBreakpoint = true;
	m_bStepOverGotNextAddress = false;
	m_stepOverAddressBreakpoint = 0;
	m_bStepOverBreakNextInstruction = false;
}

void CPU6502::ClearStepOverBreakpoint()  noexcept
{
	m_bEnableStepOverBreakpoint = false;
	m_bStepOverGotNextAddress = false;
	m_stepOverAddressBreakpoint = 0;
	m_bStepOverBreakNextInstruction = false;
	m_bHasStepped = false;
}

void CPU6502::SetStepOutWithRtsRtiPlaTsx() noexcept
{
    m_bEnableStepOutWithRtsRtiPlaPlpTxs = true;
	m_stepOutStackPointer = this->mSP;
}

void CPU6502::ClearStepOutWithRtsRtiPlaTsx() noexcept
{
	m_bEnableStepOutWithRtsRtiPlaPlpTxs = false;
	m_stepOutStackPointer = 0xff;
}

void CPU6502::SetStepOutWithRtsRti()
{
    m_bEnableStepOutWithRtsRti = true;
	m_stepOutStackPointer = this->mSP;
}

void CPU6502::ClearStepOutWithRtsRti() noexcept
{
	m_bEnableStepOutWithRtsRti = false;
	m_stepOutStackPointer = 0xff;
}

void CPU6502::ClearTemporaryBreakpoints() noexcept
{
	this->ClearStepCountBreakpoint();
	this->ClearStepOverBreakpoint();
	this->ClearBreakOnInterruptTaken();
	this->ClearStepOutWithRtsRtiPlaTsx();
}

bool CPU6502::SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
{
	BreakpointItem breakpoint((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address, enabled, initialSkipOnHitCount, currentSkipOnHitCount);
	return this->m_pIBreakpointManager->BM_SetBreakpoint(breakpoint);
}

bool CPU6502::GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, BreakpointItem& breakpoint)
{
	BreakpointItem key((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address);
	return this->m_pIBreakpointManager->BM_GetBreakpoint(key, breakpoint);
}

// Returns: 
// -1: No execute breakpoint is hit.
// > 0: The hit count remaining from a counted breakpoint.
// 0: An execute breakpoint is hit.
int CPU6502::CheckExecute(bit16 address, bool bHitIt)
{
	int i = -1;

	if (m_bEnableStepCountBreakpoint)
	{
		if (m_StepCountRemaining > 0)
		{
			--m_StepCountRemaining;
		}

		if (m_StepCountRemaining == 0)
		{
			m_bEnableStepCountBreakpoint = false;
			return 0;
		}
	}

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

	if (m_bEnableStepOutWithRtsRtiPlaPlpTxs)
	{
		if (this->RDY != 0)
		{
			if (this->m_cpu_sequence == C_RTS_5 || this->m_cpu_sequence == C_RTI_5)
			{
				if ((signed)this->mSP > (signed)m_stepOutStackPointer)
				{
					return 0;
				}
			}
			else if (this->m_cpu_sequence == TXS_IMPLIED)
			{
				if ((signed)this->mX > (signed)m_stepOutStackPointer)
				{
					return 0;
				}
			}
			else if (this->m_cpu_sequence == C_PLP_IMPLIED_3 || this->m_cpu_sequence == C_PLA_IMPLIED_3)
			{
				if ((signed)this->mSP > (signed)m_stepOutStackPointer)
				{
					return 0;
				}
			}
		}
	}

	if (m_bEnableStepOutWithRtsRti)
	{
		if (this->RDY != 0)
		{
			if (this->m_cpu_sequence == C_RTS_5 || this->m_cpu_sequence == C_RTI_5)
			{
				if ((signed)this->mSP > (signed)m_stepOutStackPointer)
				{
					return 0;
				}
			}
		}
	}

	// Check for opcode fetch cycle
	if (this->IsOpcodeFetch())
	{	
		if (this->PROCESSOR_INTERRUPT == 0)
		{
			// An interrupt sequence is not currently being taken.
			// This is a normal instruction opcode.
			BreakpointItem bp;
			if (GetBreakpoint(DBGSYM::BreakpointType::Execute, address, bp))
			{
				if (bp.enabled)
				{
					i = bp.currentSkipOnHitCount;
					if (bHitIt && i == 0)
					{
						bp.currentSkipOnHitCount = bp.initialSkipOnHitCount;
						m_pIBreakpointManager->BM_SetBreakpoint(bp);
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
	BreakpointItem bp;
	return GetBreakpoint(bptype, address, bp);
}

void CPU6502::DeleteBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
	{
		this->m_pIBreakpointManager->BM_DeleteBreakpoint(bp);
	}
}

void CPU6502::EnableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
	{
		bp.enabled = true;
		this->m_pIBreakpointManager->BM_SetBreakpoint(bp);
	}
}

void CPU6502::DisableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
	{
		bp.enabled = false;
		this->m_pIBreakpointManager->BM_SetBreakpoint(bp);
	}
}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
