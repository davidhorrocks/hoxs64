#include <tchar.h>
#include <comdef.h>
#include <stdio.h>
#include <string>
#include "ErrorLogger.h"

bool ErrorLogger::HideMessageBox = false;

bool ErrorLogger::HideWindow = false;

void ErrorLogger::Log(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) noexcept
{
	if (ErrorLogger::HideMessageBox)
	{
		_ftprintf(stdout, TEXT("%s: %s\r\n"), lpCaption, lpText);
	}
	else
	{
		MessageBox(hWnd, lpText, lpCaption, uType);
	}
}

void ErrorLogger::Log(std::string message) noexcept
{
	try
	{
		std::string error_message = "Error:" + message;
		MessageBoxA(nullptr, error_message.c_str(), "Error", MB_ICONERROR);
	}
	catch (std::exception)
	{
		MessageBoxA(nullptr, message.c_str(), "Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(std::wstring message) noexcept
{
	try
	{
		std::wstring error_message = L"Error:" + message;
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
	catch (std::exception)
	{
		MessageBoxW(nullptr, message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(HRESULT hr, std::string message) noexcept
{
	try
	{
		_com_error error(hr);
		std::wstring error_message = L"Error:" + StringConverter::StringToWideString(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
	catch (std::exception)
	{
		MessageBoxA(nullptr, message.c_str(), "Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(HRESULT hr, std::wstring message) noexcept
{
	try
	{
		_com_error error(hr);
		std::wstring error_message = L"Error:" + message + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}
	catch (std::exception)
	{
		MessageBoxW(nullptr, message.c_str(), L"Error", MB_ICONERROR);
	}
}

void ErrorLogger::Log(COMException& exception) noexcept
{
	std::wstring error_message = exception.what();
	MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
}
