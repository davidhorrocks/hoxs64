#pragma once
class CPU6510;
class CIA1;
class CIA2;
class VIC6569;
class Cart;
class SID64;

class CPU6510 : public CPU6502, public IC6510
{
public:
	static const int ShellExitCpu64Halt = 0xFF;
	CPU6510();
	~CPU6510();
	CPU6510(const CPU6510&) = delete;
	CPU6510& operator=(const CPU6510&) = delete;
	CPU6510(CPU6510&&) = delete;
	CPU6510& operator=(CPU6510&&) = delete;

	bit8 IRQ_VIC = 0;	
	bit8 IRQ_CIA = 0;
	bit8 IRQ_CRT = 0;
	bit8 NMI_CIA = 0;
	bit8 NMI_CRT = 0;
	bool m_bIsWriteCycle = false;
	bool bExitOnHltInstruction = false;

	HRESULT Init(IC64 *pIC64, IC64Event *pIC64Event, int ID, CIA1 *cia1, CIA2 *cia2, VIC6569 *vic, ISid64 *sid, Cart *cart, RAM64 *ram, ITape *tape, IBreakpointManager *pIBreakpointManager);
	void SetCassetteSense(bit8 sense);
	void CheckPortFade(ICLK sysclock);	

	virtual void Reset6510(ICLK sysclock, bool poweronreset);
	virtual ICLK Get6510CurrentClock();
	virtual void Set6510CurrentClock(ICLK sysclock);
	virtual void Set_VIC_IRQ(ICLK sysclock);
	virtual void Clear_VIC_IRQ();
	virtual void Set_CIA_IRQ(ICLK sysclock);
	virtual void Clear_CIA_IRQ();
	virtual void Set_CRT_IRQ(ICLK sysclock);
	virtual void Clear_CRT_IRQ();
	virtual void Set_CIA_NMI(ICLK sysclock);
	virtual void Clear_CIA_NMI();
	virtual void Set_CRT_NMI(ICLK sysclock);
	virtual void Clear_CRT_NMI();
	virtual void ConfigureMemoryMap();
	virtual bool Get_EnableDebugCart();
	virtual void Set_EnableDebugCart(bool bEnable);
	void InitReset(ICLK sysclock, bool poweronreset);

	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	virtual bit8 ReadByte(bit16 address);
	virtual void WriteByte(bit16 address, bit8 data);

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
	virtual void OnHltInstruction();
	void cpu_port();
	void GetState(SsCpuMain &state);
	void SetState(const SsCpuMain &state);
private:
	CIA1* pCia1 = nullptr;
	CIA2 *pCia2 = nullptr;
	VIC6569 *pVic = nullptr;
	Cart *pCart = nullptr;
	RAM64 *ram = nullptr;
	ITape *tape = nullptr;
	IC64Event *pIC64Event = nullptr;
	IC64 *pIC64 = nullptr;

	//devices
	IRegister *cia1 = nullptr;
	IRegister *cia2 = nullptr;
	IRegister *vic = nullptr;
	ISid64 *sid = nullptr;
	IRegister *cart = nullptr;

	bit8 **m_ppMemory_map_read = nullptr;
	bit8 **m_ppMemory_map_write = nullptr;
	bit8 *m_piColourRAM = nullptr;

	bit8 cpu_io_data = 0;
	bit8 cpu_io_ddr = 0;
	bit8 cpu_io_output = 0;
	bit8 cpu_io_readoutput = 0;
	bit8 LORAM = 0;
	bit8 HIRAM = 0;
	bit8 CHAREN = 0;
	bit8 CASSETTE_WRITE = 0;
	bit8 CASSETTE_MOTOR = 0;
	bit8 CASSETTE_SENSE = 0;
	ICLK m_fade7clock = 0;
	ICLK m_fade6clock = 0;
	bool bEnableDebugCart = false;	

	virtual void SyncChips();
	virtual void check_interrupts1();
	virtual void check_interrupts0();
	virtual void CheckForCartFreeze();
	void write_cpu_io_data(bit8 data);
	void write_cpu_io_ddr(bit8 data, ICLK sysclock);
};
