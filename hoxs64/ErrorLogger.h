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

	static void Log(HWND hWnd, PCWSTR lpText, PCWSTR lpCaption, UINT uType);

	static void Log(const std::string message);

	static void Log(const std::wstring message);

	static void Log(HRESULT hr, const std::string message);

	static void Log(HRESULT hr, const std::wstring message);

	static void Log(const COMException& exception);

	static void LogInfo(LPCSTR message);

private:
	static void LogToStdOut(PCWSTR pszCaption, PCWSTR pszMessage);
	static void LogToStdOut(LPCSTR pszCaption, LPCSTR pszMessage);
};