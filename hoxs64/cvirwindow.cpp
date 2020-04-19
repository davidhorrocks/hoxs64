#include "cvirwindow.h"

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
	LRESULT lr;
	LRESULT br;
	// Get a pointer to the window class object.
	CVirWindow* pWin = (CVirWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE:
		// Since this is the first time that we can get ahold of
		// a pointer to the window class object, all messages that might
		// have been sent before this are never seen by the Windows object
		// and only get passed on to the DefWindowProc

		// Get the initial creation pointer to the window object
		pWin = (CVirWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;

		// Set it's protected m_hWnd member variable to ensure that
		// member functions have access to the correct window handle.
		pWin->m_hWnd = hWnd;

		// Set its USERDATA to point to the window object
#pragma warning(disable:4244)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWin);
#pragma warning(default:4244)

		br = (BOOL)pWin->WindowProc(hWnd, uMsg, wParam, lParam);
		if (br)
		{
			//pWin->m_pKeepAlive = pWin->shared_from_this();
			return br;
		}
		else
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			return FALSE;
		}
		break;
	case WM_NCDESTROY:
		// This is our signal to destroy the window object.
		if (pWin)
		{
			lr = pWin->WindowProc(hWnd, uMsg, wParam, lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			pWin->m_hWnd = 0;
			//pWin->m_pKeepAlive.reset();
			pWin->WindowRelease();
			pWin = 0;
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

LRESULT CALLBACK MdiFrameWindowProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT lr;
	LRESULT br;
	// Get a pointer to the window class object.
	CVirMdiFrameWindow* pWin = reinterpret_cast<CVirMdiFrameWindow*>((LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_NCCREATE:
		// Since this is the first time that we can get ahold of
		// a pointer to the window class object, all messages that might
		// have been sent before this are never seen by the Windows object
		// and only get passed on to the DefWindowProc

		// Get the initial creation pointer to the window object
		pWin = (CVirMdiFrameWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;

		// Set it's protected m_hWnd member variable to ensure that
		// member functions have access to the correct window handle.
		pWin->m_hWnd = hWnd;

		// Set its USERDATA to point to the window object
#pragma warning(disable:4244)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWin);
#pragma warning(default:4244)

		br = (BOOL)pWin->MdiFrameWindowProc(hWnd, pWin->m_hWndMDIClient, uMsg, wParam, lParam);
		if (br)
		{
			//pWin->m_pKeepAlive = pWin->shared_from_this();
			return br;
		}
		else
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			return FALSE;
		}
		break;
	case WM_DESTROY:
		if (pWin)
		{
			lr = pWin->MdiFrameWindowProc(hWnd, pWin->m_hWndMDIClient, uMsg, wParam, lParam);
			pWin->m_hWndMDIClient = 0;
			return lr;
		}
		break;
	case WM_NCDESTROY:
		// This is our signal to destroy the window object.
		if (pWin)
		{
			lr = pWin->MdiFrameWindowProc(hWnd, pWin->m_hWndMDIClient, uMsg, wParam, lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			pWin->m_hWnd = 0;
			//pWin->m_pKeepAlive.reset();
			pWin->WindowRelease();
			pWin = nullptr;
			return lr;
		}
		break;
	default:
		break;
	}

	// Call its message proc method.
	if (NULL != pWin)
		return (pWin->MdiFrameWindowProc(hWnd, pWin->m_hWndMDIClient, uMsg, wParam, lParam));
	else
		return (DefFrameProc(hWnd, 0, uMsg, wParam, lParam));
}


LRESULT CALLBACK MdiChildWindowProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT lr;
	LRESULT br;
	MDICREATESTRUCT* pMdiCreateStruct;
	// Get a pointer to the window class object.
	CVirMdiChildWindow* pWin = (CVirMdiChildWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE:
		// Since this is the first time that we can get ahold of
		// a pointer to the window class object, all messages that might
		// have been sent before this are never seen by the Windows object
		// and only get passed on to the DefWindowProc

		// Get the initial creation pointer to the window object
		pMdiCreateStruct = (MDICREATESTRUCT*)((CREATESTRUCT*)lParam)->lpCreateParams;
		pWin = (CVirMdiChildWindow*)pMdiCreateStruct->lParam;


		// Set it's protected m_hWnd member variable to ensure that
		// member functions have access to the correct window handle.
		pWin->m_hWnd = hWnd;

		// Set its USERDATA to point to the window object
#pragma warning(disable:4244)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWin);
#pragma warning(default:4244)

		br = (BOOL)pWin->WindowProc(hWnd, uMsg, wParam, lParam);
		if (br)
		{
			//pWin->m_pKeepAlive = pWin->shared_from_this();
			return br;
		}
		else
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			return FALSE;
		}
		break;
	case WM_NCDESTROY:
		// This is our signal to destroy the window object.
		if (pWin)
		{
			lr = pWin->WindowProc(hWnd, uMsg, wParam, lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			pWin->m_hWnd = 0;
			//pWin->m_pKeepAlive.reset();
			pWin->WindowRelease();
			pWin = 0;
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
		return (DefMDIChildProc(hWnd, uMsg, wParam, lParam));
}

LRESULT CALLBACK GlobalSubClassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CVirWindow* pWin = NULL;
	if (hWnd != NULL)
	{
		pWin = (CVirWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (NULL != pWin)
			return (pWin->SubclassWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return 0;
}

WNDPROC CBaseVirWindow::SubclassChildWindow(HWND hWnd)
{
#pragma warning(disable:4244)
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	WNDPROC pOldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) ::GlobalSubClassWindowProc);
#pragma warning(default:4244)
	return pOldProc;
}

WNDPROC CBaseVirWindow::SubclassChildWindow(HWND hWnd, WNDPROC proc)
{
#pragma warning(disable:4244)
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	WNDPROC pOldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)proc);
#pragma warning(default:4244)
	return pOldProc;
}

LRESULT CBaseVirWindow::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CBaseVirWindow::SetSize(int w, int h)
{
	return (int)SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
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

void CVirWindow::GetMinWindowSize(int& w, int& h)
{
	w = 0;
	h = 0;
}

HWND CVirMdiFrameWindow::CreateMDIClientWindow(UINT clientId, UINT firstChildId, int iWindowMenuIndex)
{
	CLIENTCREATESTRUCT ccs;
	// Retrieve the handle to the window menu and assign the 
	// first child window identifier. 
	ZeroMemory(&ccs, sizeof(ccs));

	ccs.hWindowMenu = GetSubMenu(GetMenu(m_hWnd), iWindowMenuIndex);
	ccs.idFirstChild = firstChildId;

	// Create the MDI client window. 
	m_hWndMDIClient = ::CreateWindowEx(0, TEXT("MDICLIENT"), (LPCTSTR)NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
		0, 0, 0, 0, m_hWnd, (HMENU)LongToPtr(clientId), m_hInst, (LPSTR)&ccs);
	return m_hWndMDIClient;
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
	CVirDialog* pdlg = (CVirDialog*)(LONG_PTR)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
	BOOL br;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// Get a pointer to the window class object.
		pdlg = (CVirDialog*)lParam;

#pragma warning(disable:4244)
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)pdlg);
#pragma warning(default:4244)
		pdlg->m_hWnd = hWndDlg;
		br = pdlg->DialogProc(hWndDlg, uMsg, wParam, lParam);
		if (br)
		{
			//pdlg->m_pKeepAlive = pdlg->shared_from_this();
			return br;
		}
		else
		{
			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, 0);
			return FALSE;
		}
		// Set the USERDATA to point to the class object.
		break;
	case WM_NCDESTROY:
		// This is our signal to destroy the window object.
		if (pdlg)
		{
			LRESULT lr = pdlg->DialogProc(hWndDlg, uMsg, wParam, lParam);
			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, 0);
			pdlg->m_hWnd = 0;
			//pdlg->m_pKeepAlive.reset();
			pdlg->WindowRelease();
			pdlg = 0;
			return lr;
		}
		break;

	default:
		break;
	}

	// Call its message proc method.
	if (pdlg != (CVirDialog*)0)
		return (pdlg->DialogProc(hWndDlg, uMsg, wParam, lParam));
	else
		return (FALSE);
}

