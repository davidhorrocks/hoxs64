#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <assert.h>
#include "defines.h"
#include "carray.h"
#include "mlist.h"
#include "CDPI.h"
#include <d3d9.h>
#include "utils.h"

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: WindowProc

	Summary:  Standard WindowProc callback function that forwards Windows
			messages on to the CVirWindow::WindowProc method.  This
			Window procedure expects that it will receive a "this"
			pointer as the lpCreateParams member passed as part of the
			WM_NCCREATE message.  It saves the "this" pointer in the
			GWL_USERDATA field of the window structure.

	Args:     HWND hWnd,
				Window handle.
			UINT uMsg,
				Windows message.
			WPARAM wParam,
				First message parameter (word sized).
			LPARAM lParam);
				Second message parameter (long sized).

	Returns:  LRESULT.  Windows window procedure callback return value.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
LRESULT CALLBACK WindowProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	// Get a pointer to the window class object.
	CVirWindow* pWin = (CVirWindow*) (LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE:
		// Since this is the first time that we can get ahold of
		// a pointer to the window class object, all messages that might
		// have been sent before this are never seen by the Windows object
		// and only get passed on to the DefWindowProc

		// Get the initial creation pointer to the window object
		pWin = (CVirWindow *) ((CREATESTRUCT *)lParam)->lpCreateParams;

		// Set it's protected m_hWnd member variable to ensure that
		// member functions have access to the correct window handle.
		pWin->m_hWnd = hWnd;

		// Set its USERDATA to point to the window object
		#pragma warning(disable:4244)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWin);
		#pragma warning(default:4244)
		break;

	case WM_DESTROY:
		// This is our signal to destroy the window object.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		if (pWin)
		{
			EventArgs e;
			pWin->EsOnDestroy.Raise(pWin, e);
			LRESULT lr = pWin->WindowProc(hWnd, uMsg, wParam, lParam);
			pWin->m_hWnd = 0;
			if (pWin->m_AutoDelete)
			{
				delete pWin;
			}
			pWin = NULL;
			return lr;
		}
		break;
	default:
		break;
	}

	// Call its message proc method.
	if (NULL != pWin)
		return (pWin->WindowProc(hWnd, uMsg, wParam, lParam));
	else
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

LRESULT CALLBACK GlobalSubClassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
CVirWindow* pWin = NULL;
	if (hWnd != NULL)
	{
		pWin = (CVirWindow*) (LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (NULL != pWin)
			return (pWin->SubclassWindowProc(hWnd, uMsg, wParam, lParam));
		
		//HWND hWndParent = GetParent(hWnd);
		//if (hWndParent)
		//{
		//	pWin = (CVirWindow*) (LONG_PTR)GetWindowLongPtr(hWndParent, GWLP_USERDATA);
		//	if (NULL != pWin)
		//		return (pWin->SubclassWindowProc(hWnd, uMsg, wParam, lParam));
		//}
	}
	return 0;
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Method:   CVirWindow::Create

	Summary:  Envelopes the Windows' CreateWindow call.  Uses its
			window-creation data pointer to pass the 'this' pointer.

	Args:     LPTSTR lpszClassName,
				Address of registered class name.
			LPTSTR lpszWindowName,
				Address of window name/title.
			DWORD dwStyle,
				Window style.
			int x,
				Horizontal position of window.
			int y,
				Vertical position of window.
			int nWidth,
				Window width.
			int nHeight,
				Window height.
			HWND hwndParent,
				Handle of parent or owner window.
			HMENU hmenu,
				Handle of menu, or child window identifier.
			HINSTANCE hinst)
				Handle of application instance.

	Modifies: m_hWnd, m_hInst.

	Returns:  HWND (Window handle) of the created window.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HWND CVirWindow::CreateVirWindow(
	DWORD dwExStyle,
	LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInst)
{
	// Remember the passed instance handle in a member variable of the
	// C++ Window object.
	m_hInst = hInst;

	// Call the Win32 API to create the window.
	m_hWnd = ::CreateWindowEx(
		dwExStyle,
		lpszClassName,
		lpszWindowName,
		dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInst,
		this);

	return (m_hWnd);
}

void CVirWindow::GetMinWindowSize(int &w, int &h)
{
	w=0;
	h=0;
}

HWND CVirWindow::CreateMDIClientWindow(UINT clientId,  UINT firstChildId)
{
CLIENTCREATESTRUCT ccs; 
	// Retrieve the handle to the window menu and assign the 
	// first child window identifier. 
	ZeroMemory(&ccs , sizeof(ccs));

	ccs.hWindowMenu = GetSubMenu(GetMenu(m_hWnd), 1); //WINDOWMENU
	ccs.idFirstChild = firstChildId; 

	// Create the MDI client window. 
	m_hWndMDIClient = ::CreateWindowEx(0, TEXT("MDICLIENT"), (LPCTSTR) NULL, 
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL, 
		0, 0, 0, 0, m_hWnd, (HMENU) LongToPtr(clientId), m_hInst, (LPSTR) &ccs); 
	return m_hWndMDIClient;
}

WNDPROC CVirWindow::SubclassChildWindow(HWND hWnd)
{
	#pragma warning(disable:4244)
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) this);
	WNDPROC pOldProc = (WNDPROC) (LONG_PTR)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) ::GlobalSubClassWindowProc);
	#pragma warning(default:4244)
	return pOldProc;
}

