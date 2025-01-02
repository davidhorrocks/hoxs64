#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
#include "resource.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "savestate.h"
#include "register.h"
#include "FDI.h"
#include "p64.h"
#include "d64.h"
#include "t64.h"
#include "tap.h"
#include "sidfile.h"
#include "c64file.h"

const bit8 C64File::FTN_DEL[3] = {'D', 'E', 'L'};
const bit8 C64File::FTN_SEQ[3] = {'S', 'E', 'Q'};
const bit8 C64File::FTN_PRG[3] = {'P', 'R', 'G'};
const bit8 C64File::FTN_USR[3] = {'U', 'S', 'R'};
const bit8 C64File::FTN_REL[3] = {'R', 'E', 'L'};
const bit8 C64File::FTN_CLR[3] = {0xa0, 0xa0, 0xa0};

C64File::~C64File()
{
	CleanUp();
}

HRESULT C64File::Init()
{
	if (bInitDone)
	{
		return S_OK;
	}

	_FileType = ef_UNKNOWN;
	ClearError();
	HRESULT hr;

	hr = disk.Init();
	if (FAILED(hr))
	{
		SetError(E_FAIL, TEXT("C64File disk.init() failed."));
		return hr;
	}

	hr = directory.Init();
	if (FAILED(hr))
	{
		disk.Clean();
		SetError(E_FAIL, TEXT("C64File directory.init() failed."));
		return hr;
	}

	bInitDone = true;
	return S_OK;
}

void C64File::CleanUp() noexcept
{
	try
	{
		bInitDone = false;
		t64.CleanUp();
		disk.Clean();
		_FileType = ef_UNKNOWN;
	}
	catch(...)
	{
	}
}

HRESULT C64File::GetC64FileType(const TCHAR filename[],enum eC64FileType &filetype)
{
	HRESULT hr;
	bool br;
	
	filetype = ef_UNKNOWN;
	hr = IsFDI(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_FDI;
		return S_OK;
	}

	hr = IsT64(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_T64;
		return S_OK;
	}

	hr = IsTAP(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_TAP;
		return S_OK;
	}

	hr = IsP00(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_P00;
		return S_OK;
	}

	hr = IsG64(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_G64;
		return S_OK;
	}
	
	hr = IsP64(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_P64;
		return S_OK;
	}

	hr = IsSID(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_SID;
		return S_OK;
	}

	hr = IsD64(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_D64;
		return S_OK;
	}

	hr = IsPRG(filename, br);
	if (FAILED(hr))
	{
		return hr;
	}

	if (br)
	{
		filetype = ef_PRG;
		return S_OK;
	}

	return S_OK;
}

C64File::eC64FileType C64File::GetFileType() const
{
	return _FileType;
}

int C64File::GetFileCount() const
{
	switch(_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		return (int)directory.aItems.Count();
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		return t64.numberItems;
	case ef_TAP:
		return 0;
	case ef_SID:
		return sidLoader.sfh.songs;
	default:
		return 0;
	}

}

//bit8 C64File::blankname[C64DISKFILENAMELENGTH]=
//{
//		0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,
//		0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0
//};

int C64File::GetC64DisknameLength()
{
	switch(_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		return GetC64FileNameLength(directory.Name, sizeof(directory.Name));
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		return GetC64FileNameLength(t64.t64Header.containerName, T64::CONTAINERNAMELENGTH);
	case ef_TAP:
		return 0;
	case ef_SID:
		return (int)strnlen(&sidLoader.sfh.name[0], sizeof(sidLoader.sfh.name));
	default:
		return 0;
	}
}

int C64File::GetC64Diskname(bit8 *outBuffer, int bufferLength)
{
int i,j,k;
	switch(_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		i = GetC64DisknameLength();
		j = (bufferLength < i) ? bufferLength : i;
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, directory.Name, j);
		}

		return i;
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		i = GetC64DisknameLength();
		j = (bufferLength < i) ? bufferLength : i;
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, t64.t64Header.containerName, j);
		}

		return i;
	case ef_TAP:
		return 0;
	case ef_SID:
		i = GetC64DisknameLength();
		j = (bufferLength < i) ? bufferLength : i;
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, &sidLoader.sfh.name[0], j);
			for (k = 0; k < bufferLength; k++)
			{
				outBuffer[k] = StringConverter::ConvertCommandLineAnsiToPetAscii(outBuffer[k]);
			}
		}

		return i;
	default:
		return 0;
	}
}