CVirDialog::CVirDialog() noexcept
{
	m_bIsModeless = false;
	m_hInst = NULL;
	m_hWnd = NULL;
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
	m_bIsModeless = true;
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
	m_bIsModeless = true;
	m_hInst = hInst;

	iResult = ::CreateDialogIndirectParam(
		hInst,
		lpTemplate,
		hWndOwner,
		(DLGPROC)::DialogProc,
		(LPARAM)this);

	return (iResult);
}

/**************************************************************************/

CTabPageDialog::CTabPageDialog() noexcept
{
	m_hInst = 0;
	m_hWnd = 0;
	m_lpTemplate = 0;
	m_lpTemplateEx = 0;
	m_pagetext = nullptr;
	m_bIsCreated = false;
	m_pageindex = 0;
	m_dlgId = 0;
	m_owner = nullptr;
}

CTabPageDialog::~CTabPageDialog() noexcept
{
	if (m_pagetext)
	{
		delete[] m_pagetext;
		m_pagetext = NULL;
	}

}

HRESULT CTabPageDialog::init(CTabDialog* owner, int dlgId, LPTSTR szText, int pageindex)
{
	m_owner = owner;
	m_dlgId = dlgId;
	m_pageindex = pageindex;
	m_pagetext = new TCHAR[lstrlen(szText) + 1];
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

BOOL CTabPageDialog::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_owner->OnPageEvent(this, hWndDlg, uMsg, wParam, lParam);
}


