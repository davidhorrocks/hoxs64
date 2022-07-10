#pragma once
#include <windows.h>
#include <tchar.h>
#include "defines.h"
#include "register.h"
#include "errormsg.h"
#include "savestate.h"

#define APPWARN_UNKNOWNCARTTYPE MAKE_HRESULT(0, 0xa00, 2)

#define CARTCOMMON_ARRAYCOUNTREGISTERARRAY 512
#define CARTCOMMON_ARRAYCOUNTSTATE 64

# pragma pack (1)

struct CrtHeader
{
	bit8 Signature[16] = {};
	bit32 FileHeaderLength = 0;
	bit8 VersionHigh = 0;
	bit8 VersionLow = 0;
	bit16 HardwareType = 0;
	bit8 EXROM = 0;
	bit8 GAME = 0;
	bit8 SubType = 0;
	bit8 Reserved[5] = {};
	bit8 CartridgeName[32] = {};
};

struct CrtChip
{
	bit8 Signature[4] = {};
	bit32 TotalPacketLength = 0;
	bit16 ChipType = 0;
	bit16 BankLocation = 0;
	bit16 LoadAddressRange = 0;
	bit16 ROMImageSize = 0;
};

struct SsCartStateHeader
{
	bit32 size = 0;
}
;
struct SsCartMemoryHeader
{
	bit32 size = 0;
	bit32 ramsize = 0;
	bit32 banks = 0;
};

struct SsCartChip
{
	CrtChip chip;
	bit16 allocatedSize = 0;
};

struct SsCartBank
{
	bit32 bank = 0;
	SsCartChip roml;
	SsCartChip romh;
};

struct SsCartCommonV0
{
	bit32 size = 0;
	bit8 reg1 = 0;
	bit8 reg2 = 0;
	bit8 GAME = 0;
	bit8 EXROM = 0;
	bit8 m_bIsCartAttached = 0;
	bit8 m_bIsCartIOActive = 0;
	bit8 m_bIsCartRegActive = 0;
	bit8 m_iSelectedBank = 0;

	bit8 m_bEnableRAM = 0;
	bit8 m_bAllowBank = 0;
	bit8 m_bREUcompatible = 0;
	bit8 m_bFreezePending = 0;
	bit8 m_bFreezeMode = 0;
	bit32 m_state0 = 0;
	bit32 m_state1 = 0;
	bit32 m_state2 = 0;
	bit32 m_state3 = 0;
};

struct SsCartCommonV1
{
	bit32 size = 0;
	bit32 CurrentClock;
	bit8 registerArray[CARTCOMMON_ARRAYCOUNTREGISTERARRAY];
	bit8 GAME = 0;
	bit8 EXROM = 0;
	bit8 DMA = 1;
	bit8 m_bIsCartAttached = 0;
	bit8 m_bIsCartIOActive = 0;
	bit8 m_bIsCartRegActive = 0;
	bit8 m_iSelectedBank = 0;
	bit8 m_bEnableRAM = 0;
	bit8 m_bAllowBank = 0;
	bit8 m_bREUcompatible = 0;
	bit8 m_bFreezePending = 0;
	bit8 m_bFreezeMode = 0;
	bit8 m_statebyte[CARTCOMMON_ARRAYCOUNTSTATE];
	bit16 m_stateushort[CARTCOMMON_ARRAYCOUNTSTATE];
	bit32 m_stateuint[CARTCOMMON_ARRAYCOUNTSTATE];
	bool m_statebool[CARTCOMMON_ARRAYCOUNTSTATE];
};

# pragma pack ()

struct CrtChipAndData
{
	CrtChipAndData();
	~CrtChipAndData();
	CrtChip chip;
	bit8 *pData = nullptr;
	bool ownData = false;
	bit16 allocatedSize = 0;
	bit16 romOffset = 0;
	__int64 iFileIndex = 0;
};

struct CrtBank
{
	CrtBank();
	virtual ~CrtBank();
	bit16 bank = 0;
	CrtChipAndData chipAndDataLow;
	CrtChipAndData chipAndDataHigh;
};

typedef shared_ptr<CrtBank> Sp_CrtBank;
typedef vector<Sp_CrtBank> CrtBankList;
typedef vector<Sp_CrtBank>::iterator CrtBankListIter;
typedef vector<Sp_CrtBank>::const_iterator CrtBankListConstIter;

class ICartInterface : public IRegister
{
public:
	virtual bit8 Get_GAME() = 0;
	virtual bit8 Get_EXROM() = 0;
	virtual bit8 Get_DMA() = 0;

