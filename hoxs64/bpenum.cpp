#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "register.h"
#include "bpenum.h"


BpEnum::BpEnum(BpMap *m) 
{
	m_map = m;
	Reset();
}

int BpEnum::GetCount()
{
	size_t c = m_map->size();
	if (c > MAXLONG)
		c = MAXLONG;
	return (LONG)c;
}
bool BpEnum::GetNext(Sp_BreakpointItem& v)
{
	if (m_it == m_map->end())
		return false;

	v = m_it->second;
	m_it++;
	return true;
}
void BpEnum::Reset()
{
	this->m_it = m_map->begin();
}

BpEnum::~BpEnum()
{
}
