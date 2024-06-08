#pragma once
#include <Windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "dx_version.h"
#include "boost2005.h"
#include <string>

#define PACKVERSION(major,minor) MAKELONG(minor,major)

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY (5)
#endif

//#define DWM_EC_DISABLECOMPOSITION		0
//#define DWM_EC_ENABLECOMPOSITION		1
//#define WM_DWMCOMPOSITIONCHANGED		0x031E

struct ImageInfo
{
	int BitmapImageResourceId;
	int BitmapMaskResourceId;
	int IconResourceId;
};

struct ButtonInfo
{
	int ImageIndex;
	TCHAR *ButtonText;
	TCHAR *ToolTipText;
	BYTE Style;
	int CommandId;
};

class G
{
private:
	G();
	static const int MonitorFontPointsY = 12;
	static bool m_bHasCachedCommonControlsVersion;
	static DWORD m_dwCachedCommonControlsVersion;
	static const size_t FORMAT_STRING_INITIAL_SIZE = 512;
	static const size_t FORMAT_STRING_MAX_SIZE = 0x3fffffffUL;
public:
	static const size_t MaxApplicationPathLength = 0x7fff;

	static BOOL WaitMessageTimeout(DWORD dwTimeout) noexcept;
	static HRESULT GetVersion_Res(LPCTSTR filename, VS_FIXEDFILEINFO* p_vinfo);
	static DLGTEMPLATE* WINAPI DoLockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName);
	static void ShowLastError(HWND);
	static TCHAR* GetLastWin32ErrorTString();
	static TCHAR* GetLastWin32ErrorTString(DWORD err);
	static std::wstring GetLastWin32ErrorWString();
	static std::wstring GetLastWin32ErrorWString(DWORD err);
	static std::string GetLastWin32ErrorString();
	static std::string GetLastWin32ErrorString(DWORD err);
	static void ArrangeOKCancel(HWND hwndDlg);
	static LPTSTR GetStringRes(int id);
	static BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
	static std::wstring format_string(const wchar_t* fmt, ...);
	static std::wstring format_string(const wchar_t* fmt, va_list vl);
	static std::string format_string(const char* fmt, ...);
	static std::string format_string(const char* fmt, va_list vl);
	static HRESULT InitFail(HWND hWnd, HRESULT hRet, const wchar_t* szError, ...);
	static bool IsWindowsVistaOrLater();
	static bool IsWinVerSupportInitializeCriticalSectionAndSpinCount();
	static bool IsMultiCore() noexcept;
	static void PaintRect(HDC hdc, RECT* rect, COLORREF colour);
	static void AutoSetComboBoxHeight(HWND hWnd, int maxHeight);
	static void AutoSetComboBoxHeight(HWND hWndParent, int controls[], int count, int maxHeight);
	static HRESULT GetClsidFromRegValue(HKEY hKey, LPCTSTR lpValueName, GUID* pId);
	static HRESULT SaveClsidToRegValue(HKEY hKey, LPCTSTR lpValueName, const GUID* pId);
	static void DebugMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
	static void InitOfn(OPENFILENAME& ofn, HWND hWnd, LPTSTR szTitle, TCHAR szInitialFile[], int chInitialFile, LPTSTR szFilter, TCHAR szReturnFile[], int chReturnFile);
	static void RectToWH(const RECT& rc, LONG& x, LONG& y, LONG& w, LONG& h);
	static BOOL DrawDefText(HDC hdc, int x, int y, LPCTSTR text, int len, int* nextx, int* nexty);
	static void GetWorkArea(RECT& rcWorkArea);
	static void GetMonitorWorkAreaFromWindow(HWND hWnd, RECT& rcWorkArea);
	static BOOL DrawBitmap(HDC hDC, INT x, INT y, HBITMAP hBitmap, DWORD dwROP);
	//Win98 WinNT 4 SP6
	//static BOOL IsWindowEnabled(HWND hWnd, bool &bResult);
	static HIMAGELIST CreateImageListNormal(HINSTANCE m_hInst, HWND hWnd, int tool_dx, int tool_dy, const ImageInfo tbImageList[], int countOfImageList);
	static HWND CreateRebar(HINSTANCE hInst, HWND hWnd, HWND hwndTB, int rebarid);
	static HWND CreateToolBar(HINSTANCE hInst, HWND hWnd, int toolbarid, HIMAGELIST hImageListToolBarNormal, const ButtonInfo buttonInfo[], int length, int buttonWidth, int buttonHeight);
	static void EnsureWindowPosition(HWND hWnd);
	static int GetEditLineString(HWND hEditControl, int linenumber, LPTSTR buffer, int cchBuffer);
	static int GetEditLineSzString(HWND hEditControl, int linenumber, LPTSTR buffer, int cchBuffer);
	static LPTSTR GetMallocEditLineSzString(HWND hEditControl, int linenumber);
	static DWORD GetDllVersion(LPCTSTR lpszDllName);
	static DWORD CachedCommonControlsVersion();
	static HRESULT GetTextSize(HWND hWnd, LPCTSTR szText, SIZE& sizeText);
	static int CalcListViewMinWidth(HWND hWnd, ...);
	static HBITMAP CreateResizedBitmap(HDC hdc, HBITMAP hBmpSrc, int newwidth, int newheight, bool bAllowShrink, bool bAllowStretch);
	static HRESULT SetRebarBandBitmap(HWND hWndRB, int iBandIndex, HBITMAP hBmpSrc);
	static int GetMonitorFontPixelsY();
	static bool GetCurrentFontLineBox(HDC hdc, LPSIZE size);
	static HFONT CreateMonitorFont();
	static bool IsStringBlank(const std::wstring s) noexcept;
	static bool IsStringBlank(LPCTSTR ps) noexcept;
	static bool IsWhiteSpace(TCHAR ch) noexcept;
	static __int64 FileSeek(HANDLE hfile, __int64 distance, DWORD moveMethod);
	static unsigned __int64 FileSize(HANDLE hfile);
	static void InitRandomSeed();
	static const TCHAR EmptyString[1];
	static bool IsLargeGameDevice(const DIDEVCAPS& didevcaps);
	static void LTrim(std::string& s);
	static void RTrim(std::string& s);
	static void Trim(std::string& s);
	static void LTrim(std::wstring& s);
	static void RTrim(std::wstring& s);
	static void Trim(std::wstring& s);
	static HRESULT LoadStringResource(HINSTANCE hInstance, UINT id, std::wstring& s);
	static HRESULT LoadAppPath(std::wstring& s);
};

class ComboTextAndValue
{
public:
	LPCTSTR Text;
	DWORD Value;
};
