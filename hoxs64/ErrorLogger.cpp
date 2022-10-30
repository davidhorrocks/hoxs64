#include <tchar.h>
#include <comdef.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "ErrorLogger.h"

bool ErrorLogger::HideMessageBox = false;

bool ErrorLogger::HideWindow = false;

void ErrorLogger::LogToStdOut(const std::wstring& caption, const std::wstring& message)
{
	std::wcout << caption << ": " << message << std::endl;
}

void ErrorLogger::LogToStdOut(const std::string& caption, const std::string& message)
{
	std::cout << caption << ": " << message << std::endl;
}

void ErrorLogger::Log(PCSTR lpText, PCSTR lpCaption, UINT uType)
{
	Log(nullptr, lpText, lpCaption, uType);
}

void ErrorLogger::Log(HWND hWnd, PCSTR lpText, PCSTR lpCaption, UINT uType)
{
	LogToStdOut(lpCaption, lpText);
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxA(hWnd, lpText, lpCaption, uType);
	}
}

void ErrorLogger::Log(PCWSTR lpText, PCWSTR lpCaption, UINT uType)
{
	Log(nullptr, lpText, lpCaption, uType);
}

void ErrorLogger::Log(HWND hWnd, PCWSTR lpText, PCWSTR lpCaption, UINT uType)
{
	LogToStdOut(lpCaption, lpText);
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxW(hWnd, lpText, lpCaption, uType);
	}
}

void ErrorLogger::Log(const std::string& message)
{
	Log(nullptr, message);
}

void ErrorLogger::Log(HWND hWnd, const std::string& message)
{
	Log(hWnd, StringConverter::StringToWideString(message));
}

void ErrorLogger::Log(const std::wstring& message)
{
	Log(nullptr, message);
}

void ErrorLogger::Log(HWND hWnd, const std::wstring& message)
{
	LogToStdOut(L"Error", message);
	if (!ErrorLogger::HideMessageBox)
	{
		std::wstring error_message = L"Error: " + message;
		MessageBoxW(hWnd, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(HRESULT hr, const std::string& message)
{
	Log(nullptr, hr, message);
}

void ErrorLogger::Log(HWND hWnd, HRESULT hr, const std::string& message)
{
	Log(hWnd, hr, StringConverter::StringToWideString(message));
}

void ErrorLogger::Log(HRESULT hr, const std::wstring& message)
{
	Log(nullptr, hr, message);
}

void ErrorLogger::Log(HWND hWnd, HRESULT hr, const std::wstring& message)
{
	LogToStdOut(L"Error", message);
	if (!ErrorLogger::HideMessageBox)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(hWnd, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(const COMException& exception)
{
	Log(nullptr, exception);
}

void ErrorLogger::Log(HWND hWnd, const COMException& exception)
{
	std::wstring error_message = exception.what();
	LogToStdOut(L"Error", error_message);
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxW(hWnd, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}
