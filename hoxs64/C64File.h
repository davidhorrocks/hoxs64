#pragma once
#include "wfs.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "savestate.h"
#include "FDI.h"
#include "p64.h"
#include "d64.h"
#include "t64.h"
#include "tap.h"
#include "sidfile.h"
#include "errormsg.h"

struct C64DirectoryItem
{
	enum D64FileType
	{
		DEL = 0x0,
		SEQ = 0x1,
		PRG = 0x2,
		USR = 0x3,
		REL = 0x4
	};
	static const int DIRECTORYITEMNAMELENGTH = 24;
	D64FileType ItemType;
	bit8 ItemCode;
	bit8 Track;
	bit8 Sector;
	bit8 Name[DIRECTORYITEMNAMELENGTH];
	bit16 Size;

};

class C64Directory
{
public:
	static const int DIRECTORYNAMELENGTH = 24;
	static const int D64DISKNAMELENGTH = 16;
	static const int D64FILENAMELENGTH = 16;
	static const int D64MAXDIRECTORYITEMCOUNT = 6008;
	HRESULT Init();
	bit8 Name[DIRECTORYNAMELENGTH];
	CArray<C64DirectoryItem> aItems;
	void LoadDirectory(bit8 *d64Binary, bool bPrgFilesOnly);
	void Clear();
	static HRESULT LoadFileImage(bit8 *d64Binary, const bit8 c64Filename[C64DISKFILENAMELENGTH], bit8 **ppFileData, bit32* pFileSize);
private:
	static HRESULT LoadItem(bit8 *d64Binary, int currentTrack , int currentSector, int currentEntry, C64DirectoryItem& fileItem);
	static int MoveNextEntry(bit8 *d64Binary, int& currentTrack , int& currentSector, int& currentEntry);
	static bool IsDiskSectorNumberOK(int currentTrack , int currentSector);
};

class C64File : public ErrorMsg
{
public:
	enum eC64FileType
	{
        ef_P64,
		ef_FDI,
		ef_G64,
		ef_D64,
		ef_PRG,
		ef_P00,
		ef_T64,
		ef_TAP,
		ef_SID,
		ef_UNKNOWN
	};

	C64File() = default;
	~C64File();
	C64File(const C64File&) = default;
	C64File(C64File&&) = default;
	C64File& operator=(const C64File&) = default;
	C64File& operator=(C64File&&) = default;

	static HRESULT IsFDI(const TCHAR filename[], bool &result);
	static HRESULT IsP64(const TCHAR filename[], bool &result);
	static HRESULT IsG64(const TCHAR filename[], bool &result);
	static HRESULT IsD64(const TCHAR filename[], bool &result);
	static HRESULT IsT64(const TCHAR filename[], bool &result);
	static HRESULT IsTAP(const TCHAR filename[], bool &result);
	static HRESULT IsPRG(const TCHAR filename[], bool &result);
	static HRESULT IsP00(const TCHAR filename[], bool &result);
	static HRESULT IsSID(const TCHAR filename[], bool &result);
	static HRESULT GetC64FileType(const TCHAR filename[],enum eC64FileType &filetype);
	static int GetC64FileNameLength(const bit8 filename[], unsigned int bufferLength);
	static void GetC64ClearFileName(bit8 filename[], unsigned int bufferLength);
	static int CompareC64FileNames(const bit8 filename1[], unsigned int length1, const bit8 filename2[], unsigned int length2);
	//static bit8 ConvertPetAsciiToScreenCode(bit8 petascii);
	//static bit8 ConvertAnsiToPetAscii(bit8 petascii);
	//static bit8 ConvertCommandLineAnsiToPetAscii(bit8 ch);
	//static unsigned char ConvertPetAsciiToAnsi(bit8 ch);
	static bool FilenameHasExtension(const TCHAR* filename, const TCHAR* ext);

	void ClearDirectory();
	HRESULT LoadDirectory(const TCHAR filename[], int maxcount, int &count, bool bPrgFilesOnly, HANDLE hevtQuit);
	HRESULT LoadFileImage(const TCHAR filename[], const bit8 c64FileName[C64DISKFILENAMELENGTH], bit8 **ppFileData, bit32* pFileSize);
	enum eC64FileType GetFileType() const;
	int GetFileCount() const;
	int GetDirectoryItemNameLength(int);
	int GetDirectoryItemName(int index, bit8 *outBuffer, int bufferLength);
	int GetC64DisknameLength();
	int GetC64Diskname(bit8 *outBuffer, int bufferLength);
	int GetOriginalDirectoryIndex(int);
	HRESULT Init();
	const bit8 *GetDirectoryItemTypeName(int index);

	static const bit8 FTN_DEL[3];
	static const bit8 FTN_SEQ[3];
	static const bit8 FTN_PRG[3];
	static const bit8 FTN_USR[3];
	static const bit8 FTN_REL[3];
	static const bit8 FTN_CLR[3];
	static const char formatsong[];
private:
	bool bInitDone = false;
	bit8 mDirectoryItemNameBuffer[20];
	void CleanUp() noexcept;
	eC64FileType _FileType;
	T64 t64;
	GCRDISK disk;
	C64Directory directory;
	SIDLoader sidLoader;
};
