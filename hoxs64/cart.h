#ifndef __CART_H__
#define __CART_H__

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

# pragma pack ()

struct CrtChipAndData
{
	CrtChipAndData();
	virtual ~CrtChipAndData();
	CrtChip chip;
	bit8 *pData;
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

//typedef shared_ptr<CrtChipAndData> Sp_CrtChipAndData;
typedef shared_ptr<CrtBank> Sp_CrtBank;

//struct LessChipAndDataBank
//{
//	bool operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const;
//};

//typedef vector<Sp_CrtChipAndData> CrtChipAndDataList;
//typedef vector<Sp_CrtChipAndData>::iterator CrtChipAndDataIter;
//typedef vector<Sp_CrtChipAndData>::const_iterator CrtChipAndDataConstIter;


typedef std::vector<Sp_CrtBank> CrtBankList;
typedef std::vector<Sp_CrtBank>::iterator CrtBankListIter;
typedef std::vector<Sp_CrtBank>::const_iterator CrtBankListConstIter;

class Cart : public IRegister, public ErrorMsg
{
public:
	class CartType
	{
	public:
		enum ECartType
		{
			Normal_Cartridge = 0,
			Action_Replay = 1,//AR5 + AR6 + AR4.x
			Final_Cartridge_III = 3,
			Simons_Basic = 4,
			Ocean_1 = 5,
			Fun_Play = 7,
			Super_Games = 8,
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
	Cart();
	~Cart();
	void Init(IC6510 *pCpu, bit8 *pC64RamMemory);
	HRESULT LoadCrtFile(LPCTSTR filename);
	void DetachCart();
	bool IsCartAttached();
	bool IsUltimax();
	void InitReset(ICLK sysclock);
	void UpdateIO();
	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	bit8 ReadROML(bit16 address);
	bit8 ReadUltimaxROML(bit16 address);
	bit8 ReadROMH(bit16 address);
	bit8 ReadUltimaxROMH(bit16 address);
	void WriteROML(bit16 address, bit8 data);
	void WriteUltimaxROML(bit16 address, bit8 data);
	void WriteROMH(bit16 address, bit8 data);
	void WriteUltimaxROMH(bit16 address, bit8 data);


	function<bit8 (bit16)> OnReadROML;
	function<bit8 (bit16)> OnReadUltimaxROML;
	function<bit8 (bit16)> OnReadROMH;
	function<bit8 (bit16)> OnReadUltimaxROMH;
	function<void (bit16, bit8)> OnWriteROML;
	function<void (bit16, bit8)> OnWriteUltimaxROML;
	function<void (bit16, bit8)> OnWriteROMH;
	function<void (bit16, bit8)> OnWriteUltimaxROMH;

	bit8 ReadROML_Zaxxon(bit16 address);
	bit8 ReadUltimaxROML_Zaxxon(bit16 address);

	bit8 ReadROML_EasyFlash(bit16 address);
	bit8 ReadROMH_EasyFlash(bit16 address);
	bit8 ReadUltimaxROML_EasyFlash(bit16 address);
	bit8 ReadUltimaxROMH_EasyFlash(bit16 address);

	void WriteROML_EasyFlash(bit16 address);
	void WriteROMH_EasyFlash(bit16 address);
	void WriteUltimaxROML_EasyFlash(bit16 address);
	void WriteUltimaxROMH_EasyFlash(bit16 address);

	bool IsCartIOActive();
	int GetTotalCartMemoryRequirement();
	void ConfigureMemoryMap();
	void CheckForCartFreeze();
	void CartFreeze();
	void CartReset();
	bool IsSupported(CartType::ECartType hardwareType);
	bool IsSupported();

	CrtHeader m_crtHeader;
	CrtBankList m_lstBank;
	bit8 *m_pCartData;
	bit8 *m_pZeroBankData;

	bit8 reg1;
	bit8 reg2;

	bit8 GAME;
	bit8 EXROM;
	bool m_bIsCartAttached;
	bool m_bIsCartIOActive;
	bool m_bIsCartRegActive;
	bool m_bEnableRAM;
	bool m_bAllowBank;
	bool m_bREUcompatible;
	bit8 m_iSelectedBank;
	bit8 m_bFreezePending;
	bit8 m_bFreezeDone;
	bool m_bDE01WriteDone;
	bit16 m_iRamBankOffset;
	bool m_bSimonsBasic16K;

	bit8 *m_ipROML_8000;
	bit8 *m_ipROMH_A000;
	bit8 *m_ipROMH_E000;
protected:
	
private:
	static const int RAMRESERVEDSIZE;
	static const int ZEROBANKOFFSET;
	void CleanUp();
	int GetTotalCartMemoryRequirement(CrtBankList lstBank);
	void BankRom();
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;
	bool m_bEffects;
	bool m_bActionReplayMk2Rom;
	int m_iActionReplayMk2EnableRomCounter;
	int m_iActionReplayMk2DisableRomCounter;
	ICLK m_clockLastDE00Write;
	ICLK m_clockLastDF40Read;

	bit8 m_iEasyFlashCommandByte;
	bit8 m_iEasyFlashCommandCycle;
	bit8 m_iEasyFlashStatus;
	bit8 m_iEasyFlashByteWritten;
};

#endif