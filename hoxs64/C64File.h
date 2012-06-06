#ifndef __C64FILE_H__
#define __C64FILE_H__

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
	static HRESULT LoadFileImage(bit8 *d64Binary, const bit8 c64Filename[C64DISKFILENAMELENGTH], bit8 **ppFileData, bit16* pFileSize);
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
	C64File();
	~C64File();
	HRESULT Init();
	HRESULT IsFDI(TCHAR filename[], bool &result);
	HRESULT IsG64(TCHAR filename[], bool &result);
	HRESULT IsD64(TCHAR filename[], bool &result);
	HRESULT IsT64(TCHAR filename[], bool &result);
	HRESULT IsTAP(TCHAR filename[], bool &result);
	HRESULT IsPRG(TCHAR filename[], bool &result);
	HRESULT IsP00(TCHAR filename[], bool &result);
	HRESULT IsSID(TCHAR filename[], bool &result);

	HRESULT GetC64FileType(TCHAR filename[],enum eC64FileType &filetype);
	void ClearDirectory();
	HRESULT LoadDirectory(TCHAR filename[], int maxcount, int &count, bool bPrgFilesOnly, HANDLE hevtQuit);
	HRESULT LoadFileImage(TCHAR filename[], const bit8 c64FileName[C64DISKFILENAMELENGTH], bit8 **ppFileData, bit16* pFileSize);
	enum eC64FileType GetFileType();
	int GetFileCount();

	int GetDirectoryItemNameLength(int);
	int GetDirectoryItemName(int index, bit8 *outBuffer, int bufferLength);
	int GetC64DisknameLength();
	int GetC64Diskname(bit8 *outBuffer, int bufferLength);
	int GetOriginalDirectoryIndex(int);
	const bit8 *GetDirectoryItemTypeName(int index);

	static int GetC64FileNameLength(const bit8 filename[], int bufferLength);
	static int CompareC64FileNames(const bit8 filename1[], int length1, const bit8 filename2[], int length2);

	static const bit8 FTN_DEL[3];
	static const bit8 FTN_SEQ[3];
	static const bit8 FTN_PRG[3];
	static const bit8 FTN_USR[3];
	static const bit8 FTN_REL[3];
	static const bit8 FTN_CLR[3];
	static bit8 ConvertPetAsciiToScreenCode(bit8 petascii);
private:
	static bit8 blankname[C64DISKFILENAMELENGTH];
	bit8 mDirectoryItemNameBuffer[C64DISKFILENAMELENGTH];
	void CleanUp();
	static bool FilenameHasExtention(TCHAR filename[], TCHAR ext[]);
	eC64FileType _FileType;
	T64 t64;
	GCRDISK disk;
	C64Directory directory;
	SIDLoader sidLoader;
};
#endif
