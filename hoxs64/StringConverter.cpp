#include <algorithm>
#include "StringConverter.h"

/**************************************************************************/

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: AnsiToUc

	Summary:  Convert an ANSI 'multibyte' string into a UNICODE 'wide
			character' string.

	Args:     LPSTR pszAnsi
				Pointer to a caller's input ANSI string.
			LPWSTR pwszUc
				Pointer to a caller's output UNICODE wide string.
			int cAnsiCharsToConvert
				Character count. If 0 then use length of pszAnsi.

	Returns:  HRESULT
				Standard result code. NOERROR for success.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
HRESULT StringConverter::AnsiToUc(
	LPCSTR pszAnsi,
	LPWSTR pwszUc,
	int cAnsiCharsToConvert)  noexcept
{
	int chout = 0;
	return StringConverter::AnsiToUc(pszAnsi, pwszUc, cAnsiCharsToConvert, chout);
}

HRESULT StringConverter::AnsiToUc(
	LPCSTR pszAnsi,
	LPWSTR pwszUc,
	int cAnsiCharsToConvert,
	int& cchOut) noexcept
{
	int cch = 0;

	cchOut = 0;
	if (pszAnsi == nullptr)
	{
		return E_POINTER;
	}

	if (0 == cAnsiCharsToConvert)
	{
		cch = -1;
	}
	else
	{
		cch = cAnsiCharsToConvert;
	}

	int cSize = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cch, nullptr, 0);
	if (cSize == 0)
	{
		return E_FAIL;
	}

	if (nullptr == pwszUc)
	{
		cchOut = cSize;
		return S_OK;
	}

	int r = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cch, pwszUc, cSize);
	if (0 == r)
	{
		return  E_FAIL;
	}

	cchOut = r;
	return S_OK;
}

HRESULT StringConverter::AnsiToUcRequiredBufferLength(
	LPCSTR pszAnsi,
	int cAnsiCharsToConvert,
	int& cchOut
) noexcept
{
	return AnsiToUc(pszAnsi, nullptr, cAnsiCharsToConvert, cchOut);
}

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: StringConverter::UcToAnsi

	Summary:  Convert a UNICODE 'wide character' input string to an output
			ANSI 'multi-byte' string.

	Args:     LPWSTR pwszUc
				Pointer to a caller's input UNICODE wide string.
			LPSTR pszAnsi
				Pointer to a caller's output ANSI string.
			int cWideCharsToConvert
				Character count. If 0 then use length of pszUc.

	Returns:  HRESULT
				Standard result code. NOERROR for success.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
HRESULT StringConverter::UcToAnsi(
	LPCWSTR pwszUc,
	LPSTR pszAnsi,
	int cWideCharsToConvert,
	int& cchOut) noexcept
{
	int cch = 0;

	cchOut = 0;
	if (pwszUc == nullptr)
	{
		return E_POINTER;
	}

	if (0 == cWideCharsToConvert)
	{
		cch = -1;
	}
	else
	{
		cch = cWideCharsToConvert;
	}

	int cSize = WideCharToMultiByte(CP_ACP, 0, pwszUc, cch, nullptr, 0, nullptr, nullptr);
	if (cSize == 0)
	{
		return E_FAIL;
	}

	if (nullptr == pszAnsi)
	{
		cchOut = cSize;
		return S_OK;
	}

	int r = WideCharToMultiByte(CP_ACP, 0, pwszUc, cch, pszAnsi, cSize, nullptr, nullptr);
	if (0 == r)
	{
		return  E_FAIL;
	}

	cchOut = r;
	return S_OK;
}

HRESULT StringConverter::UcToAnsi(
	LPCWSTR pwszUc,
	LPSTR pszAnsi,
	int cWideCharsToConvert) noexcept
{
	int cchOut = 0;
	return UcToAnsi(pwszUc, pszAnsi, cWideCharsToConvert, cchOut);
}

HRESULT StringConverter::UcToAnsiRequiredBufferLength(LPCWSTR pwszUc, int cWideCharsToConvert, int& cchOut)  noexcept
{
	return UcToAnsi(pwszUc, nullptr, cWideCharsToConvert, cchOut);
}

