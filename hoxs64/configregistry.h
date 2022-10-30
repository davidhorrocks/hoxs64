#pragma once
#include <windows.h>
#include <string>
#include <map>
#include "configdatasource.h"

class ConfigRegistry : public IConfigDataSource
{
public:
    ConfigRegistry() noexcept;
    ~ConfigRegistry() override;
    ConfigRegistry(const ConfigRegistry&) = delete;
    ConfigRegistry(ConfigRegistry&&) = delete;
    ConfigRegistry& operator=(const ConfigRegistry&) = delete;
    ConfigRegistry& operator=(ConfigRegistry&&) = delete;

    void SetLocation(std::wstring location) override;
    HRESULT ReadDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD& dwValue) override;
    HRESULT ReadGUID(LPCTSTR sectionName, LPCTSTR valueName, GUID& guid) override;
    HRESULT ReadString(LPCTSTR sectionName, LPCTSTR valueName, LPTSTR buffer, DWORD& cchBuffer) override;
    HRESULT ReadByteList(LPCTSTR sectionName, LPCTSTR valueName, LPBYTE buffer, DWORD& cbBuffer) override;
    HRESULT ReadDWordList(LPCTSTR sectionName, LPCTSTR valueName, LPDWORD buffer, DWORD& cdwBuffer) override;
    HRESULT WriteDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD dwValue) override;
    HRESULT WriteGUID(LPCTSTR sectionName, LPCTSTR valueName, const GUID& guid) override;
    HRESULT WriteString(LPCTSTR sectionName, LPCTSTR valueName, LPCTSTR buffer, DWORD cchBuffer) override;
    HRESULT WriteByteList(LPCTSTR sectionName, LPCTSTR valueName, const LPBYTE buffer, DWORD cbBuffer) override;
    HRESULT WriteDWordList(LPCTSTR sectionName, LPCTSTR valueName, const LPDWORD buffer, DWORD cdwBuffer) override;
    bool IsFile() override;
    HRESULT ParseFile() override;
    bool WriteToFile() override;    
    void Close() noexcept override;

    static LSTATUS RegReadDWordOrStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD dwValue) noexcept;
    static LSTATUS RegReadTStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, TCHAR* lpszData, LPDWORD lpcchData) noexcept;
private:
    bool GetSectionKey(LPCTSTR section, bool writeAccess, HKEY& result);
    struct LessBasicString
    {
        bool operator()(const std::basic_string<TCHAR>& x, const std::basic_string<TCHAR>& y) const noexcept;
    };

    std::basic_string<TCHAR> strRootSettings;
    std::basic_string<TCHAR> strNextSettings;
    std::map<std::basic_string<TCHAR>, HKEY, LessBasicString> mapReadOnlyRegKey;
    std::map<std::basic_string<TCHAR>, HKEY, LessBasicString> mapReadWriteRegKey;
    typedef std::map<std::basic_string<TCHAR>, HKEY, LessBasicString>::iterator RegIt;
};