const char C64File::formatsong[] = "Song %d";

int C64File::GetDirectoryItemNameLength(int index)
{
	int i;
	switch(_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		if (index < 0 || index >= (int)directory.aItems.Count())
		{
			return 0;
		}

		return GetC64FileNameLength(directory.aItems[index].Name, sizeof(directory.aItems[index].Name));
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		if (index < 0 || index >= t64.numberItems)
		{
			return 0;
		}

		return GetC64FileNameLength(&t64.filteredDirectory[index]->item->c64Filename[0], T64::FILENAMELENGTH);
	case ef_TAP:
		return 0;
	case ef_SID:
		if (index < 0 || index >= sidLoader.sfh.songs || index > 0xff)
		{
			return 0;
		}

		mDirectoryItemNameBuffer[0] = 0;
		sprintf_s((char *)&mDirectoryItemNameBuffer[0], _countof(mDirectoryItemNameBuffer) - 1, formatsong, (int)index);
		i = (int)strnlen((char *)mDirectoryItemNameBuffer, _countof(mDirectoryItemNameBuffer));
		return i;
	default:
		return 0;
	}
}

int C64File::GetDirectoryItemName(int index, bit8 *outBuffer, int bufferLength)
{
	int i,j,k;
	switch(_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		if (index < 0 || index >= (int)directory.aItems.Count())
		{
			return 0;
		}

		i = GetDirectoryItemNameLength(index);
		j = (bufferLength < i) ? bufferLength : i;
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, directory.aItems[index].Name, j);
		}

		return i;
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		if (index < 0 || index >= t64.numberItems)
		{
			return 0;
		}

		i = GetDirectoryItemNameLength(index);
		j = (bufferLength < i) ? bufferLength : i;
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, &t64.filteredDirectory[index]->item->c64Filename[0], j);
		}

		return i;
	case ef_TAP:
		return 0;
	case ef_SID:
		if (index < 0 || index >= sidLoader.sfh.songs || index > 0xff)
		{
			return 0;
		}

		mDirectoryItemNameBuffer[0] = 0;
		sprintf_s((char *)&mDirectoryItemNameBuffer[0], _countof(mDirectoryItemNameBuffer) - 1, formatsong, (int)index + 1);
		i = (int)strnlen((char *)mDirectoryItemNameBuffer, _countof(mDirectoryItemNameBuffer));
		j = std::min(bufferLength, i);
		j = (int)std::min((size_t)j, (size_t)_countof(mDirectoryItemNameBuffer));
		if (outBuffer)
		{
			memcpy_s(outBuffer, bufferLength, mDirectoryItemNameBuffer, j);
			for (k = 0; k < bufferLength; k++)
			{
				outBuffer[k] = StringConverter::ConvertCommandLineAnsiToPetAscii(outBuffer[k]);
			}
		}

		return i;
	default:
		return 0;
	}
}

const bit8* C64File::GetDirectoryItemTypeName(int index)
{
	switch (_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
		if (index < 0 || index >= (int)directory.aItems.Count())
			return 0;
		switch (directory.aItems[index].ItemType)
		{
		case C64DirectoryItem::DEL:
			return FTN_DEL;
		case C64DirectoryItem::PRG:
			return FTN_PRG;
		case C64DirectoryItem::SEQ:
			return FTN_SEQ;
		case C64DirectoryItem::USR:
			return FTN_USR;
		case C64DirectoryItem::REL:
			return FTN_REL;
		default:
			return FTN_CLR;
		}
	case ef_PRG:
		return 0;
	case ef_P00:
		return 0;
	case ef_T64:
		return FTN_CLR;
	case ef_TAP:
		return 0;
	case ef_SID:
		return FTN_CLR;
	default:
		return 0;
	}
}


