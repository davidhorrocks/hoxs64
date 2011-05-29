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

struct SIDFileHeader2
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
	WORD flags;
	BYTE startPage;
	BYTE pageLength;
	WORD reserved;
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
	HRESULT LoadSIDDriverResource(WORD loadAddress,BYTE *pMemory);

	HRESULT LoadSID(bit8 *ramMemory, TCHAR *filename, bool bUseDefaultStartSong, WORD startSong);
	HRESULT LoadSID(bit8 *ramMemory, bool bUseDefaultStartSong, WORD startSong);

	BYTE *pSIDFile;
	BYTE *pDriver;
	SIDFileHeader2 sfh;
	BOOL RSID,RSID_BASIC;
	DWORD lenSID;
	WORD startSID;
	DWORD driverSize;
	WORD codeSize;

	WORD driverLoadAddress;

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
};

#endif