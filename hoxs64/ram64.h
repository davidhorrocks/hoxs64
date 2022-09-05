#pragma once
#include <string>
#include "cart.h"
#include "errormsg.h"

class RAM64 : public ErrorMsg
{
public:
	RAM64() noexcept;
	~RAM64();
	RAM64(const RAM64&) = delete;
	RAM64& operator=(const RAM64&) = delete;
	RAM64(RAM64&&) = delete;
	RAM64& operator=(RAM64&&) = delete;

	HRESULT Init(const wchar_t* pwszAppDirectory, Cart *cart);
	void InitReset(bool poweronreset);
	void Reset(bool poweronreset);
	void ConfigureMMU(bit8 index, bit8 ***p_memory_map_read, bit8 ***p_memory_map_write);
	void ConfigureVICMMU(bit8 index, bit8 ***p_vic_memory_map_read, bit8 **p_vic_3fff_ptr);
	int GetCurrentCpuMmuMemoryMap();
	MEM_TYPE GetCpuMmuReadMemoryType(bit16 address, int memorymap);
	MEM_TYPE GetCpuMmuWriteMemoryType(bit16 address, int memorymap);
	MEM_TYPE GetCurrentCpuMmuReadMemoryType(bit16 address);
	MEM_TYPE GetCurrentCpuMmuWriteMemoryType(bit16 address);
	bit8 *GetCpuMmuIndexedPointer(MEM_TYPE mt);

	MEM_TYPE MMU_MT_read[32][16] = {};
	MEM_TYPE MMU_MT_write[32][16] = {};
	bit8 *MMU_read[32][16] = {};
	bit8 *MMU_write[32][16] = {};
	bit8 *VicMMU_read[4][4] = {};
	bit8 *miMemory = nullptr;
	bit8 *miKernal = nullptr;
	bit8 *miBasic = nullptr;
	bit8 *miIO = nullptr;
	bit8 *miCharGen = nullptr;
	bit8 tmp_data[0x10000] = {};
	bit8* mMemoryRestore = nullptr;
	bit8 *mMemory = nullptr;
	bit8 *mKernal = nullptr;
	bit8 *mBasic = nullptr;
	bit8 *mIO = nullptr;
	bit8 *mColorRAM = nullptr;
	bit8 *mCharGen = nullptr;
private:
	static constexpr int MEM_RAM64 = 64 * 1024;
	static constexpr int MEM_KERNAL = 8 * 1024;
	static constexpr int MEM_BASIC = 8 * 1024;
	static constexpr int MEM_CHARGEN = 4 * 1024;
	static constexpr int MEM_IO = 4 * 1024;
	static constexpr int MEM_TOTALSIZE = (MEM_RAM64 + MEM_KERNAL + MEM_BASIC + MEM_CHARGEN + MEM_IO);

	Cart *m_pCart = nullptr;
	bit8 m_iCurrentCpuMmuIndex = 0;
	std::wstring wsAppDirectory;
	HRESULT	Allocate64Memory();
	void Free64Memory() noexcept;
	void Zero64MemoryPointers() noexcept;
	void InitMMU();
	void InitMMU_0();
	void LoadResetPattern();
	random_device rd;
	mt19937 randengine_main;
};