WNDPROC CVirWindow::SubclassChildWindow(HWND hWnd, WNDPROC proc)
{
	#pragma warning(disable:4244)
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) this);
	WNDPROC pOldProc = (WNDPROC) (LONG_PTR)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) proc);
	#pragma warning(default:4244)
	return pOldProc;
}

LRESULT CVirWindow::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CVirWindow::SetSize(int w, int h)
{
	return (int)SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: DialogProc

	Summary:  The general dialog procedure callback function.  Used by all
			CVirDialog class objects.  This procedure is the DialogProc.
			registered for all dialogs created with the CVirDialog class.
			It uses the parameter passed with the WM_INITDIALOG message
			to identify the dialog classes' "this" pointer which it then
			stores in the window structure's GWL_USERDATA field.
			All subsequent messages can then be forwarded on to the
			correct dialog class's DialogProc method by using the pointer
			stored in the GWL_USERDATA field.

	Args:     HWND hWndDlg,
				Handle of dialog box.
			UINT uMsg,
				Message.
			WPARAM wParam,
				First message parameter (word sized).
			LPARAM lParam);
				Second message parameter (long sized).

	Returns:  BOOL.  Return of the CVirDialog::DialogProc method.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
INT_PTR CALLBACK DialogProc(
				HWND hWndDlg,
				UINT uMsg,
				WPARAM wParam,
				LPARAM lParam)
{

	// Get a pointer to the window class object.
	CVirDialog* pdlg = (CVirDialog*) (LONG_PTR)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// Get a pointer to the window class object.
		pdlg = (CVirDialog*) lParam;

		// Assign the m_hWnd member variable.
		pdlg->m_hWnd = hWndDlg;

		// Set the USERDATA to point to the class object.
		#pragma warning(disable:4244)
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR) pdlg);
		#pragma warning(default:4244)
		break;

	default:
		break;
	}

	// Call its message proc method.
	if (pdlg != (CVirDialog *) 0)
	return (pdlg->DialogProc(hWndDlg, uMsg, wParam, lParam));
	else
	return (FALSE);
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Method:   CVirDialog::ShowDialog

	Summary:  Creates the dialog so that it's DialogProc member function can
			be invoked.  The dialog box object exists until deleted by the
			caller.  It can be shown any number of times.  This function is
			analgous to Windows' DialogBox function.  The main difference
			being that you don't specify a DialogProc; you override the
			pure virtal function CVirDialog::DialogProc.

	Args:     HINSTANCE hInst,
				Handle of the module instance.  Needed to specify the
				module instance for fetching the dialog template resource
				(ie, from either a host EXE or DLL).
			LPTSTR lpszTemplate,
				Identifies the dialog box template.
			HWND hwndOwner)
				Handle of the owner window.

	Modifies: m_hInst.

	Returns:  Return value from the DialogBoxParam Windows API function.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
INT_PTR CVirDialog::ShowDialog(
		HINSTANCE hInst,
		LPTSTR lpszTemplate,
		HWND hWndOwner)
{
	INT_PTR iResult;

	// Assign the module instance handle in the Dialog object.
	m_hInst = hInst;

	// Create and show the dialog on screen.  Load the dialog resource
	// from the specified module instance (could be a module other than
	// that of the EXE that is running--the resources could be in a DLL
	// that is calling this ShowDialog).  Pass the 'this' pointer to the
	// dialog object so that it can be assigned inside the dialog object
	// during WM_INITDIALOG and later available to the dailog procedure
	// via the GWL_USERDATA associated with the dialog window.
	iResult = ::DialogBoxParam(
				hInst,
				lpszTemplate,
				hWndOwner,
				(DLGPROC)::DialogProc,
				(LPARAM)this);

	return (iResult);
}
/*M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HWND CVirDialog::ShowModelessDialog(
		HINSTANCE hInst,
		LPCTSTR szTemplate,
		HWND hWndOwner)
{
	HWND iResult;

	m_hInst = hInst;

	iResult = ::CreateDialogParam(
				hInst,
				szTemplate,
				hWndOwner,
				(DLGPROC)::DialogProc,
				(LPARAM)this);

	return (iResult);
}
/*M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HWND CVirDialog::ShowModelessDialogIndirect(
		HINSTANCE hInst,
		LPCDLGTEMPLATE lpTemplate,
		HWND hWndOwner)
{
	HWND iResult;

	m_hInst = hInst;

	iResult = ::CreateDialogIndirectParam(
				hInst,
				lpTemplate,
				hWndOwner,
				(DLGPROC)::DialogProc,
				(LPARAM)this);

	return (iResult);
}

HWND CVirDialog::GetHwnd()
{
	return m_hWnd;
}

/**************************************************************************/

CTabPageDialog::CTabPageDialog()
{
	m_hInst=0;
	m_hWnd=0;
	m_lpTemplate=0;
	m_lpTemplateEx=0;
	m_pagetext = NULL;
	m_bIsCreated = false;
}

CTabPageDialog::~CTabPageDialog()
{
	if (m_pagetext)
	{
		delete [] m_pagetext;
		m_pagetext=NULL;
	}

}

HRESULT CTabPageDialog::init(CTabDialog *owner, int dlgId, LPTSTR szText,int pageindex)
{
	m_owner = owner;
	m_dlgId = dlgId;
	m_pageindex = pageindex;
	m_pagetext = new TCHAR[lstrlen(szText)+1];
	if (m_pagetext == NULL)
		return E_OUTOFMEMORY;

	lstrcpy(m_pagetext, szText);


	m_lpTemplate = G::DoLockDlgRes(m_hInst, MAKEINTRESOURCE(dlgId));
	if (m_lpTemplate)
	{
		m_lpTemplateEx = (LPCDLGTEMPLATEEX)m_lpTemplate;
		if (m_lpTemplateEx->signature == 0xffff)
			m_lpTemplate = 0;
		else
			m_lpTemplateEx = 0;
		return S_OK;
	}
	else
		return E_FAIL;
}

