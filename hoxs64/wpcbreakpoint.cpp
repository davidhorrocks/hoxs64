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
#include "wpcbreakpoint.h"
#include "resource.h"

const TCHAR WpcBreakpoint::ClassName[] = TEXT("Hoxs64WpcBreakpoint");

#define LVBREAKPOINT (1001)

WpcBreakpoint::WpcBreakpoint()
{
	m_hLvBreak = NULL;
}

WpcBreakpoint::~WpcBreakpoint()
{
}

HRESULT WpcBreakpoint::Init()
{
	return S_OK;
}

HRESULT WpcBreakpoint::RegisterClass(HINSTANCE hInstance)
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

HWND WpcBreakpoint::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

LRESULT WpcBreakpoint::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
	if (pcs == NULL)
		return -1;
	RECT rcWin;
	GetClientRect(hWnd, &rcWin);
	m_hLvBreak = CreateWindowEx(0, WC_LISTVIEWW, TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, hWnd, (HMENU)LVBREAKPOINT, pcs->hInstance, NULL);
	if (!m_hLvBreak)
		return -1;
	hr = InitListViewColumns(m_hLvBreak);
	if (FAILED(hr))
		return -1;
	hr = FillListView();
	if (FAILED(hr))
		return -1;
	return 0;
}

 HRESULT WpcBreakpoint::InitListViewColumns(HWND hWndListView)
 {
	 int r;
	 LVCOLUMN lvc;

	 ZeroMemory(&lvc, sizeof(lvc));
	 lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	 lvc.iSubItem = (int)LvBreakColumnIndex::Cpu;
	 lvc.fmt = LVCFMT_LEFT;
	 lvc.pszText = TEXT("CPU");
	 lvc.cx = 100;
	 r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Cpu, &lvc);
	 if (r == -1)
		 return E_FAIL;

	 ZeroMemory(&lvc, sizeof(lvc));
	 lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	 lvc.iSubItem = (int)LvBreakColumnIndex::Address;
	 lvc.fmt = LVCFMT_LEFT;
	 lvc.pszText = TEXT("Address");
	 lvc.cx = 100;
	 r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Address, &lvc);
	 if (r == -1)
		 return E_FAIL;
	 return S_OK;
}

HRESULT WpcBreakpoint::FillListView()
{
	return S_OK;
}

LRESULT WpcBreakpoint::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int w = (short)LOWORD(lParam);  // horizontal position of cursor 
	int h = (short)HIWORD(lParam);
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	MoveWindow(m_hLvBreak, 0, 0, w, h, TRUE);
	return 0;
}


LRESULT WpcBreakpoint::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;
}