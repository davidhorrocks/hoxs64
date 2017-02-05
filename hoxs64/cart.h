#ifndef __CART_H__
#define __CART_H__

#include "savestate.h"

#define APPWARN_UNKNOWNCARTTYPE MAKE_HRESULT(0, 0xa00, 2)

# pragma pack (1)

struct CrtHeader
{
	bit8 Signature[16];
	bit32 FileHeaderLength;
	bit16 Version;
	bit16 HardwareType;
	bit8 EXROM;
	bit8 GAME;
	bit8 Reserved[6];
	bit8 CartridgeName[32];
};

struct CrtChip
{
	bit8 Signature[4];
	bit32 TotalPacketLength;
	bit16 ChipType;
	bit16 BankLocation;
	bit16 LoadAddressRange;
	bit16 ROMImageSize;
};

struct SsCartStateHeader
{
	bit32 size;
}
;
struct SsCartMemoryHeader
{
	bit32 size;
	bit32 ramsize;
	bit32 banks;
};

struct SsCartChip
{
	CrtChip chip;
	bit16 allocatedSize;
};

struct SsCartBank
{
	bit32 bank;
	SsCartChip roml;
	SsCartChip romh;
};

struct SsCartCommon
{
	bit32 size;
	bit8 reg1;
	bit8 reg2;

	bit8 GAME;
	bit8 EXROM;
	bit8 m_bIsCartAttached;
	bit8 m_bIsCartIOActive;
	bit8 m_bIsCartRegActive;
	bit8 m_iSelectedBank;

	bit8 m_bEnableRAM;
	bit8 m_bAllowBank;
	bit8 m_bREUcompatible;
	bit8 m_bFreezePending;
	bit8 m_bFreezeMode;
};

# pragma pack ()

struct CrtChipAndData
{
	CrtChipAndData();
	~CrtChipAndData();
	CrtChip chip;
	bit8 *pData;
	bool ownData;
	bit16 allocatedSize;
	bit16 romOffset;
	__int64 iFileIndex;
};

struct CrtBank
{
	CrtBank();
	virtual ~CrtBank();
	bit16 bank;
	CrtChipAndData chipAndDataLow;
	CrtChipAndData chipAndDataHigh;
};

typedef shared_ptr<CrtBank> Sp_CrtBank;

//typedef shared_ptr<CrtChipAndData> Sp_CrtChipAndData;
//struct LessChipAndDataBank
//{
//	bool operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const;
//};

typedef std::vector<Sp_CrtBank> CrtBankList;
typedef std::vector<Sp_CrtBank>::iterator CrtBankListIter;
typedef std::vector<Sp_CrtBank>::const_iterator CrtBankListConstIter;

class ICartInterface : public IRegister
{
public:
	virtual bit8 Get_GAME()=0;
	virtual bit8 Get_EXROM()=0;

	virtual bit8 ReadROML(bit16 address)=0;
	virtual bit8 ReadROMH(bit16 address)=0;
	virtual bit8 ReadUltimaxROML(bit16 address)=0;
	virtual bit8 ReadUltimaxROMH(bit16 address)=0;

	virtual void WriteROML(bit16 address, bit8 data)=0;
	virtual void WriteROMH(bit16 address, bit8 data)=0;
	virtual void WriteUltimaxROML(bit16 address, bit8 data)=0;
	virtual void WriteUltimaxROMH(bit16 address, bit8 data)=0;

	virtual bit8 MonReadROML(bit16 address)=0;
	virtual bit8 MonReadROMH(bit16 address)=0;
	virtual bit8 MonReadUltimaxROML(bit16 address)=0;
	virtual bit8 MonReadUltimaxROMH(bit16 address)=0;

	virtual void MonWriteROML(bit16 address, bit8 data)=0;
	virtual void MonWriteROMH(bit16 address, bit8 data)=0;
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data)=0;
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data)=0;

	virtual bool IsUltimax()=0;
	virtual bool IsCartIOActive()=0;
	virtual bool IsCartAttached()=0;
	virtual void Set_IsCartAttached(bool isAttached) = 0;

	virtual void CartFreeze()=0;
	virtual void CartReset()=0;
	virtual void CheckForCartFreeze()=0;

	virtual void AttachCart()=0;
	virtual void DetachCart()=0;
	virtual void ConfigureMemoryMap()=0;

	virtual bit8 *Get_RomH()=0;
	virtual void PreventClockOverflow() = 0;

	virtual HRESULT LoadState(IStream *pfs) = 0;
	virtual HRESULT SaveState(IStream *pfs) = 0;

	virtual HRESULT InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData) = 0;
};

class Cart;

class CartCommon : public ICartInterface
{
public:
	CartCommon(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);
	virtual ~CartCommon();

	void InitReset(ICLK sysclock, bool poweronreset);

	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	virtual bit8 Get_GAME();
	virtual bit8 Get_EXROM();

	virtual bit8 ReadROML(bit16 address);
	virtual bit8 ReadROMH(bit16 address);
	virtual bit8 ReadUltimaxROML(bit16 address);
	virtual bit8 ReadUltimaxROMH(bit16 address);

	virtual void WriteROML(bit16 address, bit8 data);
	virtual void WriteROMH(bit16 address, bit8 data);
	virtual void WriteUltimaxROML(bit16 address, bit8 data);
	virtual void WriteUltimaxROMH(bit16 address, bit8 data);

