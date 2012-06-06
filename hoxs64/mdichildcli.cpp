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

#include "IC64.h"

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
#include "wpccli.h"
#include "mdichildcli.h"
#include "mdidebuggerframe.h"

#include "resource.h"

const TCHAR CMDIChildCli::ClassName[] = TEXT("CMDIChildCli");


CMDIChildCli::CMDIChildCli(IC64 *c64, IAppCommand *pIAppCommand, HFONT hFont)
{
	this->c64 = c64;
	this->m_pIAppCommand = pIAppCommand;
	this->m_hFont = hFont;
}

HRESULT CMDIChildCli::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wc; 
 
    // Register the MDI child window class.  
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = ClassName; 
	//wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC) ::MdiChildWindowProc; 
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
	m_pWinMdiFrame = pWinMdiFrame;
	HWND hWnd = ::CreateMDIWindow(ClassName, TEXT("CLI"), WS_MAXIMIZE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWinMdiFrame->Get_MDIClientWindow(), this->GetHinstance(), (LPARAM)this);
	return hWnd;
}

HRESULT CMDIChildCli::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
shared_ptr<WpcCli> pWpcCli;
	try
	{
		pWpcCli = shared_ptr<WpcCli>(new WpcCli(c64, m_pIAppCommand, m_hFont));
		if (pWpcCli == NULL)
			throw std::bad_alloc();
	}
	catch(std::exception&)
	{
	}
	if (pWpcCli != 0)
	{
		RECT rcClient;
		if (GetClientRect(hWnd, &rcClient))
		{
			HWND hWndWpcCli = pWpcCli->Create(this->GetHinstance(), hWnd, NULL, 0, 0, rcClient.right- rcClient.left, rcClient.bottom - rcClient.top, (HMENU)1000);
			if (hWndWpcCli != 0)
			{
				m_pWinWpcCli = pWpcCli;
				return S_OK;
			}
		}
	}
	return E_FAIL;
}

void CMDIChildCli::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return;	
	if (wParam == SIZE_MINIMIZED)
		return;

	int w = LOWORD(lParam);
	int h = HIWORD(lParam);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;

	shared_ptr<WpcCli> pWinWpcCli =  m_pWinWpcCli.lock();
	HWND hWndCli = pWinWpcCli->GetHwnd();
	if (pWinWpcCli!=0 && hWndCli !=0)
	{
		SetWindowPos(hWndCli, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOZORDER);
	}
}

LRESULT CMDIChildCli::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	switch(uMsg)
	{
	case WM_CREATE:
		hr = OnCreate(hWnd, uMsg, wParam, lParam);
		if (FAILED(hr))
			return -1;
		return 0;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		break;
	}
	return ::DefMDIChildProc(hWnd, uMsg, wParam, lParam);
}