int C64File::GetOriginalDirectoryIndex(int index)
{
	HRESULT hr = Init();
	if (FAILED(hr))
	{
		return 0;
	}

	switch (_FileType)
	{
	case ef_FDI:
	case ef_P64:
	case ef_G64:
	case ef_D64:
	case ef_PRG:
	case ef_P00:
	case ef_TAP:
	case ef_SID:
		return index;
	case ef_T64:
		if (index<0 || index > t64.numberItems)
		{
			return 0;
		}
		else
		{
			return t64.filteredDirectory[index]->orginalIndex;
		}
	default:
		return 0;
	}
}

void C64File::ClearDirectory()
{
	_FileType = ef_UNKNOWN;
	directory.Clear();
}

HRESULT C64File::LoadDirectory(const TCHAR filename[], int maxcount, int &count, bool bPrgFilesOnly, HANDLE hevtQuit)
{
	eC64FileType filetype;
	HRESULT hr = Init();
	if (FAILED(hr))
	{
		return hr;
	}

	ClearDirectory();
	if (_tcslen(filename) == 0)
	{
		return E_FAIL;
	}

	hr = GetC64FileType(filename, filetype);
	if (FAILED(hr))
	{
		return hr;
	}

	switch(filetype)
	{
	case ef_T64:
		hr = t64.LoadT64Directory(filename, maxcount);
		if (FAILED(hr))
		{
			return SetError(t64);
		}

		break;
	case ef_FDI:
		disk.mhevtQuit = hevtQuit;
		hr = disk.LoadFDIFromFile(filename);
		if (SUCCEEDED(hr))
		{
			disk.ConvertP64toGCR();
			hr = S_OK;
			if (hevtQuit)
			{
				if (WaitForSingleObject(hevtQuit, 0) == WAIT_OBJECT_0)
				{
					hr = E_FAIL;
				}
			}

			if (SUCCEEDED(hr))
			{
				hr = disk.ConvertGCRToD64(disk.m_d64TrackCount);
			}
		}

		directory.LoadDirectory(disk.m_pD64Binary, bPrgFilesOnly);
		disk.mhevtQuit = NULL;
		break;
	case ef_P64:
		disk.mhevtQuit = hevtQuit;
		hr = disk.LoadP64FromFile(filename);
		if (SUCCEEDED(hr))
		{
			disk.ConvertP64toGCR();
			hr = S_OK;
			if (hevtQuit)
			{
				if (WaitForSingleObject(hevtQuit, 0) == WAIT_OBJECT_0)
				{
					hr = E_FAIL;
				}
			}

			if (SUCCEEDED(hr))
			{
				hr = disk.ConvertGCRToD64(disk.m_d64TrackCount);
			}
		}
		directory.LoadDirectory(disk.m_pD64Binary, bPrgFilesOnly);
		disk.mhevtQuit = NULL;
		break;
	case ef_G64:
		disk.mhevtQuit = hevtQuit;
		hr = disk.LoadG64FromFile(filename, false);
		if (SUCCEEDED(hr))
		{
			hr = disk.ConvertGCRToD64(disk.m_d64TrackCount);
		}

		directory.LoadDirectory(disk.m_pD64Binary, bPrgFilesOnly);
		disk.mhevtQuit = NULL;
		break;
	case ef_D64:
		disk.mhevtQuit = hevtQuit;
		hr = disk.LoadD64FromFile(filename, false, false);
		directory.LoadDirectory(disk.m_pD64Binary, bPrgFilesOnly);
		disk.mhevtQuit = NULL;
		break;
	case ef_SID:
		hr = sidLoader.LoadSIDFile(filename);
		if (FAILED(hr))
		{
			return SetError(sidLoader);
		}

		break;
	}

	_FileType=filetype;
	count = GetFileCount();
	return S_OK;
}