	virtual bit8 ReadROML(bit16 address) = 0;
	virtual bit8 ReadROMH(bit16 address) = 0;
	virtual bit8 ReadUltimaxROML(bit16 address) = 0;
	virtual bit8 ReadUltimaxROMH(bit16 address) = 0;

	virtual void WriteROML(bit16 address, bit8 data) = 0;
	virtual void WriteROMH(bit16 address, bit8 data) = 0;
	virtual void WriteUltimaxROML(bit16 address, bit8 data) = 0;
	virtual void WriteUltimaxROMH(bit16 address, bit8 data) = 0;

	virtual bit8 MonReadROML(bit16 address) = 0;
	virtual bit8 MonReadROMH(bit16 address) = 0;
	virtual bit8 MonReadUltimaxROML(bit16 address) = 0;
	virtual bit8 MonReadUltimaxROMH(bit16 address) = 0;

	virtual void MonWriteROML(bit16 address, bit8 data) = 0;
	virtual void MonWriteROMH(bit16 address, bit8 data) = 0;
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data) = 0;
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data) = 0;

	virtual bool IsREU() = 0;
	virtual bool IsUltimax() = 0;
	virtual bool IsCartIOActive(bit16 address, bool isWriting) = 0;
	virtual bool IsCartAttached() = 0;
	virtual void Set_IsCartAttached(bool isAttached) = 0;

	virtual void CartFreeze() = 0;
	virtual void CartReset() = 0;
	virtual void CheckForCartFreeze() = 0;
	virtual bool SnoopWrite(bit16 address, bit8 data) = 0;
	virtual void SnoopRead(bit16 address, bit8 data) = 0;
	virtual void AttachCart() = 0;
	virtual void DetachCart() = 0;
	virtual void ConfigureMemoryMap() = 0;

	virtual bit8* Get_RomH() = 0;
	virtual bit8* Get_RomL() = 0;
	virtual void PreventClockOverflow() = 0;

	virtual HRESULT LoadState(IStream* pfs, int version) = 0;
	virtual HRESULT SaveState(IStream* pfs) = 0;

	virtual HRESULT InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData) = 0;
};

class Cart;

class CartCommon : public ICartInterface
{
public:
	CartCommon(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);
	virtual ~CartCommon();

	CartCommon() = delete;
	CartCommon(const CartCommon&) = delete;
	CartCommon& operator=(const CartCommon&) = delete;
	CartCommon(CartCommon&&) = delete;
	CartCommon& operator=(CartCommon&&) = delete;

	const int LatestVersion = 1;
	void InitReset(ICLK sysclock, bool poweronreset);
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;
	bit8 Get_GAME() override;
	bit8 Get_EXROM() override;
	bit8 Get_DMA() override;
	bit8 ReadROML(bit16 address) override;
	bit8 ReadROMH(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;
	bit8 ReadUltimaxROMH(bit16 address) override;
	void WriteROML(bit16 address, bit8 data) override;
	void WriteROMH(bit16 address, bit8 data) override;
	void WriteUltimaxROML(bit16 address, bit8 data) override;
	void WriteUltimaxROMH(bit16 address, bit8 data) override;
	bit8 MonReadROML(bit16 address) override;
	bit8 MonReadROMH(bit16 address) override;
	bit8 MonReadUltimaxROML(bit16 address) override;
	bit8 MonReadUltimaxROMH(bit16 address) override;
	void MonWriteROML(bit16 address, bit8 data) override;
	void MonWriteROMH(bit16 address, bit8 data) override;
	void MonWriteUltimaxROML(bit16 address, bit8 data) override;
	void MonWriteUltimaxROMH(bit16 address, bit8 data) override;
	bool IsREU() override;
	bool IsUltimax() override;
	bool IsCartIOActive(bit16 address, bool isWriting) override;
	bool IsCartAttached() override;
	void Set_IsCartAttached(bool isAttached) override;
	void CartFreeze() override;
	void CartReset() override;
	void CheckForCartFreeze() override;
	bool SnoopWrite(bit16 address, bit8 data) override;
	void SnoopRead(bit16 address, bit8 data) override;
	void AttachCart() override;
	void DetachCart() override;
	void ConfigureMemoryMap() override;
	bit8* Get_RomH() override;
	bit8* Get_RomL() override;
	void PreventClockOverflow() override;
	HRESULT LoadState(IStream* pfs, int version) override;
	HRESULT SaveState(IStream* pfs) override;
	HRESULT InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData) override;
	virtual HRESULT SaveMemoryState(IStream* pfs);
protected:
	virtual void UpdateIO() = 0;
	void BankRom();
	virtual unsigned int GetStateBytes(int version, void* pstate);
	virtual HRESULT SetStateBytes(int version, void* pstate, unsigned int size);

