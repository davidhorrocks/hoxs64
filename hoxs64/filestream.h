#pragma once
class FileStream : public IStream
{
private:
    FileStream(HANDLE hFile, bool bOwnFileHandle);
    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;
    FileStream(FileStream&&) = delete;
    FileStream& operator=(FileStream&&) = delete;
public:
    virtual ~FileStream();

public:
    HRESULT static CreateObject(LPCTSTR pName, IStream ** ppStream, bool fWrite);

    HRESULT static CreateObject(HANDLE hFile, bool bOwnFileHandle, IStream ** ppStream);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override;
	
	ULONG STDMETHODCALLTYPE AddRef(void) override;

    ULONG STDMETHODCALLTYPE Release(void) override;

    // ISequentialStream Interface
public:
    HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) override;

    HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten) override;

    // IStream Interface
public:
    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override;

    HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override;
    
    HRESULT STDMETHODCALLTYPE Commit(DWORD) override;

    HRESULT STDMETHODCALLTYPE Revert(void) override;

    HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    
    HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    
    HRESULT STDMETHODCALLTYPE Clone(IStream **) override;

    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer) override;

    HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag) override;

private:
    HANDLE _hFile;
    LONG _refcount;
	bool _bOwnFileHandle;
public:
	static BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
	static BOOL SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
};
