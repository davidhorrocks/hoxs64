#ifndef __BPENUM_H__
#define __BPENUM_H__

typedef std::map<Sp_BreakpointKey, Sp_BreakpointItem, LessBreakpointKey> BpMap;
typedef std::map<Sp_BreakpointKey, Sp_BreakpointItem, LessBreakpointKey>::iterator BpIter;

struct BpEnum : IEnumBreakpointItem
{
public:

	BpEnum(BpMap *m);
	virtual ~BpEnum();
	virtual int GetCount();
	virtual bool GetNext(Sp_BreakpointItem& v);
	virtual void Reset();
private:
	BpMap* m_map;
	BpIter m_it;
};

#endif