/**************************************************************************/

CTabDialog::CTabDialog() noexcept
{
	m_hwndDisplay = 0;
	m_hWnd = 0;
	m_hInst = 0;
	m_hwndTab = 0;
	m_tabctl_id = 0;
	m_current_page_index = -1;
	m_rcDisplay = { 0,0,0,0 };
}

CTabDialog::~CTabDialog()
{
	FreePages();
}

void CTabDialog::FreePages() noexcept
{
	m_vecpages.clear();
}

INT_PTR CTabDialog::ShowDialog(HINSTANCE hInst, LPTSTR lpszTemplate, HWND hWndOwner)
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

HRESULT CTabDialog::SetPages(int pagecount, struct tabpageitem* pageitem)
{
	int i;
	HRESULT hr;

	FreePages();
	m_vecpages.reserve(pagecount);
	if (m_vecpages.capacity() == 0)
		return E_OUTOFMEMORY;

	for (i = 0; i < pagecount; i++)
	{
		shared_ptr<CTabPageDialog> pTabPageDialog = shared_ptr<CTabPageDialog>(new CTabPageDialog());
		if (pTabPageDialog == 0)
		{
			FreePages();
			return E_OUTOFMEMORY;
		}
		m_vecpages.push_back(pTabPageDialog);
		hr = m_vecpages[i]->init(this, pageitem[i].pageid, pageitem[i].lpszText, i);
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
	if (hwndParent == NULL)
	{
		return FALSE;
	}

	SetWindowPos(hwndDlg, HWND_TOP,
		m_rcDisplay.left, m_rcDisplay.top,
		0, 0, SWP_NOSIZE);
	return TRUE;
}

BOOL CTabDialog::OnTabbedDialogInit(HWND hwndDlg)
{
	DWORD dwDlgBase = GetDialogBaseUnits();
	TCITEM tie;
	RECT rcTab, rcTemp;
	HWND hwndButtonOK;
	HWND hwndButtonCancel;
	RECT rcButtonOK;
	RECT rcButtonCancel;
	SIZE sizeButtonOK = { 0 , 0 };
	SIZE sizeButtonCancel = { 0 , 0 };
	size_t i;
	BOOL bResult = FALSE;
	const LONG DialogMargin = 2;
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
	rcTemp.right = DialogMargin;
	rcTemp.bottom = DialogMargin;
	MapDialogRect(hwndDlg, &rcTemp);
	int cxMargin = abs(rcTemp.right - rcTemp.left);
	int cyMargin = abs(rcTemp.bottom - rcTemp.top);

	// Add a tab for each of the three child dialog boxes. 
	if (m_vecpages.size() > 0)
	{
		for (i = m_vecpages.size(); i > 0; i--)
		{
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = -1;
			tie.pszText = m_vecpages[i - 1]->m_pagetext;
			TabCtrl_InsertItem(m_hwndTab, 0, &tie);
		}
	}

	// Lock the resources for the three child dialog boxes. 
	//apRes[0] = DoLockDlgRes(m_hInst, MAKEINTRESOURCE(IDD_KEYPAGE1)); 

	// Determine the bounding rectangle for all child dialog boxes. 
	SetRectEmpty(&rcTab);
	for (i = 0; i < m_vecpages.size(); i++)
	{
		if (m_vecpages[i]->m_lpTemplate != 0)
		{
			if (m_vecpages[i]->m_lpTemplate->cx > rcTab.right)
				rcTab.right = m_vecpages[i]->m_lpTemplate->cx;
			if (m_vecpages[i]->m_lpTemplate->cy > rcTab.bottom)
				rcTab.bottom = m_vecpages[i]->m_lpTemplate->cy;
		}
		else if (m_vecpages[i]->m_lpTemplateEx != 0)
		{
			if (m_vecpages[i]->m_lpTemplateEx->cx > rcTab.right)
				rcTab.right = m_vecpages[i]->m_lpTemplateEx->cx;
			if (m_vecpages[i]->m_lpTemplateEx->cy > rcTab.bottom)
				rcTab.bottom = m_vecpages[i]->m_lpTemplateEx->cy;
		}
	}

	MapDialogRect(hwndDlg, &rcTab);

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

	hwndButtonOK = GetDlgItem(hwndDlg, IDOK);
	if (hwndButtonOK)
	{
		GetWindowRect(hwndButtonOK, &rcButtonOK);
		sizeButtonOK.cx = rcButtonOK.right - rcButtonOK.left;
		sizeButtonOK.cy = rcButtonOK.bottom - rcButtonOK.top;
	}

	hwndButtonCancel = GetDlgItem(hwndDlg, IDCANCEL);
	if (hwndButtonCancel)
	{
		GetWindowRect(hwndButtonCancel, &rcButtonCancel);
		sizeButtonCancel.cx = rcButtonCancel.right - rcButtonCancel.left;
		sizeButtonCancel.cy = rcButtonCancel.bottom - rcButtonCancel.top;
	}

	if (false)
	{
		// Move the OK button below the tab control. 
		if (hwndButtonOK)
		{
			SetWindowPos(hwndButtonOK, NULL,
				rcTab.left, rcTab.bottom + cyMargin, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER);

			// Move the Cancel button to the right of the OK. 
			if (hwndButtonCancel)
			{
				SetWindowPos(hwndButtonCancel, NULL,
					rcTab.left + sizeButtonOK.cx + cxMargin,
					rcTab.bottom + cyMargin, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER);
			}
		}
	}
	else
	{
		if (hwndButtonCancel)
		{
			// Move the Cancel button below the tab control. 
			SetWindowPos(hwndButtonCancel, NULL,
				rcTab.right - sizeButtonCancel.cx,
				rcTab.bottom + cyMargin, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER);
			if (hwndButtonOK)
			{
				// Move the OK button to the left of the Cancel. 
				SetWindowPos(hwndButtonOK, NULL,
					rcTab.right - sizeButtonCancel.cx - sizeButtonOK.cx - cxMargin,
					rcTab.bottom + cyMargin, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER);
			}
		}
	}

	DWORD dwStyle = GetWindowLongW(hwndDlg, GWL_STYLE);
	DWORD dwStyleEx = GetWindowLongW(hwndDlg, GWL_EXSTYLE);
	RECT rcFrame;
	CopyRect(&rcFrame, &rcTab);
	AdjustWindowRectEx(&rcFrame, dwStyle, FALSE, dwStyleEx);

	// Size the dialog box. 
	SetWindowPos(hwndDlg, NULL, 0, 0,
		rcFrame.right - rcFrame.left + 2 * cxMargin,
		rcFrame.bottom - rcFrame.top + sizeButtonOK.cy + 3 * cyMargin,
		SWP_NOMOVE | SWP_NOZORDER);

	// Simulate selection of the first item. 
	TabCtrl_SetCurSel(m_hwndTab, 0);

	return OnSelChanged(hwndDlg);
}

BOOL CTabDialog::OnSelChanged(HWND hwndDlg)
{
	size_t iSel;
	// Destroy the current child dialog box, if any. 
	if (IsWindow(m_hwndDisplay))
	{
		ShowWindow(m_hwndDisplay, SW_HIDE);
		UpdateWindow(m_hwndDisplay);
	}
	m_hwndDisplay = NULL;

	iSel = m_current_page_index = TabCtrl_GetCurSel(m_hwndTab);

	if (iSel >= m_vecpages.size())
		return FALSE;

	if (!m_vecpages[iSel]->m_bIsCreated)
	{
		// Create the new child dialog box. 
		if (m_vecpages[iSel]->m_lpTemplate != 0)
			m_hwndDisplay = CreateDialogIndirectParam(m_hInst, m_vecpages[iSel]->m_lpTemplate, hwndDlg, ::DialogProc, (LPARAM)(this->m_vecpages[iSel].get()));
		else if (m_vecpages[iSel]->m_lpTemplateEx != 0)
			m_hwndDisplay = CreateDialogIndirectParam(m_hInst, (LPCDLGTEMPLATE)m_vecpages[iSel]->m_lpTemplateEx, hwndDlg, ::DialogProc, (LPARAM)(this->m_vecpages[iSel].get()));
		if (m_hwndDisplay)
			m_vecpages[iSel]->m_bIsCreated = true;
	}
	else
	{
		m_hwndDisplay = m_vecpages[iSel]->m_hWnd;
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
					SetWindowPos(m_hwndDisplay, NULL, rcDisplay.left, rcDisplay.top, rcDisplay.right - rcDisplay.left, rcDisplay.bottom - rcDisplay.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
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

shared_ptr<CTabPageDialog> CTabDialog::GetPage(int i)
{
	return m_vecpages[i];
}

HRESULT CTabDialog::CreateAllPages()
{
	size_t i;
	HRESULT hr = S_OK;
	HWND hWnd = nullptr;
	for (i = 0; i < m_vecpages.size(); i++)
	{
		if (!m_vecpages[i]->m_bIsCreated)
		{
			if (m_vecpages[i]->m_lpTemplate != 0)
			{
				hWnd = CreateDialogIndirectParam(m_hInst, m_vecpages[i]->m_lpTemplate, m_hWnd, ::DialogProc, (LPARAM)(this->m_vecpages[i].get()));
			}
			else if (m_vecpages[i]->m_lpTemplateEx != 0)
			{
				hWnd = CreateDialogIndirectParam(m_hInst, (LPCDLGTEMPLATE)m_vecpages[i]->m_lpTemplateEx, m_hWnd, ::DialogProc, (LPARAM)(this->m_vecpages[i].get()));
			}

			if (hWnd)
			{
				m_vecpages[i]->m_bIsCreated = true;
				ShowWindow(hWnd, SW_HIDE);
			}
			else
			{
				hr = E_FAIL;
			}
		}
	}

	return hr;
}