#include <windows.h>
#include <assert.h>
#include <RichEdit.h>
#include <tchar.h>
#include <winuser.h>
#include <Dwmapi.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "defines.h"
#include "ErrorLogger.h"
#include "StringConverter.h"
#include "CDPI.h"
#include "utils.h"

//Static constructed members for G
const TCHAR G::EmptyString[1] = TEXT("");

DLGTEMPLATE* WINAPI G::DoLockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName)
{
	HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG);
	if (hrsrc)
	{
		HGLOBAL hglb = LoadResource(hinst, hrsrc);
		if (hglb)
			return (DLGTEMPLATE*)LockResource(hglb);
	}
	return NULL;
}

void G::ShowLastError(HWND hWnd)
{
LPVOID lpMsgBuf=NULL;
DWORD err,r;

	err = GetLastError();

	if (err!=0)
	{
		r = FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		if (r!=0)
		{
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			G::DebugMessageBox(hWnd, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	}
}

TCHAR *G::GetLastWin32ErrorTString()
{
	DWORD err = GetLastError();
	return GetLastWin32ErrorTString(err);
}

TCHAR *G::GetLastWin32ErrorTString(DWORD err)
{
TCHAR *lpMsgBuf = NULL;
DWORD r;

	if (err != 0)
	{
		r = FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL 
		);
		if (r!=0)
		{
			return lpMsgBuf;
		}
	}
	return NULL;
}

std::wstring G::GetLastWin32ErrorWString()
{
	return GetLastWin32ErrorWString(GetLastError());
}

std::wstring G::GetLastWin32ErrorWString(DWORD err)
{
	wchar_t* lpMsgBuf = NULL;
	DWORD r;

	std::wstring message;
	if (err != 0)
	{
		r = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language	
			(LPWSTR)&lpMsgBuf,
			0,
			NULL
		);

		if (r != 0)
		{			
			message.append(lpMsgBuf);
		}
	}

	return message;
}

std::string G::GetLastWin32ErrorString()
{
	return GetLastWin32ErrorString(GetLastError());
}

std::string G::GetLastWin32ErrorString(DWORD err)
{
	char* lpMsgBuf = NULL;
	DWORD r;

	std::string message;
	if (err != 0)
	{
		r = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language	
			(LPSTR)&lpMsgBuf,
			0,
			NULL
		);

		if (r != 0)
		{
			message.append(lpMsgBuf);
		}
	}

	return message;
}

/*****************************************************************/


/******************************************************************************\
*
*  FUNCTION:    GetStringRes (int id INPUT ONLY)
*
*  COMMENTS:    Load the resource string with the ID given, and return a
*               pointer to it.  Notice that the buffer is common memory so
*               the string must be used before this call is made a second time.
*
\******************************************************************************/

LPTSTR G::GetStringRes (int id)
{
	static TCHAR buffer[MAX_PATH+1];

	buffer[0]=0;
	LoadString (GetModuleHandle (NULL), id, buffer, MAX_PATH);
	return buffer;
}

void G::ArrangeOKCancel(HWND hwndDlg)
{
HWND hwndButtonOK;
HWND hwndButtonCancel;
RECT rcDlg;
RECT rcTemp;
RECT rcButtonOK;
RECT rcButtonCancel;
SIZE sizeButtonOK = {0, 0};
SIZE sizeButtonCancel = {0, 0};
SIZE margin = {0, 0};

const bool ShowLeft = false;
const LONG DialogMargin = 2;

	GetClientRect(hwndDlg, &rcDlg);

	SetRectEmpty(&rcTemp);
	rcTemp.right = DialogMargin;
	rcTemp.bottom = DialogMargin;
	MapDialogRect(hwndDlg, &rcTemp);
	margin.cx = abs(rcTemp.right - rcTemp.left);
	margin.cy = abs(rcTemp.bottom - rcTemp.top);

	hwndButtonOK = GetDlgItem(hwndDlg, IDOK); 
	if (hwndButtonOK)
	{
		if (GetWindowRect(hwndButtonOK, &rcButtonOK))
		{
			ScreenToClient(hwndDlg, (LPPOINT)&rcButtonOK.left);
			ScreenToClient(hwndDlg, (LPPOINT)&rcButtonOK.right);
			sizeButtonOK.cx = rcButtonOK.right - rcButtonOK.left;
			sizeButtonOK.cy = rcButtonOK.bottom - rcButtonOK.top;
		}
	}

	hwndButtonCancel = GetDlgItem(hwndDlg, IDCANCEL); 
	if (hwndButtonCancel)
	{
		if (GetWindowRect(hwndButtonCancel, &rcButtonCancel))
		{
			ScreenToClient(hwndDlg, (LPPOINT)&rcButtonCancel.left);
			ScreenToClient(hwndDlg, (LPPOINT)&rcButtonCancel.right);
			sizeButtonCancel.cx = rcButtonCancel.right - rcButtonCancel.left;
			sizeButtonCancel.cy = rcButtonCancel.bottom - rcButtonCancel.top;
		}
	}

	if (ShowLeft)
	{
		if (hwndButtonOK)
		{
			// Move the OK button. 
			SetWindowPos(hwndButtonOK, NULL, 
				margin.cx + rcDlg.left, 
				rcDlg.bottom - sizeButtonOK.cy - margin.cy, 
				0, 0, SWP_NOSIZE | SWP_NOZORDER); 

		}

		if (hwndButtonCancel)
		{
			// Move the Cancel button to the right of the OK.
			SetWindowPos(hwndButtonCancel, NULL,
				margin.cx + rcDlg.left + sizeButtonOK.cx + margin.cx,
				rcDlg.bottom - sizeButtonCancel.cy - margin.cy, 
				0, 0, SWP_NOSIZE | SWP_NOZORDER); 
		}
	}
	else
	{
		if (hwndButtonOK)
		{
			// Move the OK button. 
			SetWindowPos(hwndButtonOK, NULL, 
				rcDlg.right - margin.cx - sizeButtonCancel.cx - margin.cx - sizeButtonOK.cx,
				rcDlg.bottom - sizeButtonOK.cy - margin.cy, 
				0, 0, SWP_NOSIZE | SWP_NOZORDER); 

		}

		if (hwndButtonCancel)
		{
			// Move the Cancel button to the right of the OK.
			SetWindowPos(hwndButtonCancel, NULL,
				rcDlg.right - margin.cx - sizeButtonCancel.cx,
				rcDlg.bottom - sizeButtonCancel.cy - margin.cy, 
				0, 0, SWP_NOSIZE | SWP_NOZORDER); 
		}
	}
}

