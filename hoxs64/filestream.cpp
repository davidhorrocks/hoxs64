#include <windows.h>
#include <tchar.h>
#include "filestream.h"
#include "wfs.h"

FileStream::FileStream(HANDLE hFile, bool bOwnFileHandle) 
{
	_refcount = 1;
	_hFile = hFile;
	_bOwnFileHandle = bOwnFileHandle;
}

FileStream::~FileStream()
{
	if (_bOwnFileHandle)
	{
		if (_hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(_hFile);
		}
	}
}

HRESULT FileStream::CreateObject(LPCTSTR pName, IStream ** ppStream, bool fWrite)
{
	HANDLE hFile = ::CreateFile(Wfs::EnsureLongNamePrefix(pName).c_str(), fWrite ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ,
		NULL, fWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());
        
	return CreateObject(hFile, true, ppStream);
}

HRESULT FileStream::CreateObject(HANDLE hFile, bool bOwnFileHandle, IStream ** ppStream)
{
IStream * pStream;
	pStream = new FileStream(hFile, bOwnFileHandle);     
	if(ppStream != NULL)
	{
		*ppStream = pStream;
		return S_OK;
	}
	else
	{
		return E_OUTOFMEMORY;
	}
}

HRESULT FileStream::QueryInterface(REFIID iid, void ** ppvObject)
{ 
	if (iid == __uuidof(IUnknown)
		|| iid == __uuidof(IStream)
		|| iid == __uuidof(ISequentialStream))
	{
		IStream* pstrm = this;
		*ppvObject = pstrm;
		AddRef();
		return S_OK;
	} else
		return E_NOINTERFACE; 
}

ULONG FileStream::AddRef(void) 
{ 
	return (ULONG)InterlockedIncrement(&_refcount); 
}

ULONG FileStream::Release(void) 
{
	ULONG res = (ULONG) InterlockedDecrement(&_refcount);
	if (res == 0) 
		delete this;
	return res;
}

HRESULT FileStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	BOOL rc = ReadFile(_hFile, pv, cb, pcbRead, NULL);
	return (rc) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT FileStream::Write(void const* pv, ULONG cb, ULONG* pcbWritten)
{
	BOOL rc = WriteFile(_hFile, pv, cb, pcbWritten, NULL);
	return rc ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT FileStream::SetSize(ULARGE_INTEGER)
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) 
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::Commit(DWORD)                                      
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::Revert(void) 
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
{ 
	return E_NOTIMPL;   
}
    
HRESULT FileStream::Clone(IStream **)
{ 
	return E_NOTIMPL;   
}

HRESULT FileStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{ 
	DWORD dwMoveMethod;

	switch(dwOrigin)
	{
	case STREAM_SEEK_SET:
		dwMoveMethod = FILE_BEGIN;
		break;
	case STREAM_SEEK_CUR:
		dwMoveMethod = FILE_CURRENT;
		break;
	case STREAM_SEEK_END:
		dwMoveMethod = FILE_END;
		break;
	default:   
		return STG_E_INVALIDFUNCTION;
		break;
	}

	if (FileStream::SetFilePointerEx(_hFile, liDistanceToMove, (PLARGE_INTEGER) lpNewFilePointer, dwMoveMethod) == 0)
		return HRESULT_FROM_WIN32(GetLastError());
	return S_OK;
}

HRESULT FileStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
{
	if (FileStream::GetFileSizeEx(_hFile, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
		return HRESULT_FROM_WIN32(GetLastError());
	return S_OK;
}

BOOL FileStream::GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
DWORD loSize, hiSize;
	loSize = ::GetFileSize(hFile, &hiSize);
	if (INVALID_FILE_SIZE == loSize && GetLastError() != NOERROR)
	{
		return FALSE;
	}
	else
	{
		lpFileSize->LowPart = loSize;
		lpFileSize->HighPart = (LONG)hiSize;
		return TRUE;
	}
}

BOOL FileStream::SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
LONG hiValue;
	hiValue = liDistanceToMove.HighPart;
	DWORD loValue = ::SetFilePointer(hFile, liDistanceToMove.LowPart, &hiValue, dwMoveMethod);
	if (loValue == INVALID_SET_FILE_POINTER && GetLastError() != NOERROR)
	{
		return FALSE;
	}
	else
	{
		lpNewFilePointer->LowPart = loValue;
		lpNewFilePointer->HighPart = hiValue;
		return TRUE;
	}
}