HRESULT C64File::LoadFileImage(const TCHAR filename[], const bit8 c64FileName[C64DISKFILENAMELENGTH], bit8** ppFileData, bit32* pFileSize)
{
	HRESULT hr = Init();
	if (FAILED(hr))
	{
		return hr;
	}

	eC64FileType filetype;

	if (ppFileData != nullptr)
	{
		*ppFileData = 0;
	}

	if (pFileSize != nullptr)
	{
		*pFileSize = 0;
	}

	if (_tcslen(filename) == 0)
	{
		return E_FAIL;
	}

	hr = GetC64FileType(filename, filetype);
	if (FAILED(hr))
	{
		return hr;
	}

	switch (filetype)
	{
	case ef_FDI:
		hr = disk.LoadFDIFromFile(filename);
		if (SUCCEEDED(hr))
		{
			disk.ConvertP64toGCR();
			disk.ConvertGCRToD64(disk.m_d64TrackCount);
		}
		break;
	case ef_P64:
		hr = disk.LoadP64FromFile(filename);
		if (SUCCEEDED(hr))
		{
			disk.ConvertP64toGCR();
			disk.ConvertGCRToD64(disk.m_d64TrackCount);
		}
		break;
	case ef_G64:
		hr = disk.LoadG64FromFile(filename, false);
		if (SUCCEEDED(hr))
		{
			disk.ConvertGCRToD64(disk.m_d64TrackCount);
		}
		break;
	case ef_D64:
		hr = disk.LoadD64FromFile(filename, false, false);
		break;
	default:
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		hr = C64Directory::LoadFileImage(disk.m_pD64Binary, c64FileName, ppFileData, pFileSize);
	}

	if (FAILED(hr))
	{
		return SetError(disk);
	}

	return S_OK;
}

bool C64File::FilenameHasExtension(const TCHAR *filename, const TCHAR *ext)
{
	return Wfs::FilenameHasExtension(filename, ext);
}

HRESULT C64File::IsFDI(const TCHAR filename[], bool &result)
{
	DWORD r;
	struct FDIHeader fdiHeader;
	DWORD file_size, bytes_read;
	HANDLE hfile=0;
	struct FDITrackDescription *fdiTrackDescription = &fdiHeader.trackDescription[0];

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".fdi")))
	{
		return S_OK;
	}

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	r = ReadFile(hfile, &fdiHeader, sizeof(struct FDIHeader), &bytes_read, NULL);
	if (r==0 || sizeof(struct FDIHeader)!=bytes_read)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	CloseHandle(hfile);
	
	if (_memicmp(&fdiHeader.signature[0],"Formatted Disk Image file\r\n", 27)!=0)
	{	
		return S_OK;
	}

	fdiHeader.ltrack = wordswap(fdiHeader.ltrack);
	if (fdiHeader.type != 1)
	{
		//5.25 inch check
		return S_OK;
	}

	if (fdiHeader.lhead != 0)
	{
		return S_OK;
	}

	result = true;
	return S_OK;
}


HRESULT C64File::IsT64(const TCHAR filename[], bool &result)
{
	DWORD r;
	struct T64Header t64Header;
	DWORD file_size, bytes_read, byteCount;
	HANDLE hfile=0;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".t64")))
	{
		return S_OK;
	}

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	byteCount = sizeof(struct T64Header);
	r = ReadFile(hfile, &t64Header, byteCount, &bytes_read, NULL);
	if (r==0 || byteCount!=bytes_read)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	if (!_stricmp((char *)&t64Header.header[0], "C64"))
	{
		CloseHandle(hfile);
		return S_OK;
	}

	CloseHandle(hfile);
	result = true;
	return S_OK;
}

HRESULT C64File::IsSID(const TCHAR filename[], bool &result)
{
	DWORD r;
	DWORD file_size, bytes_read, bytesToRead;
	HANDLE hfile=0;
	SIDFileHeader2 sfh;
	const int sizeheader = 2;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".sid")))
	{
		return S_OK;
	}

	hfile = CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	if (file_size < sizeof(sfh))
	{ 
		CloseHandle(hfile);
		return E_FAIL;
	}

	bytesToRead = sizeof(struct SIDFileHeader1);
	r=ReadFile(hfile, &sfh, bytesToRead, &bytes_read, NULL);
	if (r==0)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	if (bytes_read!=bytesToRead)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	CloseHandle(hfile);
	if (memcmp(sfh.magicID ,"PSID", 4)!=0 && memcmp(sfh.magicID ,"RSID", 4)!=0)
	{
		return S_OK;
	}

	result = true;
	return S_OK;
}

