#pragma once
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

class ErrorMsg
{
public:
	ErrorMsg();
	virtual ~ErrorMsg();
	ErrorMsg(const ErrorMsg&) = default;
	ErrorMsg(ErrorMsg&&) = default;
	ErrorMsg& operator=(const ErrorMsg&) = default;
	ErrorMsg& operator=(ErrorMsg&&) = default;

	TCHAR errorText[300];
	HRESULT errorValue;

	bool OK();
	void ClearError();
	HRESULT SetError(HRESULT hRet, LPCTSTR szError, ...);
	HRESULT SetError(const ErrorMsg& err);
	HRESULT SetErrorFromGetLastError();
	HRESULT SetErrorFromGetLastError(DWORD err, LPCTSTR szError);
	void DisplayError(HWND hWnd, const TCHAR title[]);
	const TCHAR *GetLastWindowsErrorString();
	static void ShowMessage(HWND hWnd, UINT uType, LPCTSTR szTitle, LPCTSTR szError, ...);
	static const LPCTSTR ERR_OUTOFMEMORY;

protected:
	const TCHAR *localLastWindowsErrorString;
};

