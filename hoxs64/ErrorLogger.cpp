#include <tchar.h>
#include <comdef.h>
#include <stdio.h>
#include <string>
#include "ErrorLogger.h"

bool ErrorLogger::HideMessageBox = false;

bool ErrorLogger::HideWindow = false;

void ErrorLogger::LogToStdOut(PCWSTR pszCaption, PCWSTR pszMessage)
{
	fwprintf(stdout, L"%s: %s\r\n", pszCaption, pszMessage);
}

void ErrorLogger::LogToStdOut(LPCSTR pszCaption, LPCSTR pszMessage)
{
	fprintf(stdout, "%s: %s\r\n", pszCaption, pszMessage);
}

void ErrorLogger::LogInfo(LPCSTR message)
{
	//MessageBoxA(NULL, message, "Info", 0L);
}

void ErrorLogger::Log(HWND hWnd, PCSTR lpText, PCSTR lpCaption, UINT uType)
{
	LogToStdOut(lpCaption, lpText);
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxA(hWnd, lpText, lpCaption, uType);
	}
}

void ErrorLogger::Log(HWND hWnd, PCWSTR lpText, PCWSTR lpCaption, UINT uType)
{
	LogToStdOut(lpCaption, lpText);
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxW(hWnd, lpText, lpCaption, uType);
	}
}

void ErrorLogger::Log(const std::string message)
{
	LogToStdOut("Error", message.c_str());
	if (!ErrorLogger::HideMessageBox)
	{
		std::string error_message = "Error: " + message;
		MessageBoxA(nullptr, error_message.c_str(), "Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(const std::wstring message)
{
	LogToStdOut(L"Error", message.c_str());
	if (!ErrorLogger::HideMessageBox)
	{
		std::wstring error_message = L"Error: " + message;
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(HRESULT hr, const std::string message)
{
	LogToStdOut("Error", message.c_str());
	if (!ErrorLogger::HideMessageBox)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + StringConverter::StringToWideString(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(HRESULT hr, const std::wstring message)
{
	LogToStdOut(L"Error", message.c_str());
	if (!ErrorLogger::HideMessageBox)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(const COMException& exception)
{
	std::wstring error_message = exception.what();
	LogToStdOut(L"Error", error_message.c_str());
	if (!ErrorLogger::HideMessageBox)
	{
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
}
