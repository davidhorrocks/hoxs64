#define OEMRESOURCE 
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"

#include "mlist.h"
#include "carray.h"
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
	oldy = -4;
	fMoved = FALSE;
	fDragMode = FALSE;


	m_pParentWindow = pParentWindow;
	m_hWndRebar = hWndRebar;
	m_hInstance = hInstance;

	m_iSizerGap = m_dpi.ScaleX(WPANELSIZEGAP);

	return S_OK;
}

HRESULT WPanelManager::CreateNewPanel(WPanel::InsertionStyle::EInsertionStyle style)
{
	HWND hWndPanel = NULL;
	WPanel *pwp = new WPanel();
	HRESULT hr = pwp->Init(this);
	if (SUCCEEDED(hr))
	{
		HWND hWndPanel = pwp->Create(m_hInstance, TEXT("TOOL"), 0, 0, 200, 100, (HMENU) 1001);
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

void WPanelManager::SizePanels(HWND hWnd, int w, int h)
{

BOOL br;
RECT rcAbsRebar;
int heightRebar = 0;
RECT rcWin;
RECT rcMdiClient;
	SetRect(&rcWin, 0, 0, w, h);
	
	if (m_hWndRebar != NULL)
	{
		br = GetWindowRect(m_hWndRebar, &rcAbsRebar);
		if (br == FALSE)
			return;
		heightRebar = rcAbsRebar.bottom - rcAbsRebar.top;

		SetWindowPos(m_hWndRebar, 0, 0, 0, w, heightRebar, SWP_NOREPOSITION | SWP_NOZORDER);

		SetRect(&rcWin, 0, heightRebar, w, h);
	}

	if (rcWin.right - rcWin.left <= 2*m_iSizerGap)
		return;
	if (rcWin.bottom - rcWin.top <= 2*m_iSizerGap)
		return;
	
	InflateRect(&rcWin, -m_iSizerGap, - m_iSizerGap);

	CopyRect(&rcMdiClient, &rcWin);

	int iMinHeightMdiClient = heightRebar;
	int iMinHeightPanel = heightRebar;
	RECT rcPanel;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = p->Next())
	{
		if (p->m_data == NULL)
			continue;
		WPanel *pwp = p->m_data;
		if (pwp == NULL)
			continue;
		SIZE szPref;
		pwp->GetPreferredSize(&szPref);
		rcPanel.left = rcWin.left;
		rcPanel.right = rcWin.right;
		//rcPanel.top = rcWin.top;
		//rcPanel.bottom = rcWin.bottom;

		if (szPref.cy <= rcWin.bottom - rcWin.top)
		{
			rcPanel.top = rcWin.top;
			rcPanel.bottom = rcWin.top + szPref.cy;
		}
		else
		{
			rcPanel.top = rcWin.top;
			rcPanel.bottom = rcWin.bottom;
		}
		int heightPanel = rcPanel.bottom - rcPanel.top;
		int widthPanel = rcPanel.right - rcPanel.left;
		int heightClientPart = rcWin.bottom - rcWin.top;
		int widthClientPart = rcWin.bottom - rcWin.top;
		if (heightPanel <= heightClientPart - iMinHeightMdiClient - m_iSizerGap)
		{
			//Size mdi client window
			SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.bottom - heightPanel - m_iSizerGap);
			SetRect(&rcPanel, rcWin.left, rcMdiClient.bottom + m_iSizerGap, rcWin.right, rcWin.bottom);
		}
		else if (iMinHeightMdiClient + iMinHeightPanel + m_iSizerGap <= heightClientPart)
		{
			//The panel's height is too tall so shrink it.
			heightPanel = heightClientPart - iMinHeightMdiClient - m_iSizerGap;
			SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.top + iMinHeightMdiClient);
			SetRect(&rcPanel, rcWin.left, rcMdiClient.bottom + m_iSizerGap, rcWin.right, rcWin.bottom);
		}
		else
		{
			//The the min height panel plus the min height mdiclient window will not fit so zero size the mdiclient and fit only the panel.
			heightPanel = iMinHeightPanel;
			SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.top);
			SetRect(&rcPanel, rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
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
	//if (m_pParentWindow)
	//{
	//	HWND hWndMDIClient = m_pParentWindow->Get_MDIClientWindow();
	//	if (hWndMDIClient)
	//	{
	//		if (rcWin.right - rcWin.left > 0 && rcWin.bottom - rcWin.top > 0)
	//			MoveWindow(hWndMDIClient, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, TRUE);
	//		else
	//			MoveWindow(hWndMDIClient, rcWin.left, rcWin.top, 0, 0, TRUE);
	//	}
	//}
}

CVirWindow *WPanelManager::Get_ParentWindow()
{
	return m_pParentWindow;
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

LRESULT WPanelManager::Splitter_OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	HDC hdc;
	RECT rect;



	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	GetWindowRect(hwnd, &rect);

	//convert the mouse coordinates relative to the top-left of
	//the window
	ClientToScreen(hwnd, &pt);
	pt.x -= rect.left;
	pt.y -= rect.top;
	
	//same for the window coordinates - make them relative to 0,0
	OffsetRect(&rect, -rect.left, -rect.top);
	
	if(pt.y < 0) pt.y = 0;
	if(pt.y > rect.bottom-4) 
	{
		pt.y = rect.bottom-4;
	}

	fDragMode = TRUE;

	SetCapture(hwnd);

	hdc = GetWindowDC(hwnd);
	DrawXorBar(hdc, 1,pt.y - 2, rect.right-2,4);
	ReleaseDC(hwnd, hdc);
	
	oldy = pt.y;
		
	return 0;
}


LRESULT WPanelManager::Splitter_OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT rect;

	POINT pt;
	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	if(fDragMode == FALSE)
		return 0;
	
	GetWindowRect(hwnd, &rect);

	ClientToScreen(hwnd, &pt);
	pt.x -= rect.left;
	pt.y -= rect.top;
	
	OffsetRect(&rect, -rect.left, -rect.top);

	if(pt.y < 0) pt.y = 0;
	if(pt.y > rect.bottom-4) 
	{
		pt.y = rect.bottom-4;
	}

	hdc = GetWindowDC(hwnd);
	DrawXorBar(hdc, 1,oldy - 2, rect.right-2,4);			
	ReleaseDC(hwnd, hdc);

	oldy = pt.y;

	fDragMode = FALSE;

	//convert the splitter position back to screen coords.
	GetWindowRect(hwnd, &rect);
	pt.x += rect.left;
	pt.y += rect.top;

	//now convert into CLIENT coordinates
	ScreenToClient(hwnd, &pt);
	GetClientRect(hwnd, &rect);
	nSplitterPos = pt.y;
	
	//position the child controls
	//TODO
	//SizeWindowContents(rect.right,rect.bottom);

	ReleaseCapture();

	return 0;
}

