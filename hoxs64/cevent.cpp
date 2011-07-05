#include <windows.h>
#include <assert.h>
#include "CArray.h"
#include "MList.h"
#include "cevent.h"

EventSource::EventSource()
{
}

EventSource::~EventSource()
{
HSink p;

	for (p = m_lstSink.Head(); p != NULL; )
	{
		HSink t=p;
		p = p->Next();
		m_lstSink.Remove(t);
	}
}

HSink EventSource::Add(EventSink *sink)
{
	HSink hs = NULL;
	HRESULT hr = this->m_lstSink.Append(sink, &hs);
	if (SUCCEEDED(hr))
		return hs;
	else
		return NULL;
}

void EventSource::Remove(HSink hs)
{
HSink p;
	for (p = m_lstSink.Head(); p != NULL; p = p->Next())
	{
		if (hs == p)
		{
			m_lstSink.Remove(hs);
			return;
		}
	}
}

void EventSource::Raise(void *sender, EventArgs& e)
{
HSink p;
	for (p = m_lstSink.Head(); p != NULL; p = p->Next())
	{
		EventSink *p_es = p->m_data;
		if (p_es != NULL)
			p_es->Sink(sender, e);
	}
}
