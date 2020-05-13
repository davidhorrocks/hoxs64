#pragma once
class DiskInterface;

class CPUDisk : public CPU6502
{
public:
	CPUDisk();
	~CPUDisk();
	CPUDisk(const CPUDisk&) = delete;
	CPUDisk& operator=(const CPUDisk&) = delete;
	CPUDisk(CPUDisk&&) = delete;
	CPUDisk& operator=(CPUDisk&&) = delete;

	HRESULT Init(IC64 *pIC64, IC64Event *pIC64Event, int ID, IRegister *via1, IRegister *via2, DiskInterface *disk, bit8 *pMappedRAM, bit8 *pMappedROM, IBreakpointManager *pIBreakpointManager);

	IC64Event *pIC64Event = nullptr;
	IC64 *pIC64 = nullptr;
	bit8 *pMappedRAM = nullptr;
	bit8 *pMappedROM = nullptr;

	//Interrupting devices
	IRegister *via1 = nullptr;
	IRegister *via2 = nullptr;

	DiskInterface *disk = nullptr;

	bit8 IRQ_VIA1 = 0;	
	bit8 IRQ_VIA2 = 0;

	void Set_VIA1_IRQ(ICLK sysclock);
	void Clear_VIA1_IRQ();
	void Set_VIA2_IRQ(ICLK sysclock);
	void Clear_VIA2_IRQ();

	void SyncChips(bool isWriteCycle) override;
	void check_interrupts1() override;
	void check_interrupts0() override;

	void SyncVFlag();

	void InitReset(ICLK sysclock, bool poweronreset);
	
	//IRegister
	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	void GetState(SsCpuDisk &state);
	void SetState(const SsCpuDisk &state);

	bit8 ReadByte(bit16 address) override;
	void WriteByte(bit16 address, bit8 data) override;

	//IMonitorCpu
	bit8 MonReadByte(bit16 address, int memorymap) override;
	void MonWriteByte(bit16 address, bit8 data, int memorymap) override;
	int GetCurrentCpuMmuMemoryMap() override;
	MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap) override;
	MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap) override;
};
