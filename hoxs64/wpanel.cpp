#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hexconv.h"
#include "dchelper.h"
#include "wpanel.h"
#include "resource.h"

#define PANELCHILDID (1001)

const TCHAR WPanel::ClassName[] = TEXT("Hoxs64WPanel");


WPanel::WPanel()
{
	m_pIWPanelManager = NULL;
	m_pChildWin = NULL;
	m_szPreferredSize.cx = 0;
	m_szPreferredSize.cy = 0;
	m_hrgSizerTop = NULL;
}

WPanel::~WPanel()
{
	if (m_pIWPanelManager)
	{
		if (m_hrgSizerTop)
		{
			DeleteObject(m_hrgSizerTop);
			m_hrgSizerTop = NULL;
		}
	}
}

HRESULT WPanel::Init(IWPanelManager *pIWPanelManager, Sp_CVirWindow pChildWin)
{
	m_pIWPanelManager = pIWPanelManager;
	m_pChildWin = pChildWin;
	m_szPreferredSize.cx = 150;
	m_szPreferredSize.cy = 200;
	m_dpi.ScaleSize(&m_szPreferredSize);

	m_hrgSizerTop = CreateRectRgn(0, 0, 1, 1);
	if(!m_hrgSizerTop)
		return E_FAIL;

	return S_OK;
}

void WPanel::UpdateSizerRegion(const RECT& rcWindow)
{
	if (m_hrgSizerTop && m_pIWPanelManager)
	{
		RECT rc;
		CopyRect(&rc, &rcWindow);
		int gap = m_pIWPanelManager->Get_SizerGap();
		SetRectRgn(m_hrgSizerTop, rc.left, rc.top - gap, rc.right, rc.top);
	}
}

void WPanel::GetParentRect(RECT *prcParent)
{
	m_pIWPanelManager->Get_RootRect(prcParent);
}

HRESULT WPanel::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_OWNDC;//; CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(WPanel *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
}

void WPanel::GetPreferredSize(SIZE *psz)
{
	if (psz)
		*psz = m_szPreferredSize;
}

void WPanel::SetPreferredSize(const SIZE *psz)
{
	if (psz)
		m_szPreferredSize = *psz;
}

void WPanel::GetCurrentSize(SIZE *psz)
{
	if (psz)
	{
		psz->cx = 0;
		psz->cy = 0;
		HWND hWnd = this->GetHwnd();
		if (hWnd)
		{
			RECT rc;
			if (GetWindowRect(hWnd, &rc))
			{
				OffsetRect(&rc, -rc.left, -rc.top);
				psz->cx = rc.right;
				psz->cy = rc.bottom;
			}
		}
	}
}

HWND WPanel::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	if (m_pIWPanelManager == NULL)
		return NULL;
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_CHILD | WS_CAPTION |  WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

HWND WPanel::Show()
{
	return this->m_hWnd;
}

LRESULT WPanel::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int w = (short)LOWORD(lParam);  // horizontal position of cursor 
	int h = (short)HIWORD(lParam);
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	if (this->m_pChildWin)
		m_pChildWin->SetSize(w, h);
	return 0;
}

LRESULT WPanel::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pChildWin == NULL)
		return -1;
	RECT rcWin;
	GetClientRect(hWnd, &rcWin);
	int x, y, w, h;
	x = rcWin.left;
	y = rcWin.top;
	w = rcWin.right - rcWin.left;
	h = rcWin.bottom - rcWin.top;
	HWND hWndPanelChild = m_pChildWin->Create(m_hInst, hWnd, TEXT(""), x, y, w, h, (HMENU)PANELCHILDID);
	if (!hWndPanelChild)
		return -1;
	return 0;
}

LRESULT WPanel::OnSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(wParam & 0xfff0)
	{
	case SC_MOVE:
	case SC_MAXIMIZE:
	case SC_MINIMIZE:
	case SC_RESTORE:
		return 0;
	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
}

LRESULT WPanel::OnHitTestNCA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT WPanel::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(m_hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize(m_hWnd, uMsg, wParam, lParam);
	case WM_NCHITTEST:
		return OnHitTestNCA(m_hWnd, uMsg, wParam, lParam);
	case WM_SYSCOMMAND:
		return OnSysCommand(m_hWnd, uMsg, wParam, lParam);
	case WM_DESTROY:
		Sp_WPanel p= Sp_WPanel(this, null_deleter());
		//m_pIWPanelManager->OnDestroyWPanel(p);
		break;
	}
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
