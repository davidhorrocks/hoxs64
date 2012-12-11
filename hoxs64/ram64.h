#ifndef __ram64_H__
#define __ram64_H__

class RAM64 : public ErrorMsg
{
public:
	RAM64();
	~RAM64();
	HRESULT Init(TCHAR *szAppDirectory);
	void Reset();
	void ConfigureMMU(bit8 index, bit8 ***p_memory_map_read, bit8 ***p_memory_map_write);
	void ConfigureVICMMU(bit8 index, bit8 ***p_vic_memory_map_read, bit8 **p_vic_3fff_ptr);
	int GetCurrentCpuMmuMemoryMap();
	MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap);
	MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap);
	void AttachCart(Cart &cart);

	MEM_TYPE MMU_MT_read[32][16];
	MEM_TYPE MMU_MT_write[32][16];
	bit8 *MMU_read[32][16];
	bit8 *MMU_write[32][16];
	bit8 *VicMMU_read[4][4];
	bit8 *miMemory;
	bit8 *miKernal;
	bit8 *miBasic;
	bit8 *miIO;
	bit8 *miCharGen;
	bit8 *miROML;
	bit8 *miROMH;
	bit8 *miROML_ULTIMAX;
	bit8 *miROMH_ULTIMAX;
	bit8 *miEXRAM;
	bit8 tmp_data[0x10000];

	bit8 *mMemory;
	bit8 *mKernal;
	bit8 *mBasic;
	bit8 *mIO;
	bit8 *mColorRAM;
	bit8 *mCharGen;
	Cart cart;
private:
	bit8 m_iCurrentCpuMmuIndex;
	TCHAR m_szAppDirectory[MAX_PATH+1];
	HRESULT	Allocate64Memory();
	void Free64Memory();
	void Zero64MemoryPointers();
	void InitMMU();
	void InitMMU_0();
	void LoadResetPattern();
	bit8 *GetCpuMmuIndexedPointer(MEM_TYPE mt);
};

#endif