HRESULT C64File::IsD64(const TCHAR filename[], bool &result)
{
	DWORD file_size;
	HANDLE hfile=0;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".d64")))
	{
		return S_OK;
	}

	hfile = CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	CloseHandle(hfile);
	switch (file_size)
	{
	case 174848://35 track, no errors
	case 175531://35 track, 683 error bytes
	case 196608://40 track, no errors
	case 197376://40 track, 768 error bytes
		break;
	default:
		return S_OK;
	}

	result = true;
	return S_OK;
}

HRESULT C64File::IsG64(const TCHAR filename[], bool &result)
{
	DWORD file_size, bytes_read, byteCount;
	HANDLE hfile=0;
	DWORD r;
	struct G64Header g64Header;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".g64")))
	{
		return S_OK;
	}

	hfile = CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	byteCount = sizeof(g64Header);
	r = ReadFile(hfile, &g64Header, byteCount, &bytes_read, NULL);
	if (r == 0 || bytes_read != byteCount)
	{
		CloseHandle(hfile);
		return S_OK;
	}

	if (_memicmp(&g64Header.signature[0],"GCR-1541", 8)!=0)
	{
		return S_OK;
	}

	CloseHandle(hfile);
	result = true;
	return S_OK;
}

HRESULT C64File::IsP64(const TCHAR filename[], bool &result)
{
	DWORD file_size, bytes_read, byteCount;
	HANDLE hfile=0;
	DWORD r;
	const char signature[] = "P64-1541";
	char signaturebuffer[8];

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".p64")))
	{
		return S_OK;
	}

	hfile = CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	byteCount = sizeof(signaturebuffer);
	r = ReadFile(hfile, &signaturebuffer[0], byteCount, &bytes_read, NULL);
	if (r==0 || bytes_read!=byteCount)
	{
		CloseHandle(hfile);
		return S_OK;
	}

	if (_memicmp(signature, signaturebuffer, byteCount)!=0)
	{
		return S_OK;
	}

	CloseHandle(hfile);
	result = true;
	return S_OK;
}

HRESULT C64File::IsTAP(const TCHAR filename[], bool &result)
{
	DWORD r;
	DWORD file_size, bytes_read, byteCount;
	HANDLE hfile=0;
	RAWTAPE rt;
	result = false;
	if (!FilenameHasExtension(filename, TEXT(".tap")))
	{
		return S_OK;
	}

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	if (file_size > 5000000L || file_size < 50L){
		CloseHandle(hfile);
		return S_OK;
	}
	byteCount= sizeof(rt);
	r=ReadFile(hfile, &rt, byteCount, &bytes_read,NULL);
	if (r==0 || bytes_read!=byteCount)
	{
		CloseHandle(hfile);
		return S_OK;
	}

	CloseHandle(hfile);
	if (_strnicmp((char*)&rt, "C64-TAPE-RAW", sizeof(rt.Signature)) != 0)
	{
		return S_OK;
	}
	if (rt.Version != 0 && rt.Version != 1)
	{
		return S_OK;
	}

	result = true;
	return S_OK;
}
HRESULT C64File::IsPRG(const TCHAR filename[], bool &result)
{
DWORD file_size;
HANDLE hfile=0;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".prg")))
	{
		return S_OK;
	}

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	CloseHandle(hfile);
	if ((file_size-2) > 0xffff || file_size <= 2)
	{
		//subtract two bytes for the load address header
		return S_OK;
	}

	result = true;
	return S_OK;
}
HRESULT C64File::IsP00(const TCHAR filename[], bool &result)
{
DWORD r;
DWORD file_size, bytes_read, byteCount;
HANDLE hfile=0;
P00Header header;

	result = false;
	if (!FilenameHasExtension(filename, TEXT(".p00")))
	{
		return S_OK;
	}

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return E_FAIL;
	}

	if ((file_size-sizeof(header)+1) > 0xffff || file_size < sizeof(header))
	{
		CloseHandle(hfile);
		return S_OK;
	}

	byteCount= sizeof(header);
	r=ReadFile(hfile, &header, byteCount, &bytes_read,NULL);
	if (r==0 || bytes_read!=byteCount)
	{
		CloseHandle(hfile);
		return S_OK;
	}

	CloseHandle(hfile);
	if (_strnicmp((char*)&header.Signature, "C64File", sizeof(header.Signature)) != 0)
	{
		return S_OK;
	}

	result = true;
	return S_OK;
}

