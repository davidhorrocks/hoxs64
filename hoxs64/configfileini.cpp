#include "configfileini.h"
#include "IniFileManager/IniFileManager.h"
#include "StringConverter.h"
#include "utils.h"

ConfigFileIni::ConfigFileIni() noexcept
{
    loadOnce = true;
}

ConfigFileIni::~ConfigFileIni()
{
    Close();
}

void ConfigFileIni::Init()
{
    
}

void ConfigFileIni::SetLocation(std::wstring location)
{
    this->location = location;
    loadOnce = true;
}

HRESULT ConfigFileIni::ParseFile()
{
    if (iniFile.parse(location))
    {
        return S_OK;
    }
    else
    {
        if (!iniFile.lastErrorMessage.empty())
        {
            throw std::exception(StringConverter::WideStringToString(CP_UTF8, iniFile.lastErrorMessage).c_str());
        }
        else
        {
            return E_FAIL;
        }
    }
}

bool ConfigFileIni::isBlank(const std::wstring& s)
{
    std::wstring::const_iterator p;
    for (p = s.cbegin(); p != s.cend(); p++)
    {
        if (std::isspace(*p))
        {
            continue;
        }

        if ((*p) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool ConfigFileIni::isHexPrefix(const std::wstring& s, bool ignoreWhitespace)
{    
    const static std::wstring wsHexPrefix = L"0x";
    ignoreWhitespace = true;
    size_t i = 0;
    std::wstring::const_iterator p;
    for (p = s.cbegin(); p != s.cend(); p++, i++)
    {
        if (ignoreWhitespace && std::isspace(*p))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    auto f = std::search(p, s.cend(), wsHexPrefix.cbegin(), wsHexPrefix.cend(), [](wint_t  a, wint_t  b) {return towupper(a) == towupper(b); });
    if (f != s.cend())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigFileIni::parseDWord(const std::wstring& s, DWORD& result)
{
    result = 0;
    if (s.length() == 0)
    {
        return false;
    }
    
    size_t idx;
    int base;
    if (isHexPrefix(s, true))
    {
        base = 16;
    }
    else
    {
        base = 10;
    }

    try
    {
        result = std::stoi(s, &idx, base);
    }
    catch(...)
    {
        return false;
    }

    return true;
}

HRESULT ConfigFileIni::ReadDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD& dwValue)
{
    dwValue = 0;
    KeyValueAssign* ka;
    if (iniFile.read(sectionName, valueName, &ka))
    {
        if (ka->values.size() > 0)
        {
            if (parseDWord(ka->values[0], dwValue))
            {
                return S_OK;
            }
        }
    }

    return IConfigDataSource::ErrorNotFound;
}

HRESULT ConfigFileIni::ReadGUID(LPCTSTR sectionName, LPCTSTR valueName, GUID& guid)
{
    KeyValueAssign* ka;
    if (iniFile.read(sectionName, valueName, &ka))
    {
        if (ka->values.size() > 0)
        {
            if (!isBlank(ka->values[0]))
            {
                if (CLSIDFromString(ka->values[0].c_str(), &guid) == NOERROR)
                {
                    return S_OK;
                }
                else
                {
                    return E_FAIL;
                }
            }
        }
    }

    return IConfigDataSource::ErrorNotFound;
}

/// <summary>
/// 
/// </summary>
/// <param name="sectionName"></param>
/// <param name="valueName"></param>
/// <param name="buffer"></param>
/// <param name="cchBuffer">if buffer is null then cchBuffer returns the required buffer size in characters including room for the zero terminator
/// if buffer is not null then cchBuffer specifies the buffer size in characters which must include room for the zero terminator
/// </param>
/// <returns></returns>
HRESULT ConfigFileIni::ReadString(LPCTSTR sectionName, LPCTSTR valueName, LPTSTR buffer, DWORD& cchBuffer)
{
    if (buffer == nullptr)
    {
        cchBuffer = 0;
    }

    KeyValueAssign* ka;
    if (iniFile.read(sectionName, valueName, &ka))
    {
        if (ka->values.size() > 0)
        {
            const std::wstring strvalue = ka->values[0];
            size_t k = 0;
            k = strvalue.length();
            if (k >= MAXUINT)
            {
                return E_FAIL;
            }

            if (k == 0 || strvalue[k - 1] != L'\0')
            {
                k++;
            }

            if (buffer == nullptr)
            {
                cchBuffer = (DWORD)k;
                return S_OK;
            }

            if (k > MAXUINT)
            {
                return E_FAIL;
            }

            if (k > cchBuffer)
            {
                return IConfigDataSource::ErrorMoreData;
            }

            errno_t er = wcsncpy_s(buffer, k, strvalue.c_str(), _TRUNCATE);
            if (er == 0 || er == STRUNCATE)
            {
                cchBuffer = (DWORD)k;
                return S_OK;
            }
            else
            {
                return E_FAIL;
            }
        }
    }

    return IConfigDataSource::ErrorNotFound;
}

HRESULT ConfigFileIni::ReadByteList(LPCTSTR sectionName, LPCTSTR valueName, LPBYTE buffer, DWORD& cbBuffer)
{
    std::vector<BYTE> data;
    if (buffer == nullptr)
    {
        cbBuffer = 0;
    }

    KeyValueAssign* ka;
    if (iniFile.read(sectionName, valueName, &ka))
    {
        if (buffer == nullptr)
        {
            if (ka->values.size() > MAXUINT)
            {
                cbBuffer = MAXUINT;
            }
            else
            {
                cbBuffer = (DWORD)(ka->values.size() & MAXUINT);
            }

            return S_OK;
        }

        if (ka->values.size() > 0)
        {
            size_t k = 0;
            DWORD v;
            std::vector<std::wstring>::const_iterator p;
            for (p = ka->values.cbegin(); data.size() < MAXUINT && p != ka->values.cend(); p++)
            {
                if (isBlank(*p))
                {
                    data.push_back(0);
                }
                else if (parseDWord(*p, v))
                {
                    data.push_back(v & 0xFF);
                }
                else
                {
                    data.push_back(0);
                }
            }

            k = data.size();
            if (buffer == nullptr)
            {
                cbBuffer = (DWORD)k;
                return S_OK;
            }

            if (k > MAXUINT || k > cbBuffer)
            {
                return IConfigDataSource::ErrorMoreData;
            }

            size_t i = 0;
            std::vector<BYTE>::const_iterator q;
            for (q = data.cbegin(); i < cbBuffer && q != data.cend(); q++, i++)
            {
                buffer[i] = *q;
            }

            cbBuffer = (DWORD)i;
            return S_OK;
        }
        else
        {
            cbBuffer = 0;
            return S_OK;
        }
    }

    return IConfigDataSource::ErrorNotFound;
}

HRESULT ConfigFileIni::ReadDWordList(LPCTSTR sectionName, LPCTSTR valueName, LPDWORD buffer, DWORD& cdwBuffer)
{
    std::vector<DWORD> data;
    if (buffer == nullptr)
    {
        cdwBuffer = 0;
    }

    KeyValueAssign* ka;
    if (iniFile.read(sectionName, valueName, &ka))
    {
        if (buffer == nullptr)
        {
            if (ka->values.size() > MAXUINT)
            {
                cdwBuffer = MAXUINT;
            }
            else
            {
                cdwBuffer = (DWORD)(ka->values.size() & MAXUINT);
            }

            return S_OK;
        }

        if (ka->values.size() > 0)
        {
            size_t k = 0;
            if (buffer == nullptr)
            {
                cdwBuffer = 0;
            }

            DWORD v;
            std::vector<std::wstring>::const_iterator p;
            for (p = ka->values.cbegin(); data.size() < MAXUINT && p != ka->values.cend(); p++)
            {
                if (isBlank(*p))
                {
                    data.push_back(0);
                }
                else if (parseDWord(*p, v))
                {
                    data.push_back(v);
                }
                else
                {
                    data.push_back(0);
                }
            }

            k = data.size();
            if (buffer == nullptr)
            {
                cdwBuffer = (DWORD)k;
                return S_OK;
            }

            if (k > MAXUINT || k > cdwBuffer)
            {
                return IConfigDataSource::ErrorMoreData;
            }

            size_t i = 0;
            std::vector<DWORD>::const_iterator q;
            for (q = data.cbegin(); i < cdwBuffer && q != data.cend(); q++, i++)
            {
                buffer[i] = *q;
            }

            cdwBuffer = (DWORD)i;
            return S_OK;
        }
        else
        {
            cdwBuffer = 0;
            return S_OK;
        }
    }

    return IConfigDataSource::ErrorNotFound;
}

HRESULT ConfigFileIni::WriteDWord(LPCTSTR sectionName, LPCTSTR valueName, DWORD dwValue)
{
    std::wstring s = G::format_string(L"0x%08lX", (unsigned long)dwValue);
    this->iniFile.ensureKeyValue(sectionName, valueName, s);
    return S_OK;
}

HRESULT ConfigFileIni::WriteGUID(LPCTSTR sectionName, LPCTSTR valueName, const GUID& guid)
{
    wchar_t t[50];
    int n = StringFromGUID2(guid, t, _countof(t) - 1);
    if (n <= 0 || n >= _countof(t))
    {
        return E_FAIL;
    }

    t[n] = 0;
    std::wstring s = t;
    this->iniFile.ensureKeyValue(sectionName, valueName, s);
    return S_OK;
}


/// <summary>
/// 
/// </summary>
/// <param name="sectionName"></param>
/// <param name="valueName"></param>
/// <param name="buffer"></param>
/// <param name="cchBuffer">cchBuffer should include zero terminater</param>
/// <returns></returns>
HRESULT ConfigFileIni::WriteString(LPCTSTR sectionName, LPCTSTR valueName, LPCTSTR buffer, DWORD cchBuffer)
{
    std::wstring s;
    if (cchBuffer > 0)
    {
        if (buffer[cchBuffer - 1] != '\0')
        {
            s.append(buffer, cchBuffer);
        }
        else
        {
            if (cchBuffer - 1 > 0)
            {
                s.append(buffer, cchBuffer - 1);
            }
        }
    }

    this->iniFile.ensureKeyValue(sectionName, valueName, s);
    return S_OK;
}

HRESULT ConfigFileIni::WriteByteList(LPCTSTR sectionName, LPCTSTR valueName, const LPBYTE buffer, DWORD cbBuffer)
{
    size_t i;
    std::vector<std::wstring> values;
    for (i = 0; i < cbBuffer; i++)
    {
        values.push_back(StringConverter::format_string(L"0x%02hhX", (unsigned char)buffer[i]));
    }

    this->iniFile.ensureKeyValues(sectionName, valueName, values);
    return S_OK;
}

HRESULT ConfigFileIni::WriteDWordList(LPCTSTR sectionName, LPCTSTR valueName, const LPDWORD buffer, DWORD cdwBuffer)
{
    size_t i;
    std::vector<std::wstring> values;
    for (i = 0; i < cdwBuffer; i++)
    {
        values.push_back(StringConverter::format_string(L"0x%08lX", (unsigned long)buffer[i]));
    }

    this->iniFile.ensureKeyValues(sectionName, valueName, values);
    return S_OK;
}

bool ConfigFileIni::IsFile()
{
    return true;
}

bool ConfigFileIni::WriteToFile()
{
    if (!location.empty())
    {
        iniFile.sortSections();
        iniFile.saveFile(location);
    }

    return true;
}

void ConfigFileIni::Close() noexcept
{
}
