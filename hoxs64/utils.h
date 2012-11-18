#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include "oldos_multimon.h"
#include "boost2005.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"

#define PACKVERSION(major,minor) MAKELONG(minor,major)

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY (5)
#endif

//struct NM_COMMAND

// pragma pack (1)
typedef struct tagOFN_500EXA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
#ifdef _MAC
   LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
   void *       pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
} OPENFILENAME_500EXA, *LPOPENFILENAME_500EXA;

typedef struct tagOFN_500EXW {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCWSTR      lpstrFilter;
   LPWSTR       lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPWSTR       lpstrFile;
   DWORD        nMaxFile;
   LPWSTR       lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCWSTR      lpstrInitialDir;
   LPCWSTR      lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCWSTR      lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
#ifdef _MAC
   LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
   void *       pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
} OPENFILENAME_500EXW, *LPOPENFILENAME_500EXW;

#ifdef UNICODE
typedef OPENFILENAME_500EXW OPENFILENAME_500EX;
typedef LPOPENFILENAME_500EXW LPOPENFILENAME_500EX;
#else
typedef OPENFILENAME_500EXA OPENFILENAME_500EX;
typedef LPOPENFILENAME_500EXA LPOPENFILENAME_400EX;
#endif // UNICODE

// pragma pack ()

#if (WINVER < 0x0600)
#define DWM_EC_DISABLECOMPOSITION		0
#define DWM_EC_ENABLECOMPOSITION		1
#define WM_DWMCOMPOSITIONCHANGED		0x031E
#endif

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
	static bool s_bInitLateBindLibraryCallsDone;
	static bool m_bHasCachedCommonControlsVersion;
	static DWORD m_dwCachedCommonControlsVersion;
public:
	static void InitLateBindLibraryCalls();
	static bool IsMultiMonitorApiOk();
	static bool IsDwmApiOk();
	static bool DwmIsCompositionEnabled();
	static LPGETMONITORINFO s_pFnGetMonitorInfo;
	static LPMONITORFROMRECT s_pFnMonitorFromRect;
	static LPMONITORFROMWINDOW s_pFnMonitorFromWindow;
	static LPDWMISCOMPOSITIONENABLED s_pFnDwmIsCompositionEnabled;
	static LPDWMENABLECOMPOSITION s_pFnDwmEnableComposition;
	static LPDWMGETWINDOWATTRIBUTE s_pFnDwmGetWindowAttribute;

	static BOOL WaitMessageTimeout(DWORD dwTimeout);
	static HRESULT GetVersion_Res(LPTSTR filename, VS_FIXEDFILEINFO *p_vinfo);
	static HRESULT AnsiToUc(LPCSTR pszAnsi, LPWSTR pwszUc, int cAnsiCharsToConvert);
	static HRESULT AnsiToUc(LPCSTR pszAnsi, LPWSTR pwszUc, int cAnsiCharsToConvert, int& cchOut);
	static HRESULT AnsiToUcRequiredBufferLength(LPCSTR pszAnsi, int cAnsiCharsToConvert, int &cchOut);
	static HRESULT UcToAnsi(LPCWSTR pwszUc, LPSTR pszAnsi, int cWideCharsToConvert);
	static HRESULT UcToAnsi(LPCWSTR pwszUc, LPSTR pszAnsi, int cWideCharsToConvert, int& cchOut);
	static HRESULT UcToAnsiRequiredBufferLength(LPCWSTR pwszUc, int cWideCharsToConvert, int &cchOut);
	static BSTR AllocBStr(LPCTSTR pszString);
	static DLGTEMPLATE * WINAPI DoLockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName);
	static void ShowLastError(HWND);
	static TCHAR *GetLastWin32ErrorString();
	static void ArrangeOKCancel(HWND hwndDlg);
	static LPTSTR GetStringRes (int id);
	static BOOL CenterWindow (HWND hwndChild, HWND hwndParent);
	static HRESULT InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...);
	static bool IsEnhancedWinVer();
	static bool IsWindows7();
	static bool IsWindowsVistaOrLater();
	static bool IsWin98OrLater();
	static bool IsWinVerSupportInitializeCriticalSectionAndSpinCount();
	static bool IsMultiCore();
	static BOOL GetWindowRect6(HWND hWnd, LPRECT lpRect);
	static void PaintRect(HDC hdc, RECT *rect, COLORREF colour);
	static void AutoSetComboBoxHeight(HWND hWnd,  int maxHeight);
	static void AutoSetComboBoxHeight(HWND hWndParent, int controls[], int count, int maxHeight);
	static HRESULT GetClsidFromRegValue(HKEY hKey, LPCTSTR lpValueName, GUID *pId);
	static HRESULT SaveClsidToRegValue(HKEY hKey, LPCTSTR lpValueName, GUID *pId);

	static int Rand(int min, int max);
	static void InitOfn(OPENFILENAME_500EX& ofn, HWND hWnd, LPTSTR szTitle, TCHAR szInitialFile[], int chInitialFile, LPTSTR szFilter, TCHAR szReturnFile[], int chReturnFile);
	static void RectToWH(const RECT& rc, LONG& x, LONG& y, LONG& w, LONG& h);
	static BOOL DrawDefText(HDC hdc, int x, int y, LPCTSTR text, int len, int* nextx, int* nexty);
	static void GetWorkArea(RECT& rcWorkArea);
	static void GetMonitorWorkAreaFromWindow(HWND hWnd, RECT& rcWorkArea);
	static BOOL DrawBitmap (HDC hDC, INT x, INT y, HBITMAP hBitmap, DWORD dwROP);
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
	static HFONT CreateMonitorFont();
	static bool IsStringBlank(LPCTSTR ps);
	static bool IsWhiteSpace(TCHAR ch);
};