int C64File::GetC64FileNameLength(const bit8 filename[], unsigned int bufferLength)
{
	unsigned int i;
	if (filename == NULL)
	{
		return 0;
	}

	for (i = 0; i < bufferLength; i++)
	{
		if (filename[i] == 0xA0)
		{
			break;
		}
	}

	return i;
}

void C64File::GetC64ClearFileName(bit8 filename[], unsigned int bufferLength)
{
	if (filename == NULL)
	{
		return;
	}

	bufferLength = std::max(bufferLength, C64DISKFILENAMELENGTH);
	memset(filename, 0xA0, bufferLength);
}

int C64File::CompareC64FileNames(const bit8 filename1[], unsigned int bufferLength1, const bit8 filename2[], unsigned int bufferLength2)
{
	unsigned int i, j, len1, len2;
	if (filename1 == NULL && filename2 == NULL)
	{
		return 0;
	}

	if (filename1 == NULL)
	{
		return -1;
	}

	if (filename2 == NULL)
	{
		return 1;
	}

	len1 = GetC64FileNameLength(filename1, bufferLength1);
	len2 = GetC64FileNameLength(filename2, bufferLength2);
	j = std::min(len1, len2);
	for (i = 0; i < j; i++)
	{
		if (filename2[i] > filename1[i])
		{
			return -1;
		}
		else if (filename2[i] < filename1[i])
		{
			return 1;
		}
	}

	if (len2 > len1)
	{
		return -1;
	}
	else if (len2 < len1)
	{
		return 1;
	}

	return 0;
}

//bit8 C64File::ConvertPetAsciiToScreenCode(bit8 petascii)
//{
//	bit8 addition = 0;
//	if (petascii <= 31)
//		addition = +128;
//	else if (petascii <= 63)
//		addition = 0;
//	else if (petascii <= 95)
//		addition = +192;
//	else if (petascii <= 127)
//		addition = +224;
//	else if (petascii <= 159)
//		addition = +64;
//	else if (petascii <= 191)
//		addition = +192;
//	else if (petascii <= 223)
//		addition = +128;
//	else if (petascii <= 254)
//		addition = +128;
//	else
//		addition = +95;
//
//	return (petascii + addition) & 0xff;
//}
//
//unsigned char C64File::ConvertPetAsciiToAnsi(bit8 ch)
//{
//	if (ch <= ' ')
//	{
//		return ' ';
//	}
//
//	switch (ch)
//	{
//	case 0x5c:// £ Pound
//		return 0xa3;
//	case 0x5f:
//		return '~';
//	case 0x60:
//		return 0x97;
//	case 0x6d:
//		return '\\';
//	}
//
//	if (ch >= 0x8d && ch <= 0x9f)
//	{
//		return ' ';
//	}
//
//	return ch;
//}
//
//bit8 C64File::ConvertAnsiToPetAscii(bit8 ch)
//{
//	if (ch <= ' ')
//	{
//		return ' ';
//	}
//
//	if (ch >= 'a' && ch <= 'z')
//	{
//		return (ch - 0x20) & 0xff;
//	}
//
//	switch (ch)
//	{
//	case 0xa3:// £ Pound
//		return 0x5c;
//	case '~':
//		return 0x5f;
//	case '\\':
//		return 0x6d;
//	case '_':
//		return 0xa4;
//	default:
//		break;
//	}
//
//	if (ch >= 0x8d && ch <= 0x9f)
//	{
//		return ' ';
//	}
//
//	return ch;
//}
//
//bit8 C64File::ConvertCommandLineAnsiToPetAscii(bit8 ch)
//{
//	if (ch >= 'A' && ch <= 'Z')
//	{
//		return (ch + 32) & 0xff;
//	}
//	else if (ch >= 'a' && ch <= 'z')
//	{
//		return (ch - 32) & 0xff;
//	}
//
//	switch (ch)
//	{
//	case 0xa3:// £ Pound
//		return 0x5c;
//	case '`':
//		return 0x27;
//	case '\\':
//		return 0x6d;
//	default:
//		break;
//	}
//
//	return ch;
//}

