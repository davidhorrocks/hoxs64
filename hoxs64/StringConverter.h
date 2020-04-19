#pragma once
#include <tchar.h>
#include <comdef.h>
#include <string>

class StringConverter
{
public:
	static HRESULT AnsiToUc(LPCSTR pszAnsi, LPWSTR pwszUc, int cAnsiCharsToConvert) noexcept;
	static HRESULT AnsiToUc(LPCSTR pszAnsi, LPWSTR pwszUc, int cAnsiCharsToConvert, int& cchOut) noexcept;
	static HRESULT AnsiToUcRequiredBufferLength(LPCSTR pszAnsi, int cAnsiCharsToConvert, int& cchOut) noexcept;
	static HRESULT StringConverter::UcToAnsi(LPCWSTR pwszUc, LPSTR pszAnsi, int cWideCharsToConvert) noexcept;
	static HRESULT StringConverter::UcToAnsi(LPCWSTR pwszUc, LPSTR pszAnsi, int cWideCharsToConvert, int& cchOut) noexcept;
	static HRESULT StringConverter::UcToAnsiRequiredBufferLength(LPCWSTR pwszUc, int cWideCharsToConvert, int& cchOut) noexcept;
	static BSTR AllocBStr(LPCTSTR pszString) noexcept;
	static TCHAR* MallocFormattedString(LPCTSTR pszFormatString, ...) noexcept;
	static wchar_t* MallocFormattedStringW(const wchar_t *pszFormatString, ...) noexcept;
	static char* MallocFormattedStringA(const char *pszFormatString, ...) noexcept;
	static std::wstring StringToWideString(std::string str);
	static std::string WideStringToString(std::wstring str);
	static bool IsEmptyOrWhiteSpace(std::wstring);
};
