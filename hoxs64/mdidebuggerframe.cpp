#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <vector>
#include <list>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"

#include "errormsg.h"

#include "C64.h"

#include "utils.h"
#include "edln.h"
#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpcbreakpoint.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "mdidebuggerframe.h"
#include "resource.h"

#define TOOLBUTTON_WIDTH_96 (16)
#define TOOLBUTTON_HEIGHT_96 (16)


const TCHAR CMDIDebuggerFrame::ClassName[] = TEXT("Hoxs64MDIDebuggerFrame");
const TCHAR CMDIDebuggerFrame::MenuName[] = TEXT("MENU_MDI_DEBUGGER");

//TCHAR CMDIDebuggerFrame::ChildClassName[] = TEXT("Hoxs64MDIDebuggerChild");

const ImageInfo CMDIDebuggerFrame::TB_ImageList[] = 
{
	{IDB_DEBUGGERTRACE, IDB_DEBUGGERTRACEMASK},
	{IDB_DEBUGGERTRACEFRAME, IDB_DEBUGGERTRACEFRAMEMASK},
	{IDB_DEBUGGERSTOP, IDB_DEBUGGERSTOPMASK}
};

const ButtonInfo CMDIDebuggerFrame::TB_StepButtons[] = 
{
	{0, TEXT("Trace"), IDM_STEP_TRACE},
	{1, TEXT("Trace Frame"), IDM_STEP_TRACEFRAME},
	{2, TEXT("Stop"), IDM_STEP_STOP}
};

CMDIDebuggerFrame::CMDIDebuggerFrame(C64 *c64, IMonitorCommand *pMonitorCommand, CConfig *cfg, CAppStatus *appStatus)
	:
	c64(c64),
	m_debugCpuC64(CPUID_MAIN, c64, pMonitorCommand, TEXT("C64 - cpu")),
	m_debugCpuDisk(CPUID_DISK, c64, pMonitorCommand, TEXT("Disk - cpu"))
{
	m_hWndMDIClient = NULL;
	m_hWndRebar = NULL;
	m_hWndTooBar = NULL;
	m_hImageListToolBarNormal = NULL;
	m_bIsCreated = false;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->m_pMonitorCommand = pMonitorCommand;
}

CMDIDebuggerFrame::~CMDIDebuggerFrame()
{
	Cleanup();
}

HRESULT CMDIDebuggerFrame::Init()
{
HRESULT hr;

	do
	{
		hr = m_debugCpuC64.Init(this);
		if (FAILED(hr))
			break;

		hr = m_debugCpuDisk.Init(this);
		if (FAILED(hr))
			break;

		hr = AdviseEvents();
		if (FAILED(hr))
			break;

		hr = S_OK;
	} while (false);
	if (FAILED(hr))
	{
		Cleanup();
		return hr;
	}
	else
	{
		return S_OK;
	}
};

void CMDIDebuggerFrame::Cleanup()
{
	UnadviseEvents();

	if (m_hImageListToolBarNormal != NULL)
	{
		ImageList_Destroy(m_hImageListToolBarNormal);
		m_hImageListToolBarNormal = NULL;
	}
}

HRESULT CMDIDebuggerFrame::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wc; 
 
    // Register the frame window class. 
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

    wc.style         = 0; 
    wc.lpfnWndProc   = (WNDPROC)::WindowProc; 
    wc.cbClsExtra    = 0; 
    wc.cbWndExtra    = sizeof(CMDIDebuggerFrame *); 
    wc.hInstance     = hInstance; 
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHIP1)); 
	//wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL)); 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1); 
    wc.lpszMenuName  = MenuName; 
    wc.lpszClassName = ClassName; 
    if (!RegisterClassEx(&wc) ) 
        return E_FAIL; 
 
    //// Register the MDI child window class.  
    //wc.lpfnWndProc   = (WNDPROC) MPMDIChildWndProc; 
    //wc.hIcon         = LoadIcon(hInst, IDNOTE); 
    //wc.lpszMenuName  = (LPCTSTR) NULL; 
    //wc.cbWndExtra    = CBWNDEXTRA; 
    //wc.lpszClassName = ChildClassName; 
 
    //if (!RegisterClassEx(&wc)) 
    //    return FALSE; 
 
    return S_OK; 

};

void CMDIDebuggerFrame::OnTrace(void *sender, EventArgs& e)
{
	SetMenuState();
}

void CMDIDebuggerFrame::OnShowDevelopment(void *sender, EventArgs& e)
{
	SetMenuState();
}