HRESULT C64Directory::Init()
{
HRESULT hr;
	hr = aItems.Resize(144);
	return hr;
}

void C64Directory::LoadDirectory(bit8* d64Binary, bool bPrgFilesOnly)
{
	int currentTrack;
	int currentSector;
	int currentEntry;
	int j;

	aItems.ClearCount();
	memset(this->Name, 0xA0, sizeof(this->Name));
	if (d64Binary == NULL)
	{
		return;
	}

	memcpy_s(this->Name, sizeof(this->Name), &d64Binary[GCRDISK::D64_info[17].file_offset + 0x90], D64DISKNAMELENGTH);
	currentTrack = 18;
	currentSector = 1;
	currentEntry = 0;
	for (j = 0; j < C64Directory::D64MAXDIRECTORYITEMCOUNT; j++)
	{
		C64DirectoryItem item;
		if (LoadItem(d64Binary, currentTrack, currentSector, currentEntry, item) != S_OK)
		{
			break;
		}

		if (!bPrgFilesOnly || (bPrgFilesOnly && item.ItemType == C64DirectoryItem::PRG))
		{
			if (item.ItemCode != 0)
			{
				aItems.Append(item);
			}
		}

		if (MoveNextEntry(d64Binary, currentTrack, currentSector, currentEntry) != 0)
		{
			break;
		}
	}
}

HRESULT C64Directory::LoadFileImage(bit8 *d64Binary, const bit8 c64Filename[C64DISKFILENAMELENGTH], bit8 **ppFileData, bit32* pFileSize)
{
	C64Directory dir;
	HRESULT hr;
	unsigned int i,j;
	bit32 p,q;
	unsigned int currentTrack;
	unsigned int currentSector;
	unsigned int nextTrack;
	unsigned int nextSector;
	bit32 fileSize;
	constexpr int MAXSECTORS = GCRDISK::D64MaxCbm42TrackFreeFileBlocks;

	if (ppFileData != nullptr)
	{
		*ppFileData = 0;
	}

	if (pFileSize != nullptr)
	{
		*pFileSize = 0;
	}

	fileSize = 0;
	hr = dir.Init();
	if (FAILED(hr))
	{
		return hr;
	}

	dir.LoadDirectory(d64Binary, true);
	for (i = 0; i < dir.aItems.Count(); i++)
	{
		if ((dir.aItems[i].ItemType) != C64DirectoryItem::PRG)
		{
			continue;
		}

		if (c64Filename == NULL)
		{
			break;
		}

		if (C64File::CompareC64FileNames(c64Filename, C64DISKFILENAMELENGTH, dir.aItems[i].Name, sizeof(dir.aItems[i].Name)) == 0)
		{
			break;
		}
	}

	if (i >= (int)dir.aItems.Count())
	{
		return E_FAIL;
	}

	currentTrack = dir.aItems[i].Track;
	currentSector = dir.aItems[i].Sector;
	if (IsDiskSectorNumberOK(currentTrack, currentSector))
	{
		nextTrack = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 0];
		nextSector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 1];
		fileSize = 0;
		for (j = 0; j < MAXSECTORS; j++)
		{

			if (nextTrack == 0)
			{
				if (nextSector >= 0xff)
				{
					fileSize += (0x100 - 2);
				}
				else if (nextSector >= 1)
				{
					fileSize += (nextSector - 1);
				}
			}
			else
			{
				if (!IsDiskSectorNumberOK(nextTrack, nextSector))
				{
					break;
				}

				fileSize += (0x100 - 2);
			}

			if (nextTrack == 0)
			{
				break;
			}

			currentTrack = nextTrack;
			currentSector = nextSector;
			nextTrack = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 0];
			nextSector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 1];
		}
	}

	if (ppFileData == 0)
	{
		return S_OK;
	}

	if (pFileSize != nullptr)
	{
		*pFileSize = fileSize;
	}

	if (fileSize > 0)
	{
		if (ppFileData != nullptr)
		{
			*ppFileData = (bit8*)GlobalAlloc(GPTR, fileSize);
			if (*ppFileData == 0)
			{
				return E_OUTOFMEMORY;
			}
		}

		currentTrack = dir.aItems[i].Track;
		currentSector = dir.aItems[i].Sector;
		nextTrack = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 0];
		nextSector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 1];
		p = 0;
		q = 0;
		for (j = 0; j < MAXSECTORS; j++)
		{
			if (nextTrack == 0)
			{
				if (nextSector >= 0xff)
				{
					q = 0x100 - 2;
				}
				else if (nextSector >= 1)
				{
					q = nextSector - 1;
				}
				else
				{
					q = 0;
				}
			}
			else
			{
				if (!IsDiskSectorNumberOK(nextTrack, nextSector))
				{
					break;
				}

				q = 0x100 - 2;
			}

			assert(p + q <= fileSize);
			if (q > 0)
			{
				if (ppFileData != nullptr)
				{
					memcpy_s(&(*ppFileData)[p], fileSize - p, &d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 2], q);
				}
			}

			p += q;
			if (nextTrack == 0)
			{
				break;
			}

			currentTrack = nextTrack;
			currentSector = nextSector;
			nextTrack = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 0];
			nextSector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 1];
		}

		assert(p == fileSize);
	}

	return S_OK;
}

