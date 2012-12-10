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
};

typedef std::shared_ptr<CrtChipAndData> Sp_CrtChipAndData;

struct LessChipAndDataBank
{
	bool operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const;
};

typedef std::list<Sp_CrtChipAndData> CrtChipAndDataList;
typedef std::list<Sp_CrtChipAndData>::iterator CrtChipAndDataIter;


class Cart : public ErrorMsg
{
public:
	Cart();
	~Cart();
	HRESULT LoadCrtFile(LPCTSTR filename);

private:
	CrtHeader m_crtHeader;
	CrtChipAndDataList m_lstChipAndData;
	void CleanUp();
};

#endif