#include <windows.h>
#include <tchar.h>
#include "memorystream.h"

MemoryStream::MemoryStream(BYTE *pBuffer, ULONGLONG bufferSize, bool bOwnBuffer) 
{
	_refcount = 1;
	_pBuffer = pBuffer;
	_bOwnBuffer = bOwnBuffer;
	_position = 0;
	_bufferSize = bufferSize;
}

MemoryStream::~MemoryStream()
{
	FreeBuffer();
}

HRESULT MemoryStream::CreateObject(ULONG bufferSize, IStream ** ppStream)
{
	PBYTE buf = NULL;
	if (bufferSize)
	{
		buf = (PBYTE)malloc(bufferSize);
		if (buf==NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	HRESULT hr = CreateObject(buf, bufferSize, true, ppStream);
	if (FAILED(hr))
	{
		free(buf);
	}
	return hr;
}

HRESULT MemoryStream::CreateObject(PBYTE pBuffer, ULONGLONG bufferSize, bool bOwnBuffer, IStream ** ppStream)
{
IStream * pStream;
	pStream = new MemoryStream(pBuffer, bufferSize, bOwnBuffer);     
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

HRESULT MemoryStream::QueryInterface(REFIID iid, void ** ppvObject)
{ 
	if (iid == __uuidof(IUnknown)
		|| iid == __uuidof(IStream)
		|| iid == __uuidof(ISequentialStream))
	{
		*ppvObject = static_cast<IStream*>(this);
		AddRef();
		return S_OK;
	} else
		return E_NOINTERFACE; 
}

ULONG MemoryStream::AddRef(void) 
{ 
	return (ULONG)InterlockedIncrement(&_refcount); 
}

ULONG MemoryStream::Release(void) 
{
	ULONG res = (ULONG) InterlockedDecrement(&_refcount);
	if (res == 0)
	{
		delete this;
	}
	return res;
}

HRESULT MemoryStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	HRESULT hr;
	ULONG count;
	if (_position >= _bufferSize)
	{
		count = 0;
		if (pcbRead)
		{
			*pcbRead = 0;
		}
		hr = S_FALSE;
	}
	else
	{
		if (cb > _bufferSize - _position)
		{
			count = (ULONG)(_bufferSize - _position);
			hr = S_FALSE;
		}
		else
		{
			count = cb;
			hr = S_OK;
		}
		for(ULONG i = 0; i < count; i++, _position++)
		{
			((PBYTE)pv)[i] = _pBuffer[_position];
		}
	}
	if (pcbRead)
	{
		*pcbRead = count;
	}
	return hr;
}

HRESULT MemoryStream::Write(void const* pv, ULONG cb, ULONG* pcbWritten)
{
	HRESULT hr;
	ULONG count;
	if (_position >= _bufferSize)
	{
		count = 0;
		hr = STG_E_MEDIUMFULL;
	}
	else
	{
		if (cb > _bufferSize - _position)
		{
			count = (ULONG)(_bufferSize - _position);
			hr = S_FALSE;
		}
		else
		{
			count = cb;
			hr = S_OK;
		}
		for(ULONG i = 0; i < count; i++, _position++)
		{
			_pBuffer[_position] = ((PBYTE)pv)[i];
		}
	}
	if (pcbWritten)
	{
		*pcbWritten = count;
	}
	return S_OK;
}

HRESULT MemoryStream::SetSize(ULARGE_INTEGER)
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) 
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::Commit(DWORD)                                      
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::Revert(void) 
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
{ 
	return E_NOTIMPL;   
}
    
HRESULT MemoryStream::Clone(IStream **)
{ 
	return E_NOTIMPL;   
}

HRESULT MemoryStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{ 
	HRESULT hr = E_FAIL;
	ULARGE_INTEGER p;
	switch(dwOrigin)
	{
	case STREAM_SEEK_SET:
		p.QuadPart = 0;		
		break;
	case STREAM_SEEK_CUR:		
		p.QuadPart = _position;
		break;
	case STREAM_SEEK_END:
		p.QuadPart = _bufferSize;
		break;
	default:   
		return STG_E_INVALIDFUNCTION;
	}
	ULARGE_INTEGER next;
	next.QuadPart= p.QuadPart + liDistanceToMove.QuadPart;
	if ((next.QuadPart <= this->_bufferSize) && (next.QuadPart >= 0))
	{
		_position = next.QuadPart;
		hr = S_OK;
	}
	if (lpNewFilePointer)
	{
		lpNewFilePointer->QuadPart = _position;
	}
	return hr;
}

HRESULT MemoryStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
{
	pStatstg->cbSize.QuadPart = _bufferSize;
	return S_OK;
}

void MemoryStream::FreeBuffer()
{
	if (_bOwnBuffer)
	{
		if (_pBuffer != NULL)
		{
			free(_pBuffer);
			_pBuffer = NULL;
		}
		_bOwnBuffer = false;
	}
	_position = 0;
	_bufferSize = 0;
}

HRESULT MemoryStream::GetBuffer(PBYTE *ppBuffer)
{
	*ppBuffer = this->_pBuffer;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryStream::SetBuffer(PBYTE pBuffer, ULARGE_INTEGER bufferSize, bool bOwnBuffer)
{
	FreeBuffer();
	_pBuffer = pBuffer;
	_bOwnBuffer = bOwnBuffer;
	_position = 0;
	_bufferSize = bufferSize.QuadPart;
	return S_OK;
}