	CrtHeader m_crtHeader = {};
	CrtBankList* m_plstBank = nullptr;
	bit8* m_pCartData = nullptr;
	bit8* m_pZeroBankData = nullptr;
	unsigned int m_amountOfExtraRAM;
	bit8* m_ipROML = nullptr;
	bit8* m_ipROMH = nullptr;

	bit8& reg1;
	bit8& reg2;
	bit8 registerArray[CARTCOMMON_ARRAYCOUNTREGISTERARRAY];

	bit8 GAME = 0;
	bit8 EXROM = 0;
	bit8 DMA = 1;
	bool m_bIsREU = false;
	bool m_bIsCartAttached = false;
	bool m_bIsCartIOActive = false;
	bool m_bIsCartRegActive = false;
	bit8 m_iSelectedBank = 0;

	bool m_bEnableRAM = false;
	bool m_bAllowBank = false;
	bool m_bREUcompatible = false;
	bit8 m_bFreezePending = 0;
	bit8 m_bFreezeMode = 0;
	bit16 m_iRamBankOffsetIO = 0;
	bit16 m_iRamBankOffsetRomL = 0;

	IC6510* m_pCpu = nullptr;
	IVic* m_pVic = nullptr;
	bit8* m_pC64RamMemory = nullptr;
	bool m_bEffects = true;
protected:
	bit8 m_statebyte[CARTCOMMON_ARRAYCOUNTSTATE];
	bit16 m_stateushort[CARTCOMMON_ARRAYCOUNTSTATE];
	bit32 m_stateuint[CARTCOMMON_ARRAYCOUNTSTATE];
	bool m_statebool[CARTCOMMON_ARRAYCOUNTSTATE];
	CrtBankList m_lstStateBank;
private:
	void CleanUp() noexcept;
	static void UpgradeVersion0ToVersion1(const SsCartCommonV0& in, SsCartCommonV1& out);
	void LoadStateFromStructure(const SsCartCommonV1 state);
	void SaveStateToStructure(SsCartCommonV1 state);

friend SsCartCommonV1;
};

class Cart : public ICartInterface, public ErrorMsg
{
public:
	class CartType
	{
	public:
		enum ECartType
		{
			Normal_Cartridge = 0,
			Action_Replay = 1,//AR5 + AR6 + AR4.x
			KCS_Power = 2,
			Final_Cartridge_III = 3,
			Simons_Basic = 4,
			Ocean_1 = 5,
			Fun_Play = 7,
			Super_Games = 8,
			Epyx_FastLoad = 10,
			System_3 = 15,
			Dinamic = 17,
			Zaxxon = 18,
			Magic_Desk = 19,
			Action_Replay_4 = 30,
			EasyFlash = 32,
			Action_Replay_3 = 35,
			Retro_Replay = 36,
			Action_Replay_2 = 50,
		};
	};
	class ChipType
	{
	public:
		enum EChipType
		{
			ROM = 0,
			RAM = 1,
			EPROM = 2,
		};
	};

	Cart() noexcept;
	~Cart() noexcept;
	Cart(const Cart&) = delete;
	Cart& operator=(const Cart&) = delete;
	Cart(Cart&&) = delete;
	Cart& operator=(Cart&&) = delete;

	static const int MAXBANKS = 256;
	static const bit32 NOCOMPRESSION = 0;
	static const bit32 HUFFCOMPRESSION = 1;
	static const bit8 S_SIGHEADER[14];
	static const bit8 S_SIGCHIP[5];
	static const bit8 S_CART_NAME_REU1750[9];

	void Init(IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);
	HRESULT LoadCrtFile(LPCTSTR filename);
	HRESULT LoadReu1750();
	shared_ptr<ICartInterface> CreateCartInterface(const CrtHeader& crtHeader);
	static bool IsSupported(CartType::ECartType hardwareType);
	bool IsSupported();

	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	bit8 Get_GAME() override;
	bit8 Get_EXROM() override;
	bit8 Get_DMA() override;