BOOL CTabPageDialog::DialogProc(HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return m_owner->OnPageEvent(this, hWndDlg, uMsg, wParam, lParam);
}


/**************************************************************************/

CTabDialog::CTabDialog()
{
	m_pagecount=0;
	m_hwndDisplay=0;
	m_hWnd=0;
	m_hInst=0;
	m_hwndTab=0;
	m_pages=NULL;
	m_tabctl_id=0;
	m_current_page_index = -1;
}

CTabDialog::~CTabDialog()
{
	FreePages();
}

void CTabDialog::FreePages()
{

	if (m_pages)
	{
		delete [] m_pages;
		m_pages = NULL;
		m_pagecount = 0;
	}
}

INT_PTR CTabDialog::ShowDialog(HINSTANCE hInst,	LPTSTR lpszTemplate, HWND hWndOwner)
{
	INT_PTR iResult;

	// Assign the module instance handle in the Dialog object.
	m_hInst = hInst;

	iResult = ::DialogBoxParam(hInst,
				lpszTemplate,
				hWndOwner,
				(DLGPROC)::DialogProc,
				(LPARAM)this);

	return (iResult);
}

void CTabDialog::SetTabID(int tabctl_id)
{
	m_tabctl_id = tabctl_id;
}

HRESULT CTabDialog::SetPages(int pagecount, struct tabpageitem *pageitem)
{
int i;
HRESULT hr;

	FreePages();
	m_pages = new CTabPageDialog[pagecount];
	if (m_pages==0)
		return E_OUTOFMEMORY;
	m_pagecount = pagecount;

	for (i=0 ; i<pagecount ; i++)
	{
		hr = m_pages[i].init(this, pageitem[i].pageid, pageitem[i].lpszText, i);
		if (FAILED(hr))
		{
			FreePages();
			return hr;
		}
	}
	return S_OK;
}

BOOL CTabDialog::OnChildDialogInit(HWND hwndDlg) 
{ 
	HWND hwndParent = GetParent(hwndDlg); 
	if (hwndParent==NULL)
		return FALSE;
	SetWindowPos(hwndDlg, HWND_TOP, 
		m_rcDisplay.left, m_rcDisplay.top, 
		0, 0, SWP_NOSIZE); 
	return TRUE;
} 

BOOL CTabDialog::OnTabbedDialogInit(HWND hwndDlg) 
{ 
	DWORD dwDlgBase = GetDialogBaseUnits(); 
	TCITEM tie; 
	RECT rcTab,rcTemp; 
	HWND hwndButton; 
	RECT rcButton; 
	int i; 
	BOOL bResult=FALSE;
	// Create the tab control. 
	InitCommonControls(); 

	if (m_tabctl_id)
	{
		m_hwndTab = GetDlgItem(hwndDlg, m_tabctl_id);
		if (m_hwndTab == NULL) {
			return FALSE;
		}
	}
	else
		return FALSE;

	SetRectEmpty(&rcTemp);
	rcTemp.right=1;
	rcTemp.bottom=1;
	MapDialogRect(hwndDlg, &rcTemp);
	int cxMargin = abs(rcTemp.right - rcTemp.left);
	int cyMargin = abs(rcTemp.bottom - rcTemp.top);

	// Add a tab for each of the three child dialog boxes. 
	for (i = m_pagecount -1; i >= 0; i--) 
	{ 
		tie.mask = TCIF_TEXT | TCIF_IMAGE; 
		tie.iImage = -1; 
		tie.pszText = m_pages[i].m_pagetext; 
		TabCtrl_InsertItem(m_hwndTab, 0, &tie); 
	}
 
	// Lock the resources for the three child dialog boxes. 
	//apRes[0] = DoLockDlgRes(m_hInst, MAKEINTRESOURCE(IDD_KEYPAGE1)); 
 
	// Determine the bounding rectangle for all child dialog boxes. 
	SetRectEmpty(&rcTab);
	for (i = 0; i < m_pagecount; i++)
	{ 
		if (m_pages[i].m_lpTemplate != 0)
		{
			if (m_pages[i].m_lpTemplate->cx > rcTab.right)
				rcTab.right = m_pages[i].m_lpTemplate->cx; 
			if (m_pages[i].m_lpTemplate->cy > rcTab.bottom)
				rcTab.bottom = m_pages[i].m_lpTemplate->cy;
		}
		else if (m_pages[i].m_lpTemplateEx != 0)
		{
			if (m_pages[i].m_lpTemplateEx->cx > rcTab.right) 
				rcTab.right = m_pages[i].m_lpTemplateEx->cx; 
			if (m_pages[i].m_lpTemplateEx->cy > rcTab.bottom) 
				rcTab.bottom = m_pages[i].m_lpTemplateEx->cy; 
		}
	}

	MapDialogRect(hwndDlg ,&rcTab);
 
	// Calculate how large to make the tab control, so 
	// the display area can accommodate all the child dialog boxes. 
	TabCtrl_AdjustRect(m_hwndTab, TRUE, &rcTab); 

	OffsetRect(&rcTab, cxMargin - rcTab.left, cyMargin - rcTab.top); 
 
	// Calculate the display rectangle. 
	CopyRect(&m_rcDisplay, &rcTab); 
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &m_rcDisplay); 
 
	// Set the size and position of the tab control, buttons, 
	// and dialog box. 
	SetWindowPos(m_hwndTab, NULL, rcTab.left, rcTab.top, 
			rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, 
			SWP_NOZORDER); 
 
	// Move the first button below the tab control. 
	hwndButton = GetDlgItem(hwndDlg, IDOK); 
	SetWindowPos(hwndButton, NULL, 
			rcTab.left, rcTab.bottom + cyMargin, 0, 0, 
			SWP_NOSIZE | SWP_NOZORDER); 
 
	// Determine the size of the button. 
	GetWindowRect(hwndButton, &rcButton); 
	rcButton.right -= rcButton.left; 
	rcButton.bottom -= rcButton.top; 
 
	// Move the second button to the right of the first. 
	hwndButton = GetDlgItem(hwndDlg, IDCANCEL); 
	SetWindowPos(hwndButton, NULL, 
		rcTab.left + rcButton.right + cxMargin, 
		rcTab.bottom + cyMargin, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER); 
 
	// Size the dialog box. 
	SetWindowPos(hwndDlg, NULL, 0, 0, 
		rcTab.right + cyMargin + 
		2 * GetSystemMetrics(SM_CXDLGFRAME), 
		rcTab.bottom + rcButton.bottom + 2 * cyMargin + 
		2 * GetSystemMetrics(SM_CYDLGFRAME) + 
		GetSystemMetrics(SM_CYCAPTION), 
		SWP_NOMOVE | SWP_NOZORDER); 
 
	// Simulate selection of the first item. 
	TabCtrl_SetCurSel(m_hwndTab, 0);

	return OnSelChanged(hwndDlg); 
} 

