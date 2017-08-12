#ifndef __D1541_H__
#define __D1541_H__

class DiskInterface;

class CPUDisk : public CPU6502
{
public:
	CPUDisk();
	~CPUDisk();

	HRESULT Init(IC64 *pIC64, IC64Event *pIC64Event, int ID, IRegister *via1, IRegister *via2, DiskInterface *disk, bit8 *pMappedRAM, bit8 *pMappedROM, IBreakpointManager *pIBreakpointManager);

	IC64Event *pIC64Event;
	IC64 *pIC64;
	bit8 *pMappedRAM;
	bit8 *pMappedROM;

	//Interrupting devices
	IRegister *via1;
	IRegister *via2;

	DiskInterface *disk;

	bit8 IRQ_VIA1;	
	bit8 IRQ_VIA2;

	void Set_VIA1_IRQ(ICLK sysclock);
	void Clear_VIA1_IRQ();
	void Set_VIA2_IRQ(ICLK sysclock);
	void Clear_VIA2_IRQ();

	virtual void SyncChips();
	virtual void check_interrupts1();
	virtual void check_interrupts0();

	void SyncVFlag();

	void InitReset(ICLK sysclock, bool poweronreset);
	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	void GetState(SsCpuDisk &state);
	void SetState(const SsCpuDisk &state);

	//
	virtual bit8 ReadByte(bit16 address);
	virtual void WriteByte(bit16 address, bit8 data);

	//IMonitorCpu
	virtual bit8 MonReadByte(bit16 address, int memorymap);
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap);
	virtual int GetCurrentCpuMmuMemoryMap();
	MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap);
	MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap);	
};

#endif