LRESULT WPanelManager::Splitter_OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT rect;

	POINT pt;

	if(fDragMode == FALSE) return 0;

	pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	pt.y = (short)HIWORD(lParam);

	GetWindowRect(hwnd, &rect);

	ClientToScreen(hwnd, &pt);
	pt.x -= rect.left;
	pt.y -= rect.top;

	OffsetRect(&rect, -rect.left, -rect.top);

	if(pt.y < 0) pt.y = 0;
	if(pt.y > rect.bottom-4) 
	{
		pt.y = rect.bottom-4;
	}

	if(pt.y != oldy && wParam & MK_LBUTTON)
	{
		hdc = GetWindowDC(hwnd);
		DrawXorBar(hdc, 1,oldy - 2, rect.right-2,4);
		DrawXorBar(hdc, 1,pt.y - 2, rect.right-2,4);
			
		ReleaseDC(hwnd, hdc);

		oldy = pt.y;
	}

	return 0;
}


bool WPanelManager::Splitter_OnSetCursor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	if (!GetCursorPos(&pt))
		return false;
	if (!ScreenToClient(hwnd, &pt))
		return false;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = p->Next())
	{
		if (p->m_data == NULL)
			continue;
		WPanel *pwp = p->m_data;
		if (pwp == NULL)
			continue;
		HRGN rgn = pwp->m_hrgSizerTop;
		if (rgn)
		{
			if (PtInRegion(rgn, pt.x, pt.y))
			{
				HCURSOR cur = (HCURSOR )LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENS), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
				SetCursor(cur);
				return true;
			}
		}
	}
	return false;
}

