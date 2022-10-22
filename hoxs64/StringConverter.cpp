#include <algorithm>
#include "StringConverter.h"

HRESULT StringConverter::MultiByteToUc(UINT codepage, LPCSTR pszMb, int cchMbCharsToConvert, LPWSTR pwszUc, int cchOutputBuffer) noexcept
{
	int chout = 0;
	return StringConverter::MultiByteToUc(codepage, pszMb, cchMbCharsToConvert, pwszUc, cchOutputBuffer, chout);
}

HRESULT StringConverter::MultiByteToUc(UINT codepage, LPCSTR pszMb, int cchMbCharsToConvert, LPWSTR pwszUc, int cchOutputBuffer, int& cchOut) noexcept
{
	int cch = 0;
	cchOut = 0;
	if (pszMb == nullptr)
	{
		return E_POINTER;
	}

	cch = cchMbCharsToConvert;
	int cSize = MultiByteToWideChar(codepage, 0, pszMb, cch, nullptr, 0);
	if (cSize == 0)
	{
		return E_FAIL;
	}

	if (nullptr == pwszUc)
	{
		cchOut = cSize;
		return S_OK;
	}

	int r = MultiByteToWideChar(codepage, 0, pszMb, cch, pwszUc, cchOutputBuffer);
	if (0 == r)
	{
		return  E_FAIL;
	}

	cchOut = r;
	return S_OK;
}

HRESULT StringConverter::MultiByteToUcRequiredBufferLength(UINT codepage, LPCSTR pszMb, int cbMbCharsToConvert, int& cchOut) noexcept
{
	return MultiByteToUc(codepage, pszMb, cbMbCharsToConvert, nullptr, 0, cchOut);
}

HRESULT StringConverter::UcToMultiByte(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, LPSTR pszMb, int cbOutputBuffer) noexcept
{
	int cchOut = 0;
	return UcToMultiByte(codepage, pwszUc, cchWideCharsToConvert, pszMb, cbOutputBuffer, cchOut);
}

HRESULT StringConverter::UcToMultiByte(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, LPSTR pszMb, int cbOutputBuffer, int& cchOut) noexcept
{
	int cch = 0;

	cchOut = 0;
	if (pwszUc == nullptr)
	{
		return E_POINTER;
	}

	cch = cchWideCharsToConvert;
	int cSize = WideCharToMultiByte(codepage, 0, pwszUc, cch, nullptr, 0, nullptr, nullptr);
	if (cSize == 0)
	{
		return E_FAIL;
	}

	if (nullptr == pszMb)
	{
		cchOut = cSize;
		return S_OK;
	}

	int r = WideCharToMultiByte(codepage, 0, pwszUc, cch, pszMb, cbOutputBuffer, nullptr, nullptr);
	if (0 == r)
	{
		return  E_FAIL;
	}

	cchOut = r;
	return S_OK;
}

HRESULT StringConverter::UcToMultiByteRequiredBufferLength(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, int& cchOut)  noexcept
{
	return UcToMultiByte(codepage, pwszUc, cchWideCharsToConvert, nullptr, 0, cchOut);
}

BSTR StringConverter::AllocBStr(LPCTSTR pszString) noexcept
{
	BSTR bstr = nullptr;
#ifdef UNICODE
	bstr = SysAllocString(pszString);
#else
	int ich = 0;
	LPWSTR pwstr;
	int lenSourceStr = lstrlen(pszString);
	if (lenSourceStr > 0)
	{
		if (SUCCEEDED(MultiByteToUc(CP_ACP, pszString, -1, nullptr, 0, ich)))
		{
			pwstr = (LPWSTR)malloc((ich + 1) * sizeof(wchar_t));
			if (pwstr)
			{
				if (SUCCEEDED(MultiByteToUc(pszString, -1, pwstr, ich)))
				{
					pwstr[ich] = 0;
					bstr = SysAllocString(pwstr);
				}
			}

			free(pwstr);
		}
	}
	else
	{
		bstr = SysAllocString(L"");
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
	constexpr size_t maxsize = static_cast<size_t>(1024*1024)*5;
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
	constexpr size_t maxsize = static_cast<size_t>(1024 * 1024) * 5;
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
	constexpr size_t maxsize = static_cast<size_t>(1024 * 1024) * 5;
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

std::wstring StringConverter::StringToWideString(const std::string str)
{
	return StringToWideString(CP_ACP, str);
}

std::wstring StringConverter::StringToWideString(UINT codepage, const std::string str)
{	
	std::string errmsg;
	std::wstring s;
	if (str.length() == 0 || str[0] == '\0')
	{
		return std::wstring();
	}

	if (str.length() >= MaxLength)
	{
		throw new std::exception("StringToWideString failed. String too long.");
	}

	int cchRequired = 0;
	HRESULT hr = MultiByteToUcRequiredBufferLength(codepage, str.c_str(), (int)str.length(), cchRequired);
	if (FAILED(hr))
	{				
		//errmsg = GetLastWin32ErrorString();
		throw new std::exception("StringToWideString failed.");
	}

	LPWSTR pwstr = new WCHAR[cchRequired + 1];
	try
	{
		int cchCopied = cchRequired;
		hr = MultiByteToUc(codepage, str.c_str(), (int)str.length(), pwstr, cchCopied);
		if (SUCCEEDED(hr))
		{
			if (cchCopied > cchRequired)
			{
				cchCopied = cchRequired;
			}

			pwstr[cchCopied] = 0;
			s = std::wstring(pwstr);
		}
		else
		{
			throw new std::exception("StringToWideString failed.");
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

std::string StringConverter::WideStringToString(const std::wstring str)
{
	return WideStringToString(CP_ACP, str);
}

std::string StringConverter::WideStringToString(UINT codepage, const std::wstring str)
{
	std::string s;
	if (str.length() == 0 || str[0] == '\0')
	{
		return std::string();
	}

	if (str.length() >= MaxLength)
	{
		throw new std::exception("WideStringToString failed. String too long.");
	}

	int cchRequired = 0;
	HRESULT hr = UcToMultiByteRequiredBufferLength(codepage, str.c_str(), (int)str.length(), cchRequired);
	if (FAILED(hr))
	{

		//errmsg = GetLastWin32ErrorString();
		throw new std::exception("WideStringToString failed.");
	}

	LPSTR pstr = new CHAR[cchRequired + 1];
	try
	{
		int cchCopied = cchRequired;
		hr = UcToMultiByte(codepage, str.c_str(), (int)str.length(), pstr, cchRequired, cchCopied);
		if (SUCCEEDED(hr))
		{
			if (cchCopied > cchRequired)
			{
				cchCopied = cchRequired;
			}

			pstr[cchCopied] = 0;
			s = std::string(pstr);
		}
		else
		{
			throw new std::exception("WideStringToString failed.");
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

bool StringConverter::IsEmptyOrWhiteSpace(const std::wstring s)
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


bool StringConverter::CompareStrCaseInsensitive(const std::string& str1, const std::string& str2)
{
	return ((str1.size() == str2.size()) && std::equal(str1.cbegin(), str1.cend(), str2.cbegin(), 
		[](char c1, char c2) 
		{
			return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
		}
		));
}

bool StringConverter::CompareStrCaseInsensitive(const std::wstring& str1, const std::wstring& str2)
{
	return ((str1.size() == str2.size()) && std::equal(str1.cbegin(), str1.cend(), str2.cbegin(),
		[](wchar_t c1, wchar_t c2)
		{
			return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
		}
	));
}
