#define OEMRESOURCE 
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "wpanel.h"
#include "wpanelmanager.h"

#define WPANELSIZEGAP (5)

WPanelManager::WPanelManager()
{
}

WPanelManager::~WPanelManager()
{
}

HRESULT WPanelManager::Init(HINSTANCE hInstance, CVirWindow *pParentWindow, HWND hWndRebar)
{
	m_bIsRootRectValid = false;
	m_oldy = -4;
	m_fMoved = FALSE;
	m_fDragMode = FALSE;

	m_pParentWindow = pParentWindow;
	m_hWndRebar = hWndRebar;
	m_hInstance = hInstance;

	m_iSizerGap = m_dpi.ScaleX(WPANELSIZEGAP);

	return S_OK;
}

HRESULT WPanelManager::CreateNewPanel(WPanel::InsertionStyle::EInsertionStyle style, LPTSTR pszTitle, CVirWindow *pChildWin)
{
	HWND hWndPanel = NULL;
	WPanel *pwp = new WPanel();
	HRESULT hr = pwp->Init(this, pChildWin);
	if (SUCCEEDED(hr))
	{
		CVirWindow *pWin = this->Get_ParentWindow();
		HWND hWndPanel = pwp->Create(m_hInstance, pWin->GetHwnd(), pszTitle, 0, 0, 200, 100, (HMENU) 1001);
		if (hWndPanel)
		{
			hr = this->m_WpList.Append(pwp);
			if (SUCCEEDED(hr))
			{
				return S_OK;
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}

	if (hWndPanel)
	{
		DestroyWindow(hWndPanel);
	}
	else if (pwp)
	{
		delete pwp;
	}
	return hr;
}

int WPanelManager::Get_SizerGap()
{
	return m_iSizerGap;
}

int WPanelManager::GetMinWindowHeight()
{
	int mins = m_dpi.PointsToPixels(15);
	RECT rcMinWindow;
	SetRect(&rcMinWindow, 0, 0, mins, mins);
	AdjustWindowRectEx(&rcMinWindow, WS_CAPTION, false, 0);
	return rcMinWindow.bottom - rcMinWindow.top;
}

void WPanelManager::SizePanels(HWND hWnd, int x, int y, int w, int h)
{
RECT rcRootInner;
RECT rcMdiClient;

	SetRect(&m_rcRoot, x, y, x+w, y+h);
	m_bIsRootRectValid = true;
	CopyRect(&rcRootInner, &m_rcRoot);

	if ((m_rcRoot.right - m_rcRoot.left > 2 * m_iSizerGap))
	{
		InflateRect(&rcRootInner, -m_iSizerGap, 0);
	}
	else
	{
		rcRootInner.left = rcRootInner.right = x + w/2;
	}
	if ((m_rcRoot.bottom - m_rcRoot.top > 2 * m_iSizerGap))
	{
		InflateRect(&rcRootInner, 0, - m_iSizerGap);
	}
	else
	{
		rcRootInner.top = rcRootInner.bottom = y + h/2;
	}

	int mins = GetMinWindowHeight();
	int iMinHeightMdiClient = mins;
	int iMinHeightPanel = mins;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = p->Next())
	{
		if (p->m_data == NULL)
			continue;
		WPanel *pwp = p->m_data;
		if (pwp == NULL)
			continue;
		SIZE szPref;
		pwp->GetPreferredSize(&szPref);

		int heightClientPart = rcRootInner.bottom - rcRootInner.top;
		int widthClientPart = rcRootInner.right - rcRootInner.left;
		int heightPanel;
		int widthPanel = rcRootInner.right - rcRootInner.left;
		if (szPref.cy <= rcRootInner.bottom - rcRootInner.top)
		{
			heightPanel = szPref.cy;
		}
		else
		{
			heightPanel = rcRootInner.bottom - rcRootInner.top;
		}
		RECT rcPanel;
		if (heightPanel <= heightClientPart - iMinHeightMdiClient - m_iSizerGap)
		{
			//Size mdi client window
			SetRect(&rcMdiClient, rcRootInner.left, rcRootInner.top, rcRootInner.right, rcRootInner.bottom - heightPanel - m_iSizerGap);
			SetRect(&rcPanel, rcRootInner.left, rcMdiClient.bottom + m_iSizerGap, rcRootInner.right, rcRootInner.bottom);
		}
		else if (iMinHeightMdiClient + iMinHeightPanel + m_iSizerGap <= heightClientPart)
		{
			//The panel's height is too tall so shrink it.
			heightPanel = heightClientPart - iMinHeightMdiClient - m_iSizerGap;
			SetRect(&rcMdiClient, rcRootInner.left, rcRootInner.top, rcRootInner.right, rcRootInner.top + iMinHeightMdiClient);
			SetRect(&rcPanel, rcRootInner.left, rcMdiClient.bottom + m_iSizerGap, rcRootInner.right, rcRootInner.bottom);
		}
		else
		{
			//The the min height panel plus the min height mdiclient window will not fit so zero size the mdiclient and fit only the panel.
			heightPanel = iMinHeightPanel;
			SetRect(&rcMdiClient, rcRootInner.left, rcRootInner.top, rcRootInner.right, rcRootInner.top);
			SetRect(&rcPanel, rcRootInner.left, rcRootInner.top, rcRootInner.right, rcRootInner.bottom);
		}
		pwp->UpdateSizerRegion(rcPanel);
		HWND hWndPanel = p->m_data->GetHwnd();
		if (hWndPanel)
		{
			if (rcPanel.right - rcPanel.left > 0 && rcPanel.bottom - rcPanel.top > 0)
				MoveWindow(hWndPanel, rcPanel.left, rcPanel.top, rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top, TRUE);
			else
				MoveWindow(hWndPanel, rcPanel.left, rcPanel.top, 0, 0, TRUE);
		}
	}

	if (m_pParentWindow)
	{
		HWND hWndMDIClient = m_pParentWindow->Get_MDIClientWindow();
		if (hWndMDIClient)
		{
			if (rcMdiClient.right - rcMdiClient.left > 0 && rcMdiClient.bottom - rcMdiClient.top > 0)
				MoveWindow(hWndMDIClient, rcMdiClient.left, rcMdiClient.top, rcMdiClient.right - rcMdiClient.left, rcMdiClient.bottom - rcMdiClient.top, TRUE);
			else
				MoveWindow(hWndMDIClient, rcMdiClient.left, rcMdiClient.top, 0, 0, TRUE);
		}
	}
}

CVirWindow *WPanelManager::Get_ParentWindow()
{
	return m_pParentWindow;
}


WPanel *WPanelManager::GetPanelSizerFromClientPos(int x, int y, LPRECT prcSizerBar)
{
POINT pt;

	pt.x = x;
	pt.y = y;

	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = p->Next())
	{
		if (p->m_data == NULL)
			continue;
		WPanel *pwp = p->m_data;
		if (pwp == NULL)
			continue;
		HRGN rgn = pwp->m_hrgSizerTop;
		if (rgn == NULL)
			continue;
		if (rgn)
		{
			if (PtInRegion(rgn, pt.x, pt.y))
			{
				if (prcSizerBar)
				{
					SetRectEmpty(prcSizerBar);
					GetRgnBox(pwp->m_hrgSizerTop, prcSizerBar);
				}
				return pwp;
			}
		}
	}
	return NULL;
}

