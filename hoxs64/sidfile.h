#ifndef __SIDFILE_H__
#define __SIDFILE_H__

# pragma pack (1)
struct SIDFileHeader1
{
	char magicID[4];
	WORD version;
	WORD dataOffset;
	WORD loadAddress;
	WORD initAddress;
	WORD playAddress;
	WORD songs;
	WORD startSong;
	DWORD speed;
	char name[32];
	char author[32];
	char copyright[32];
};

struct SIDFileHeader2 : SIDFileHeader1
{
	WORD flags;
	BYTE startPage;
	BYTE pageLength;
	BYTE secondSIDAddress;
};

struct SIDFileHeader3 : SIDFileHeader2
{
	BYTE thirdSIDAddress;
};

struct fixedheader16
{
	WORD non64marker;
	BYTE magicnumber[3];
	BYTE version;
	WORD mode;
	WORD tbase;
	WORD tlen;
	WORD dbase;
	WORD dlen;
	WORD bbase;
	WORD blen;
	WORD zbase;
	WORD zlen;
	WORD stack;
};

struct fixedheader32
{
	WORD non64marker;
	BYTE magicnumber[3];
	BYTE version;
	WORD mode;
	DWORD tbase;
	DWORD tlen;
	DWORD dbase;
	DWORD dlen;
	DWORD bbase;
	DWORD blen;
	DWORD zbase;
	DWORD zlen;
	DWORD stack;
};

# pragma pack ()


class SIDLoader : public ErrorMsg
{
public:
	SIDLoader();
	~SIDLoader();


	void cleanup();
	HRESULT LoadSIDFile(TCHAR *filename);
	HRESULT LoadSIDDriverResource(bit16 loadAddress, bit8 *pMemory);
	HRESULT LoadSID(bit8 *ramMemory, TCHAR *filename, bool bUseDefaultStartSong, bit16 startSong);
	HRESULT LoadSID(bit8 *ramMemory, bool bUseDefaultStartSong, bit16 startSong);

	static bit16 MakeSidAddressFromByte(bit8 rsidByte);

	bit8 *pSIDFile;
	bit8 *pDriver;
	SIDFileHeader3 sfh;
	bool IsRSID;
	bool IsRSID_BASIC;
	bit32 lenSID;
	bit16 startSID;
	bit32 driverSize;
	bit16 codeSize;
	bit16 driverLoadAddress;

private:
	BOOL eof;
	DWORD sizetlen;
	BYTE *src;
	struct fixedheader32 fh;
	DWORD readByte();
	DWORD readWord();
	DWORD readDWord();
	DWORD readSizetWord();
	DWORD readSizetDWord();
	BOOL driverspace(DWORD len);
	HRESULT SetErrorCouldNotLoadDriverDueToData(bit16 address, bit16 codeSize);
	HRESULT SetErrorCouldNotLoadDriverDueToFreePages(bit32 startPage, bit32 pageLength);	
};

#endif