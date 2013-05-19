#ifndef __CPU6510_H__
#define __CPU6510_H__

class CPU6510;
class CIA1;
class CIA2;
class VIC6569;
class Cart;
class SID64;

class CPU6510 : public CPU6502, public IC6510
{
public:
	CPU6510();
	~CPU6510();
	HRESULT Init(IC64Event *pIC64Event, int ID, CIA1 *cia1, CIA2 *cia2, VIC6569 *vic, SID64 *sid, Cart *cart, RAM64 *ram, ITape *tape, IBreakpointManager *pIBreakpointManager);
	void SetCassetteSense(bit8 sense);

	bit8 IRQ_VIC;	
	bit8 IRQ_CIA;
	bit8 IRQ_CRT;
	bit8 NMI_CIA;
	bit8 NMI_CRT;

	bool m_bIsWriteCycle;

	void CheckPortFade(ICLK sysclock);
	virtual ICLK GetCurrentClock();
	void Set_VIC_IRQ(ICLK sysclock);
	void Clear_VIC_IRQ();
	void Set_CIA_IRQ(ICLK sysclock);
	void Clear_CIA_IRQ();
	void Set_CRT_IRQ(ICLK sysclock);
	void Clear_CRT_IRQ();
	void Set_CIA_NMI(ICLK sysclock);
	void Clear_CIA_NMI();
	void Set_CRT_NMI(ICLK sysclock);
	void Clear_CRT_NMI();

	void InitReset(ICLK sysclock);
	//IRegister
	virtual void Reset(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	virtual bit8 ReadByte(bit16 address);
	virtual void WriteByte(bit16 address, bit8 data);

	void ConfigureMemoryMap();

	//IMonitorCpu
	virtual bit8 MonReadByte(bit16 address, int memorymap);
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap);
	virtual void GetCpuState(CPUState& state);
	virtual void SetDdr(bit8 v);
	virtual void SetData(bit8 v);
	virtual int GetCurrentCpuMmuMemoryMap();
	virtual MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap);
	virtual MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap);

	void AddClockDelay();
	virtual void PreventClockOverflow();
private:
	CIA1 *pCia1;
	CIA2 *pCia2;
	VIC6569 *pVic;
	Cart *pCart;
	RAM64 *ram;
	ITape *tape;
	IC64Event *pIC64Event;

	//devices
	IRegister *cia1;
	IRegister *cia2;
	IRegister *vic;
	IRegister *sid;
	IRegister *cart;

	bit8 **m_ppMemory_map_read;
	bit8 **m_ppMemory_map_write;
	bit8 *m_piColourRAM;

	bit8 cpu_io_data;
	bit8 cpu_io_ddr;
	bit8 cpu_io_output;
	bit8 cpu_io_readoutput;
	bit8 LORAM,HIRAM,CHAREN,CASSETTE_WRITE,CASSETTE_MOTOR,CASSETTE_SENSE;
	ICLK m_fade7clock;
	ICLK m_fade6clock;

	virtual void SyncChips();
	virtual void check_interrupts1();
	virtual void check_interrupts0();
	virtual void CheckForCartFreeze();
	void cpu_port();
	void write_cpu_io_data(bit8 data);
	void write_cpu_io_ddr(bit8 data, ICLK sysclock);
};

#endif
