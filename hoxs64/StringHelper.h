#pragma once
#include <string>
#include "StringConverter.h"

class StringHelper
{
public:
	static std::string GetDirectoryFromPath(const std::string& filepath);
	static std::string GetFileExtension(const std::string& filename);
};