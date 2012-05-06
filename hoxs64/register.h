#ifndef __REGISTER_H__
#define __REGISTER_H__

class IBase
{
protected:
	virtual ~IBase(){};
};

struct BreakpointKey
{
	BreakpointKey();
	BreakpointKey(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address);
	DBGSYM::MachineIdent::MachineIdent machineident;
	DBGSYM::BreakpointType::BreakpointType bptype;
	bit16 address;
	int vic_line;
	int vic_cycle;
	int Compare(const BreakpointKey v) const;
	bool operator<(const BreakpointKey v) const;
	bool operator>(const BreakpointKey v) const;
	bool operator==(const BreakpointKey y) const;
};

struct BreakpointItem : public BreakpointKey
{
	BreakpointItem();
	BreakpointItem(DBGSYM::MachineIdent::MachineIdent machineident, DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount);
	bool enabled;
	int initialSkipOnHitCount;
	int currentSkipOnHitCount;
};

typedef shared_ptr<BreakpointKey> Sp_BreakpointKey;
typedef shared_ptr<BreakpointItem> Sp_BreakpointItem;


typedef enum tagMemoryType : int
{
	MT_DEFAULT = 0,
	MT_RAM = 1,
	MT_IO = 2,
	MT_CHARGEN = 4,
	MT_BASIC = 8,
	MT_KERNAL = 16,
	MT_NOTCONNECTED = 32
} MEM_TYPE;

class IRegister
{
public:
	virtual void Reset(ICLK sysclock)=0;
	virtual void ExecuteCycle(ICLK sysclock)=0;
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock)=0;
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data)=0;
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock)=0;
	ICLK CurrentClock;
};

class ILightPen
{
public:
	virtual void SetLPLine(bit8 lineState)=0;
	virtual void SetLPLineClk(ICLK sysclock, bit8 lineState)=0;
};

class ITape
{
public:
	virtual void SetMotorWrite(bool motor, bit8 write)=0;
	virtual void PressPlay()=0;
	virtual void PressStop()=0;
	virtual void Rewind()=0;
	virtual void Eject()=0;
};

class ITapeEvent
{
public:
	virtual void Pulse(ICLK sysclock)=0;
	virtual void EndOfTape(ICLK sysclock)=0;
};

class IEnumBreakpointItem : IBase
{
public:
	virtual int GetCount() = 0;
	virtual bool GetNext(Sp_BreakpointItem& v) = 0;
	virtual void Reset() = 0;
};

class IBreakpointManager
{
public:
	virtual bool BM_SetBreakpoint(Sp_BreakpointItem bp) = 0;
	virtual bool BM_GetBreakpoint(Sp_BreakpointKey k, Sp_BreakpointItem &bp) = 0;
	virtual void BM_DeleteBreakpoint(Sp_BreakpointKey k) = 0;
	virtual void BM_EnableBreakpoint(Sp_BreakpointKey k) = 0;
	virtual void BM_DisableBreakpoint(Sp_BreakpointKey k) = 0;
	virtual void BM_EnableAllBreakpoints() = 0;
	virtual void BM_DisableAllBreakpoints() = 0;
	virtual void BM_DeleteAllBreakpoints() = 0;
	virtual IEnumBreakpointItem *BM_CreateEnumBreakpointItem() = 0;
};

class IC64BreakEvent
{
public:
	virtual void BreakExecuteCpu64()=0;
	virtual void BreakExecuteCpuDisk()=0;
	virtual void BreakVicRasterCompare()=0;
	virtual void BreakpointChanged()=0;
};

class IC64Event : public IC64BreakEvent
{
public:
	virtual void SetBusy(bool bBusy)=0;
	virtual void DiskMotorLed(bool bOn)=0;
	virtual void DiskDriveLed(bool bOn)=0;
	virtual void DiskWriteLed(bool bOn)=0;
	virtual void ShowErrorBox(LPCTSTR title, LPCTSTR message)=0;
};

class IC64
{
public:
	virtual void HardReset(bool bCancelAutoload)=0;
	virtual void SoftReset(bool bCancelAutoload)=0;
	virtual void PostHardReset(bool bCancelAutoload)=0;
	virtual void PostSoftReset(bool bCancelAutoload)=0;
};

struct CPUState
{
	bit8 A;
	bit8 X;
	bit8 Y;
	bit16 PC;
	bit16 PC_CurrentOpcode;
	bit8 SP;
	bit8 Flags;
	bit8 PortDdr;
	bit8 PortDataStored;
	int processor_interrupt;
	int cpu_sequence;
	bit8 BA;
	ICLK clock;
	int opcode;
	int cycle;
	bool IsInterruptInstruction;
};

struct LessBreakpointKey
{
	bool operator()(const Sp_BreakpointKey x, const Sp_BreakpointKey y) const;
};

class IMonitorCpu
{
public:
	virtual int GetCpuId()=0;
	/*
	address: The 16 bit address as seen by the Cpu
	memorymap: The memory map.
	A negative numbers means use the current memory map.
	*/
	virtual bit8 MonReadByte(bit16 address, int memorymap)=0;
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap)=0;
	virtual int GetCurrentCpuMmuMemoryMap()=0;
	virtual MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap)=0;
	virtual MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap)=0;
	virtual void GetCpuState(CPUState& state)=0;
	virtual bool IsBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)=0;
	virtual void DeleteBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)=0;
	virtual void EnableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)=0;
	virtual void DisableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address)=0;
	virtual bool SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)=0;
	virtual bool GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, Sp_BreakpointItem& breakpoint)=0;
	virtual void SetBreakOnInterruptTaken()=0;
	virtual void ClearBreakOnInterruptTaken()=0;
	virtual void SetPC(bit16 address) = 0;
	virtual void SetA(bit8 v) = 0;
	virtual void SetX(bit8 v) = 0;
	virtual void SetY(bit8 v) = 0;
	virtual void SetSR(bit8 v) = 0;
	virtual void SetSP(bit8 v) = 0;
	virtual void SetDdr(bit8 v) = 0;
	virtual void SetData(bit8 v) = 0;
};


class IMonitorVic
{
public:
	virtual bit16 GetRasterLine()=0;
	virtual bit8 GetRasterCycle()=0;
	virtual bool GetBreakpointRasterCompare(int line, int cycle, Sp_BreakpointItem& breakpoint) = 0;
	virtual bool SetBreakpointRasterCompare(int line, int cycle, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) = 0; 
	virtual int CheckBreakpointRasterCompare(int line, int cycle, bool bHitIt) = 0;
};

class IMonitorDisk
{
public:
	virtual bit8 GetHalfTrackIndex()=0;
};

class IDefaultCpu
{
public:
	virtual int GetCpuId()=0;
	virtual IMonitorCpu *GetCpu()=0;
};

#endif