BOOL CTabDialog::OnSelChanged(HWND hwndDlg) 
{ 
int iSel;
	// Destroy the current child dialog box, if any. 
	if (IsWindow(m_hwndDisplay)) 
	{
		ShowWindow(m_hwndDisplay, SW_HIDE);
		UpdateWindow(m_hwndDisplay);
	}
	m_hwndDisplay=NULL;

	iSel = m_current_page_index = TabCtrl_GetCurSel(m_hwndTab); 

	if (iSel<0)
		return FALSE;

	if (m_pagecount<1)
		return FALSE;

	if (!m_pages[iSel].m_bIsCreated)
	{
		// Create the new child dialog box. 
		if (m_pages[iSel].m_lpTemplate !=0)
			m_hwndDisplay = CreateDialogIndirectParam (m_hInst, m_pages[iSel].m_lpTemplate, hwndDlg, ::DialogProc, (LPARAM) &(this->m_pages[iSel])); 
		else if (m_pages[iSel].m_lpTemplateEx !=0)
			m_hwndDisplay = CreateDialogIndirectParam (m_hInst, (LPCDLGTEMPLATE)m_pages[iSel].m_lpTemplateEx, hwndDlg, ::DialogProc, (LPARAM) &(this->m_pages[iSel])); 
		if (m_hwndDisplay)
			m_pages[iSel].m_bIsCreated = true;
	}
	else
	{
		m_hwndDisplay = m_pages[iSel].m_hWnd;
	}
	if (m_hwndDisplay != NULL)
	{
		RECT rcDisplay;
		ZeroMemory(&rcDisplay, sizeof(rcDisplay));			
		if (GetWindowRect(m_hwndTab, &rcDisplay))
		{
			if (ScreenToClient(m_hwndTab, (LPPOINT)&rcDisplay.left))
			{
				if (ScreenToClient(m_hwndTab, (LPPOINT)&rcDisplay.right))
				{
					TabCtrl_AdjustRect(m_hwndTab, FALSE, &rcDisplay);
					SetWindowPos(m_hwndDisplay, NULL, rcDisplay.left, rcDisplay.top, rcDisplay.right - rcDisplay.left, rcDisplay.bottom - rcDisplay.top, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOMOVE);
				}
			}
		}
		ShowWindow(m_hwndDisplay, SW_SHOW);
		UpdateWindow(m_hwndDisplay);
	}

	if (m_hwndDisplay)
		return TRUE;
	else
		return FALSE;
}

CTabPageDialog *CTabDialog::GetPage(int i)
{
	return &m_pages[i];
}

HRESULT CTabDialog::CreateAllPages()
{
int i;
HRESULT hr = S_OK;
HWND hWnd;
	for (i=0 ; i<m_pagecount ; i++)
	{
		if (!m_pages[i].m_bIsCreated)
		{
			if (m_pages[i].m_lpTemplate !=0)
				hWnd = CreateDialogIndirectParam (m_hInst, m_pages[i].m_lpTemplate, m_hWnd, ::DialogProc, (LPARAM) &(this->m_pages[i])); 
			else if (m_pages[i].m_lpTemplateEx !=0)
				hWnd = CreateDialogIndirectParam (m_hInst, (LPCDLGTEMPLATE)m_pages[i].m_lpTemplateEx, m_hWnd, ::DialogProc, (LPARAM) &(this->m_pages[i])); 
			if (hWnd)
			{
				m_pages[i].m_bIsCreated = true;
				ShowWindow(hWnd, SW_HIDE);
			}
			else
				hr = E_FAIL;
		}
	}
	return hr;
}

/**************************************************************************/

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: AnsiToUc

	Summary:  Convert an ANSI 'multibyte' string into a UNICODE 'wide
			character' string.

	Args:     LPSTR pszAnsi
				Pointer to a caller's input ANSI string.
			LPWSTR pwszUc
				Pointer to a caller's output UNICODE wide string.
			int cAnsiCharsToConvert
				Character count. If 0 then use length of pszAnsi.

	Returns:  HRESULT
				Standard result code. NOERROR for success.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