bool G::m_bHasCachedCommonControlsVersion = false;
DWORD G::m_dwCachedCommonControlsVersion = 0;

BOOL G::WaitMessageTimeout(DWORD dwTimeout) noexcept
{
	return MsgWaitForMultipleObjectsEx(0, nullptr, dwTimeout, QS_ALLINPUT, MWMO_INPUTAVAILABLE) == WAIT_TIMEOUT;
}

HRESULT G::GetVersion_Res(LPCTSTR filename, VS_FIXEDFILEINFO* p_vinfo)
{
	void* pv = nullptr;
	HRSRC v = 0;
	HGLOBAL hv = nullptr;
	UINT l = 0;
	DWORD lDummy = 0;
	VS_FIXEDFILEINFO* p_vinfotmp = nullptr;
	HRESULT hr = E_FAIL;

	l = GetFileVersionInfoSize(filename, &lDummy);
	if (l == 0)
	{
		return E_FAIL;
	}

	pv = malloc(l);
	if (!pv)
	{
		return E_FAIL;
	}

	if (GetFileVersionInfo(filename, 0, l, pv))
	{
		if (VerQueryValue(pv, TEXT("\\"), (LPVOID*)& p_vinfotmp, &l))
		{
			*p_vinfo = *p_vinfotmp;
			hr = S_OK;
		}
	}

	if (pv)
	{
		free(pv);
	}

	return hr;
}

//
//   FUNCTION: CenterWindow(HWND, HWND)
//
//   PURPOSE: Centers one window over another.
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
//       This functionwill center one window over another ensuring that
//    the placement of the window is within the 'working area', meaning
//    that it is both within the display limits of the screen, and not
//    obscured by the tray or other framing elements of the desktop.
BOOL G::CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent, rWorkArea;
	int     wChild, hChild, wParent, hParent;
	int     xNew, yNew;
	BOOL  bResult;

	// Get the Height and Width of the child window
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	// Get the Height and Width of the parent window
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	// Get the limits of the 'workarea'
	bResult = SystemParametersInfo(
		SPI_GETWORKAREA,  // system parameter to query or set
		sizeof(RECT),
		&rWorkArea,
		0);
	if (!bResult) {
		rWorkArea.left = rWorkArea.top = 0;
		rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
		rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	// Calculate new X position, then adjust for workarea
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < rWorkArea.left) {
		xNew = rWorkArea.left;
	} else if ((xNew+wChild) > rWorkArea.right) {
		xNew = rWorkArea.right - wChild;
	}

	// Calculate new Y position, then adjust for workarea
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < rWorkArea.top) {
		yNew = rWorkArea.top;
	} else if ((yNew+hChild) > rWorkArea.bottom) {
		yNew = rWorkArea.bottom - hChild;
	}

	// Set it, and return
	return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void G::DebugMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	ErrorLogger::Log(hWnd, lpText, lpCaption, uType);
}

HRESULT G::InitFail(HWND hWnd, HRESULT hRet, const wchar_t* szError, ...)
{
	va_list         vl;
	va_start(vl, szError);
	std::wstring message = format_string(szError, vl);
	va_end(vl);
	G::DebugMessageBox(hWnd, message.c_str(), APPNAME, MB_OK | MB_ICONEXCLAMATION);
	return hRet;
}

std::wstring G::format_string(const wchar_t* format, ...)
{
	va_list vl;
	std::wstring result;
	size_t cchbuffer = FORMAT_STRING_INITIAL_SIZE;
	wchar_t* buffer = nullptr;
	while (1)
	{
		buffer = new wchar_t[cchbuffer];
		if (buffer == nullptr)
		{
			throw std::bad_alloc();
		}

		va_start(vl, format);
		int copied = _vsnwprintf_s(buffer, cchbuffer, cchbuffer - 1, format, vl);
		va_end(vl);
		if (copied == -1)
		{
			delete[] buffer;
			buffer = nullptr;
			cchbuffer *= 2;
			if (cchbuffer >= FORMAT_STRING_MAX_SIZE)
			{
				throw std::exception("format_string buffer required is too long.");
			}
		}
		else if (copied < 0 || (size_t)copied >= cchbuffer)
		{
			delete[] buffer;
			buffer = nullptr;
			throw std::exception("format_string error.");
		}
		else
		{
			buffer[copied] = 0;
			break;
		}
	}

	if (buffer != nullptr)
	{
		result.append(buffer);
		delete[] buffer;
		buffer = nullptr;
	}

	return result;
}

std::wstring G::format_string(const wchar_t* format, va_list vl)
{
	std::wstring result;
	size_t cchbuffer = FORMAT_STRING_INITIAL_SIZE;
	wchar_t* buffer = nullptr;
	while (1)
	{
		buffer = new wchar_t[cchbuffer];
		if (buffer == nullptr)
		{
			throw std::bad_alloc();
		}

		int copied = _vsnwprintf_s(buffer, cchbuffer, cchbuffer - 1, format, vl);
		if (copied == -1)
		{
			delete[] buffer;
			buffer = nullptr;
			cchbuffer *= 2;
			if (cchbuffer >= FORMAT_STRING_MAX_SIZE)
			{
				throw std::exception("format_string buffer required is too long.");
			}
		}
		else if (copied < 0 || (size_t)copied >= cchbuffer)
		{
			delete[] buffer;
			buffer = nullptr;
			throw std::exception("format_string error.");
		}
		else
		{
			buffer[copied] = 0;
			break;
		}
	}

	if (buffer != nullptr)
	{
		result.append(buffer);
		delete[] buffer;
		buffer = nullptr;
	}

	return result;
}

