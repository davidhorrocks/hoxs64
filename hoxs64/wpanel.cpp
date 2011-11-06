#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>

#include "defines.h"
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

HRESULT WPanel::Init()
{
	return S_OK;
}

HWND WPanel::Create(HINSTANCE hInstance, CVirWindow *pParentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU ctrlID)
{
	if (pParentWindow == NULL)
		return NULL;
	return CVirWindow::Create(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, pParentWindow->GetHwnd(), ctrlID, hInstance);
}

HWND WPanel::Show()
{
	return this->m_hWnd;
}

