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

	static void Log(HWND hWnd, PCSTR lpText, PCSTR lpCaption, UINT uType);
	static void Log(PCSTR lpText, PCSTR lpCaption, UINT uType);

	static void Log(HWND hWnd, PCWSTR lpText, PCWSTR lpCaption, UINT uType);
	static void Log(PCWSTR lpText, PCWSTR lpCaption, UINT uType);

	static void Log(HWND hWnd, const std::string& message);
	static void Log(const std::string& message);

	static void Log(HWND hWnd, const std::wstring& message);
	static void Log(const std::wstring& message);

	static void Log(HWND hWnd, HRESULT hr, const std::string& message);
	static void Log(HRESULT hr, const std::string& message);

	static void Log(HWND hWnd, HRESULT hr, const std::wstring& message);
	static void Log(HRESULT hr, const std::wstring& message);

	static void Log(HWND hWnd, const COMException& exception);
	static void Log(const COMException& exception);

private:
	static void LogToStdOut(const std::wstring& caption, const std::wstring& message);
	static void LogToStdOut(const std::string& caption, const std::string& message);
};