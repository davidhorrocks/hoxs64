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
#include "c6502.h"
#include "break_point.h"


BreakpointKey::BreakpointKey()
{
	this->machine = 0;
	this->address = 0;
}

BreakpointKey::BreakpointKey(int machine, bit16 address)
{
	this->machine = machine;
	this->address = address;
}

int BreakpointKey::Compare(BreakpointKey y) const
{
const BreakpointKey& x = *this;
	if (x.machine < y.machine)
		return -1;
	else if (x.machine > y.machine)
		return 1;

	if (x.address < y.address)
		return -1;		
	else if (x.address > y.address)
		return 1;
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

BreakpointItem::BreakpointItem(int machine, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
	: BreakpointKey(machine, address)
{
	this->enabled = enabled;
	this->initialSkipOnHitCount = initialSkipOnHitCount;
	this->currentSkipOnHitCount = currentSkipOnHitCount;
}

CPU6502::BpEnum::BpEnum(BpMap *m) 
{
	m_map = m;
	Reset();
}

int CPU6502::BpEnum::GetCount()
{
	size_t c = m_map->size();
	if (c > MAXLONG)
		c = MAXLONG;
	return (LONG)c;
}
bool CPU6502::BpEnum::GetNext(Sp_BreakpointItem& v)
{
	if (m_it == m_map->end())
		return false;

	v = m_it->second;
	m_it++;
	return true;
}
void CPU6502::BpEnum::Reset()
{
	this->m_it = m_map->begin();
}

CPU6502::BpEnum::~BpEnum()
{
}

IEnumBreakpointItem *CPU6502::CreateEnumBreakpointExecute()
{
	BpEnum *r= new BpEnum(&this->MapBpExecute);
	return r;
}

void CPU6502::ClearAllBreakpoints()
{
	MapBpExecute.clear();
	MapBpRead.clear();
	MapBpWrite.clear();
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

bool CPU6502::SetExecute(bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
{
	if (MapBpExecute.size() >= BREAK_LIST_SIZE)
		return false;
	
	Sp_BreakpointItem v(new BreakpointItem(this->ID, address, enabled, initialSkipOnHitCount, currentSkipOnHitCount));
	Sp_BreakpointKey k(new BreakpointKey(this->ID, address));
	if (k == 0 || v == 0)
		return false;
	MapBpExecute[k] = v;
	return true;
}

bool CPU6502::SetRead(bit16 address, int count)
{
	if (MapBpRead.size() >= BREAK_LIST_SIZE)
		return false;
	Sp_BreakpointItem v(new BreakpointItem(this->ID, address, true, count, 0));
	Sp_BreakpointKey k(new BreakpointKey(this->ID, address));
	if (k == 0 || v == 0)
		return false;
	MapBpRead[k] = v;
	return true;
}


bool CPU6502::SetWrite(bit16 address, int count)
{
	if (MapBpWrite.size() >= BREAK_LIST_SIZE)
		return false;
	Sp_BreakpointItem v(new BreakpointItem(this->ID, address, true, count, 0));
	Sp_BreakpointKey k(new BreakpointKey(this->ID, address));
	if (k == 0 || v == 0)
		return false;
	MapBpWrite[k] = v;
	return true;
}

int CPU6502::CheckExecute(bit16 address, bool bHitIt)
{
int i = -1;
	BreakpointKey key(this->ID, address);
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

int CPU6502::CheckRead(bit16 address, bool bHitIt)
{
int i = -1;
	BreakpointKey key(this->ID, address);
	Sp_BreakpointKey k(&key, null_deleter());
	BpMap::iterator it;
	it = MapBpRead.find(k);
	if (it != MapBpExecute.end())
	{
		Sp_BreakpointItem& bp = it->second;
		if (bp->enabled)
		{
			int i = bp->currentSkipOnHitCount;
			if (bHitIt && i == 0)
			{
				bp->currentSkipOnHitCount = bp->initialSkipOnHitCount;
			}
		}			
	}
	return i;
} 

int CPU6502::CheckWrite(bit16 address, bool bHitIt)
{
int i = -1;
	BreakpointKey key(this->ID, address);
	Sp_BreakpointKey k(&key, null_deleter());
	BpMap::iterator it;
	it = MapBpWrite.find(k);
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

bool CPU6502::IsBreakPoint(bit16 address)
{
	BreakpointKey key(this->ID, address);
	Sp_BreakpointKey k(&key, null_deleter());
	if (MapBpExecute.find(k) != MapBpExecute.end())
		return true;
	if (MapBpRead.find(k) != MapBpRead.end())
		return true;
	if (MapBpWrite.find(k) != MapBpWrite.end())
		return true;
	return false;
}


void CPU6502::ClearBreakPoint(bit16 address)
{
	BreakpointKey key(this->ID, address);
	Sp_BreakpointKey k(&key, null_deleter());
	this->MapBpExecute.erase(k);
	this->MapBpRead.erase(k);
	this->MapBpWrite.erase(k);

}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