HRESULT G::AnsiToUc(
			LPCSTR pszAnsi,
			LPWSTR pwszUc,
			int cAnsiCharsToConvert)
{
	int chout = 0;
	return AnsiToUc(pszAnsi, pwszUc, cAnsiCharsToConvert, chout);
}

HRESULT G::AnsiToUc(
			LPCSTR pszAnsi,
			LPWSTR pwszUc,
			int cAnsiCharsToConvert,
			int& chOut)
{
HRESULT hr = E_POINTER;
int cSize;
int cOut;
int cch;

	chOut = 0;
	if (NULL != pszAnsi && NULL != pwszUc)
	{
		if (0 == cAnsiCharsToConvert)
			cch = -1;
		else
			cch = cAnsiCharsToConvert;

		cSize = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cch, NULL, 0);

		if (0 != cSize)
		{
			cOut = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cch, pwszUc, cSize);

			if (0 == cOut)
				hr = E_FAIL;
			else
			{
				hr = NOERROR;
				chOut = cOut;
			}
		}
		else
			hr = E_FAIL;
	}

	return hr;
}

HRESULT G::AnsiToUcRequiredBufferLength(
			LPCSTR pszAnsi,
			int cAnsiCharsToConvert,
			int &cOut
			)
{
HRESULT hr = E_POINTER;
int cSize;
int cch;
	if (NULL != pszAnsi)
	{
		if (0 == cAnsiCharsToConvert)
			cch = -1;
		else
			cch = cAnsiCharsToConvert;

		cSize = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cch, NULL, 0);
		if (0 != cSize)
		{
			cOut = cSize;
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}
	return hr;
}

/*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
	Function: UcToAnsi

	Summary:  Convert a UNICODE 'wide character' input string to an output
			ANSI 'multi-byte' string.

	Args:     LPWSTR pwszUc
				Pointer to a caller's input UNICODE wide string.
			LPSTR pszAnsi
				Pointer to a caller's output ANSI string.
			int cWideCharsToConvert
				Character count. If 0 then use length of pszUc.

	Returns:  HRESULT
				Standard result code. NOERROR for success.
F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
HRESULT G::UcToAnsi(
			LPCWSTR pwszUc,
			LPSTR pszAnsi,
			int cWideCharsToConvert)
{
HRESULT hr = E_POINTER;
int cSize;
int cOut;
int cch;
	if (NULL != pszAnsi && NULL != pwszUc)
	{

		if (0 == cWideCharsToConvert)
			cch = -1;
		else
			cch = cWideCharsToConvert;

		cSize = WideCharToMultiByte(CP_ACP,0,pwszUc,cch,NULL,0,NULL,NULL);

		if (0 != cSize)
		{
			cOut = WideCharToMultiByte(CP_ACP,0,pwszUc,-1,pszAnsi,cSize,NULL,NULL);
			if (0 != cOut)
				hr = NOERROR;
		}
		else
			hr = E_FAIL;
	}
	return hr;
}

HRESULT G::UcToAnsiRequiredBufferLength(LPCWSTR pwszUc, int cWideCharsToConvert, int &cOut)
{
HRESULT hr = E_POINTER;
int cSize;
int cch;

	if (NULL != pwszUc)
	{

		if (0 == cWideCharsToConvert)
			cch = -1;
		else
			cch = cWideCharsToConvert;

		cSize = WideCharToMultiByte(CP_ACP,0,pwszUc,cch,NULL,0,NULL,NULL);

		if (0 != cSize)
		{
			cOut = cSize;
			hr = S_OK;
		}
		else
			hr = E_FAIL;
	}
	return hr;
}

DLGTEMPLATE * WINAPI G::DoLockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName) 
{ 
	HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 
	if (hrsrc)
	{
		HGLOBAL hglb = LoadResource(hinst, hrsrc); 
		if (hglb)
			return (DLGTEMPLATE *) LockResource(hglb); 
	}
	return NULL;
} 


/*****************************************************************/

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
			::MessageBox( hWnd, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	}
}

TCHAR *G::GetLastWin32ErrorString()
{
TCHAR *lpMsgBuf=NULL;
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
HWND hwndButton;
RECT rcButton;
	// Move the first button below the tab control. 
	hwndButton = GetDlgItem(hwndDlg, IDOK); 
	GetWindowRect(hwndButton, &rcButton); 
	ScreenToClient(hwndDlg, (LPPOINT)&rcButton.left);
	ScreenToClient(hwndDlg, (LPPOINT)&rcButton.right);
	rcButton.right -= rcButton.left; 
	rcButton.bottom -= rcButton.top; 

	// Move the second button to the right of the first. 
	hwndButton = GetDlgItem(hwndDlg, IDCANCEL); 
	SetWindowPos(hwndButton, NULL, 
		rcButton.left + rcButton.right + 1, 
		rcButton.top, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER); 
}

