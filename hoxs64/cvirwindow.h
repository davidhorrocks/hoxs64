#pragma once
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "utils.h"

extern INT_PTR CALLBACK DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK MdiChildWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK MdiFrameWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK GlobalSubClassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CVirWindow_EventSink_OnDestroy : public EventSink<EventArgs>
{
protected:
	int Sink(void* sender, EventArgs& e) override
	{
		OnDestroy(sender, e);
		return 0;
	}

	virtual void OnDestroy(void* sender, EventArgs& e) = 0;
};

class CBaseVirWindow// : protected enable_shared_from_this<CBaseVirWindow>
{
public:
	CBaseVirWindow() noexcept
	{
		m_hInst = 0;
		m_hWnd = 0;
	};

	CBaseVirWindow(const CBaseVirWindow&) = default;
	CBaseVirWindow& operator=(const CBaseVirWindow&) = default;
	CBaseVirWindow(CBaseVirWindow&&) = default;
	CBaseVirWindow& operator=(CBaseVirWindow&&) = default;
	virtual ~CBaseVirWindow() = default;

	HWND GetHwnd(void)
	{
		return(m_hWnd);
	}

	HINSTANCE GetHinstance(void)
	{
		if (m_hInst == 0)
		{
			m_hInst = GetModuleHandle(NULL);
		}

		return m_hInst;
	}

	virtual int SetSize(int w, int h);
	virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void WindowInUse() {}
	virtual void WindowRelease() {}
protected:
	HINSTANCE m_hInst = 0;
	HWND m_hWnd = 0;

	WNDPROC SubclassChildWindow(HWND hWnd);
	WNDPROC SubclassChildWindow(HWND hWnd, WNDPROC proc);
private:
	friend LRESULT CALLBACK::GlobalSubClassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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

	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x, int y, int w, int h, HMENU hMenu) = 0;

	virtual void GetMinWindowSize(int& w, int& h);

	//shared_ptr<CVirWindow> shared_from_this()
	//{
	//	//return static_pointer_cast<CVirWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	//	return static_pointer_cast<CVirWindow>(CBaseVirWindow::shared_from_this());
	//}
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
	friend LRESULT CALLBACK::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend LRESULT CALLBACK::MdiChildWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
	CVirDialog() noexcept;

	CVirDialog(const CVirDialog&) = default;

	CVirDialog& operator=(const CVirDialog&) = delete;

	CVirDialog(CVirDialog&&) = delete;

	CVirDialog& operator=(CVirDialog&&) = delete;

	~CVirDialog() = default;

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

	//shared_ptr<CVirDialog> shared_from_this()
	//{
	//	return static_pointer_cast<CVirDialog, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	//}
protected:
	bool m_bIsModeless;
	// Tell the compiler that the outside DialogProc callback is a friend
	// of this class and can get at its protected data members.
	friend INT_PTR CALLBACK::DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);
};

typedef shared_ptr<CVirDialog> Sp_CVirDialog;

class CVirMdiFrameWindow : public CVirWindow
{
public:

	CVirMdiFrameWindow() noexcept
	{
		m_hWndMDIClient = NULL;
	}

	CVirMdiFrameWindow(const CVirMdiFrameWindow&) = default;

	CVirMdiFrameWindow& operator=(const CVirMdiFrameWindow&) = default;

	CVirMdiFrameWindow(CVirMdiFrameWindow&&) = default;

	CVirMdiFrameWindow& operator=(CVirMdiFrameWindow&&) = default;

	~CVirMdiFrameWindow() = default;

	HWND CreateMDIClientWindow(UINT clientId, UINT firstChildId, int iWindowMenuIndex);
	// Get the protected handle of this window.

	HWND Get_MDIClientWindow()
	{
		return m_hWndMDIClient;
	}

	//shared_ptr<CVirMdiFrameWindow> shared_from_this()
	//{
	//	return static_pointer_cast<CVirMdiFrameWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	//}
protected:
	virtual LRESULT MdiFrameWindowProc(
		HWND hWnd,
		HWND hWndMDIClient,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam) = 0;

	HWND m_hWndMDIClient;
private:
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return 0;
	}

	friend LRESULT CALLBACK::MdiFrameWindowProc(
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
	//shared_ptr<CVirMdiChildWindow> shared_from_this()
	//{
	//	return static_pointer_cast<CVirMdiChildWindow, CBaseVirWindow>(CBaseVirWindow::shared_from_this());
	//}
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	friend LRESULT CALLBACK::MdiChildWindowProc(
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
} DLGTEMPLATEEX, * LPDLGTEMPLATEEX;

typedef const DLGTEMPLATEEX* LPCDLGTEMPLATEEX;
/*C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

class CTabDialog;

class CTabPageDialog : public CVirDialog
{
public:
	CTabPageDialog() noexcept;
	CTabPageDialog(const CTabPageDialog&) = default;
	CTabPageDialog& operator=(const CTabPageDialog&) = default;
	CTabPageDialog(CTabPageDialog&&) = default;
	CTabPageDialog& operator=(CTabPageDialog&&) = default;
	~CTabPageDialog() noexcept;
	HRESULT init(CTabDialog*, int, LPTSTR, int);
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	friend class CTabDialog;
	int m_pageindex;
protected:
private:
	CTabDialog* m_owner;
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
	CTabDialog() noexcept;
	CTabDialog(const CTabDialog&) = default;
	CTabDialog& operator=(const CTabDialog&) = default;
	CTabDialog(CTabDialog&&) = default;
	CTabDialog& operator=(CTabDialog&&) = default;
	~CTabDialog() noexcept;

	HRESULT SetPages(int, struct tabpageitem*);
	void FreePages() noexcept;
	void SetTabID(int tabctl_id);
	shared_ptr<CTabPageDialog> GetPage(int i);
	virtual BOOL OnTabbedDialogInit(HWND hwndDlg);
	virtual BOOL OnChildDialogInit(HWND hwndDlg);
	virtual BOOL OnSelChanged(HWND);
	virtual BOOL OnPageEvent(CTabPageDialog* page, HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	INT_PTR ShowDialog(HINSTANCE hInst, LPTSTR lpszTemplate, HWND hWndOwner) override;
	BOOL DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam) = 0;
	friend INT_PTR CALLBACK::DialogProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);
	int m_current_page_index;
protected:
	vector<shared_ptr<CTabPageDialog>>	m_vecpages;
	//int m_pagecount;
	HWND m_hwndTab;       // tab control 
	HWND m_hwndDisplay;   // current child dialog box 
	RECT m_rcDisplay;     // display rectangle for the tab control 
	int m_tabctl_id;
	HRESULT CreateAllPages();
	CDPI m_dpi;
};