BSTR StringConverter::AllocBStr(LPCTSTR pszString) noexcept
{
	BSTR bstr = nullptr;
#ifdef UNICODE
	bstr = SysAllocString(pszString);
#else
	int ich = 0;
	LPWSTR pwstr;
	if (SUCCEEDED(AnsiToUc(pszString, NULL, 0, ich)))
	{
		pwstr = (LPWSTR)malloc(ich * sizeof(wchar_t));
		if (pwstr)
		{
			if (SUCCEEDED(AnsiToUc(pszString, pwstr, 0, ich)))
			{
				bstr = SysAllocString(pwstr);
			}
		}

		free(pwstr);
	}
#endif
	return bstr;
}

#pragma warning( push )
#pragma warning( disable : 26492)
#pragma warning( disable : 26477)
TCHAR* StringConverter::MallocFormattedString(LPCTSTR pszFormatString, ...) noexcept
{
	va_list vl = nullptr;
	va_start(vl, pszFormatString);
	constexpr size_t maxsize = 1024*1024*5;
	TCHAR* s = nullptr;
	for (size_t size = 1024; size < maxsize; size = size * 5)
	{
		if (s != nullptr)
		{
			free(s);
			s = nullptr;
		}

		s = static_cast<TCHAR*>(malloc(size * sizeof(TCHAR)));
		if (s != nullptr)
		{
			int written = _vsntprintf_s(s, size, _TRUNCATE, pszFormatString, vl);
			s[size - 1] = TEXT('\0');
			if ((size_t)written >= size - 1)
			{
				continue;
			}
		}

		break;
	}

	va_end(vl);
	return s;
}

wchar_t* StringConverter::MallocFormattedStringW(const wchar_t *pszFormatString, ...) noexcept
{
	va_list vl = nullptr;
	va_start(vl, pszFormatString);
	constexpr size_t maxsize = 1024 * 1024 * 5;
	wchar_t* s = nullptr;
	for (size_t size = 1024; size < maxsize; size = size * 5)
	{
		if (s != nullptr)
		{
			free(s);
			s = nullptr;
		}

		s = static_cast<wchar_t*>(malloc(size * sizeof(wchar_t)));
		if (s != nullptr)
		{
			int written = _vsnwprintf_s(s, size, _TRUNCATE, pszFormatString, vl);
			s[size - 1] = TEXT('\0');
			if ((size_t)written >= size - 1)
			{
				continue;
			}
		}

		break;
	}

	va_end(vl);
	return s;
}

char* StringConverter::MallocFormattedStringA(const char* pszFormatString, ...) noexcept
{
	va_list vl = nullptr;
	va_start(vl, pszFormatString);
	constexpr size_t maxsize = 1024 * 1024 * 5;
	char* s = nullptr;
	for (size_t size = 1024; size < maxsize; size = size * 5)
	{
		if (s != nullptr)
		{
			free(s);
			s = nullptr;
		}

		s = static_cast<char*>(malloc(size * sizeof(char)));
		if (s != nullptr)
		{
			int written = _vsnprintf_s(s, size, _TRUNCATE, pszFormatString, vl);
			s[size - 1] = TEXT('\0');
			if ((size_t)written >= size - 1)
			{
				continue;
			}
		}

		break;
	}

	va_end(vl);
	return s;
}

#pragma warning( pop ) 

std::wstring StringConverter::StringToWideString(std::string str)
{
	std::wstring s;
	if (str.length() == 0 || str[0] == '\0')
	{
		return std::wstring();
	}

	LPWSTR pwstr = new WCHAR[str.length() + 1];
	try
	{
		HRESULT hr = AnsiToUc(str.c_str(), pwstr, 0);
		if (SUCCEEDED(hr))
		{
			s = std::wstring(pwstr);
		}
		else
		{
			s = std::wstring(L"?");
		}
	}
	catch (...)
	{
		delete[] pwstr;
		pwstr = nullptr;
		throw;
	}

	delete[] pwstr;
	return s;
}

std::string StringConverter::WideStringToString(std::wstring str)
{
	std::string s;
	if (str.length() == 0 || str[0] == '\0')
	{
		return std::string();
	}

	LPSTR pstr = new CHAR[str.length() + 1];
	try
	{
		HRESULT hr = UcToAnsi(str.c_str(), pstr, 0);
		if (SUCCEEDED(hr))
		{
			s = std::string(pstr);
		}
		else
		{
			s = std::string("?");
		}
	}
	catch (...)
	{
		delete[] pstr;
		pstr = nullptr;
		throw;
	}

	delete[] pstr;
	return s;
}

bool StringConverter::IsEmptyOrWhiteSpace(std::wstring s)
{
	for (size_t i = 0; i < s.length(); i++)
	{
		if (!std::isspace(s[i]))
		{
			return false;
		}
	}

	return true;
}
