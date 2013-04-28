#include <windows.h>
#include <commctrl.h>
#include "CDPI.h"
#include "utils.h"
#include "tchar.h"
#include "errormsg.h"
#include "stdio.h"
#include "defines.h"

const LPCTSTR ErrorMsg::ERR_OUTOFMEMORY = TEXT("Out of memory.");

ErrorMsg::ErrorMsg()
{
	ClearError();
}


bool ErrorMsg::OK()
{
	if (SUCCEEDED(errorValue))
		return true;
	else
		return false;
}

void ErrorMsg::ClearError()
{
	errorText[0] = TEXT('\0');
	errorValue = S_OK;
}

HRESULT ErrorMsg::SetError(HRESULT hRet, LPCTSTR szError, ...)
{
    va_list         vl;
	va_start(vl, szError);
	errorValue = hRet;
	if (szError != NULL)
	{
		_vsntprintf_s(errorText, _countof(errorText), _TRUNCATE, szError, vl);
		errorText[_countof(errorText)-1] = TEXT('\0');
	}
	else
	{
		errorText[0] = TEXT('\0');
	}
	va_end(vl);
    return hRet;
}

HRESULT ErrorMsg::SetError(ErrorMsg& err)
{
	errorValue = err.errorValue;
	_tcsncpy_s(errorText, _countof(errorText), err.errorText, _countof(errorText)-1);
	errorText[_countof(errorText)-1]=TEXT('\0');
    return errorValue;
}

HRESULT ErrorMsg::SetErrorFromGetLastError(HRESULT hRet)
{
	return SetErrorFromGetLastError(hRet, NULL);
}

HRESULT ErrorMsg::SetErrorFromGetLastError(HRESULT hRet, LPCTSTR szError)
{
	TCHAR *s = G::GetLastWin32ErrorString();
	if (s == NULL)
	{
		SetError(hRet, szError);
	}
	else if (szError == NULL)
	{
		SetError(hRet, s);
		LocalFree(s);
	}
	else
	{
		SetError(hRet, TEXT("%s\n\n%s"), szError, s);
		LocalFree(s);
	}
	return hRet;
}

void ErrorMsg::DisplayError(HWND hWnd, const TCHAR title[])
{
	MessageBox(hWnd, errorText, title, MB_OK | MB_ICONEXCLAMATION);
}

void ErrorMsg::ShowMessage(HWND hWnd, UINT uType, LPCTSTR szTitle, LPCTSTR szError, ...)
{
    TCHAR            szBuff[302];
    va_list         vl;

    va_start(vl, szError);
	_vsntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, szError, vl);
	szBuff[299]=0;
	if (szTitle==NULL)
		MessageBox(hWnd, szBuff, APPNAME, MB_OK | uType);
	else
		MessageBox(hWnd, szBuff, szTitle, MB_OK | uType);
    va_end(vl);
}
