#ifndef __BPENUM_H__
#define __BPENUM_H__

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