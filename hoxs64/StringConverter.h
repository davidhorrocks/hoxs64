#pragma once
#include <tchar.h>
#include <comdef.h>
#include <string>
#include "bits.h"

class StringConverter
{
public:
	static constexpr size_t FORMAT_STRING_INITIAL_SIZE = 512;
	static constexpr size_t FORMAT_STRING_MAX_SIZE = 0x3fffffffUL;
	static constexpr unsigned int MaxLength = 0x7FFFFFFF;
	static HRESULT MultiByteToUc(UINT codepage, LPCSTR pszMb, int cbMbCharsToConvert, LPWSTR pwszUc, int cchOutputBuffer) noexcept;
	static HRESULT MultiByteToUc(UINT codepage, LPCSTR pszMb, int cbMbCharsToConvert, LPWSTR pwszUc, int cchOutputBuffer, int& cchOut) noexcept;
	static HRESULT MultiByteToUcRequiredBufferLength(UINT codepage, LPCSTR pszMb, int cbMbCharsToConvert, int& cchOut) noexcept;
	static HRESULT UcToMultiByte(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, LPSTR pszMb, int cbOutputBuffer) noexcept;
	static HRESULT UcToMultiByte(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, LPSTR pszMb, int cbOutputBuffer, int& cchOut) noexcept;
	static HRESULT UcToMultiByteRequiredBufferLength(UINT codepage, LPCWSTR pwszUc, int cchWideCharsToConvert, int& cchOut) noexcept;
	static BSTR AllocBStr(LPCTSTR pszString) noexcept;
	static TCHAR* MallocFormattedString(LPCTSTR pszFormatString, ...) noexcept;
	static wchar_t* MallocFormattedStringW(const wchar_t *pszFormatString, ...) noexcept;
	static char* MallocFormattedStringA(const char *pszFormatString, ...) noexcept;
	static std::wstring StringToWideString(const std::string& str);
	static std::wstring StringToWideString(UINT codepage, const std::string& str);
	static std::string WideStringToString(const std::wstring str);
	static std::string WideStringToString(UINT codepage, const std::wstring str);
	static bool IsEmptyOrWhiteSpace(const std::wstring);
	static bool CompareStrCaseInsensitive(const std::string& str1, const std::string& str2);
	static bool CompareStrCaseInsensitive(const std::wstring& str1, const std::wstring& str2);
	static std::wstring format_string(const wchar_t* format, ...);
	static std::string format_string(const char* format, ...);
	static std::string WideStringToPetscii(const std::wstring str);
	static std::string AnsiStringToPetscii(const std::string str);

	static bit8 ConvertPetAsciiToScreenCode(bit8 petascii);
	static bit8 ConvertAnsiToPetAscii(bit8 petascii);
	static bit8 ConvertCommandLineAnsiToPetAscii(bit8 ch);
	static bit8 ConvertPetAsciiToAnsi(bit8 ch);
};