/*
BOOL G::IsWindowEnabled(HWND hWnd, bool &bResult)
{
WINDOWINFO wi;
	memset(&wi, 0, sizeof(WINDOWINFO));
	wi.cbSize = sizeof(WINDOWINFO);
	BOOL r = GetWindowInfo(hWnd, &wi);
	if (r)
	{
		bResult = (wi.dwStyle & WS_DISABLED)==0;
	}
	return r;
}
VS_FIXEDFILEINFO *G::GetVersion_Res_Old()
{
void *pv=NULL;
HRSRC v=0;
HGLOBAL hv=NULL;
UINT l=0;
VS_FIXEDFILEINFO *p_vinfo=NULL;
	v=FindResource(NULL,MAKEINTRESOURCE(VS_VERSION_INFO),RT_VERSION);
	if (v)
	{
		hv=LoadResource(NULL,v);
		if (hv)
		{
			pv=LockResource(hv);
			if (pv)
			{
				//l=sizeof(VS_FIXEDFILEINFO);
				if (VerQueryValue(pv,TEXT("\\"),(void **) &p_vinfo, &l))
				{
					return p_vinfo;
				}
			}
		}
	}
	return NULL;
}
*/

LPGETMONITORINFO G::s_pFnGetMonitorInfo = NULL;
LPMONITORFROMRECT G::s_pFnMonitorFromRect = NULL;
LPMONITORFROMWINDOW G::s_pFnMonitorFromWindow = NULL;
LPDWMISCOMPOSITIONENABLED G::s_pFnDwmIsCompositionEnabled = NULL;
LPDWMENABLECOMPOSITION G::s_pFnDwmEnableComposition = NULL;

bool G::m_bHasCachedCommonControlsVersion = false;
DWORD G::m_dwCachedCommonControlsVersion = 0;
bool G::s_bInitLateBindLibraryCallsDone = false;
void G::InitLateBindLibraryCalls()
{
	if (s_bInitLateBindLibraryCallsDone)
		return;
	HMODULE hUser32 = GetModuleHandle(TEXT("USER32"));
	if (hUser32 ) 
	{
		OSVERSIONINFOA osvi = {0}; osvi.dwOSVersionInfoSize = sizeof(osvi); GetVersionExA((OSVERSIONINFOA*)&osvi);
		bool bNT = (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId); 

		s_pFnGetMonitorInfo = (LPGETMONITORINFO) (bNT ? GetProcAddress(hUser32, "GetMonitorInfoW") : GetProcAddress(hUser32, "GetMonitorInfoA"));
		s_pFnMonitorFromRect = (LPMONITORFROMRECT) (GetProcAddress(hUser32, "MonitorFromRect"));
		s_pFnMonitorFromWindow = (LPMONITORFROMWINDOW) (GetProcAddress(hUser32, "MonitorFromWindow"));

	}
	if (G::IsWindowsVistaOrLater())
	{
		HMODULE hDwmapi = LoadLibrary(TEXT("Dwmapi.dll"));
		if (hDwmapi) 
		{
			s_pFnDwmIsCompositionEnabled = (LPDWMISCOMPOSITIONENABLED) GetProcAddress(hDwmapi, "DwmIsCompositionEnabled");
			s_pFnDwmEnableComposition = (LPDWMENABLECOMPOSITION) GetProcAddress(hDwmapi, "DwmEnableComposition");
		}
	}
	s_bInitLateBindLibraryCallsDone = true;
}

bool G::IsMultiMonitorApiOk()
{
	return s_pFnGetMonitorInfo!=NULL && s_pFnMonitorFromRect != NULL && s_pFnMonitorFromWindow != NULL;
}

bool G::IsDwmApiOk()
{
	return s_pFnDwmIsCompositionEnabled!=NULL && s_pFnDwmEnableComposition != NULL;
}

BOOL G::WaitMessageTimeout(DWORD dwTimeout)
{
	return MsgWaitForMultipleObjects(0, NULL, FALSE, dwTimeout, QS_ALLINPUT) == WAIT_TIMEOUT;
}

HRESULT G::GetVersion_Res(LPTSTR filename, VS_FIXEDFILEINFO *p_vinfo)
{
void *pv=NULL;
HRSRC v=0;
HGLOBAL hv=NULL;
UINT l=0;
DWORD lDummy = 0;
VS_FIXEDFILEINFO *p_vinfotmp=NULL;
HRESULT hr = E_FAIL;

	l = GetFileVersionInfoSize(filename, &lDummy);
	if (l==0)
		return E_FAIL;

	pv = malloc(l);
	if (!pv)
		return E_FAIL;

	if (GetFileVersionInfo(filename, 0, l, pv))
	{
		if (VerQueryValue(pv, TEXT("\\"), (LPVOID *)&p_vinfotmp, &l))
		{
			*p_vinfo = *p_vinfotmp;
			hr = S_OK;
		}
	}
	if (pv)
		free(pv);

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

HRESULT G::InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...)
{
	TCHAR            szBuff[302];
	va_list         vl;

	va_start(vl, szError);
	_vsntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, szError, vl);
	szBuff[299]=0;
	MessageBox(hWnd, szBuff, APPNAME, MB_OK | MB_ICONEXCLAMATION);
	va_end(vl);
	return hRet;
}

void G::InitOfn(OPENFILENAME_500EX& ofn, HWND hWnd, LPTSTR szTitle, TCHAR szInitialFile[], int chInitialFile, LPTSTR szFilter, TCHAR szReturnFile[], int chReturnFile)
{
	ZeroMemory(&ofn, sizeof(ofn));
	if (G::IsEnhancedWinVer())
		ofn.lStructSize = sizeof(OPENFILENAME_500EX);
	else
		ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	if (szFilter!=NULL)
	{
		ofn.lpstrFilter = szFilter;
		ofn.nFilterIndex =1;
	}
	if (szInitialFile!=NULL)
	{
		ofn.lpstrFile = szInitialFile;
		ofn.nMaxFile = chInitialFile;
	}
	if (szReturnFile!=NULL)
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

bool G::IsEnhancedWinVer()
{
	OSVERSIONINFO ver;
	memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (! ::GetVersionEx(&ver) )
	{
		return false;
	}

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ver.dwMajorVersion >= 5)
			return true;
	}
	else if ( (ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
	{
		if ((ver.dwMajorVersion == 4) && (ver.dwMinorVersion >= 90))
			return true;
	}
	return false;
}

bool G::IsWindows7()
{
	OSVERSIONINFO ver;
	memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (! ::GetVersionEx(&ver) )
	{
		return false;
	}

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ver.dwMajorVersion >= 6 || (ver.dwMajorVersion == 6 && ver.dwMinorVersion >= 1))
			return true;
	}
	return false;
}

