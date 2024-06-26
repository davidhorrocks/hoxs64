#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include "boost2005.h"
#include "wfs.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "resource.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "t64.h"


WrappedT64DirectoryItem::WrappedT64DirectoryItem()
{
	orginalIndex = 0;
	item = NULL;
}

WrappedT64DirectoryItem::WrappedT64DirectoryItem(int orginalIndex, T64DirectoryItem *item)
{
	this->orginalIndex = orginalIndex;
	this->item = item;
}

int T64::CompareT64DirItemOffset(WrappedT64DirectoryItem * const &a, WrappedT64DirectoryItem * const &b)
{
	//-ve means a is before b;
	//+ve means b is before a;
	return (int)(a->item->offset - b->item->offset);
}

int T64::CompareT64DirItemOriginalIndex(WrappedT64DirectoryItem * const &a, WrappedT64DirectoryItem * const &b)
{
	//-ve means a is before b;
	//+ve means b is before a;
	return (int)(a->orginalIndex - b->orginalIndex);
}

T64::T64()
{
	data=0;
	t64Item=0;
	wrappedT64Item=0;
	ClearError();
	numberItems=0;
	memset(&t64Header, 0, sizeof(t64Header));
}


T64::~T64()
{
	CleanUp();
}

void T64::CleanUp()
{
	if (data)
		free(data);
	data=0;
	if (t64Item)
		free(t64Item);
	if (wrappedT64Item)
		free(wrappedT64Item);
	t64Item=0;
	wrappedT64Item=0;
	filteredDirectory.Clear();
	dirList.Clear();
	numberItems=0;
	memset(&t64Header, 0, sizeof(t64Header));
}

HRESULT T64::LoadT64Directory(const std::wstring filename, int maxcount)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read, byteCount;
LARGE_INTEGER file_size;
WORD i;
int numItems;
HRESULT hr;

	ClearError();
	CleanUp();
	numItems=0;
	hfile=0;

	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."), &filename[0]);
	}

	if (!GetFileSizeEx(hfile, &file_size))
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not open %s."), filename);
	}

	if (file_size.QuadPart > MAXUINT32 || file_size.QuadPart < 0)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("File %s is too large."), filename);
	}

	byteCount = sizeof(struct T64Header);
	r = ReadFile(hfile, &t64Header, byteCount, &bytes_read, NULL);
	if (r==0 || byteCount!=bytes_read)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not read from %s."), filename);
	}

	if (!_stricmp((char *)&t64Header.header[0], "C64"))
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Invalid T64 file %s"), filename);
	}


	if (t64Header.maxEntries == 0)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("The directory is empty."));
	}

	if (t64Header.maxEntries > maxcount)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("The directory is too large to load."));
	}

	t64Item = (T64DirectoryItem *)malloc(sizeof(struct T64DirectoryItem) * t64Header.maxEntries);
	if (!t64Item)
	{
		CloseHandle(hfile);
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
	}

	wrappedT64Item = (WrappedT64DirectoryItem *)malloc(sizeof(struct WrappedT64DirectoryItem) * t64Header.maxEntries);
	if (!wrappedT64Item)
	{
		CloseHandle(hfile);
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
	}

	byteCount = sizeof(struct T64DirectoryItem) * t64Header.maxEntries;
	r = ReadFile(hfile, t64Item, byteCount, &bytes_read, NULL);
	if (r==0 || byteCount!=bytes_read)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not read from %s."), filename);
	}

	CloseHandle(hfile);
	for (i=0 ; i < t64Header.maxEntries ; i++)
	{
		wrappedT64Item[i].item = &t64Item[i];
		wrappedT64Item[i].orginalIndex = i;
		if (t64Item[i].offset > file_size.LowPart)
		{
			return SetError(E_FAIL, TEXT("Invalid file structure."));
		}

		t64Item[i].mySize=0;
		if (t64Item[i].offset == 0)
		{
			continue;
		}

		FixT64TrailingSpaces(t64Item[i]);
		hr = dirList.Append(&wrappedT64Item[i]);
		if (FAILED(hr))
		{
			return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
		}
	}

	dirList.MergeSort(CompareT64DirItemOffset);

	T64DirectoryItemListElement *element;
	element = dirList.Head();
	DWORD t=0;
	for (i=0 ; i < dirList.Count() && element!=0L ; i++, element=element->Next())
	{
		if (element->Next() != NULL)
		{
			element->m_data->item->mySize = element->Next()->m_data->item->offset - element->m_data->item->offset;
		}
		else
		{
			element->m_data->item->mySize = file_size.LowPart - element->m_data->item->offset;
		}
		if (element->m_data->item->mySize > 0x10000)
		{
			return SetError(E_FAIL, TEXT("Invalid file structure."));
		}
	}

	dirList.MergeSort(CompareT64DirItemOriginalIndex);
	for (i=0, element = dirList.Head(); i < dirList.Count() ; i++, element=element->Next())
	{

		if (element->m_data->item->mySize == 0)
		{
			continue;
		}

		if (element->m_data->item->offset == 0)
		{
			continue;
		}

		hr = filteredDirectory.Append(element->m_data);
		if (FAILED(hr))
		{
			return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
		}

		numItems++;
	}

	numberItems = numItems;
	return S_OK;
}

void T64::FixT64TrailingSpaces(T64DirectoryItem& t64Item)
{
	unsigned int i;
	unsigned int j;
	bool hasNonSpace = false;
	for (i = 0; i < _countof(t64Item.c64Filename); i++)
	{
		j = _countof(t64Item.c64Filename) - i - 1;
		bit8 ch = t64Item.c64Filename[j];
		if (ch != 0xA0 && ch != 0x20)
		{
			hasNonSpace = true;
		}

		if (!hasNonSpace)
		{
			t64Item.c64Filename[j] = 0xA0;
		}
	}

	if (t64Item.c64Filename[j] == 0xA0)
	{
		strncpy((char *) & t64Item.c64Filename[0], "?", _countof(t64Item.c64Filename));
	}
}

HRESULT T64::LoadT64File(const std::wstring filename, DWORD offset, DWORD size)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,byteCount;
LARGE_INTEGER file_size;

	ClearError();
	if (data)
	{
		free(data);
	}

	data = 0;
	if (size > C64MAXFILESIZE)
	{
		return SetError(E_FAIL, TEXT("Bad T64 file size."));
	}

	data = (BYTE *) malloc(size);
	if (data == 0)
	{
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));	
	}

	hfile=0;
	hfile=CreateFile(Wfs::EnsureLongNamePrefix(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."), filename.c_str());
	}

	if (!GetFileSizeEx(hfile, &file_size))
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not open %s."), filename.c_str());
	}

	r = SetFilePointer (hfile, offset, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not seek in file %s."), filename.c_str());
	}

	byteCount = size;
	r = ReadFile(hfile, data, byteCount, &bytes_read, NULL);
	if (r == 0 || byteCount != bytes_read)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not read from %s."), filename.c_str());
	}

	return S_OK;
}
