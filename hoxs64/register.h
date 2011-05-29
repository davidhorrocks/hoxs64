#ifndef __REGISTER_H__
#define __REGISTER_H__

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

class IC64BreakEvent
{
public:
	virtual void BreakExecuteCpu64()=0;
	virtual void BreakExecuteCpuDisk()=0;
};

class IC64Event : public IC64BreakEvent
{
public:
	virtual void SetBusy(bool bBusy)=0;
	virtual void DiskMotorLed(bool bOn)=0;
	virtual void DiskDriveLed(bool bOn)=0;
	virtual void DiskWriteLed(bool bOn)=0;
};

class IC64
{
public:
	virtual void HardReset(bool bCancelAutoload)=0;
	virtual void SoftReset(bool bCancelAutoload)=0;
	virtual void PostHardReset(bool bCancelAutoload)=0;
	virtual void PostSoftReset(bool bCancelAutoload)=0;
};

class IMonitorEvent
{
public:
	virtual void Resume(IMonitorEvent *sender)=0;
	virtual void Trace(IMonitorEvent *sender)=0;
	virtual void TraceFrame(IMonitorEvent *sender)=0;
	virtual void ExecuteC64Clock(IMonitorEvent *sender)=0;
	virtual void ExecuteDiskClock(IMonitorEvent *sender)=0;
	virtual void ExecuteC64Instruction(IMonitorEvent *sender)=0;
	virtual void ExecuteDiskInstruction(IMonitorEvent *sender)=0;
	virtual void UpdateApplication(IMonitorEvent *sender)=0;
	virtual HWND ShowDevelopment(IMonitorEvent *sender) = 0;
	virtual bool IsRunning(IMonitorEvent *sender)=0;

	virtual HRESULT Advise(IMonitorEvent *sink)=0;
	virtual void Unadvise(IMonitorEvent *sink)=0;
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
	virtual void GetCpuState(CPUState& state)=0;
	virtual bool IsBreakPoint(bit16 address)=0;
	virtual void ClearBreakPoint(bit16 address)=0;
	virtual bool SetExecute(bit16 address, unsigned long count)=0;
	virtual void SetBreakOnInterruptTaken()=0;
	virtual void ClearBreakOnInterruptTaken()=0;
};


class IMonitorVic
{
public:
	virtual bit16 GetRasterLine()=0;
	virtual bit8 GetRasterCycle()=0;
};


#endif