void CMDIDebuggerFrame::SetMenuState()
{
	if (!m_pMonitorCommand)
		return ;
	if (!m_hWnd)
		return ;
	HMENU hMenu = GetMenu(m_hWnd);
	if (!hMenu)
		return ;
	UINT state;
	UINT stateOpp;
	UINT stateTb;
	UINT stateTbOpp;
	if (m_pMonitorCommand->IsRunning())
	{
		state = MF_DISABLED | MF_GRAYED;
		stateOpp = MF_ENABLED;
		stateTb = 0;
		stateTbOpp = TBSTATE_ENABLED;
	}
	else
	{
		state = MF_ENABLED;
		stateOpp = MF_DISABLED | MF_GRAYED;
		stateTb = TBSTATE_ENABLED;
		stateTbOpp = 0;
	}
	EnableMenuItem(hMenu, IDM_STEP_TRACE, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_TRACEFRAME, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_STOP, MF_BYCOMMAND | stateOpp);

	if (m_hWndTooBar!=NULL)
	{
		SendMessage(m_hWndTooBar, TB_SETSTATE, IDM_STEP_TRACE, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, IDM_STEP_TRACEFRAME, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, IDM_STEP_STOP, stateTbOpp);
	}
}

HWND CMDIDebuggerFrame::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_OVERLAPPED | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, hWndParent, hMenu, hInstance);
}

HRESULT CMDIDebuggerFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	hr = CreateMDIToolBars();
	if (FAILED(hr))
		return hr;
	HWND hWndMdiClient = CreateMDIClientWindow(IDC_MAIN_MDI, IDM_WINDOWCHILD);
	if (hWndMdiClient==NULL)
		return E_FAIL;

	hr = m_WPanelManager.Init(this->GetHinstance(), this, m_hWndRebar);
	if (FAILED(hr))
		return hr;

	WpcBreakpoint *pWin = new WpcBreakpoint(c64, m_pMonitorCommand);	
	hr = pWin->Init();
	if (SUCCEEDED(hr))
	{
		hr = m_WPanelManager.CreateNewPanel(WPanel::InsertionStyle::Bottom, TEXT("Breakpoints"), pWin);
		if (SUCCEEDED(hr))
		{
			pWin->m_AutoDelete = true;
			pWin = NULL;
		}
	}
	if (pWin)
	{
		delete pWin;
		pWin = NULL;
	}
	m_bIsCreated = SUCCEEDED(hr);
	return hr;
}

void CMDIDebuggerFrame::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pMonitorCommand)
		m_pMonitorCommand->Resume();
}

void CMDIDebuggerFrame::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (::IsWindow(m_hWnd))
	{
		if (m_bIsCreated)
			CConfig::SaveMDIWindowSetting(m_hWnd);
	}
	m_bIsCreated = false;
}

void CMDIDebuggerFrame::OnMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

void CMDIDebuggerFrame::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != this->m_hWnd)
		return;
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return;
	
	if (wParam == SIZE_MINIMIZED)
		return;

	int w = LOWORD(lParam);
	int h = HIWORD(lParam);

	RECT rcRootPanel;
	SetRect(&rcRootPanel, 0, 0, w, h);

	if (m_hWndRebar != NULL)
	{
		RECT rcAbsRebar;
		BOOL br = GetWindowRect(m_hWndRebar, &rcAbsRebar);
		if (br)
		{
			int heightRebar = rcAbsRebar.bottom - rcAbsRebar.top;

			SetWindowPos(m_hWndRebar, 0, 0, 0, w, heightRebar, SWP_NOREPOSITION | SWP_NOZORDER);

			SetRect(&rcRootPanel, 0, heightRebar, w, h);
		}
	}

	this->m_WPanelManager.SizePanels(hWnd, rcRootPanel.left, rcRootPanel.top, rcRootPanel.right - rcRootPanel.left, rcRootPanel.bottom - rcRootPanel.top);
}

HRESULT CMDIDebuggerFrame::CreateMDIToolBars()
{
	if (m_hImageListToolBarNormal == NULL)
	{
		m_hImageListToolBarNormal = CreateImageListNormal(m_hWnd);
		if (m_hImageListToolBarNormal == NULL)
			return E_FAIL;
	}

	m_hWndTooBar = G::CreateToolBar(m_hInst, m_hWnd, ID_TOOLBAR, m_hImageListToolBarNormal, TB_StepButtons, _countof(TB_StepButtons), m_dpi.ScaleX(TOOLBUTTON_WIDTH_96), m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96));
	if (m_hWndTooBar == NULL)
		return E_FAIL;
	m_hWndRebar = G::CreateRebar(m_hInst, m_hWnd, m_hWndTooBar, ID_RERBAR, IDB_REBARBKGND1);
	if (m_hWndRebar == NULL)
		return E_FAIL;
	return S_OK;
}

HIMAGELIST CMDIDebuggerFrame::CreateImageListNormal(HWND hWnd)
{
	int tool_dx = m_dpi.ScaleX(TOOLBUTTON_WIDTH_96);
	int tool_dy = m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96);
	return G::CreateImageListNormal(m_hInst, hWnd, tool_dx, tool_dy, TB_ImageList, _countof(TB_ImageList));
}

