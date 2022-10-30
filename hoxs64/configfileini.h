#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <map>
#include "IniFileManager/IniFileManager.h"
#include "configdatasource.h"

class ConfigFileIni : public IConfigDataSource
{
public:
    ConfigFileIni() noexcept;
    ~ConfigFileIni() override;
    ConfigFileIni(const ConfigFileIni&) = delete;
    ConfigFileIni(ConfigFileIni&&) = delete;
    ConfigFileIni& operator=(const ConfigFileIni&) = delete;
    ConfigFileIni& operator=(ConfigFileIni&&) = delete;

    void Init();
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
protected:
    IniFileManager iniFile;
private:
    bool parseDWord(const std::wstring& s, DWORD& result);
    bool isHexPrefix(const std::wstring& s, bool ignoreWhitespace);
    bool isBlank(const std::wstring& s);
    std::wstring location;
    bool loadOnce;    
};