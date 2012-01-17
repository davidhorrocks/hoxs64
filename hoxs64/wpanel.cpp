#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
#include "errormsg.h"

#include "assert.h"
#include "hexconv.h"
#include "cevent.h"
#include "dchelper.h"
#include "wpanel.h"
#include "resource.h"

const TCHAR WPanel::ClassName[] = TEXT("Hoxs64WPanel");


WPanel::WPanel()
{
	m_pIWPanelManager = NULL;
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
		m_pIWPanelManager->OnDestroyWPanel(this);
	}
}

HRESULT WPanel::Init(IWPanelManager *pIWPanelManager)
{
	m_pIWPanelManager = pIWPanelManager;
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

HWND WPanel::Create(HINSTANCE hInstance, const TCHAR title[], int x,int y, int w, int h, HMENU ctrlID)
{
	if (m_pIWPanelManager == NULL)
		return NULL;
	CVirWindow *pParentWindow = m_pIWPanelManager->Get_ParentWindow();
	if (pParentWindow == NULL)
		return NULL;
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_CAPTION |  WS_VISIBLE, x, y, w, h, pParentWindow->GetHwnd(), ctrlID, hInstance);
}

HWND WPanel::Show()
{
	return this->m_hWnd;
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
	case WM_NCHITTEST:
		return OnHitTestNCA(m_hWnd, uMsg, wParam, lParam);
	case WM_SYSCOMMAND:
		return OnSysCommand(m_hWnd, uMsg, wParam, lParam);
	case WM_DESTROY:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