	bit8 ReadROML(bit16 address) override;
	bit8 ReadROMH(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;
	bit8 ReadUltimaxROMH(bit16 address) override;

	void WriteROML(bit16 address, bit8 data) override;
	void WriteROMH(bit16 address, bit8 data) override;
	void WriteUltimaxROML(bit16 address, bit8 data) override;
	void WriteUltimaxROMH(bit16 address, bit8 data) override;

	bit8 MonReadROML(bit16 address) override;
	bit8 MonReadROMH(bit16 address) override;
	bit8 MonReadUltimaxROML(bit16 address) override;
	bit8 MonReadUltimaxROMH(bit16 address) override;

	void MonWriteROML(bit16 address, bit8 data) override;
	void MonWriteROMH(bit16 address, bit8 data) override;
	void MonWriteUltimaxROML(bit16 address, bit8 data) override;
	void MonWriteUltimaxROMH(bit16 address, bit8 data) override;

	bool IsREU() override;
	bool IsUltimax() override;
	bool IsCartIOActive(bit16 address, bool isWriting) override;
	bool IsCartAttached() override;
	void Set_IsCartAttached(bool isAttached) override;

	void CartFreeze() override;
	void CartReset() override;
	void CheckForCartFreeze() override;
	bool SnoopWrite(bit16 address, bit8 data) override;
	void SnoopRead(bit16 address, bit8 data) override;
	void AttachCart(shared_ptr<ICartInterface> spCartInterface);
	void AttachCart() override;
	void DetachCart() override;
	void ConfigureMemoryMap() override;
	bit8* Get_RomH() override;
	bit8* Get_RomL() override;
	void PreventClockOverflow() override;

	HRESULT LoadCartInterface(IStream* pfs, int version, shared_ptr<ICartInterface>& spCartInterface);
	HRESULT LoadState(IStream* pfs, int version) override;
	HRESULT SaveState(IStream* pfs) override;
	HRESULT InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData) override;

	CrtHeader m_crtHeader = {};//Cart head information
	unsigned int m_offsetToDefault8kZeroBank;
	unsigned int m_offsetToFirstChipBank;
	CrtBankList* m_plstBank = nullptr;//A list of cart ROM/EPROM banks that index the cart memory data.
	bit8* m_pCartData = nullptr; //Pointer to the cart memory data. This a block of RAM followed by 1 or more ROM/EPROM banks.
	bit8* m_pZeroBankData = nullptr; //An offset into the cart memory data that points past the initial block of RAM to the first ROM/EPROM bank.
	unsigned int m_amountOfExtraRAM;
	bool m_bIsCartDataLoaded = false;
private:
	static const unsigned int RAMRESERVEDSIZE_V0 = 64 * 1024 + 8 * 1024;// Assume 64K cart RAM + 8K zero byte bank
	static const unsigned int ZEROBANKOFFSET_V0 = 64 * 1024;
	static const unsigned int CARTRAMSIZE_V0 = 64 * 1024;
	static const unsigned int MAX_CART_EXTRA_RAM = 2 * 1024 * 1024;
	static const unsigned int MIN_CART_EXTRA_RAM = 64 * 1024;
	static const unsigned int DEFAULT_CHIP_EXTRA_MEMORY = 8 * 1024;
	static const unsigned int REU_1750_RAM = 512 * 1024;
	static unsigned int GetTotalCartMemoryRequirement(unsigned int amountOfExtraRAM, const CrtBankList* plstBank, unsigned int& offsetToDefault8kZeroBank, unsigned int& offsetToFirstChipBank);
	static bool IsHeaderReu(const CrtHeader& crtHeader) noexcept;
	static void MarkHeaderAsReu(CrtHeader& crtHeader) noexcept;

	void CleanUp() noexcept;
	IC6510* m_pCpu = nullptr;
	bit8* m_pC64RamMemory = nullptr;
	IVic* m_pVic = nullptr;
	shared_ptr<ICartInterface> m_spCurrentCart;
	friend CartCommon;
};

#include "cart_normal_cartridge.h"
#include "cart_easyflash.h"
#include "cart_retro_replay.h"
#include "cart_action_replay.h"
#include "cart_action_replay_mk4.h"
#include "cart_action_replay_mk3.h"
#include "cart_action_replay_mk2.h"
#include "cart_final_cartridge_iii.h"
#include "cart_simons_basic.h"
#include "cart_ocean1.h"
#include "cart_fun_play.h"
#include "cart_super_games.h"
#include "cart_system_3.h"
#include "cart_dinamic.h"
#include "cart_zaxxon.h"
#include "cart_magic_desk.h"
#include "cart_epyx_fastload.h"
#include "cart_kcs_power.h"
#include "cart_reu_1750.h"
