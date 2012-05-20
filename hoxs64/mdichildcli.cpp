#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "dchelper.h"

#include "C64.h"

#include "utils.h"
#include "edln.h"
#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpcbreakpoint.h"
#include "toolitemaddress.h"
#include "diagbreakpointvicraster.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "mdichildcli.h"
#include "mdidebuggerframe.h"

#include "resource.h"

const TCHAR CMDIChildCli::ClassName[] = TEXT("CMDIChildCli");

HRESULT CMDIChildCli::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wc; 
 
    // Register the MDI child window class.  
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = ClassName; 
    wc.lpfnWndProc   = (WNDPROC) ::MdiChildWindowProc; 
    //wc.cbWndExtra    = CBWNDEXTRA; 
    wc.cbWndExtra    = sizeof(CMDIChildCli *); 
    wc.hInstance     = hInstance; 
    wc.lpszMenuName  = (LPCTSTR) NULL; 
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHIP1)); 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1); 
    wc.lpszClassName = ClassName; 
    if (!RegisterClassEx(&wc) ) 
        return E_FAIL; 
 
    return S_OK; 
};


HWND CMDIChildCli::Create(shared_ptr<CVirMdiFrameWindow> pWinMdiFrame)
{
	HWND hWnd = ::CreateMDIWindow(ClassName, TEXT("CLI"), WS_MAXIMIZE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWinMdiFrame->Get_MDIClientWindow(), this->GetHinstance(), (LPARAM)this);
	if (hWnd)
	{
		m_pWinMdiFrame = pWinMdiFrame;
	}
	return hWnd;
}

LRESULT CMDIChildCli::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		//hr = OnCreate(uMsg, wParam, lParam);
		//if (FAILED(hr))
		//	return -1;
		return 0;
	}
	return ::DefMDIChildProc(hWnd, uMsg, wParam, lParam);
}