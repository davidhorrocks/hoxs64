#pragma once
#include <string>
#include "StringConverter.h"

class StringHelper
{
public:
	static std::wstring GetDirectoryFromPath(const std::wstring& filepath);
	static std::wstring GetFileExtension(const std::wstring& filename);
};