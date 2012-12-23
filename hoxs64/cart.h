#ifndef __CART_H__
#define __CART_H__

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
	CrtChipAndData(CrtChip &chip, bit8 *pData);
	virtual ~CrtChipAndData();
	CrtChip chip;
	bit8 *pData;

	__int64 iFileIndex;
};

typedef std::shared_ptr<CrtChipAndData> Sp_CrtChipAndData;

struct LessChipAndDataBank
{
	bool operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const;
};

typedef std::vector<Sp_CrtChipAndData> CrtChipAndDataList;
typedef std::vector<Sp_CrtChipAndData>::iterator CrtChipAndDataIter;
typedef std::vector<Sp_CrtChipAndData>::const_iterator CrtChipAndDataConstIter;


class Cart : public IRegister, public ErrorMsg
{
public:
	class CartType
	{
	public:
		enum ECartType
		{
			Action_Replay = 1,//AR5 + AR6
			Ocean_1 = 5,
			Magic_Desk = 19,
			Action_Replay_4 = 30,
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
	bool IsCartIOActive();
	int GetTotalCartMemoryRequirement();
	void ConfigureMemoryMap();
	void CheckForCartFreeze();
	void CartFreeze();
	void CartReset();

	CrtHeader m_crtHeader;
	CrtChipAndDataList m_lstChipAndData;
	bit8 *m_pCartData;

	bit8 reg1;
	bit8 reg2;

	bit8 GAME;
	bit8 EXROM;
	bool m_bIsCartAttached;
	bool m_bIsCartIOActive;
	bool m_bEnableRAM;
	bool m_bAllowBank;
	bool m_bREUcompatible;
	bit8 m_iSelectedBank;
	bit8 m_bFreezePending;
	bit8 m_bFreezeDone;
	bool m_bDE01WriteDone;
	bit16 m_iRamBankOffset;
protected:
	
private:
	static const int RAMRESERVEDSIZE;
	void CleanUp();
	int GetTotalCartMemoryRequirement(CrtChipAndDataList lstChip);
	IC6510 *m_pCpu;
	bit8 *m_pC64RamMemory;
};

#endif