	virtual bit8 MonReadROML(bit16 address);
	virtual bit8 MonReadROMH(bit16 address);
	virtual bit8 MonReadUltimaxROML(bit16 address);
	virtual bit8 MonReadUltimaxROMH(bit16 address);

	virtual void MonWriteROML(bit16 address, bit8 data);
	virtual void MonWriteROMH(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data);

	virtual bool IsUltimax();
	virtual bool IsCartIOActive();
	virtual bool IsCartAttached();
	virtual void Set_IsCartAttached(bool isAttached);

	virtual void CartFreeze();
	virtual void CartReset();
	virtual void CheckForCartFreeze();

	virtual void AttachCart();
	virtual void DetachCart();
	virtual void ConfigureMemoryMap();
	virtual bit8 *Get_RomH();
	virtual void PreventClockOverflow();

	virtual HRESULT LoadState(IStream *pfs);
	virtual HRESULT SaveState(IStream *pfs);
	virtual HRESULT SaveMemoryState(IStream *pfs);

	virtual HRESULT InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData);
protected:
	virtual void UpdateIO()=0;
	void BankRom();
	virtual unsigned int GetStateBytes(void *pstate);
	virtual HRESULT SetStateBytes(void *pstate, unsigned int size);

	CrtHeader m_crtHeader;
	CrtBankList *m_plstBank;
	bit8 *m_pCartData;
	bit8 *m_pZeroBankData;
	bit8 *m_ipROML_8000;
	bit8 *m_ipROMH_A000;
	bit8 *m_ipROMH_E000;

	bit8 reg1;
	bit8 reg2;

	bit8 GAME;
	bit8 EXROM;
	bool m_bIsCartAttached;
	bool m_bIsCartIOActive;
	bool m_bIsCartRegActive;
	bit8 m_iSelectedBank;

	bool m_bEnableRAM;
	bool m_bAllowBank;
	bool m_bREUcompatible;
	bit8 m_bFreezePending;
	bit8 m_bFreezeMode;
	bit16 m_iRamBankOffsetIO;
	bit16 m_iRamBankOffsetRomL;
	
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;
	bool m_bEffects;

	CrtBankList m_lstStateBank;
private:
	void CleanUp();
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
			ROM=0,
			RAM=1,
			EPROM=2,
		};
	};
	Cart();
	~Cart();
	static const int MAXBANKS = 256;

	void Init(IC6510 *pCpu, bit8 *pC64RamMemory);
	HRESULT LoadCrtFile(LPCTSTR filename);
	shared_ptr<ICartInterface> CreateCartInterface(const CrtHeader &crtHeader);
	int GetTotalCartMemoryRequirement();
	static bool IsSupported(CartType::ECartType hardwareType);
	bool IsSupported();

	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	bit8 Get_GAME();
	bit8 Get_EXROM();

	bit8 ReadROML(bit16 address);
	bit8 ReadROMH(bit16 address);
	bit8 ReadUltimaxROML(bit16 address);
	bit8 ReadUltimaxROMH(bit16 address);

	void WriteROML(bit16 address, bit8 data);
	void WriteROMH(bit16 address, bit8 data);
	void WriteUltimaxROML(bit16 address, bit8 data);
	void WriteUltimaxROMH(bit16 address, bit8 data);

	bit8 MonReadROML(bit16 address);
	bit8 MonReadROMH(bit16 address);
	bit8 MonReadUltimaxROML(bit16 address);
	bit8 MonReadUltimaxROMH(bit16 address);

	void MonWriteROML(bit16 address, bit8 data);
	void MonWriteROMH(bit16 address, bit8 data);
	void MonWriteUltimaxROML(bit16 address, bit8 data);
	void MonWriteUltimaxROMH(bit16 address, bit8 data);

	bool IsUltimax();
	bool IsCartIOActive();
	bool IsCartAttached();
	void Set_IsCartAttached(bool isAttached);
	
	void CartFreeze();
	void CartReset();
	void CheckForCartFreeze();
	void AttachCart(shared_ptr<ICartInterface> spCartInterface);
	void AttachCart();
	void DetachCart();
	void ConfigureMemoryMap();
	bit8 *Get_RomH();
	void PreventClockOverflow();

	HRESULT LoadCartInterface(IStream *pfs, shared_ptr<ICartInterface> &spCartInterface);
	HRESULT LoadState(IStream *pfs);
	HRESULT SaveState(IStream *pfs);
	HRESULT InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData);

	CrtHeader m_crtHeader;//Cart head information
	CrtBankList *m_plstBank;//A list of cart ROM/EPROM banks that index the cart memory data.
	bit8 *m_pCartData; //Pointer to the cart memory data. This a block of RAM followed by 1 or more ROM/EPROM banks.
	bit8 *m_pZeroBankData; //An offset into the cart memory data that points past the initial block of RAM to the first ROM/EPROM bank.

	bool m_bIsCartDataLoaded;
	static const bit32 NOCOMPRESSION = 0;
	static const bit32 HUFFCOMPRESSION = 1;

protected:
	
private:
	static const int RAMRESERVEDSIZE = 64 * 1024 + 8 * 1024;//Assume 64K cart RAM + 8K zero byte bank
	static const int ZEROBANKOFFSET = 64 * 1024;
	static const int CARTRAMSIZE = 64 * 1024;

	void CleanUp();
	static int GetTotalCartMemoryRequirement(CrtBankList *plstBank);
	void BankRom();
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;

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
#endif