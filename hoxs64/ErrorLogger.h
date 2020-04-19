#pragma once
#include <tchar.h>
#include <comdef.h>
#include <stdio.h>
#include <string>
#include "COMException.h"
#include "StringConverter.h"

class ErrorLogger
{
public:
	static bool HideMessageBox;

	static bool HideWindow;

	static void Log(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) noexcept;

	static void Log(std::string message) noexcept;

	static void Log(std::wstring message) noexcept;

	static void Log(HRESULT hr, std::string message) noexcept;

	static void Log(HRESULT hr, std::wstring message) noexcept;

	static void Log(COMException& exception) noexcept;
};