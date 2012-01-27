#include <windows.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include <map>
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

int BreakpointKey::Compare(const BreakpointKey& y) const
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

bool BreakpointKey::operator<(const BreakpointKey& y) const
{
	return Compare(y) < 0;
}

bool LessBreakpointKey::operator()(const BreakpointKey& x, const BreakpointKey& y)
{
	return x<y;
}

BreakpointItem::BreakpointItem()
{
	this->count = 0;
}

BreakpointItem::BreakpointItem(int machine, bit16 address) 
	: BreakpointKey(machine, address)
{
}

BreakpointItem::BreakpointItem(int count)
{
	this->count = count;
}

CPU6502::BpEnum::BpEnum(BpMap &m) 
	: m_map(m)
{
	Reset();
}

int CPU6502::BpEnum::GetCount()
{
	size_t c = m_map.size();
	if (c > MAXLONG)
		c = MAXLONG;
	return (LONG)c;
}
bool CPU6502::BpEnum::GetNext(BreakpointItem& v)
{
	if (m_it == m_map.end())
		return false;

	v = m_it->second;
	m_it++;
	return true;
}
void CPU6502::BpEnum::Reset()
{
	this->m_it = m_map.begin();
}

IEnumBreakpointItem *CPU6502::GetEnumBreakpointExecute()
{
	BpEnum *r= new BpEnum(this->MapBpExecute);
	return r;
}

void CPU6502::ClearAll()
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

bool CPU6502::SetExecute(bit16 address, int count)
{
	if (MapBpExecute.size() >= BREAK_LIST_SIZE)
		return false;
	BreakpointKey key(this->ID, address);
	BreakpointItem vl = BreakpointItem(count);
	MapBpExecute[key] = vl;
	return true;
}

bool CPU6502::SetRead(bit16 address, int count)
{
	if (MapBpRead.size() >= BREAK_LIST_SIZE)
		return false;
	BreakpointKey key(this->ID, address);
	BreakpointItem vl = BreakpointItem(count);
	MapBpRead[key] = vl;
	return true;
}


bool CPU6502::SetWrite(bit16 address, int count)
{
	if (MapBpWrite.size() >= BREAK_LIST_SIZE)
		return false;
	BreakpointKey key(this->ID, address);
	BreakpointItem vl = BreakpointItem(count);
	MapBpWrite[key] = vl;
	return true;
}

int CPU6502::CheckExecute(bit16 address)
{
	BreakpointKey key(this->ID, address);
	BpMap::const_iterator it;
	it = MapBpExecute.find(key);
	if (it == MapBpExecute.end())
		return -1;
	return 0;
} 

int CPU6502::CheckRead(bit16 address)
{
	BreakpointKey key(this->ID, address);
	BpMap::const_iterator it;
	it = MapBpRead.find(key);
	if (it == MapBpRead.end())
		return -1;
	return 0;
} 

int CPU6502::CheckWrite(bit16 address)
{
	BreakpointKey key(this->ID, address);
	BpMap::const_iterator it;
	it = MapBpWrite.find(key);
	if (it == MapBpWrite.end())
		return -1;
	return 0;
} 

bool CPU6502::IsBreakPoint(bit16 address)
{
	BreakpointKey key(this->ID, address);
	if (MapBpExecute.find(key) != MapBpExecute.end())
		return true;
	if (MapBpRead.find(key) != MapBpRead.end())
		return true;
	if (MapBpWrite.find(key) != MapBpWrite.end())
		return true;
	return false;
}


void CPU6502::ClearBreakPoint(bit16 address)
{
	BreakpointKey key(this->ID, address);
	this->MapBpExecute.erase(key);
	this->MapBpRead.erase(key);
	this->MapBpWrite.erase(key);
}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