extern INT_PTR CALLBACK DialogProc(HWND hWndDlg, UINT uMsg,  WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
extern LRESULT CALLBACK MdiChildWindowProc(HWND hWnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
extern LRESULT CALLBACK MdiFrameWindowProc(HWND hWnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
extern LRESULT CALLBACK GlobalSubClassWindowProc(HWND hWnd, UINT uMsg,WPARAM wParam,LPARAM lParam);

class CVirWindow_EventSink_OnDestroy : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnDestroy(sender, e);
		return 0;
	}
	virtual void OnDestroy(void *sender, EventArgs& e) =0;
};

class CBaseVirWindow : public std::enable_shared_from_this<CBaseVirWindow>
{
public:
	CBaseVirWindow()
	{
		m_hInst = NULL;
		m_hWnd = NULL;
	};
	virtual ~CBaseVirWindow(){};

	HWND GetHwnd(void)
	{
		return(m_hWnd);
	}
	HINSTANCE GetHinstance(void)
	{
		if (m_hInst == 0)
			m_hInst = GetModuleHandle(NULL);
		return(m_hInst);
	}

	virtual int SetSize(int w, int h);
	virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	// Application instance handle.
	HINSTANCE m_hInst;
	// Handle of this window.
	HWND m_hWnd;
	// Application instance handle.

	std::shared_ptr<CBaseVirWindow> m_pKeepAlive;
	WNDPROC SubclassChildWindow(HWND hWnd);
	WNDPROC SubclassChildWindow(HWND hWnd, WNDPROC proc);
private:
	friend LRESULT CALLBACK ::GlobalSubClassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    CVirWindow

  Summary:  Abstract base class for wrapping a window.

            This class allows a window to be cleanly wrapped in a
            c++ class.  Specifically, it provides a way for a c++ class
            to use one of its methods as a WindowProc, giving it a "this"
            pointer and allowing it to have direct access to all of its
            private members.

  Methods:  Create:
              Maps to Windows' CreateWindow function.
            WindowProc:
              Pure virtual WindowProc for the window.
            Gethwnd:
              Get the handle to the window.
            CVirWindow:
              Constructor.
            ~CVirWindow:
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class CVirWindow : public CBaseVirWindow
{
public:
	CVirWindow()
	{
	};
	virtual ~CVirWindow(){};
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu) = 0;
	virtual void GetMinWindowSize(int &w, int &h);

	shared_ptr<CVirWindow> shared_from_this()
	{
		return std::static_pointer_cast<CVirWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	}
protected:
	// Envelopes the Windows' CreateWindow function call.
	HWND CreateVirWindow(
			DWORD dwExStyle,
			LPCTSTR lpszClassName,   // Address of registered class name
			LPCTSTR lpszWindowName,  // Address of window name
			DWORD dwStyle,          // Window style
			int x,                  // Horizontal position of window
			int y,                  // Vertical position of window
			int nWidth,             // Window width
			int nHeight,            // Window height
			HWND hWndParent,        // Handle of parent or owner window
			HMENU hmenu,            // Handle of menu, or child window identifier
			HINSTANCE hinst);       // Handle of application instance
	// WindowProc is a pure virtual member function and MUST be over-ridden
	// in any derived classes.
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
private:
	friend LRESULT CALLBACK ::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend LRESULT CALLBACK ::MdiChildWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CVirWindow> Sp_CVirWindow;
typedef weak_ptr<CVirWindow> Wp_CVirWindow;

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    CVirDialog

  Summary:  Abstract base class for wrapping a Windows dialog box window.

            This class allows a dialog box to be cleanly wrapped in
            a c++ class.  Specifically, it provides a way for a c++ class
            to use one of its methods as a DialogProc, giving it a "this"
            pointer and allowing it to have direct access to all of its
            private members.

  Methods:  ShowDialog:
              Maps to Windows' DialogBox function.
            GetHwnd:
              Get the handle to the dialog window.
            DialogProc:
              Pure virtual DialogProc for the dialog box
            ~CVirDialog:
              Destructor
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class CVirDialog : public CBaseVirWindow
{
public:	
	CVirDialog();

	// Destructor
	virtual ~CVirDialog(){};

	// ShowDialog creates the Dialog (using the DialogBoxParam API).
	virtual INT_PTR ShowDialog(
		HINSTANCE hInst,
		LPTSTR lpszTemplate,
		HWND hWndOwner);

	virtual HWND ShowModelessDialog(
		HINSTANCE hInst,
		LPCTSTR szTemplate,
		HWND hWndOwner);

	virtual HWND ShowModelessDialogIndirect(
		HINSTANCE hInst,
		LPCDLGTEMPLATE lpTemplate,
		HWND hWndOwner);
	// DialogProc is a pure virtual member function and MUST be over-ridden
	// in any derived classes.
	virtual BOOL DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam) = 0;

	shared_ptr<CVirDialog> shared_from_this()
	{
		return std::static_pointer_cast<CVirDialog, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	}
protected:
  bool m_bIsModeless;
  // Tell the compiler that the outside DialogProc callback is a friend
  // of this class and can get at its protected data members.
  friend INT_PTR CALLBACK ::DialogProc(
                           HWND hWndDlg,
                           UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam);
};

typedef shared_ptr<CVirDialog> Sp_CVirDialog;

class CVirMdiFrameWindow : public CVirWindow
{
public:	
	
	CVirMdiFrameWindow()
	{
		m_hWndMDIClient = NULL;
	}

	// Destructor
	virtual ~CVirMdiFrameWindow(){};


	HWND CreateMDIClientWindow(UINT clientId,  UINT firstChildId, int iWindowMenuIndex);
	// Get the protected handle of this window.

	HWND Get_MDIClientWindow()
	{
		return m_hWndMDIClient;
	}

	shared_ptr<CVirMdiFrameWindow> shared_from_this()
	{
		return std::static_pointer_cast<CVirMdiFrameWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	}
protected:
	virtual LRESULT MdiFrameWindowProc(
		HWND hWnd,
		HWND hWndMDIClient,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam) = 0;

	HWND m_hWndMDIClient;
private:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

  friend LRESULT CALLBACK ::MdiFrameWindowProc(
                           HWND hWnd,
                           UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam);
};

typedef shared_ptr<CVirMdiFrameWindow> Sp_CVirMdiFrameWindow;
typedef weak_ptr<CVirMdiFrameWindow> Wp_CVirMdiFrameWindow;

class CVirMdiChildWindow : public CBaseVirWindow
{
public:	
	// Destructor
	virtual ~CVirMdiChildWindow(){};

	shared_ptr<CVirMdiChildWindow> shared_from_this()
	{
		return std::static_pointer_cast<CVirMdiChildWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	}
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	friend LRESULT CALLBACK ::MdiChildWindowProc(
                           HWND hWnd,
                           UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam);
};

typedef struct {
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
	/*
    sz_Or_Ord menu;
    sz_Or_Ord windowClass;
    WCHAR title[titleLen];
    WORD pointsize;
    WORD weight;
    BYTE italic;
    BYTE charset;
    WCHAR typeface[stringLen];
	*/
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;

typedef const DLGTEMPLATEEX *LPCDLGTEMPLATEEX;
/*C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

class CTabDialog;

class CTabPageDialog : public CVirDialog
{
public:
	virtual ~CTabPageDialog();
	CTabPageDialog();
	HRESULT init(CTabDialog *,int,LPTSTR,int);
	virtual BOOL DialogProc(HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

	friend class CTabDialog;
	int m_pageindex;
protected:
private:
	CTabDialog *m_owner;
	LPCDLGTEMPLATE m_lpTemplate;
	LPCDLGTEMPLATEEX m_lpTemplateEx;
	int m_dlgId;
	LPTSTR m_pagetext;
	bool m_bIsCreated;
};

/*C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

struct tabpageitem
{
	int pageid;
	LPTSTR lpszText;
};

class CTabDialog : public CVirDialog
{
public:
	CTabDialog();
	HRESULT SetPages(int,struct tabpageitem *);
	void FreePages();
	void SetTabID(int tabctl_id);
	shared_ptr<CTabPageDialog> GetPage(int i);

	virtual BOOL OnTabbedDialogInit(HWND hwndDlg);
	virtual BOOL OnChildDialogInit(HWND hwndDlg);
	virtual BOOL OnSelChanged(HWND);
	virtual BOOL OnPageEvent(CTabPageDialog *page, HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
	virtual ~CTabDialog();
	virtual INT_PTR ShowDialog(HINSTANCE hInst,	LPTSTR lpszTemplate, HWND hWndOwner);
	virtual BOOL DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam) = 0;
	friend INT_PTR CALLBACK ::DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);
	int m_current_page_index;
protected:
	std::vector<shared_ptr<CTabPageDialog>>	m_vecpages;
	//int m_pagecount;
    HWND m_hwndTab;       // tab control 
    HWND m_hwndDisplay;   // current child dialog box 
    RECT m_rcDisplay;     // display rectangle for the tab control 
	int m_tabctl_id;
	HRESULT CreateAllPages();
	CDPI m_dpi;
};

/*C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/



#endif