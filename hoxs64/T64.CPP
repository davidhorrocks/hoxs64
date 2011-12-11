#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "assert.h"
#include "defines.h"
#include "CDPI.h"
#include "resource.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "mlist.h"
#include "carray.h"
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

int T64::CompareT64DirItemOffset(WrappedT64DirectoryItem *&a, WrappedT64DirectoryItem *&b)
{
	//-ve means a is before b;
	//+ve means b is before a;
	return (int)(a->item->offset - b->item->offset);
}

int T64::CompareT64DirItemOriginalIndex(WrappedT64DirectoryItem *&a, WrappedT64DirectoryItem *&b)
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

HRESULT T64::LoadT64Directory(TCHAR filename[], int maxcount)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,file_size,byteCount;
WORD i;
int numItems;
HRESULT hr;

	ClearError();
	CleanUp();
	numItems=0;
	hfile=0;

	hfile=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."), &filename[0]);
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not open %s."), filename);
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
		if (t64Item[i].offset>file_size)
		{
			return SetError(E_FAIL, TEXT("Invalid file structure."));
		}
		t64Item[i].mySize=0;

		if (t64Item[i].offset == 0)
			continue;

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
			element->m_data->item->mySize = file_size - element->m_data->item->offset;
		}
		if (element->m_data->item->mySize > 0x10000)
		{
			return SetError(E_FAIL, TEXT("Invalid file structure."));
		}
	}

	dirList.MergeSort(CompareT64DirItemOriginalIndex);

	
	for (i=0, element = dirList.Head(); i < dirList.Count() ; i++, element=element->Next())
	{

		if (element->m_data->item->mySize==0)
			continue;

		if (element->m_data->item->offset == 0)
			continue;

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


HRESULT T64::LoadT64File(TCHAR filename[], DWORD offset, WORD size)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,file_size,byteCount;

	ClearError();
	if (data)
		free(data);
	data = 0;

	if (size > 0x10000)
	{
		return SetError(E_FAIL, TEXT("Bad T64 file size."));
	}
	data = (BYTE *) malloc(size);
	if (data == 0)
	{
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));	
	}

	hfile=0;
	hfile=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."), filename);
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not open %s."), filename);
	}

	r = SetFilePointer (hfile, offset, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not seek in file %s."), filename);
	}

	byteCount = size;
	r = ReadFile(hfile, data, byteCount, &bytes_read, NULL);
	if (r==0 || byteCount!=bytes_read)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL, TEXT("Could not read from %s."), filename);
	}
	return S_OK;
}
