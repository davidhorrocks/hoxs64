#pragma once

#include "mlist.h"
#include "carray.h"

template<class A>
class EventSink;

class EventArgs
{
};

typedef MListElement<void *> *HSink;


template<class A>
class EventSource
{
public:
	EventSource() noexcept
	{

	}

	virtual ~EventSource()
	{
		RemoveAll();
	}

	EventSource(const EventSource&) = delete;
	EventSource& operator=(const EventSource&) = delete;
	EventSource(EventSource&&) = delete;
	EventSource& operator=(EventSource&&) = delete;

	HSink Advise(EventSink<A> *sink)
	{
		MListElement<EventSink<A> *> *hs = NULL;
		HRESULT hr = this->m_lstSink.Append(sink);
		if (SUCCEEDED(hr))
		{
			hs = m_lstSink.Tail();
			EventLink<A> k;
			k.elementsink = hs;
			k.source = this;
			if (sink->_AddLink(k) != NULL)
			{
				return (HSink)hs;
			}
			else
			{
				this->m_lstSink.Remove(hs);
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}

	void Unadvise(HSink hs)
	{
		MListElement<EventSink<A> *> * p;
		EventSink<A> *eventsink;
		for (p = m_lstSink.Head(); p != NULL; p = p->Next())
		{
			if (hs == p)
			{
				eventsink = p->m_data;
				if (eventsink!=NULL)
				{
					eventsink->_RemoveEventLinksBySourceElement(hs);
				}

				m_lstSink.Remove(hs);
				return;
			}
		}
	}

	void Unadvise(EventSink<A> *sink)
	{
	MListElement<EventSink<A> *> * p;
	MListElement<EventSink<A> *> * t;
	EventSink<A> *eventsink;
		for (p = m_lstSink.Head(); p != NULL;)
		{
			t = p;
			p = p->Next();
			if (sink == t->m_data)
			{
				eventsink = t->m_data;
				if (eventsink!=NULL)
				{
					eventsink->_RemoveEventLinksBySource(this);
				}

				m_lstSink.Remove(t);				
			}
		}
	}

	void Raise(void *sender, A &e)
	{
	MListElement<EventSink<A> *> * p;
		for (p = m_lstSink.Head(); p != NULL; p = p->Next())
		{
			EventSink<A> *p_es = p->m_data;
			if (p_es != nullptr)
			{
				p_es->Sink(sender, e);
			}
		}
	}
private:
	MList<EventSink<A> *> m_lstSink;

	void RemoveAll() noexcept
	{
		MListElement<EventSink<A> *> * p = nullptr;
		MListElement<EventSink<A> *> * t = nullptr;
		EventSink<A> *eventsink = nullptr;
		for (p = m_lstSink.Head(); p != nullptr; )
		{
			t = p;
			p = p->Next();
			eventsink = t->m_data;
			if (eventsink != nullptr)
			{
				eventsink->_RemoveEventLinksBySource(this);
			}

			m_lstSink.Remove(t);
		}
	}

	void _RemoveSink(const EventSink<A> *sink) noexcept
	{
		MListElement<EventSink<A> *> * p;
		MListElement<EventSink<A> *> * t;
		for (p = m_lstSink.Head(); p != nullptr;)
		{
			t = p;
			p = p->Next();
			if (sink == t->m_data)
			{
				m_lstSink.Remove(t);				
			}
		}
	}
	friend EventSink<A>;
};

template<class A>
class EventLink
{
public:
	EventSource<A> *source = nullptr;
	MListElement<EventSink<A> *> * elementsink = nullptr;
};

template<class A>
class EventSink
{
public:
	EventSink() = default;
	virtual ~EventSink()
	{
		UnadviseAll();
	}

	EventSink(const EventSink&) = default;
	EventSink& operator=(const EventSink&) = default;
	EventSink(EventSink&&) = delete;
	EventSink& operator=(EventSink&&) = delete;

	virtual int Sink(void* sender, A& e) = 0;
	void UnadviseAll() noexcept
	{
		MListElement<EventLink<A>>* p = nullptr;
		MListElement<EventLink<A>>* t = nullptr;
		EventSource<A>* s;
		for (p = m_links.Head(); p != nullptr; )
		{
			t = p;
			p = p->Next();
			s = t->m_data.source;
			if (s != nullptr)
			{
				s->_RemoveSink(this);
				m_links.Remove(t);
			}
		}
	}
private:
	MList<EventLink<A>> m_links;
	MListElement<EventLink<A>>* EventSink::_AddLink(const EventLink<A>& k) noexcept
	{
		MListElement<EventLink<A>>* r = nullptr;
		HRESULT hr = m_links.Append(k);
		if (SUCCEEDED(hr))
		{
			return m_links.Tail();
		}
		else
		{
			return nullptr;
		}
	}

	void _RemoveEventLinksBySourceElement(const MListElement<EventSink<A>*> element) noexcept
	{
		MListElement<EventLink<A>>* p = nullptr;
		MListElement<EventLink<A>>* t = nullptr;
		MListElement<EventSink<A>*> el;
		for (i = 0, p = m_links.Head(); p != nullptr; )
		{
			t = p;
			p = p->Next();
			el = t->m_data.elementsink;
			if (el == element)
			{
				m_links.Remove(t);
			}
		}
	}

	void _RemoveEventLinksBySource(const EventSource<A> * source) noexcept
	{
		MListElement<EventLink<A>>* p = nullptr;
		MListElement<EventLink<A>>* t = nullptr;
		for (p = m_links.Head(); p != nullptr; )
		{
			t = p;
			p = p->Next();
			const EventSource<A>* s = t->m_data.source;
			if (s == source)
			{
				m_links.Remove(t);
			}
		}
	}

	friend EventSource<A>;
};

