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
#include "register.h"
#include "bpenum.h"
#include "c6502.h"
#include "break_point.h"


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

IEnumBreakpointItem *CPU6502::CreateEnumBreakpointExecute()
{
	BpEnum *r= new BpEnum(&this->MapBpExecute);
	return r;
}

void CPU6502::ClearAllBreakpoints()
{
	MapBpExecute.clear();
	m_bBreakOnInterruptTaken = false;
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
	return m_bBreakOnInterruptTaken ;
}

bool CPU6502::SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
{
	if (MapBpExecute.size() >= BREAK_LIST_SIZE)
		return false;
	Sp_BreakpointItem bp;
	Sp_BreakpointItem v(new BreakpointItem((DBGSYM::MachineIdent::MachineIdent)this->ID, DBGSYM::BreakpointType::Execute, address, enabled, initialSkipOnHitCount, currentSkipOnHitCount));
	Sp_BreakpointKey k(new BreakpointKey((DBGSYM::MachineIdent::MachineIdent)this->ID, DBGSYM::BreakpointType::Execute, address));
	if (k == 0 || v == 0)
		return false;
	if (GetBreakpoint(bptype, address, bp))
	{
		*bp = *v;
	}
	else
	{
		MapBpExecute[k] = v;
	}
	return true;
}

bool CPU6502::GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, Sp_BreakpointItem& breakpoint)
{
	BreakpointKey key((DBGSYM::MachineIdent::MachineIdent)this->ID, DBGSYM::BreakpointType::Execute, address);
	Sp_BreakpointKey k(&key, null_deleter());
	BpMap::iterator it;
	it = MapBpExecute.find(k);
	if (it != MapBpExecute.end())
	{
		breakpoint =it->second;
		return true;
	}
	return false;
}

int CPU6502::CheckExecute(bit16 address, bool bHitIt)
{
int i = -1;
	BreakpointKey key((DBGSYM::MachineIdent::MachineIdent)this->ID, DBGSYM::BreakpointType::Execute, address);
	Sp_BreakpointKey k(&key, null_deleter());
	BpMap::iterator it;
	it = MapBpExecute.find(k);
	if (it != MapBpExecute.end())
	{
		Sp_BreakpointItem& bp = it->second;
		if (bp->enabled)
		{
			i = bp->currentSkipOnHitCount;
			if (bHitIt && i == 0)
			{
				bp->currentSkipOnHitCount = bp->initialSkipOnHitCount;
			}
		}			
	}
	return i;
} 

bool CPU6502::IsBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	BreakpointKey key((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address);
	Sp_BreakpointKey k(&key, null_deleter());
	if (MapBpExecute.find(k) != MapBpExecute.end())
		return true;
	return false;
}

void CPU6502::ClearBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)
{
	//BreakpointKey key((DBGSYM::MachineIdent::MachineIdent)this->ID, bptype, address);
	//Sp_BreakpointKey k(&key, null_deleter());
	//this->MapBpExecute.erase(k);
	Sp_BreakpointItem bp;
	if (this->GetBreakpoint(bptype, address, bp))
		this->MapBpExecute.erase(bp);
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

void CPU6502::EnableAllBreakpoints()
{
	for (BpMap::iterator it = MapBpExecute.begin(); it!=MapBpExecute.end(); it++)
		it->second->enabled = true;
}

void CPU6502::DisableAllBreakpoints()
{
	for (BpMap::iterator it = MapBpExecute.begin(); it!=MapBpExecute.end(); it++)
		it->second->enabled = false;
}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