std::string G::format_string(const char* format, ...)
{
	va_list vl;
	std::string result;
	size_t cchbuffer = FORMAT_STRING_INITIAL_SIZE;
	char* buffer = nullptr;
	while (1)
	{
		buffer = new char[cchbuffer];
		if (buffer == nullptr)
		{
			throw std::bad_alloc();
		}

		va_start(vl, format);
		int copied = _vsnprintf_s(buffer, cchbuffer, cchbuffer - 1, format, vl);
		va_end(vl);
		if (copied == -1)
		{
			delete[] buffer;
			buffer = nullptr;
			cchbuffer *= 2;
			if (cchbuffer >= FORMAT_STRING_MAX_SIZE)
			{
				throw std::exception("format_string buffer required is too long.");
			}
		}
		else if (copied < 0 || (size_t)copied >= cchbuffer)
		{
			delete[] buffer;
			buffer = nullptr;
			throw std::exception("format_string error.");
		}
		else
		{
			buffer[copied] = 0;
			break;
		}
	}

	if (buffer != nullptr)
	{
		result.append(buffer);
		delete[] buffer;
		buffer = nullptr;
	}

	return result;
}

std::string G::format_string(const char* format, va_list vl)
{
	std::string result;
	size_t cchbuffer = FORMAT_STRING_INITIAL_SIZE;
	char* buffer = nullptr;
	while (1)
	{
		buffer = new char[cchbuffer];
		if (buffer == nullptr)
		{
			throw std::bad_alloc();
		}

		int copied = _vsnprintf_s(buffer, cchbuffer, cchbuffer - 1, format, vl);
		if (copied == -1)
		{
			delete[] buffer;
			buffer = nullptr;
			cchbuffer *= 2;
			if (cchbuffer >= FORMAT_STRING_MAX_SIZE)
			{
				throw std::exception("format_string buffer required is too long.");
			}
		}
		else if (copied < 0 || (size_t)copied >= cchbuffer)
		{
			delete[] buffer;
			buffer = nullptr;
			throw std::exception("format_string error.");
		}
		else
		{
			buffer[copied] = 0;
			break;
		}
	}

	if (buffer != nullptr)
	{
		result.append(buffer);
		delete[] buffer;
		buffer = nullptr;
	}

	return result;
}

void G::InitOfn(OPENFILENAME& ofn, HWND hWnd, LPTSTR szTitle, TCHAR szInitialFile[], int chInitialFile, LPTSTR szFilter, TCHAR szReturnFile[], int chReturnFile)
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);

	ofn.hwndOwner = hWnd;
	if (szFilter != nullptr)
	{
		ofn.lpstrFilter = szFilter;
		ofn.nFilterIndex = 1;
	}

	if (szInitialFile != nullptr)
	{
		ofn.lpstrFile = szInitialFile;
		ofn.nMaxFile = chInitialFile;
	}

	if (szReturnFile != nullptr)
	{
		if (chReturnFile > 0)
		{
			szReturnFile[0] = 0;
		}
		ofn.lpstrFileTitle = szReturnFile;
		ofn.nMaxFileTitle = chReturnFile;
	}

	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrTitle = szTitle;
}

bool G::IsWindowsVistaOrLater()
{
#if (WINVER < 0x0601)
	OSVERSIONINFO ver;
	memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (! ::GetVersionEx(&ver) )
	{
		return false;
	}

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ver.dwMajorVersion >= 6)
			return true;
	}
	return false;
#else
	return true;
#endif
}


bool G::IsWinVerSupportInitializeCriticalSectionAndSpinCount()
{
#if (WINVER < 0x0601)
	OSVERSIONINFOEXA ver;
	memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
	if (! ::GetVersionExA((LPOSVERSIONINFOA)&ver) )
	{
		return false;
	}

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ver.dwMajorVersion >= 5)
			return true;
		else if (ver.dwMajorVersion == 4 && ver.wServicePackMajor >=3)
			return true;
	}
	else if ( (ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
	{
		return true;
	}
	return false;
#else
	return true;
#endif
}

bool G::IsMultiCore() noexcept
{
	int c = 0;
	DWORD_PTR dwProcessAffinityMask, dwSystemAffinityMask;
	dwProcessAffinityMask=0; 
	dwSystemAffinityMask=0;

	HANDLE hProcess = GetCurrentProcess();
	if (hProcess!=0)
	{
		BOOL r = GetProcessAffinityMask(hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask);
		if (r)
		{
			for(int i = 0; i < 32; i++)
			{
				if (dwSystemAffinityMask & 1)
				{
					c++;
				}

				dwSystemAffinityMask >>=1;
			}
		}
	}
	return (c > 1);
}

void G::PaintRect(HDC hdc, RECT *rect, COLORREF colour)
{
	COLORREF oldcr = SetBkColor(hdc, colour);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, TEXT(""), 0, 0);
	SetBkColor(hdc, oldcr);
}

void G::AutoSetComboBoxHeight(HWND hWndParent, int controls[], int count, int maxHeight)
{
	for (int i=0; i<count; i++)
	{
		HWND hWndCbo = GetDlgItem(hWndParent, controls[i]);
		if (hWndCbo)
		{
			G::AutoSetComboBoxHeight(hWndCbo, maxHeight);
		}
	}
}