//currentTrack is in range [1..40]
//currentSector is in range [0..20]
HRESULT C64Directory::LoadItem(bit8 *d64Binary, int currentTrack , int currentSector, int currentEntry, C64DirectoryItem& fileItem)
{
	C64DirectoryItem file;
	const int ENDOFDIRECTORY = 1;

	//if (!IsDiskSectorNumberOK(currentTrack, currentSector))
	//	return ENDOFDIRECTORY;

	file.Track = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + currentEntry * 0x20 + 3];
	file.Sector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + currentEntry * 0x20 + 4];	
	memset(file.Name, 0xA0, sizeof(file.Name));
	memcpy_s(file.Name, sizeof(file.Name), &d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + currentEntry * 0x20 + 0x5], D64FILENAMELENGTH);

	//if (file.Name[0] == 0)
	//	return ENDOFDIRECTORY;

	file.ItemCode = (d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + currentEntry * 0x20 + 0x2]);
	file.ItemType = (C64DirectoryItem::D64FileType)(file.ItemCode & 7);
	fileItem = file;
	return S_OK;
}

int C64Directory::MoveNextEntry(bit8* d64Binary, int& currentTrack, int& currentSector, int& currentEntry)
{
	int nextTrack;
	int nextSector;

	if (!IsDiskSectorNumberOK(currentTrack, currentSector))
	{
		return 1;
	}

	if (currentEntry < 0 || currentEntry >= 8)
	{
		return 1;
	}

	nextTrack = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 0];
	nextSector = d64Binary[GCRDISK::D64_info[currentTrack - 1].file_offset + currentSector * 0x100 + 1];

	if (currentEntry < 7)
	{
		currentEntry++;
		return 0;
	}

	if (nextTrack == 0)
	{
		return 1;
	}

	if (!IsDiskSectorNumberOK(nextTrack, nextSector))
	{
		return 1;
	}

	currentEntry = 0;
	currentTrack = nextTrack;
	currentSector = nextSector;
	return 0;
}

bool C64Directory::IsDiskSectorNumberOK(int currentTrack, int currentSector)
{
	if (currentTrack < 1 || currentTrack > 40)
	{
		return false;
	}

	if (currentSector >= (int)GCRDISK::D64_info[currentTrack - 1].sector_count)
	{
		return false;
	}

	return true;
}

void C64Directory::Clear()
{
	memset(this->Name, 0xA0, sizeof(this->Name));
	aItems.ClearCount();
}

