#pragma once
#include <windows.h>
#include <tchar.h>

class IConfigDataSource
{
public:
    IConfigDataSource() = default;
    IConfigDataSource(const IConfigDataSource&) = delete;
    IConfigDataSource(IConfigDataSource&&) = delete;
    IConfigDataSource& operator=(const IConfigDataSource&) = delete;
    IConfigDataSource& operator=(IConfigDataSource&&) = delete;
    virtual ~IConfigDataSource() = default;
    virtual void SetLocation(std::wstring location) = 0;
    virtual HRESULT ReadDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD& dwValue) = 0;
    virtual HRESULT ReadGUID(LPCTSTR sectionName, LPCTSTR valueName, GUID& guid) = 0;
    virtual HRESULT ReadString(LPCTSTR sectionName, LPCTSTR valueName, LPTSTR buffer, DWORD& cchBuffer) = 0;
    virtual HRESULT ReadByteList(LPCTSTR sectionName, LPCTSTR valueName, LPBYTE buffer, DWORD& cbBuffer) = 0;
    virtual HRESULT ReadDWordList(LPCTSTR sectionName, LPCTSTR valueName, LPDWORD buffer, DWORD& cdwBuffer) = 0;
    virtual HRESULT WriteDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD dwValue) = 0;
    virtual HRESULT WriteGUID(LPCTSTR sectionName, LPCTSTR valueName, const GUID& guid) = 0;
    virtual HRESULT WriteString(LPCTSTR sectionName, LPCTSTR valueName, LPCTSTR buffer, DWORD cchBuffer) = 0;
    virtual HRESULT WriteByteList(LPCTSTR sectionName, LPCTSTR valueName, const LPBYTE buffer, DWORD cbBuffer) = 0;
    virtual HRESULT WriteDWordList(LPCTSTR sectionName, LPCTSTR valueName, const LPDWORD buffer, DWORD cdwBuffer) = 0;
    virtual bool IsFile() = 0;
    virtual HRESULT ParseFile() = 0;
    virtual bool WriteToFile() = 0;
    virtual void Close() = 0;
    static constexpr HRESULT ErrorNotFound = MAKE_HRESULT(1, 0, ERROR_FILE_NOT_FOUND);
    static constexpr HRESULT ErrorMoreData = MAKE_HRESULT(1, 0, ERROR_MORE_DATA);
};