void CMDIDebuggerFrame::ShowDebugCpuC64(DBGSYM::DisassemblyPCUpdateMode pcmode, bit16 address)
{
	HRESULT hr = m_debugCpuC64.Show();
	if (SUCCEEDED(hr))
	{
		m_debugCpuC64.UpdateDisplay(pcmode, address);
	}
}

void CMDIDebuggerFrame::ShowDebugCpuDisk(DBGSYM::DisassemblyPCUpdateMode pcmode, bit16 address)
{
	HRESULT hr = m_debugCpuDisk.Show();
	if (SUCCEEDED(hr))
	{
		m_debugCpuDisk.UpdateDisplay(pcmode, address);
	}
}

void CMDIDebuggerFrame::OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int w,h;
MINMAXINFO *pMinMax = (MINMAXINFO *)lParam;
	this->GetMinWindowSize(w, h);
	pMinMax->ptMinTrackSize.x = w;
	pMinMax->ptMinTrackSize.y = h;
}

void CMDIDebuggerFrame::GetMinWindowSize(int &w, int &h)
{
	w = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
	h = GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
	
	UINT heightRebar = 0;
	if (m_hWndRebar!=0)
	{
		UINT t =(UINT)SendMessage(m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
		heightRebar = t;
	}

	h += heightRebar;
}

bool CMDIDebuggerFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case IDM_DEBUG_CPUC64:
		ShowDebugCpuC64(DBGSYM::None, 0);
		return true;
	case IDM_DEBUG_CPUDISK:
		ShowDebugCpuDisk(DBGSYM::None, 0);
		return true;
	case IDM_STEP_TRACEFRAME:
		if (!m_pMonitorCommand)
			return false;
		if (m_pMonitorCommand->IsRunning())
			return false;
		m_pMonitorCommand->TraceFrame();
		m_pMonitorCommand->UpdateApplication();
		return true;
	case IDM_STEP_TRACE:
		if (!m_pMonitorCommand)
			return false;
		this->m_pMonitorCommand->Trace();
		return true;
	case IDM_FILE_MONITOR:
	case IDM_STEP_STOP:
		if (!m_pMonitorCommand)
			return false;
		this->m_pMonitorCommand->ShowDevelopment();
		return true;
	case IDM_BREAKPOINT_DELETEALLBREAKPOINTS:
		this->m_pMonitorCommand->DeleteAllBreakpoints();
		return true;
	default:
		return false;
	}
}

LRESULT CMDIDebuggerFrame::OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
bool bHandled = m_WPanelManager.Splitter_OnSetCursor(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return TRUE;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CMDIDebuggerFrame::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnLButtonDown(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}

LRESULT CMDIDebuggerFrame::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnMouseMove(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}

LRESULT CMDIDebuggerFrame::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnLButtonUp(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}


LRESULT CMDIDebuggerFrame::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;

	switch(uMsg)
	{
	case WM_CREATE:
		hr = OnCreate(uMsg, wParam, lParam);
		if (FAILED(hr))
			return -1;
		ShowWindow(m_hWndMDIClient, SW_SHOW); 
		return 0;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_SETCURSOR:
		return OnSetCursor(hWnd, uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(hWnd, uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, uMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(hWnd, uMsg, wParam, lParam);
	case WM_MOVE:
		OnMove(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		OnClose(m_hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_DESTROY:
		OnDestroy(m_hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPU64:
		OnBreakCpu64(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPUDISK:
		OnBreakCpuDisk(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_ENTERMENULOOP:
		m_pMonitorCommand->SoundOff();
		return 0;
	case WM_EXITMENULOOP:
		m_pMonitorCommand->SoundOn();
		return 0;
	case WM_ENTERSIZEMOVE:
		m_pMonitorCommand->SoundOff();
		return 0;
	case WM_EXITSIZEMOVE:
		m_pMonitorCommand->SoundOn();
		return 0;
	default:
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;	
}

void CMDIDebuggerFrame::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowDebugCpuC64(DBGSYM::EnsurePCVisible, 0);
}

void CMDIDebuggerFrame::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowDebugCpuDisk(DBGSYM::EnsurePCVisible, 0);
}

HRESULT CMDIDebuggerFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{

		hs = m_pMonitorCommand->EsShowDevelopment.Advise((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsTrace.Advise((CMDIDebuggerFrame_EventSink_OnTrace *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hr = S_OK;
	} while (false);
	return hr;
}

void CMDIDebuggerFrame::UnadviseEvents()
{
	((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this)->UnadviseAll();
	((CMDIDebuggerFrame_EventSink_OnTrace *)this)->UnadviseAll();
}
