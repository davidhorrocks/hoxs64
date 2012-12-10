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

struct CrtChipAndData
{
	CrtChipAndData(CrtChip chip, bit8 *pData);
	CrtChip chip;
	bit8 *pData;
};

# pragma pack ()

//typedef std::shared_ptr<CrtHeader> Sp_CrtHeader;
//typedef std::shared_ptr<CrtChip> Sp_CrtChip;

class Cart : public ErrorMsg
{
public:
	Cart();
	~Cart();
	HRESULT LoadCrtFile(LPCTSTR filename);

private:
	CrtHeader m_crtHeader;
	std::vector<CrtChip> m_lstChip;
	std::vector<bit8 *> m_lstChipData;
	void CleanUp();
};

#endif