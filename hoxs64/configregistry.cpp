#include "configregistry.h"
#include "utils.h"

ConfigRegistry::ConfigRegistry() noexcept
{
}

ConfigRegistry::~ConfigRegistry()
{
    Close();
}

void ConfigRegistry::SetLocation(std::wstring location)
{

}

HRESULT ConfigRegistry::ReadDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD& dwValue)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, false, regkey))
    {
        LSTATUS ls = RegReadDWordOrStr(regkey, valueName, &dwValue);
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }

    }

    return E_FAIL;
}

HRESULT ConfigRegistry::ReadGUID(LPCTSTR sectionName, LPCTSTR valueName, GUID& guid)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, false, regkey))
    {
        return G::GetClsidFromRegValue(regkey, valueName, &guid);
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::ReadString(LPCTSTR sectionName, LPCTSTR valueName, LPTSTR buffer, DWORD& cchBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, false, regkey))
    {
        DWORD cbBuffer = cchBuffer * sizeof(TCHAR);
        LSTATUS ls = RegGetValue(regkey, nullptr, valueName, RRF_RT_REG_SZ | RRF_ZEROONFAILURE, nullptr, (LPBYTE)buffer, &cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            cchBuffer = (cbBuffer + sizeof(TCHAR) - 1) / sizeof(TCHAR);
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::ReadByteList(LPCTSTR sectionName, LPCTSTR valueName, LPBYTE buffer, DWORD& cbBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, false, regkey))
    {
        LSTATUS ls = RegQueryValueEx(regkey, valueName, nullptr, nullptr, buffer, &cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::ReadDWordList(LPCTSTR sectionName, LPCTSTR valueName, LPDWORD buffer, DWORD& cdwBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, false, regkey))
    {
        DWORD cbBuffer = cdwBuffer * sizeof(DWORD);
        LSTATUS ls = RegQueryValueEx(regkey, valueName, nullptr, nullptr, (LPBYTE)buffer, &cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            cdwBuffer = (cbBuffer + sizeof(DWORD) - 1) / sizeof(DWORD);
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::WriteDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD dwValue)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, true, regkey))
    {
        LSTATUS ls = RegSetValueEx(regkey, valueName, 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::WriteGUID(LPCTSTR sectionName, LPCTSTR valueName, const GUID& guid)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, true, regkey))
    {
        if (SUCCEEDED(G::SaveClsidToRegValue(regkey, valueName, &guid)))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::WriteString(LPCTSTR sectionName, LPCTSTR valueName, LPCTSTR buffer, DWORD cbBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, true, regkey))
    {
        LSTATUS ls = RegSetValueEx(regkey, valueName, 0, REG_SZ, (LPBYTE)buffer, cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::WriteByteList(LPCTSTR sectionName, LPCTSTR valueName, const LPBYTE buffer, DWORD cbBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, true, regkey))
    {
        LSTATUS ls = RegSetValueEx(regkey, valueName, 0, REG_BINARY, buffer, cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

HRESULT ConfigRegistry::WriteDWordList(LPCTSTR sectionName, LPCTSTR valueName, const LPDWORD buffer, DWORD cdwBuffer)
{
    HKEY regkey;
    if (GetSectionKey(sectionName, true, regkey))
    {
        DWORD cbBuffer = cdwBuffer * sizeof(DWORD);
        LSTATUS ls = RegSetValueEx(regkey, valueName, 0, REG_BINARY, (LPBYTE)buffer, cbBuffer);
        if (ls == ERROR_SUCCESS)
        {
            return S_OK;
        }
        else
        {
            return MAKE_HRESULT(1, 0, ls);
        }
    }

    return E_FAIL;
}

bool ConfigRegistry::IsFile()
{
    return false;
}

bool ConfigRegistry::WriteToFile()
{
    return true;
}

HRESULT ConfigRegistry::ParseFile()
{
    return S_OK;
}

void ConfigRegistry::Close() noexcept
{
    for (const auto& k : mapReadWriteRegKey) 
    {
        ::RegCloseKey(k.second);
    }

    for (const auto& k : mapReadOnlyRegKey)
    {
        ::RegCloseKey(k.second);
    }

    mapReadWriteRegKey.clear();
    mapReadOnlyRegKey.clear();
}

bool ConfigRegistry::GetSectionKey(LPCTSTR sectionName, bool writeAccess, HKEY& result)
{
    bool isSuccess = false;
    result = nullptr;
    HKEY hkeySection = nullptr;
    LSTATUS keyOpen = E_FAIL;
    try
    {
        RegIt itr;
        if (writeAccess)
        {
            itr = this->mapReadWriteRegKey.find(sectionName);
            if (itr != mapReadWriteRegKey.end())
            {
                result = itr->second;
                return true;
            }
        }
        else
        {
            itr = this->mapReadOnlyRegKey.find(sectionName);
            if (itr != mapReadOnlyRegKey.end())
            {
                result = itr->second;
                return true;
            }
        }

        if (strRootSettings.empty())
        {
            strRootSettings.assign(TEXT("SOFTWARE\\Hoxs64\\1.0"));
        }

        strNextSettings.assign(strRootSettings);
        strNextSettings.append(TEXT("\\"));
        strNextSettings.append(sectionName);

        if (writeAccess)
        {
            DWORD dwDisposition = 0;
            keyOpen = RegCreateKeyEx(HKEY_CURRENT_USER,
                strNextSettings.c_str(),
                0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                nullptr, &hkeySection, &dwDisposition);
        }
        else
        {
            keyOpen = RegOpenKeyEx(HKEY_CURRENT_USER,
                strNextSettings.c_str(),
                0, KEY_READ,
                &hkeySection);

        }

        if (keyOpen == ERROR_SUCCESS)
        {
            std::pair<std::basic_string<TCHAR>, HKEY> p(std::basic_string<TCHAR>(sectionName), hkeySection);
            if (writeAccess)
            {
                this->mapReadWriteRegKey.insert(p);
            }
            else
            {
                this->mapReadOnlyRegKey.insert(p);
            }

            result = hkeySection;
            hkeySection = nullptr;
            return true;
        }
        else
        {
            hkeySection = nullptr;
            return false;
        }
    }
    catch (std::exception&)
    {
        if (keyOpen == ERROR_SUCCESS && hkeySection != nullptr)
        {
            ::RegCloseKey(hkeySection);
        }

        throw;
    }

    return false;
}

LSTATUS ConfigRegistry::RegReadDWordOrStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD dwValue) noexcept
{
    LSTATUS lRetCode;
    DWORD type;
    DWORD byteLength;
    DWORD dw = 0;
    TCHAR szValue[20];
    LPDWORD lpReserved = 0;
    lRetCode = RegQueryValueEx(hKey, lpValueName, lpReserved, &type, NULL, NULL);
    if (lRetCode == ERROR_SUCCESS)
    {
        if (type == REG_DWORD)
        {
            byteLength = sizeof(dw);
            lRetCode = RegQueryValueEx(hKey, lpValueName, lpReserved, NULL, (LPBYTE)&dw, &byteLength);
            if (lRetCode == ERROR_SUCCESS)
            {
                if (dwValue != NULL)
                {
                    *dwValue = dw;
                }
            }
        }
        else if (type == REG_SZ)
        {
            DWORD charLength = _countof(szValue);
            byteLength = charLength * sizeof(TCHAR);
            lRetCode = RegReadTStr(hKey, lpValueName, lpReserved, NULL, &szValue[0], &charLength);
            if (lRetCode == ERROR_SUCCESS)
            {
                errno = 0;
                dw = _tcstoul(szValue, NULL, 10);
                if (errno == 0)
                {
                    if (dwValue != NULL)
                    {
                        *dwValue = dw;
                    }
                }
                else
                {
                    lRetCode = ERROR_FILE_NOT_FOUND;
                }
            }
        }
        else
        {
            lRetCode = ERROR_FILE_NOT_FOUND;
        }
    }

    return lRetCode;
}

LSTATUS ConfigRegistry::RegReadTStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, TCHAR* lpszData, LPDWORD lpcchData) noexcept
{
    DWORD charLength = 0;
    DWORD byteLength = 0;
    DWORD cbData = 0;
    DWORD* lpcbData = nullptr;
    if (lpcchData != NULL)
    {
        charLength = *lpcchData;
        byteLength = charLength * sizeof(TCHAR);
        cbData = byteLength;
        lpcbData = &cbData;
    }

    LSTATUS r = RegGetValue(hKey, nullptr, lpValueName, RRF_RT_REG_SZ, lpType, (LPBYTE)lpszData, lpcbData);
    if (r == ERROR_SUCCESS)
    {
        DWORD bytesCopied = cbData;
        if (lpcchData == nullptr)
        {
            return r;
        }
        else
        {
            // Assume one more char for the null terminater plus, for TCHAR 2 byte case, an odd number of bytes being rounding up to an even number of bytes.
            *lpcchData = (bytesCopied + sizeof(TCHAR) + (sizeof(TCHAR) - 1)) / sizeof(TCHAR);
        }

        if (lpszData == nullptr)
        {
            return r;
        }

        DWORD fixup = 0;
        if (sizeof(TCHAR) > 1)
        {
            //fixup if a TCHAR is cut in half.
            fixup = bytesCopied % sizeof(TCHAR);
            if (fixup != 0)
            {
                // This line would be needed if sizeof(TCHAR) were to be more than 2 bytes.
                fixup = sizeof(TCHAR) - fixup;
            }
        }

        if (fixup == 0 && bytesCopied >= sizeof(TCHAR))
        {
            if (((TCHAR*)lpszData)[(bytesCopied / sizeof(TCHAR)) - 1] == TEXT('\0'))
            {
                //If the string is already NULL terminated then return.
                *lpcchData = bytesCopied / sizeof(TCHAR);
                return r;
            }
        }

        if ((size_t)bytesCopied + fixup + sizeof(TCHAR) > byteLength)
        {
            //No room for the NULL terminator.
            return ERROR_MORE_DATA;
        }

        //Add the TCHAR null terminator plus any fix up zeros if the last TCHAR was cut in half.
        for (size_t i = bytesCopied; i < (size_t)bytesCopied + fixup + sizeof(TCHAR); i++)
        {
            lpszData[i] = 0;
        }

        *lpcchData = (bytesCopied + sizeof(TCHAR) + (sizeof(TCHAR) - 1)) / sizeof(TCHAR);
    }

    return r;
}

bool ConfigRegistry::LessBasicString::operator()(const std::basic_string<TCHAR>& x, const std::basic_string<TCHAR>& y) const noexcept
{
    return x.compare(y) < 0;
}