void WPanelManager::DrawXorBar(HDC hdc, int x1, int y1, int width, int height)
{
	static WORD _dotPatternBmp[8] = 
	{ 
		0x00aa, 0x0055, 0x00aa, 0x0055, 
		0x00aa, 0x0055, 0x00aa, 0x0055
	};

	HBITMAP hbm;
	HBRUSH  hbr, hbrushOld;

	hbm = CreateBitmap(8, 8, 1, 1, _dotPatternBmp);
	hbr = CreatePatternBrush(hbm);
	
	SetBrushOrgEx(hdc, x1, y1, 0);
	hbrushOld = (HBRUSH)SelectObject(hdc, hbr);
	
	PatBlt(hdc, x1, y1, width, height, PATINVERT);
	
	SelectObject(hdc, hbrushOld);
	
	DeleteObject(hbr);
	DeleteObject(hbm);
}

void WPanelManager::OnDestroyWPanel(WPanel *pwp)
{
	WpElement *n = NULL;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = n)
	{
		n = p->Next();
		if (p->m_data!=NULL)
		{
			if (p->m_data == pwp)
			{
				m_WpList.Remove(p);
			}
		}		
	}
}

void WPanelManager::ClipPointToRect(const RECT &rc, POINT *pt)
{
	if (pt->y < rc.top)
	{
		pt->y = rc.top;
	}
	if (pt->y > rc.bottom)
	{
		pt->y = rc.bottom;
	}
	if (pt->x < rc.left)
	{
		pt->x = rc.left;
	}
	if (pt->x > rc.right)
	{
		pt->x = rc.right;
	}
}

