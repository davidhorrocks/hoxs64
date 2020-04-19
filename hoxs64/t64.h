#pragma once
#include "errormsg.h"

struct C64Filename
{
	BYTE filename[C64DISKFILENAMELENGTH];
};

# pragma pack (1)
struct T64Header
{
	BYTE	header[32];
	WORD	version;
	WORD	maxEntries;
	WORD	usedEntries;
	WORD	notUsed;
	BYTE	containerName[24];
};

struct T64DirectoryItem
{
	BYTE	type64s;
	BYTE	type1541;
	WORD	startAddress;
	WORD	endAddress;
	WORD	notUsed;
	DWORD	offset;
	DWORD	mySize;
	BYTE	c64Filename[16];
};

struct P00Header
{
	BYTE	Signature[7];
	BYTE	Unused1;
	BYTE	C64Filename[16];
	BYTE	Unused2;
	BYTE	RelSize;
	BYTE	Data[1];
};

# pragma pack ()

struct WrappedT64DirectoryItem
{
	WrappedT64DirectoryItem();
	WrappedT64DirectoryItem(int orginalIndex, T64DirectoryItem *item);
	int orginalIndex;
	T64DirectoryItem *item;
};


typedef class MList<struct WrappedT64DirectoryItem *> T64DirectoryItemList;
typedef class MListElement<struct WrappedT64DirectoryItem *> T64DirectoryItemListElement;

typedef class CArray<struct WrappedT64DirectoryItem *> T64DirectoryItemArray;
typedef class CArrayElement<struct WrappedT64DirectoryItem *> T64DirectoryItemArrayElement;


#define MAXT64LIST (3000L)
class T64 : public ErrorMsg
{
public:
	T64();
	~T64();
	HRESULT LoadT64Directory(const TCHAR filename[], int maxcount);
	HRESULT LoadT64File(const TCHAR filename[], DWORD offset, WORD size);
	void CleanUp();

	BYTE *data;

	int numberItems;
	struct T64Header t64Header;
	struct T64DirectoryItem *t64Item;
	struct WrappedT64DirectoryItem *wrappedT64Item;

	T64DirectoryItemArray filteredDirectory;

	static const int CONTAINERNAMELENGTH=24;
	static const int FILENAMELENGTH=16;

private:
	T64DirectoryItemList dirList;

	static int T64::CompareT64DirItemOffset(WrappedT64DirectoryItem * const &a, WrappedT64DirectoryItem * const &b);
	static int T64::CompareT64DirItemOriginalIndex(WrappedT64DirectoryItem * const &a, WrappedT64DirectoryItem * const &b);
};
