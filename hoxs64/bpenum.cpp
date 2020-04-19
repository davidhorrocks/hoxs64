#include "boost2005.h"
#include "util.h"
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
	{
		c = MAXLONG;
	}

	return (LONG)c;
}

bool BpEnum::GetNext(BreakpointItem& v)
{
	if (m_it == m_map->end())
	{
		return false;
	}

	v = m_it->second;
	m_it++;
	return true;
}

void BpEnum::Reset()
{
	this->m_it = m_map->begin();
}
