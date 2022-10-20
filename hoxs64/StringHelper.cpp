#include "StringHelper.h"
#include <algorithm>
#include "wfs.h"

std::wstring StringHelper::GetDirectoryFromPath(const std::wstring& filepath)
{
	std::wstring root;
	std::wstring dir;
	std::wstring file;
	Wfs::SplitRootPath(filepath, root, dir, file);
	return Wfs::Path_Combine(root, dir);
}

std::wstring StringHelper::GetFileExtension(const std::wstring& filename)
{
	size_t off = filename.find_last_of('.');
	if (off == std::string::npos)
	{
		return {};
	}
	return std::wstring(filename.substr(off + 1));
}
