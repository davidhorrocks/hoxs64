#pragma once

struct BreakpointItem
{
	BreakpointItem() noexcept;
	virtual ~BreakpointItem() = default;
	BreakpointItem(const BreakpointItem&) = default;
	virtual BreakpointItem& operator=(const BreakpointItem&) = default;
	BreakpointItem(BreakpointItem&&) = default;
	BreakpointItem& operator=(BreakpointItem&&) = default;

	BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) noexcept;
	BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) noexcept;

	virtual int Compare(const BreakpointItem& v) const noexcept;
	virtual bool operator<(const BreakpointItem& v) const noexcept;
	virtual bool operator>(const BreakpointItem& v) const noexcept;
	virtual bool operator==(const BreakpointItem& y) const noexcept;

	DBGSYM::MachineIdent::MachineIdent machineident;
	DBGSYM::BreakpointType::BreakpointType bptype;
	bit16 address;
	int vic_line;
	int vic_cycle;
	bool enabled;
	int initialSkipOnHitCount;
	int currentSkipOnHitCount;
};

struct LessBreakpointItem
{
	bool operator()(const BreakpointItem& x, const BreakpointItem& y) const noexcept;
};

typedef map<BreakpointItem, BreakpointItem, LessBreakpointItem> BpMap;
typedef map<BreakpointItem, BreakpointItem, LessBreakpointItem>::iterator BpIter;

class IEnumBreakpointItem
{
public:
	IEnumBreakpointItem() noexcept {};
	virtual ~IEnumBreakpointItem() noexcept {};
	IEnumBreakpointItem(const IEnumBreakpointItem&) = delete;
	IEnumBreakpointItem& operator=(const IEnumBreakpointItem&) = delete;
	IEnumBreakpointItem(IEnumBreakpointItem&&) = delete;
	IEnumBreakpointItem& operator=(IEnumBreakpointItem&&) = delete;

	virtual int GetCount() = 0;
	virtual bool GetNext(BreakpointItem& v) = 0;
	virtual void Reset() = 0;
};

struct BpEnum : IEnumBreakpointItem
{
public:
	BpEnum(BpMap *m);
	int GetCount() override;
	bool GetNext(BreakpointItem& v) override;
	void Reset() override;
private:
	BpMap* m_map;
	BpIter m_it;
};
