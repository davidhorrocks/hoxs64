#pragma once
class FileStream : public IStream
{
    FileStream(HANDLE hFile, bool bOwnFileHandle);

    ~FileStream();

public:
    HRESULT static CreateObject(LPCTSTR pName, IStream ** ppStream, bool fWrite);

    HRESULT static CreateObject(HANDLE hFile, bool bOwnFileHandle, IStream ** ppStream);

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	
	virtual ULONG STDMETHODCALLTYPE AddRef(void);

    virtual ULONG STDMETHODCALLTYPE Release(void);

    // ISequentialStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead);

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten);

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER);

    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD);

    virtual HRESULT STDMETHODCALLTYPE Revert(void);

    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **);

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag);

private:
    HANDLE _hFile;
    LONG _refcount;
	bool _bOwnFileHandle;
public:
	static BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
	static BOOL SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
};
