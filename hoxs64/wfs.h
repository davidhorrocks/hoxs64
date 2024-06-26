#pragma once
#include <string>

class Wfs
{
public:
	static const std::wstring LongNamePrefixUnc;
	static const std::wstring LongNamePrefix;
	static const std::wstring ShortNamePrefixUnc;
	static const std::wstring RootPrefix;

	static bool IsAbsolutePath(const std::wstring& path);
	static std::wstring EnsureLongNamePrefix(const std::wstring& path);
	static std::wstring EnsureRemoveLongNamePrefix(const std::wstring& path);
	static void EnsureTrailingBackslash(std::wstring& s);
	static void RemoveTrailingBackslash(std::wstring& s);
	static std::wstring Path_Combine(const std::wstring& path1, const std::wstring& path2);
	static void Path_Append(std::wstring& pathToBeChanged, const std::wstring& pathToBeAppended);
	static bool FileExists(const std::wstring& path, bool* pisFound, DWORD* plastError);
	static bool FileExists(const std::wstring& path, bool* pisFound, std::wstring& errorMessage);
	static bool DirectoryExists(const std::wstring& path, bool* pisFound, DWORD* plastError);
	static bool DirectoryExists(const std::wstring& path, bool* pisFound, std::wstring& errorMessage);
	static std::wstring GetFileExtension(const std::wstring& filename);
	static std::wstring GetFileName(const std::wstring& filename);
	static std::wstring GetFileNameWithoutExtension(const std::wstring& filename);
	static bool FilenameHasExtension(const wchar_t filename[], const wchar_t ext[]);
	static void SplitRootPath(const std::wstring path, std::wstring& root, std::wstring& directorypath, std::wstring& filename);
	static void SplitRootPath(const std::wstring path, std::wstring& longnameprefix, std::wstring& root, std::wstring& directorypath, std::wstring& filename);
};