void G::AutoSetComboBoxHeight(HWND hWnd,  int maxHeight)
{
int itemHeight, count, height, limitHeight, editHeight;
RECT rc, rWorkArea;
BOOL bResult;
LRESULT lr;

	bResult = SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rWorkArea, 0);
	if (!bResult) 
	{
		rWorkArea.left = rWorkArea.top = 0;
		rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
		rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	if (hWnd == 0)
	{
		return;
	}

	lr = SendMessage(hWnd, CB_GETITEMHEIGHT, 0, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
	{
		return;
	}

	itemHeight = (int)lr;

	lr = SendMessage(hWnd, CB_GETITEMHEIGHT, -1, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
	{
		return;
	}

	editHeight = (int)lr;
	lr = SendMessage(hWnd, CB_GETCOUNT, 0, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
	{
		return;
	}

	count = (int)lr;
	height = itemHeight * count + editHeight*2;
	if (maxHeight > 0)
	{
		if (height > maxHeight)
		{
			height = maxHeight;
		}
	}

	if(!GetWindowRect(hWnd, &rc))
	{
		return ;
	}
	
	limitHeight = abs(rWorkArea.bottom - rWorkArea.top) / 3;
	if (height > limitHeight)
	{
		height = limitHeight;
	}

	SetWindowPos(hWnd, (HWND)HWND_NOTOPMOST,0,0, rc.right - rc.left, (int)height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

HRESULT G::GetClsidFromRegValue(HKEY hKey, LPCTSTR lpValueName, GUID* pId)
{
	WCHAR szwGuid[50];
	ULONG bufferBytelen;
	ULONG bytesCopied;
	ULONG charsCopied;
	LSTATUS ls;
	HRESULT hr = E_FAIL;

#ifdef UNICODE
	bufferBytelen = (_countof(szwGuid) - 1) * sizeof(TCHAR);
	bytesCopied = bufferBytelen;
	ls = RegQueryValueEx(hKey, lpValueName, NULL, NULL, (PBYTE)&szwGuid[0], &bytesCopied);
	if (ls == ERROR_SUCCESS)
	{
		charsCopied = bytesCopied / sizeof(WCHAR);
		if (charsCopied == 0 || charsCopied >= _countof(szwGuid))
		{
			return E_FAIL;
		}

		szwGuid[charsCopied] = 0;
		hr = S_OK;
	}
	else
	{
		return MAKE_HRESULT(1, 0, ls);
	}

#else
	CHAR szGuid[50];
	bufferBytelen = (_countof(szGuid) - 1) * sizeof(CHAR);
	bytesCopied = bufferBytelen;
	ls = RegQueryValueEx(hKey, lpValueName, NULL, NULL, (PBYTE)&szGuid[0], &bytesCopied);
	if (ls == ERROR_SUCCESS)
	{
		charsCopied = bytesCopied / sizeof(CHAR);
		if (charsCopied == 0 || charsCopied >= _countof(szGuid))
		{
			return E_FAIL;
		}

		szGuid[charsCopied] = 0;
		hr = StringConverter::MultiByteToUc(CP_ACP, &szGuid[0], -1, &szwGuid[0], _countof(szwGuid));
	}
#endif


	if (SUCCEEDED(hr))
	{
		if (CLSIDFromString(&szwGuid[0], pId) == NOERROR)
		{
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}
	else
	{
		return hr;
	}
}

HRESULT G::SaveClsidToRegValue(HKEY hKey, LPCTSTR lpValueName, const GUID* pId)
{
	TCHAR szValue[50]{};
	WCHAR szwValue[50]{};
	LSTATUS ls;
	HRESULT hr = E_FAIL;
	int i;

	i = StringFromGUID2(*pId, &szwValue[0], _countof(szwValue));
	if (i != 0)
	{
		szwValue[_countof(szwValue) - 1] = 0;
#ifdef UNICODE
		lstrcpy(szValue, szwValue);
		hr = S_OK;
#else
		int bytesCopied = 0;
		hr = StringConverter::UcToMultiByte(CP_ACP, &szwValue[0], -1, szValue, _countof(szwValue), bytesCopied);
#endif
		if (SUCCEEDED(hr))
		{
			ls = RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (LPBYTE)&szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
			if (ls == ERROR_SUCCESS)
			{
				return S_OK;
			}
			else
			{
				return MAKE_HRESULT(1, 0, ls);
			}
		}
	}

	return E_FAIL;
}

void G::RectToWH(const RECT& rc, LONG& x, LONG& y, LONG& w, LONG& h)
{
	x = rc.left;
	y = rc.top;
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;
}


BOOL G::DrawDefText(HDC hdc, int x, int y, LPCTSTR text, int len, int* nextx, int* nexty)
{
TEXTMETRIC tm;
	BOOL br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		if (nextx != NULL)
		{
			*nextx = x;
		}

		if (nexty != NULL)
		{
			*nexty = y;
		}

		if (len > 0)
		{
			SIZE sizeText;
			BOOL brTextExtent=FALSE;
			brTextExtent = GetTextExtentExPoint(hdc, text, len, 0, NULL, NULL, &sizeText);
			ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, text, len, NULL);
			if (brTextExtent)
			{
				if (nextx!=NULL)
				{
					*nextx = x + sizeText.cx;
				}

				if (nexty!=NULL)
				{
					*nexty = y + sizeText.cy;
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

void G::GetWorkArea(RECT& rcWorkArea)
{
	BOOL lRetCode = SystemParametersInfo(
		SPI_GETWORKAREA,  // system parameter to query or set
		sizeof(RECT),
		&rcWorkArea,
		0);
	if (!lRetCode)
	{
		rcWorkArea.left = rcWorkArea.top = 0;
		rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
		rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
}

void G::GetMonitorWorkAreaFromWindow(HWND hWnd, RECT& rcWorkArea)
{
LONG lRetCode; 
bool bGotWorkArea = false;
	SetRectEmpty(&rcWorkArea);
	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	if (GetMonitorInfo(hMonitor, &mi))
	{
		rcWorkArea = mi.rcMonitor;
		if ((mi.dwFlags & MONITORINFOF_PRIMARY) == 0)
			bGotWorkArea = true;
	}

	if (!bGotWorkArea)
	{
		// Get the limits of the 'workarea'
		lRetCode = SystemParametersInfo(
			SPI_GETWORKAREA,  // system parameter to query or set
			sizeof(RECT),
			&rcWorkArea,
			0);
		if (!lRetCode)
		{
			rcWorkArea.left = rcWorkArea.top = 0;
			rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
			rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		}
	}
}

BOOL G::DrawBitmap (HDC hDC, INT x, INT y, HBITMAP hBitmap, DWORD dwROP)
{
HDC       hDCBits;
BITMAP    Bitmap;
BOOL      bResult = FALSE;

	if (hDC!=0 && hBitmap!=0)
	{
		hDCBits = CreateCompatibleDC(hDC);
		if (hDCBits)
		{
			if (GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&Bitmap))
			{
				HBITMAP oldbmp = (HBITMAP)SelectObject(hDCBits, hBitmap);
				if (oldbmp)
				{
					bResult = BitBlt(hDC, x, y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBits, 0, 0, dwROP);
					SelectObject(hDCBits, oldbmp);
				}
				DeleteDC(hDCBits);
			}
		}
	}
	return bResult;
}

HBITMAP G::CreateResizedBitmap(HDC hdc, HBITMAP hBmpSrc, int newwidth, int newheight, bool bAllowShrink, bool bAllowStretch)
{
int r;
bool ok = false;
HBITMAP hBmpImageSz = NULL;
	HDC hMemDC_Dest = CreateCompatibleDC(hdc);
	if (hMemDC_Dest)
	{
		HDC hMemDC_Src = CreateCompatibleDC(hdc);
		if (hMemDC_Src)
		{

			BITMAP bitmapImage;
			r = GetObject(hBmpSrc, sizeof(BITMAP), &bitmapImage);
			if (r)
			{
				if (newwidth <= 0)
					newwidth = bitmapImage.bmWidth;
				if (newheight <= 0)
					newheight = bitmapImage.bmHeight;

				if (!bAllowShrink)
				{
					if (newwidth < bitmapImage.bmWidth)
						newwidth = bitmapImage.bmWidth;
					if (newheight < bitmapImage.bmHeight)
						newheight = bitmapImage.bmHeight;
				}
				if (!bAllowStretch)
				{
					if (newwidth > bitmapImage.bmWidth)
						newwidth = bitmapImage.bmWidth;
					if (newheight > bitmapImage.bmHeight)
						newheight = bitmapImage.bmHeight;
				}

				hBmpImageSz = CreateCompatibleBitmap(hdc, newwidth, newheight);
				if (hBmpImageSz)
				{
					HBITMAP hOld_BmpDest = (HBITMAP)SelectObject(hMemDC_Dest, hBmpImageSz);
					HBITMAP hOld_BmpSrc = (HBITMAP)SelectObject(hMemDC_Src, hBmpSrc);
					if (hOld_BmpDest && hOld_BmpSrc)
					{
						StretchBlt(hMemDC_Dest, 0, 0, newwidth, newheight, hMemDC_Src, 0, 0, bitmapImage.bmWidth, bitmapImage.bmHeight, SRCCOPY);
						ok = true;
					}
					if (hOld_BmpDest)
						SelectObject(hMemDC_Dest, hOld_BmpDest);
					if (hOld_BmpSrc)
						SelectObject(hMemDC_Src, hOld_BmpSrc);

					if (!ok)
					{
						DeleteObject(hBmpImageSz);
						hBmpImageSz = NULL;
					}
				}

			}
			DeleteDC(hMemDC_Src);
		}
		DeleteDC(hMemDC_Dest);
	}
	return hBmpImageSz;
}

HIMAGELIST G::CreateImageListNormal(HINSTANCE hInst, HWND hWnd, int tool_dx, int tool_dy, const ImageInfo tbImageList[], int countOfImageList)
{
	HIMAGELIST hImageList = NULL;
	int r;
	bool fail = false;

	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		HDC hMemDC_Dest = CreateCompatibleDC(hdc);
		if (hMemDC_Dest)
		{
			HDC hMemDC_Src = CreateCompatibleDC(hdc);
			if (hMemDC_Src)
			{
				hImageList = ImageList_Create(tool_dx, tool_dy, ILC_MASK | ILC_COLOR32, countOfImageList, 0);
				if (hImageList)
				{
					HBITMAP hbmpImage = NULL;
					HBITMAP hbmpMask = NULL;
					HBITMAP hBmpImageSz = NULL;
					HBITMAP hBmpMaskSz = NULL;
					HICON hIconImage = NULL;
					for (int i = 0; i < countOfImageList; i++)
					{
						if (tbImageList[i].BitmapImageResourceId != 0)
						{
							hbmpImage = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(tbImageList[i].BitmapImageResourceId), IMAGE_BITMAP, 0, 0, 0);
							if (hbmpImage == NULL)
							{
								fail = true;
								break;
							}

							BITMAP bitmapImage;
							BITMAP bitmapMask;
							r = GetObject(hbmpImage, sizeof(BITMAP), &bitmapImage);
							if (!r)
							{
								fail = true;
								break;
							}
							if (tbImageList[i].BitmapMaskResourceId != 0)
							{
								hbmpMask = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(tbImageList[i].BitmapMaskResourceId), IMAGE_BITMAP, 0, 0, 0);
								if (hbmpMask)
								{
									r = GetObject(hbmpMask, sizeof(BITMAP), &bitmapMask);
									if (!r)
									{
										fail = true;
										break;
									}
								}
							}

							hBmpImageSz = CreateCompatibleBitmap(hdc, tool_dx, tool_dy);
							if (!hBmpImageSz)
							{
								fail = true;
								break;
							}

							if (hbmpMask)
							{
								hBmpMaskSz = CreateCompatibleBitmap(hdc, tool_dx, tool_dy);
								if (!hBmpMaskSz)
								{
									fail = true;
									break;
								}
							}

							bool bOK = false;
							HBITMAP hOld_BmpDest = (HBITMAP)SelectObject(hMemDC_Dest, hBmpImageSz);
							HBITMAP hOld_BmpSrc = (HBITMAP)SelectObject(hMemDC_Src, hbmpImage);
							if (hOld_BmpDest && hOld_BmpSrc)
							{
								if (tool_dx != bitmapImage.bmWidth || tool_dy != bitmapImage.bmHeight)
								{
									StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapImage.bmWidth, bitmapImage.bmHeight, SRCCOPY);
								}
								else
								{
									BitBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, SRCCOPY);
								}

								if (hbmpMask && hBmpMaskSz)
								{
									HBITMAP hOld_BmpDest2 = (HBITMAP)SelectObject(hMemDC_Dest, hBmpMaskSz);
									HBITMAP hOld_BmpSrc2 = (HBITMAP)SelectObject(hMemDC_Src, hbmpMask);
									if (hOld_BmpDest2 && hOld_BmpSrc2)
									{
										if (tool_dx != bitmapImage.bmWidth || tool_dy != bitmapImage.bmHeight)
										{
											StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapMask.bmWidth, bitmapMask.bmHeight, SRCCOPY);
										}
										else
										{
											BitBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, SRCCOPY);
										}

										bOK = true;
									}
								}
								else
								{
									bOK = true;
								}
							}

							if (hOld_BmpDest)
							{
								SelectObject(hMemDC_Dest, hOld_BmpDest);
							}

							if (hOld_BmpSrc)
							{
								SelectObject(hMemDC_Src, hOld_BmpSrc);
							}

							if (!bOK)
							{
								fail = true;
								break;
							}

							if (hBmpMaskSz)
							{
								r = ImageList_Add(hImageList, hBmpImageSz, hBmpMaskSz);
							}
							else
							{
								r = ImageList_AddMasked(hImageList, hBmpImageSz, RGB(0xff, 0xff, 0xff));
							}

							if (r < 0)
							{
								fail = true;
								break;
							}

							if (hBmpImageSz)
							{
								DeleteObject(hBmpImageSz);
							}

							if (hBmpMaskSz)
							{
								DeleteObject(hBmpMaskSz);
							}

							if (hbmpImage)
							{
								DeleteObject(hbmpImage);
							}

							if (hbmpMask)
							{
								DeleteObject(hbmpMask);
							}

							hBmpImageSz = NULL;
							hBmpMaskSz = NULL;
							hbmpImage = NULL;
							hbmpMask = NULL;
						}
						else
						{
							hIconImage = (HICON)LoadImage(hInst, MAKEINTRESOURCE(tbImageList[i].IconResourceId), IMAGE_ICON, 0, 0, 0);
							if (hIconImage == NULL)
							{
								fail = true;
								break;
							}

							r = ImageList_AddIcon(hImageList, hIconImage);
							if (r < 0)
							{
								fail = true;
								break;
							}
						}
					}

					if (hBmpImageSz != NULL)
					{
						DeleteObject(hBmpImageSz);
						hBmpImageSz = NULL;
					}

					if (hBmpMaskSz != NULL)
					{
						DeleteObject(hBmpMaskSz);
						hBmpMaskSz = NULL;
					}

					if (hbmpImage != NULL)
					{
						DeleteObject(hbmpImage);
						hbmpImage = NULL;
					}

					if (hbmpMask != NULL)
					{
						DeleteObject(hbmpMask);
						hbmpMask = NULL;
					}

					if (fail)
					{
						if (hImageList != NULL)
						{
							ImageList_Destroy(hImageList);
							hImageList = NULL;
						}
					}
				}

				DeleteDC(hMemDC_Src);
			}
			DeleteDC(hMemDC_Dest);
		}
	}

	if (hImageList)
	{
		ImageList_SetBkColor(hImageList, CLR_NONE);
	}

	return hImageList;
}

HWND G::CreateRebar(HINSTANCE hInst, HWND hWnd, HWND hwndTB, int rebarid)
{
RECT rect;
RECT rcClient;
BOOL br;
HWND hWndRB = NULL;
REBARINFO     rbi;
REBARBANDINFO rbBand;
DWORD_PTR dwBtnSize;

	br = GetWindowRect(hWnd, &rect);
	if (!br)
		return NULL;
	br = GetClientRect(hWnd, &rcClient);
	if (!br)
		return NULL;
	hWndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
		REBARCLASSNAME,//class name
		NULL,//Title
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|CCS_NODIVIDER,
		0,0,0,0,
		hWnd,//Parent
		(HMENU) LongToPtr(rebarid),//Menu
		hInst,//application instance
		NULL);

	if (hWndRB == NULL)
		return NULL;
			
	// Initialize and send the REBARINFO structure.
	::ZeroMemory(&rbi, sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);  // Required when using this
	// structure.
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	if(!SendMessage(hWndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
		return NULL;

	// Get the height of the toolbar.
	dwBtnSize = SendMessage(hwndTB, TB_GETBUTTONSIZE, 0,0);
	int butSizeX = HIWORD(dwBtnSize);
	int butSizeY = HIWORD(dwBtnSize);

	// Initialize structure members that both bands will share.
	::ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
	rbBand.fMask  = RBBIM_COLORS | RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | RBBIM_SIZE;// | RBBIM_TEXT
	rbBand.fStyle = RBBS_CHILDEDGE /*| RBBS_FIXEDBMP*/ | RBBS_GRIPPERALWAYS;
	rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
  

	// Set values unique to the band with the toolbar.
	rbBand.lpText     = TEXT("Tool Bar");
	rbBand.hwndChild  = hwndTB;
	rbBand.cxMinChild = butSizeX;
	rbBand.cyMinChild = butSizeY;
	rbBand.cx         = rcClient.right - rcClient.left;

	int iBandNext = (int)SendMessage(hWndRB, RB_GETBANDCOUNT, (WPARAM)-1, (LPARAM)&rbBand);

	// Add the band that has the toolbar.
	SendMessage(hWndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
	return hWndRB;
}

HRESULT G::SetRebarBandBitmap(HWND hWndRB, int iBandIndex, HBITMAP hBmpSrc)
{
REBARBANDINFO rbBand;

	::ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
	if (SendMessage(hWndRB, RB_GETBANDINFO, iBandIndex, (LPARAM)&rbBand))
	{
		rbBand.hbmBack = hBmpSrc;
		rbBand.fMask |= RBBIM_BACKGROUND;
		if (SendMessage(hWndRB, RB_SETBANDINFO, iBandIndex, (LPARAM)&rbBand))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

HWND G::CreateToolBar(HINSTANCE hInst, HWND hWnd, int toolbarid, HIMAGELIST hImageListToolBarNormal, const ButtonInfo buttonInfo[], int length, int buttonWidth, int buttonHeight)
{
HWND hwndTB = NULL;
HWND ret = NULL;
int i;
LRESULT lr = 0;
TBBUTTON* tbArray = NULL;
HIMAGELIST hOldImageList = NULL;

	int ok = 0;
	do
	{
		DWORD exStyle  = 0;
		#if (_WIN32_WINNT >= _WIN32_WINNT_WINXP)
			exStyle |= TBSTYLE_EX_MIXEDBUTTONS;
		#endif
		hwndTB = ::CreateWindowEx(0, TOOLBARCLASSNAME, 	(LPTSTR) NULL, 
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS |	WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | exStyle | CCS_NORESIZE | CCS_NODIVIDER // | CCS_ADJUSTABLE
			, 0, 0, 0, 0, 
			hWnd, 
			(HMENU) LongToPtr(toolbarid), 
			hInst, 
			NULL
		);
		if (!hwndTB)
		{
			break;
		}

		SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		lr = SendMessage(hwndTB, TB_SETBUTTONSIZE, 0, MAKELONG(buttonWidth, buttonHeight));
		if (!lr)
		{
			break;
		}

		SendMessage(hwndTB, TB_SETMAXTEXTROWS, 0, 0);		 
		hOldImageList = (HIMAGELIST)SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hImageListToolBarNormal);
		tbArray = new TBBUTTON[length];
		if (!tbArray)
		{
			break;
		}

		for (i=0; i<length; i++)
		{
			ZeroMemory(&tbArray[i], sizeof(TBBUTTON));		
			tbArray[i].iBitmap = buttonInfo[i].ImageIndex;
			tbArray[i].idCommand = buttonInfo[i].CommandId;
			tbArray[i].fsStyle = buttonInfo[i].Style;
			tbArray[i].fsState = TBSTATE_ENABLED;
			tbArray[i].iString = (INT_PTR)buttonInfo[i].ButtonText;
		}
	
		lr = SendMessage(hwndTB, TB_ADDBUTTONS, length, (LPARAM)tbArray);
		if (!lr)
		{
			break;
		}

		ok = true;
	} while (false);

	if (ok)
	{
		ret = hwndTB;
		hwndTB = NULL;
	}
	else
	{
		if (hwndTB)
		{
			if (hOldImageList)
			{
				SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hOldImageList);
			}
		}
	}

	if (tbArray)
	{	
		delete[] tbArray;
	}

	return ret;
}

void G::EnsureWindowPosition(HWND hWnd)
{
RECT rcMain, rcWorkArea;
LONG lRetCode; 
bool bGotWorkArea = false;

	if (!hWnd)
	{
		return;
	}

	BOOL br = GetWindowRect(hWnd, &rcMain);
	if (!br)
	{
		return;
	}

	SetRectEmpty(&rcWorkArea);
	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	HMONITOR hMonitor = MonitorFromRect(&rcMain, MONITOR_DEFAULTTOPRIMARY);
	if (GetMonitorInfo(hMonitor, &mi))
	{
		rcWorkArea = mi.rcMonitor;
		bGotWorkArea = true;
	}

	if (!bGotWorkArea)
	{
		// Get the limits of the 'workarea'
		lRetCode = SystemParametersInfo(
			SPI_GETWORKAREA,  // system parameter to query or set
			sizeof(RECT),
			&rcWorkArea,
			0);
		if (!lRetCode)
		{
			rcWorkArea.left = rcWorkArea.top = 0;
			rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
			rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		}
	}

	int workarea_w = (rcWorkArea.right-rcWorkArea.left);
	int workarea_h = (rcWorkArea.bottom-rcWorkArea.top);
	if (rcMain.right>rcWorkArea.right)
	{
		OffsetRect(&rcMain, rcWorkArea.right - rcMain.right, 0);
	}

	if (rcMain.bottom>rcWorkArea.bottom)
	{
		OffsetRect(&rcMain, 0, rcWorkArea.bottom - rcMain.bottom);
	}

	if (rcMain.left<rcWorkArea.left)
	{
		OffsetRect(&rcMain, rcWorkArea.left - rcMain.left, 0);
	}

	if (rcMain.top<rcWorkArea.top)
	{
		OffsetRect(&rcMain, 0, rcWorkArea.top - rcMain.top);
	}

	UINT showWindowFlags;
	if (ErrorLogger::HideWindow)
	{
		showWindowFlags = SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE;
	}
	else
	{
		showWindowFlags = SWP_NOZORDER;
	}

	SetWindowPos(hWnd, HWND_TOP, rcMain.left,rcMain.top, rcMain.right-rcMain.left, rcMain.bottom-rcMain.top, showWindowFlags);
}

int G::GetEditLineString(HWND hEditControl, int linenumber, LPTSTR buffer, int cchBuffer)
{
int c;
	c = (int)SendMessage(hEditControl, EM_LINELENGTH, linenumber, 0);
	if (buffer != NULL && cchBuffer > 0)
	{
		*((LPWORD)&buffer[0]) = cchBuffer;
		c = (int)SendMessage(hEditControl, EM_GETLINE, linenumber, (LPARAM)buffer);
	}	

	if (c < 0)
	{
		c = 0;
	}

	return c;
}

int G::GetEditLineSzString(HWND hEditControl, int linenumber, LPTSTR buffer, int cchBuffer)
{
int c;
	c = (int)SendMessage(hEditControl, EM_LINELENGTH, linenumber, 0);
	if (buffer != NULL && cchBuffer > 0)
	{
		*((LPWORD)&buffer[0]) = cchBuffer;
		c = (int)SendMessage(hEditControl, EM_GETLINE, linenumber, (LPARAM)buffer);
		if (c >= cchBuffer)
		{
			c = cchBuffer - 1;
		}

		if (c < 0)
		{
			c = 0;
		}

		buffer[c] = 0;
	}	

	if (c < 0)
	{
		c = 0;
	}

	return c;
}

LPTSTR G::GetMallocEditLineSzString(HWND hEditControl, int linenumber)
{
int c;
	c = (int)SendMessage(hEditControl, EM_LINELENGTH, linenumber, 0);
	if (c < 0)
	{
		return NULL;
	}

	LPTSTR s = (LPTSTR)malloc(c + sizeof(TCHAR));
	if (!s)
	{
		return NULL;
	}

	SendMessage(hEditControl, EM_GETLINE, linenumber, (LPARAM)s);
	s[c] = 0;
	return s;
}

DWORD G::GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    /* For security purposes, LoadLibrary should be provided with a 
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
                          "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular 
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

DWORD G::CachedCommonControlsVersion()
{
	if (!m_bHasCachedCommonControlsVersion)
	{
		m_dwCachedCommonControlsVersion = GetDllVersion(TEXT("comctl32.dll"));
		m_bHasCachedCommonControlsVersion = true;
	}
	return m_dwCachedCommonControlsVersion;
}

HRESULT G::GetTextSize(HWND hWnd, LPCTSTR szText, SIZE& sizeText)
{
HRESULT hr = E_FAIL;
	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		int slen = lstrlen(szText);
		BOOL br = GetTextExtentExPoint(hdc, szText, slen, 0, NULL, NULL, &sizeText);
		if (br)
			hr = S_OK;
		ReleaseDC(hWnd, hdc);
	}
	return hr;
}

int G::CalcListViewMinWidth(HWND hWnd, ...)
{
	va_list         vl;
	LPCTSTR p;
	va_start(vl, hWnd);
	int k = 0;
	for (p = va_arg(vl, LPCTSTR); p != NULL; p = va_arg(vl, LPCTSTR))
	{
		k = std::max(k, ListView_GetStringWidth(hWnd, p));
	}
	va_end(vl);
	return k;
}

int G::GetMonitorFontPixelsY()
{
	CDPI dpi;
	return dpi.ScaleY(dpi.PointsToPixels(G::MonitorFontPointsY));
}

bool G::GetCurrentFontLineBox(HDC hdc, LPSIZE lpSize)
{
TEXTMETRIC tm;
OUTLINETEXTMETRIC otm;
	
	if (lpSize != NULL)
	{
		BOOL br = GetTextMetrics(hdc, &tm);
		if (br)
		{
			if (tm.tmPitchAndFamily & TMPF_TRUETYPE)
			{
				br = GetOutlineTextMetrics(hdc, sizeof(otm), &otm);
				if (br)
				{				
					lpSize->cy = otm.otmTextMetrics.tmHeight+ otm.otmTextMetrics.tmExternalLeading + otm.otmLineGap;
					lpSize->cx = otm.otmTextMetrics.tmAveCharWidth;
					return true;
				}
			}
			else
			{
				lpSize->cy = tm.tmHeight + tm.tmExternalLeading + 1;
				lpSize->cx = tm.tmAveCharWidth;
				return true;
			}
		}
	}

	return false;
}

HFONT G::CreateMonitorFont()
{
	CDPI dpi;
	HFONT f = 0;
	LPCTSTR lstFontName[] = { TEXT("Consolas"), TEXT("Lucida"), TEXT("Courier"), TEXT("") };
	DWORD fdwQuality = 0;
	fdwQuality |= CLEARTYPE_QUALITY;

	for (int i = 0; f == 0 && i < _countof(lstFontName); i++)
	{
		LPCTSTR fontName = lstFontName[i];
		const int height = G::GetMonitorFontPixelsY();
		f = CreateFont(
			height,
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			ANSI_CHARSET,
			OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS,
			fdwQuality,
			FIXED_PITCH | FF_DONTCARE,
			fontName);
	}

	return f;
}

bool G::IsWhiteSpace(TCHAR ch) noexcept
{
	return (ch == _T(' ') || ch == _T('\n') || ch == _T('\r') || ch == _T('\t') || ch == _T('\b') || ch == _T('\v')  || ch == _T('\f'));
}

bool G::IsStringBlank(const std::wstring s) noexcept
{
	if (s.length() > 0)
	{
		for (auto p = s.cbegin(); p != s.cend(); p++)
		{
			if (!std::isspace(*p))
			{
				return false;
			}
		}
	}

	return true;
}

bool G::IsStringBlank(LPCTSTR ps) noexcept
{
	if (ps)
	{
		for (int i=0; ps[i] != 0; i++)
		{
			if (!G::IsWhiteSpace(ps[i]))
				return false;
		}
	}
	return true;
}

__int64 G::FileSeek (HANDLE hfile, __int64 distance, DWORD moveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer (hfile, 
                                li.LowPart, 
                                &li.HighPart, 
                                moveMethod);

   if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() 
       != NO_ERROR)
   {
      li.QuadPart = -1;
   }

   return li.QuadPart;
}

bool G::TryGetFileSize(HANDLE hfile, unsigned __int64& filesize)
{
   ULARGE_INTEGER li;
   li.QuadPart = 0;
   li.LowPart = GetFileSize(hfile, &li.HighPart);
   if (li.LowPart == INVALID_FILE_SIZE)
   {
	   if (GetLastError() != NO_ERROR)
	   {
		   return false;
	   }
   }

   filesize = li.QuadPart;
   return true;
}

void G::InitRandomSeed()
{
	ULARGE_INTEGER counter = {0, 0};
	QueryPerformanceCounter((PLARGE_INTEGER)&counter);
	srand(counter.LowPart);	
}

bool G::IsLargeGameDevice(const DIDEVCAPS &didevcaps)
{
	return didevcaps.dwButtons > 32 || didevcaps.dwAxes > 8;
}

void G::LTrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
void G::RTrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
void G::Trim(std::string& s) {
	LTrim(s);
	RTrim(s);
}

void G::LTrim(std::wstring& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wchar_t ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
void G::RTrim(std::wstring& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](wchar_t ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
void G::Trim(std::wstring& s) {
	LTrim(s);
	RTrim(s);
}

HRESULT G::LoadAppPath(std::wstring& s)
{
	std::vector<TCHAR> pathbuffer;
	constexpr size_t MaxLongPathLengh = static_cast<size_t>(1024) * 64;
	constexpr unsigned int MaxLongPathDoubling = 16;
	unsigned int counter = 0;
	pathbuffer.resize(MAX_PATH + 1);
	DWORD dw = ERROR_INSUFFICIENT_BUFFER;
	for (counter = 0; pathbuffer.size() <= MaxLongPathLengh || counter < MaxLongPathDoubling; counter++)
	{
		if (counter != 0)
		{
			pathbuffer.resize(pathbuffer.size() * 2);
		}

		dw = GetModuleFileName(NULL, pathbuffer.data(), (DWORD)pathbuffer.size());
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER && dw < pathbuffer.size() && dw != 0)
		{
			dw = ERROR_SUCCESS;
			break;
		}
		else
		{
			dw = ERROR_INSUFFICIENT_BUFFER;
		}
	}

	if (dw != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	s.assign(pathbuffer.data());
	return S_OK;
}

HRESULT G::LoadStringResource(HINSTANCE hInstance, UINT id, std::wstring& s)
{
	s.clear();
	std::vector<TCHAR> pathbuffer;

	// Load resource strings
	constexpr size_t CharCountResourceBuffer = MAX_PATH + 1;// Must be 8 bytes or more for LoadString.

	// Load app title name.
	if (pathbuffer.size() < CharCountResourceBuffer)
	{
		pathbuffer.resize(CharCountResourceBuffer);
	}

	int lr = LoadString(hInstance, id, pathbuffer.data(), 0);
	if ((size_t)(unsigned int)lr >= pathbuffer.size())
	{
		pathbuffer.resize((size_t)lr + 1);
	}

	lr = LoadString(hInstance, id, pathbuffer.data(), (DWORD)pathbuffer.size());
	if (lr == 0 || (size_t)(unsigned int)lr >= pathbuffer.size())
	{
		return E_FAIL;
	}
	else
	{
		pathbuffer[lr] = 0;
	}

	s.assign(pathbuffer.data());
	return S_OK;
}