void WPanelManager::ClipPointToValidPanelSizer(POINT *pt)
{
	if (m_pPanelToSize==NULL)
		return;
	int mins = GetMinWindowHeight();
	RECT rcParent;
	m_pPanelToSize->GetParentRect(&rcParent);
	OffsetRect(&rcParent, 0, m_y_offset);
	if (rcParent.bottom - rcParent.top > 2*m_iSizerGap)
	{
		InflateRect(&rcParent, 0, -m_iSizerGap);
	}
	if (rcParent.bottom - rcParent.top > 2*mins)
	{
		InflateRect(&rcParent, 0, -mins);
	}
	ClipPointToRect(rcParent, pt);
}

void WPanelManager::Get_RootRect(RECT *prc)
{
	if (m_bIsRootRectValid)
		CopyRect(prc, &m_rcRoot);
	else
		SetRectEmpty(prc);
}

bool WPanelManager::Splitter_OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	HDC hdc;
	m_pPanelToSize = NULL;

	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	WPanel *pwp = GetPanelSizerFromClientPos(pt.x, pt.y, &m_rcSizer);
	if (pwp == NULL)
		return false;
	m_pPanelToSize = pwp;
	m_iSplitterPos = m_rcSizer.top;
	m_y_offset = pt.y - m_rcSizer.top;
	ClipPointToValidPanelSizer(&pt);

	SetCapture(hWnd);
	m_fDragMode = TRUE;
	hdc = GetDC(hWnd);
	if (hdc)
	{
		DrawXorBar(hdc, m_rcSizer.left, pt.y - m_y_offset, m_rcSizer.right - m_rcSizer.left, m_rcSizer.bottom - m_rcSizer.top);
		ReleaseDC(hWnd, hdc);
	}
	m_oldy = pt.y;

	return true;
}

bool WPanelManager::Splitter_OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	POINT pt;

	if(m_fDragMode == FALSE) return false;
	if(m_pPanelToSize == 0) return false;
	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	ClipPointToValidPanelSizer(&pt);
	if(pt.y != m_oldy && wParam & MK_LBUTTON)
	{
		hdc = GetDC(hwnd);
		if (hdc)
		{
			DrawXorBar(hdc, m_rcSizer.left, m_oldy - m_y_offset, m_rcSizer.right - m_rcSizer.left, m_rcSizer.bottom - m_rcSizer.top);
			DrawXorBar(hdc, m_rcSizer.left, pt.y - m_y_offset, m_rcSizer.right - m_rcSizer.left, m_rcSizer.bottom - m_rcSizer.top);
			ReleaseDC(hwnd, hdc);
		}		

		m_oldy = pt.y;
	}

	return true;
}

bool WPanelManager::Splitter_OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//RECT rect;

	POINT pt;
	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	if(m_fDragMode == FALSE)
		return false;
	
	ClipPointToValidPanelSizer(&pt);
	hdc = GetDC(hWnd);
	if (hdc)
	{
		DrawXorBar(hdc, m_rcSizer.left, m_oldy - m_y_offset, m_rcSizer.right - m_rcSizer.left, m_rcSizer.bottom - m_rcSizer.top);
		ReleaseDC(hWnd, hdc);
	}
	m_oldy = pt.y;

	m_fDragMode = FALSE;

	m_iSplitterPos = pt.y - m_y_offset;

	ReleaseCapture();

	if (m_pPanelToSize)
	{
		RECT rcRootPanel;
		Get_RootRect(&rcRootPanel);
		if (m_iSplitterPos >= rcRootPanel.bottom)
			m_iSplitterPos = rcRootPanel.bottom - 1;
		if (m_iSplitterPos < rcRootPanel.top)
			m_iSplitterPos = rcRootPanel.top;

		SIZE szPref;
		m_pPanelToSize->GetCurrentSize(&szPref);
		szPref.cy = szPref.cy + m_rcSizer.top - m_iSplitterPos;
		if (szPref.cy < 0)
			szPref.cy = 0;
		m_pPanelToSize->SetPreferredSize(&szPref);
		this->SizePanels(hWnd, rcRootPanel.left, rcRootPanel.top, rcRootPanel.right - rcRootPanel.left, rcRootPanel.bottom - rcRootPanel.top);
	
		m_pPanelToSize = NULL;
	}

	return true;
}

bool WPanelManager::Splitter_OnSetCursor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	if (!GetCursorPos(&pt))
		return false;
	if (!ScreenToClient(hwnd, &pt))
		return false;

	WPanel *pwp = GetPanelSizerFromClientPos(pt.x, pt.y, NULL);
	if (pwp == NULL)
		return false;
	
	HCURSOR cur = (HCURSOR )LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENS), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	SetCursor(cur);
	return true;
}
