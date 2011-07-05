#ifndef __CEVENT_H__
#define __CEVENT_H__

//typedef EVENTCALLBACK
class EventSink;

typedef class MListElement<EventSink *> MListEventSinkElement;
typedef class MList<EventSink *> MListEventSinkList;

typedef MListEventSinkElement *HSink;

class EventArgs
{
};

class EventSource
{
public:
	EventSource();
	HSink Add(EventSink *);
	void Remove(HSink hs);
	void Raise(void *sender, EventArgs& e);
private:
	~EventSource();
	MListEventSinkList m_lstSink;

};

class EventSink
{
public:
	virtual int Sink(void *sender, EventArgs &e)=0;
};


#endif