bool G::IsWindowsVistaOrLater()
{
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
}


bool G::IsWinVerSupportInitializeCriticalSectionAndSpinCount()
{
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
}

bool G::IsWin98OrLater()
{
	OSVERSIONINFO ver;
	memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (! ::GetVersionEx(&ver) )
	{
		return false;
	}

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ver.dwMajorVersion >= 5)
			return true;
	}
	else if (ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		if ((ver.dwMajorVersion == 4) && (ver.dwMinorVersion >= 10))
			return true;
	}
	return false;
}

bool G::IsMultiCore()
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
					c++;
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
			G::AutoSetComboBoxHeight(hWndCbo, maxHeight);
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
		return;
	lr = SendMessage(hWnd, CB_GETITEMHEIGHT, 0, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
		return;
	itemHeight = (int)lr;

	lr = SendMessage(hWnd, CB_GETITEMHEIGHT, -1, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
		return;
	editHeight = (int)lr;

	lr = SendMessage(hWnd, CB_GETCOUNT, 0, 0);
	if (lr == CB_ERR || lr < 0 || lr > MAXLONG)
		return;
	count = (int)lr;

	height = itemHeight * count + editHeight*2;

	if (maxHeight > 0)
	{
		if (height > maxHeight)
			height = maxHeight;
	}

	if(!GetWindowRect(hWnd, &rc))
		return ;
	
	limitHeight = abs(rWorkArea.bottom - rWorkArea.top) / 3;
	if (height > limitHeight)
		height = limitHeight;
	SetWindowPos(hWnd, (HWND)HWND_NOTOPMOST,0,0, rc.right - rc.left, (int)height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

HRESULT G::GetClsidFromRegValue(HKEY hKey, LPCTSTR lpValueName, GUID *pId)
{
WCHAR szwGuid[50];
LONG   lRetCode; 
ULONG buflen1;
#ifdef UNICODE
	buflen1 = sizeof(szwGuid);
	lRetCode = RegQueryValueEx(hKey, lpValueName, NULL, NULL, (PBYTE) &szwGuid[0], &buflen1);
#else
	TCHAR szGuid[50];
	buflen1 = sizeof(szGuid);
	lRetCode = RegQueryValueEx(hKey, lpValueName, NULL, NULL, (PBYTE) &szGuid[0], &buflen1);
	if (lRetCode == ERROR_SUCCESS)
	{
		lRetCode = G::AnsiToUc(&szGuid[0], &szwGuid[0], sizeof(szwGuid)/sizeof(WCHAR));
	}
#endif
	if (SUCCEEDED(lRetCode))
	{
		if (CLSIDFromString(&szwGuid[0], pId) == NOERROR)
			return S_OK;
		else
			return E_FAIL;
	}
	else
		return lRetCode;
}

HRESULT G::SaveClsidToRegValue(HKEY hKey, LPCTSTR lpValueName, GUID *pId)
{
TCHAR szValue[50];
WCHAR szwValue[50];
LONG   lRetCode; 
int i;

	i= StringFromGUID2(*pId, &szwValue[0], sizeof(szwValue) / sizeof(WCHAR));
	if (i!=0)
	{
		szwValue[_countof(szwValue) - 1] = 0;
#ifdef UNICODE
		lstrcpy(szValue, szwValue);
		lRetCode = ERROR_SUCCESS;
#else
		lRetCode = G::UcToAnsi(&szwValue[0], szValue, 0);
#endif
		if (SUCCEEDED(lRetCode))
		{
			lRetCode = RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
			if (SUCCEEDED(lRetCode))
			{
				return S_OK;
			}
		}

	}
	return E_FAIL;
}

int G::Rand(int min, int max)
{
	return (int)(((double) rand() / (double) RAND_MAX) * (double)max) + min;
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
		if (nextx!=NULL)
			*nextx = x;
		if (nexty!=NULL)
			*nexty = y;
		if (len > 0)
		{
			SIZE sizeText;
			BOOL brTextExtent=FALSE;
			brTextExtent = GetTextExtentExPoint(hdc, text, len, 0, NULL, NULL, &sizeText);
			ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, text, len, NULL);
			if (brTextExtent)
			{
				if (nextx!=NULL)
					*nextx = x + sizeText.cx;
				if (nexty!=NULL)
					*nexty = y + sizeText.cy;
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

BOOL G::DrawBitmap (HDC hDC, INT x, INT y, HBITMAP hBitmap, DWORD dwROP)
{
	HDC       hDCBits;
	BITMAP    Bitmap;
	BOOL      bResult;

	if (!hDC || !hBitmap)
		return FALSE;

	hDCBits = CreateCompatibleDC(hDC);
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	SelectObject(hDCBits, hBitmap);
	bResult = BitBlt(hDC, x, y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBits, 0, 0, dwROP);
	DeleteDC(hDCBits);
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
					for(int i = 0; i < countOfImageList; i++)
					{
						if (tbImageList[i].BitmapImageResourceId!=0)
						{
							hbmpImage = LoadBitmap(hInst, MAKEINTRESOURCE(tbImageList[i].BitmapImageResourceId));
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
							if (tbImageList[i].BitmapMaskResourceId!=0)
							{
								hbmpMask = LoadBitmap(hInst, MAKEINTRESOURCE(tbImageList[i].BitmapMaskResourceId));
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
								StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapImage.bmWidth, bitmapImage.bmHeight, SRCCOPY);

								if (hbmpMask && hBmpMaskSz)
								{
									HBITMAP hOld_BmpDest2 = (HBITMAP)SelectObject(hMemDC_Dest, hBmpMaskSz);
									HBITMAP hOld_BmpSrc2 = (HBITMAP)SelectObject(hMemDC_Src, hbmpMask);
									if (hOld_BmpDest2 && hOld_BmpSrc2)
									{
										StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapMask.bmWidth, bitmapMask.bmHeight, SRCCOPY);
										bOK = true;
									}
								}
								else
								{
									bOK = true;
								}
							}
							if (hOld_BmpDest)
								SelectObject(hMemDC_Dest, hOld_BmpDest);
							if (hOld_BmpSrc)
								SelectObject(hMemDC_Src, hOld_BmpSrc);
							if (!bOK)
							{
								fail = true;
								break;
							}
							if (hBmpMaskSz)
								r = ImageList_Add(hImageList, hBmpImageSz, hBmpMaskSz);
							else
								r = ImageList_AddMasked(hImageList, hBmpImageSz, RGB(0xff,0xff,0xff));
							if (r < 0)
							{
								fail = true;
								break;
							}
							if (hBmpImageSz)
								DeleteObject(hBmpImageSz);
							if (hBmpMaskSz)
								DeleteObject(hBmpMaskSz);
							if (hbmpImage)
								DeleteObject(hbmpImage);
							if (hbmpMask)
								DeleteObject(hbmpMask);
							hBmpImageSz = NULL;
							hBmpMaskSz = NULL;
							hbmpImage = NULL;
							hbmpMask = NULL;
						}
						else
						{
							hIconImage = LoadIcon(hInst, MAKEINTRESOURCE(tbImageList[i].IconResourceId));
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
							r = ImageList_AddMasked(hImageList, 0, RGB(0xff,0xff,0xff));
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
		ImageList_SetBkColor(hImageList, CLR_NONE);
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

	int ok=0;
	for (; ok==0; ok++)
	{
		hwndTB = ::CreateWindowEx(0, TOOLBARCLASSNAME, 	(LPTSTR) NULL, 
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS |	WS_CHILD | 	TBSTYLE_FLAT | TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NODIVIDER // | CCS_ADJUSTABLE
			, 0, 0, 0, 0, 
			hWnd, 
			(HMENU) LongToPtr(toolbarid), 
			hInst, 
			NULL
		);
		if (!hwndTB)
			break;
		SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		lr = SendMessage(hwndTB, TB_SETBUTTONSIZE, 0, MAKELONG(buttonWidth, buttonHeight));
		if (!lr) 
			break;

		SendMessage(hwndTB, TB_SETMAXTEXTROWS, 0, 0);
		 
		hOldImageList = (HIMAGELIST)SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hImageListToolBarNormal);

		tbArray = new TBBUTTON[length];

		if (!tbArray) 
			break;

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

	}
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
				SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hOldImageList);
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
		return;
	BOOL br = GetWindowRect(hWnd, &rcMain);
	if (!br)
		return;

	SetRectEmpty(&rcWorkArea);
	if (G::s_pFnMonitorFromRect != NULL && G::s_pFnGetMonitorInfo != NULL)
	{
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		HMONITOR hMonitor = G::s_pFnMonitorFromRect(&rcMain, MONITOR_DEFAULTTOPRIMARY);
		if (G::s_pFnGetMonitorInfo(hMonitor, &mi))
		{
			rcWorkArea = mi.rcMonitor;
			bGotWorkArea = true;
		}
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
		OffsetRect(&rcMain, rcWorkArea.right - rcMain.right, 0);
	if (rcMain.bottom>rcWorkArea.bottom)
		OffsetRect(&rcMain, 0, rcWorkArea.bottom - rcMain.bottom);

	if (rcMain.left<rcWorkArea.left)
		OffsetRect(&rcMain, rcWorkArea.left - rcMain.left, 0);
	if (rcMain.top<rcWorkArea.top)
		OffsetRect(&rcMain, 0, rcWorkArea.top - rcMain.top);

	SetWindowPos(hWnd, HWND_TOP, rcMain.left,rcMain.top, rcMain.right-rcMain.left, rcMain.bottom-rcMain.top, SWP_NOZORDER | SWP_NOSIZE);
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
		c = 0;
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
			c = cchBuffer - 1;
		if (c < 0)
			c = 0;
		buffer[c] = 0;
	}	
	if (c < 0)
		c = 0;
	return c;
}

LPTSTR G::GetMallocEditLineSzString(HWND hEditControl, int linenumber)
{
int c;
	c = (int)SendMessage(hEditControl, EM_LINELENGTH, linenumber, 0);
	if (c < 0)
		return NULL;
	LPTSTR s = (LPTSTR)malloc(c + sizeof(TCHAR));
	if (!s)
		return NULL;
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
		k = max(k, ListView_GetStringWidth(hWnd, p));
	}
	va_end(vl);
	return k;
}
