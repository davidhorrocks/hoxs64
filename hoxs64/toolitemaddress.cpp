#include <windows.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "errormsg.h"
#include "C64.h"
#include "edln.h"
#include "assembler.h"
#include "toolitemaddress.h"
#include "dchelper.h"
#include "resource.h"

#define WNDCLASS_DFREBARCHILD TEXT("Hoxs64DFRebarChild")

const TCHAR CToolItemAddress::ClassName[] = WNDCLASS_DFREBARCHILD;

CToolItemAddress::CToolItemAddress()
{
	m_pIEnterGotoAddress = NULL;
}

CToolItemAddress::~CToolItemAddress()
{
	Cleanup();
}

HRESULT CToolItemAddress::Init(HFONT hFont)
{
	m_hFont = hFont;
	return S_OK;
}

void CToolItemAddress::Cleanup()
{
}

HRESULT CToolItemAddress::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(CToolItemAddress *);
	wc.hInstance     = hInstance;
	wc.hIcon         = 0;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = 0L;
    wc.lpszClassName = WNDCLASS_DFREBARCHILD;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
}

HWND CToolItemAddress::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

HWND CToolItemAddress::CreateTextBox(HWND hWndParent, int id, int x, int y, int w, int h)
{
	HWND hwnd = CreateWindowEx(0, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_WANTRETURN, x, y, w, h, hWndParent, (HMENU)id, (HINSTANCE) m_hInst, 0); 
	if (!hwnd)
		return 0;
	return hwnd;
}

void CToolItemAddress::SetInterface(IEnterGotoAddress *pIEnterGotoAddress)
{
	m_pIEnterGotoAddress = pIEnterGotoAddress;
}

void CToolItemAddress::GetMinWindowSize(int &w, int &h)
{
	w=0;
	h=0;
}

void CToolItemAddress::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

HRESULT CToolItemAddress::GetDefaultTextBoxSize(HWND hWnd, SIZE& sizeText)
{
RECT rcEdit;
	HDC hdc = GetDC(hWnd);
	if (!hdc)
		return E_FAIL;
	DcHelper dch(hdc);
	dch.UseMapMode(MM_TEXT);
	if (m_hFont)
		dch.UseFont(m_hFont);

	TCHAR s[]= TEXT("$ABCDx");
	int slen = lstrlen(s);
	BOOL br = GetTextExtentExPoint(hdc, s, slen, 0, NULL, NULL, &sizeText);
	if (!br)
		return E_FAIL;
	SetRect(&rcEdit, 0, 0, sizeText.cx, sizeText.cy);
	InflateRect(&rcEdit, 2 * ::GetSystemMetrics(SM_CYBORDER), 2 * ::GetSystemMetrics(SM_CXBORDER));
	OffsetRect(&rcEdit, -rcEdit.left, -rcEdit.top);
	sizeText.cx = rcEdit.right;
	sizeText.cy = rcEdit.bottom;
	return S_OK;
}

HRESULT CToolItemAddress::OnCreate(HWND hWnd)
{
	HRESULT hr;
	SIZE sizeText;
	hr = GetDefaultTextBoxSize(hWnd, sizeText);
	if (FAILED(hr))
		return hr;
	m_hWndTxtAddress = CreateTextBox(hWnd, IDC_TXT_GOTOADDRESS, 0, 0, sizeText.cx, sizeText.cy);
	if (!m_hWndTxtAddress)
		return E_FAIL;
	SendMessage(m_hWndTxtAddress, EM_SETLIMITTEXT, 5, 0);
	if (m_hFont)
		SendMessage(m_hWndTxtAddress, WM_SETFONT, (WPARAM)m_hFont, FALSE);
	m_wpOrigEditProc = SubclassChildWindow(m_hWndTxtAddress);

	return S_OK;
}

void CToolItemAddress::OnDestroy(HWND hWnd)
{
	if (m_wpOrigEditProc!=NULL && m_hWndTxtAddress!=NULL)
	{
		SubclassChildWindow(m_hWndTxtAddress, m_wpOrigEditProc);
		m_wpOrigEditProc= NULL;
	}
}

LRESULT CToolItemAddress::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == NULL)
		return 0;
	if (hWnd == this->m_hWndTxtAddress)
	{
		switch(uMsg)
		{
		case WM_CHAR:
			if (wParam == VK_ESCAPE)
			{				
				return 0;
			}
			else if (wParam == VK_RETURN)
			{				
				OnEnterGotoAddress();
				return 0;
			}
			break;
		}
	}
	if (m_wpOrigEditProc)
	{
		return ::CallWindowProc(m_wpOrigEditProc, hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

bool CToolItemAddress::OnEnterGotoAddress()
{
int c;
	if (m_pIEnterGotoAddress)
	{
		c = G::GetEditLineString(m_hWndTxtAddress, 0, &m_tempAddressTextBuffer[0], _countof(m_tempAddressTextBuffer));
		m_tempAddressTextBuffer[c] = 0;
		return m_pIEnterGotoAddress->OnEnterGotoAddress(&m_tempAddressTextBuffer[0]);
	}
	return false;
}

int CToolItemAddress::GetAddressText(int linenumber, LPTSTR szBuffer, int cchBufferLength)
{
	return G::GetEditLineSzString(m_hWndTxtAddress, 0, szBuffer, cchBufferLength);
}

LRESULT CToolItemAddress::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	switch (uMsg) 
	{
	case WM_CREATE:
		hr = OnCreate(hWnd);
		if (FAILED(hr))
			return -1;
		return 0;
	case WM_DESTROY:
		OnDestroy(hWnd);
		break;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
