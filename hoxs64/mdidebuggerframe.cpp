#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"

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
#include "dchelper.h"
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
	{0, TEXT("Trace"), TEXT("Trace"), BTNS_BUTTON, IDM_STEP_TRACE},
	{1, TEXT("Trace Frame"), TEXT("Trace 1 frame"), BTNS_BUTTON, IDM_STEP_TRACEFRAME},
	{2, TEXT("Stop"), TEXT("Stop tracing"), BTNS_BUTTON, IDM_STEP_STOP}
};

CMDIDebuggerFrame::CMDIDebuggerFrame(IC64 *c64, IAppCommand *pAppCommand, CConfig *cfg, CAppStatus *appStatus)
	:
	c64(c64)
{
	m_hWndRebar = NULL;
	m_hWndTooBar = NULL;
	m_hImageListToolBarNormal = NULL;
	m_hBmpRebarNotSized = NULL;
	m_bIsCreated = false;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->m_pAppCommand = pAppCommand;

	HRESULT hr = Init();
	if (FAILED(hr))
		throw new std::runtime_error("CMDIDebuggerFrame::Init() Failed");
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
		hr = InitFonts();
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
	for (std::vector<HBITMAP>::iterator it = m_vec_hBmpRebarSized.begin(); it != m_vec_hBmpRebarSized.end(); it++)
	{
		if (*it != NULL)
		{
			DeleteObject(*it);
			*it = NULL;
		}
	}
	m_vec_hBmpRebarSized.clear();

	if (m_hBmpRebarNotSized != NULL)
	{
		DeleteObject(m_hBmpRebarNotSized);
		m_hBmpRebarNotSized = NULL;
	}
	CloseFonts();
}

HRESULT CMDIDebuggerFrame::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wc; 
 
    // Register the frame window class. 
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

    wc.style         = 0; 
    wc.lpfnWndProc   = (WNDPROC)::MdiFrameWindowProc; 
    wc.cbClsExtra    = 0; 
    wc.cbWndExtra    = sizeof(CMDIDebuggerFrame *); 
    wc.hInstance     = hInstance; 
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHIP1)); 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1); 
    wc.lpszMenuName  = MenuName; 
    wc.lpszClassName = ClassName; 
    if (!RegisterClassEx(&wc) ) 
        return E_FAIL; 
 
    return S_OK; 

};

bool CMDIDebuggerFrame::IsWinDlgModelessBreakpointVicRaster()
{
	Sp_CDiagBreakpointVicRaster pdlg = m_pdlgModelessBreakpointVicRaster.lock();
	return pdlg != NULL && IsWindow(pdlg->GetHwnd());
}

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
	if (!m_pAppCommand)
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
	if (m_pAppCommand->IsRunning())
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
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_OVERLAPPEDWINDOW, x, y, w, h, hWndParent, hMenu, hInstance);
}

#define WINDOWMENU (3)
HRESULT CMDIDebuggerFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr = E_FAIL;

	try
	{
		HDC hdc = GetDC(m_hWnd);
		if (!hdc)
			return E_FAIL;
		DcHelper dch(hdc);

		m_hBmpRebarNotSized = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_REBARBKGND1));

		hr = CreateMDIToolBars(hdc);
		if (FAILED(hr))
			return hr;
		HWND hWndMdiClient = CreateMDIClientWindow(IDC_MAIN_MDI, IDM_WINDOWCHILD, WINDOWMENU);
		if (hWndMdiClient==NULL)
			return E_FAIL;

		hr = m_WPanelManager.Init(this->GetHinstance(), this->shared_from_this(), m_hWndRebar);
		if (FAILED(hr))
			return hr;

		shared_ptr<WpcBreakpoint> pWin = shared_ptr<WpcBreakpoint>(new WpcBreakpoint(c64, m_pAppCommand));
		if (!pWin)
			throw std::bad_alloc();
		hr = m_WPanelManager.CreateNewPanel(WPanel::InsertionStyle::Bottom, TEXT("Breakpoints"), pWin);
		if (FAILED(hr))
			return hr;

		m_bIsCreated = true;
	}
	catch(std::exception&)
	{
		 hr = E_FAIL;
	}
	return hr;
}

void CMDIDebuggerFrame::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pAppCommand)
		m_pAppCommand->Resume();
}

void CMDIDebuggerFrame::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (this->IsWinDlgModelessBreakpointVicRaster())
	{
		Sp_CDiagBreakpointVicRaster pdlg = m_pdlgModelessBreakpointVicRaster.lock();
		DestroyWindow(pdlg->GetHwnd());
	}
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

HRESULT CMDIDebuggerFrame::CreateMDIToolBars(HDC hdc) throw(...)
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
	m_hWndRebar = G::CreateRebar(m_hInst, m_hWnd, m_hWndTooBar, ID_RERBAR);
	if (m_hWndRebar == NULL)
		return E_FAIL;

	if (m_hBmpRebarNotSized)
	{
		int iCountBands = (int)SendMessage(m_hWndRebar, RB_GETBANDCOUNT, 0, 0);		
		for (int iBandNext = 0; iBandNext < iCountBands; iBandNext++)
		{
			int heightBand = (int)SendMessage(m_hWndRebar, RB_GETROWHEIGHT, iBandNext, 0);
			HBITMAP hBmpSz = G::CreateResizedBitmap(hdc, m_hBmpRebarNotSized, iBandNext, heightBand, false, true);
			if (hBmpSz)
			{
				m_vec_hBmpRebarSized.push_back(hBmpSz);
				G::SetRebarBandBitmap(m_hWndRebar, iBandNext, hBmpSz);
			}
		}
	}

	return S_OK;
}

HIMAGELIST CMDIDebuggerFrame::CreateImageListNormal(HWND hWnd)
{
	int tool_dx = m_dpi.ScaleX(TOOLBUTTON_WIDTH_96);
	int tool_dy = m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96);
	return G::CreateImageListNormal(m_hInst, hWnd, tool_dx, tool_dy, TB_ImageList, _countof(TB_ImageList));
}

void CMDIDebuggerFrame::ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address) throw()
{
HRESULT hr;
	try
	{
		shared_ptr<CDisassemblyFrame> pwin = m_pWinDebugCpuC64.lock();
		if (pwin == NULL)
		{
			pwin = shared_ptr<CDisassemblyFrame>(new CDisassemblyFrame(CPUID_MAIN, c64, m_pAppCommand, TEXT("C64 - cpu"), m_hFont));
			m_pWinDebugCpuC64 = pwin;
		}
		if (pwin != NULL)
		{
			hr = pwin->Show(this->shared_from_this());
			if (SUCCEEDED(hr))
			{
				pwin->UpdateDisplay(pcmode, address);
			}
		}
	}
	catch(std::exception&)
	{
	}
}

void CMDIDebuggerFrame::ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address)
{
HRESULT hr;
	try
	{
		shared_ptr<CDisassemblyFrame> pwin = m_pWinDebugCpuDisk.lock();
		if (pwin == NULL)
		{
			pwin = shared_ptr<CDisassemblyFrame>(new CDisassemblyFrame(CPUID_DISK, c64, m_pAppCommand, TEXT("Disk - cpu"), m_hFont));		
			m_pWinDebugCpuDisk = pwin;
		}
		if (pwin != NULL)
		{
			hr = pwin->Show(this->shared_from_this());
			if (SUCCEEDED(hr))
			{
				pwin->UpdateDisplay(pcmode, address);
			}
		}
	}
	catch(std::exception&)
	{
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

void CMDIDebuggerFrame::OpenNewCli()
{
	try
	{
		shared_ptr<CMDIChildCli> pWinMdiChild = shared_ptr<CMDIChildCli>(new CMDIChildCli(c64, m_pAppCommand, m_hFont));
		if (pWinMdiChild == 0)
			throw std::bad_alloc();
		HWND hWndMdiChildCli = pWinMdiChild->Create(this->shared_from_this());
		::ShowWindow(hWndMdiChildCli, SW_SHOW);
		::UpdateWindow(hWndMdiChildCli);
	}
	catch(...)
	{
	}
}
bool CMDIDebuggerFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case IDM_FILE_NEWCLI:
		OpenNewCli();
		return true;
	case IDM_DEBUG_CPUC64:
		ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::None, 0);
		return true;
	case IDM_DEBUG_CPUDISK:
		ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::None, 0);
		return true;
	case IDM_STEP_TRACEFRAME:
		if (!m_pAppCommand)
			return false;
		if (m_pAppCommand->IsRunning())
			return false;
		m_pAppCommand->TraceFrame();
		m_pAppCommand->UpdateApplication();
		return true;
	case IDM_STEP_TRACE:
		if (!m_pAppCommand)
			return false;
		this->m_pAppCommand->Trace();
		return true;
	case IDM_FILE_MONITOR:
	case IDM_STEP_STOP:
		if (!m_pAppCommand)
			return false;
		this->m_pAppCommand->ShowDevelopment();
		return true;
	case IDM_BREAKPOINT_DELETEALLBREAKPOINTS:
		c64->GetMon()->BM_DeleteAllBreakpoints();
		return true;
	case IDM_BREAKPOINT_ENABLEALLBREAKPOINTS:
		c64->GetMon()->BM_EnableAllBreakpoints();
		return true;
	case IDM_BREAKPOINT_DISABLEALLBREAKPOINTS:
		c64->GetMon()->BM_DisableAllBreakpoints();
		return true;
	case IDM_BREAKPOINT_VICRASTER:
		//ShowDlgBreakpointVicRaster();
		ShowModelessDlgBreakpointVicRaster();
		return TRUE;
	default:
		return false;
	}
}

void CMDIDebuggerFrame::ShowDlgBreakpointVicRaster()
{
	Sp_CDiagBreakpointVicRaster pdlg(new CDiagBreakpointVicRaster(this->m_pAppCommand, this->c64));	
	if (pdlg == 0)
		return;
	pdlg->ShowDialog(this->m_hInst, MAKEINTRESOURCE(IDD_BRKVICRASTER), this->m_hWnd);
}

void CMDIDebuggerFrame::ShowModelessDlgBreakpointVicRaster()
{
	if (!this->IsWinDlgModelessBreakpointVicRaster())
	{
		Sp_CDiagBreakpointVicRaster pdlg(new CDiagBreakpointVicRaster(this->m_pAppCommand, this->c64));	
		if (pdlg == 0)
			return;
		HWND hWndOwner;
		//hWndOwner = this->m_hWnd;
		hWndOwner = this->m_pAppCommand->GetMainFrameWindow();

		HWND hwnd = pdlg->ShowModelessDialog(this->m_hInst, MAKEINTRESOURCE(IDD_BRKVICRASTER), hWndOwner);
		if (hwnd)
		{
			int x = 0, y = 0;
			RECT rcMain, rcDlg;
			bool bAutoPos = false;
			if (GetWindowRect(hWndOwner, &rcMain))
			{
				if (GetWindowRect(hwnd, &rcDlg))
				{
					OffsetRect(&rcDlg, -(rcDlg.left - rcMain.left) - (rcDlg.right-rcDlg.left), 0);
					if (rcDlg.left < 0)
					{
						OffsetRect(&rcDlg, -rcDlg.left, 0);
					}
					bAutoPos = true;
					x = rcDlg.left;
					y = rcDlg.top;
					SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
				}
			}

			ShowWindow(hwnd, SW_SHOW);
			this->m_pdlgModelessBreakpointVicRaster = pdlg;
		}
	}
	else
	{
		Sp_CDiagBreakpointVicRaster pdlg = m_pdlgModelessBreakpointVicRaster.lock();
		if (pdlg)
		{
			HWND hwnd = pdlg->GetHwnd();
			ShowWindow(hwnd, SW_SHOW);
			SetForegroundWindow(hwnd);
		}
	}
}

bool CMDIDebuggerFrame::OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_WPanelManager.Splitter_OnSetCursor(hWnd, uMsg, wParam, lParam);
}

bool CMDIDebuggerFrame::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_WPanelManager.Splitter_OnLButtonDown(hWnd, uMsg, wParam, lParam);
}

bool CMDIDebuggerFrame::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_WPanelManager.Splitter_OnMouseMove(hWnd, uMsg, wParam, lParam);
}

bool CMDIDebuggerFrame::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_WPanelManager.Splitter_OnLButtonUp(hWnd, uMsg, wParam, lParam);
}


LRESULT CMDIDebuggerFrame::MdiFrameWindowProc(HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;

	switch(uMsg)
	{
	case WM_CREATE:
		hr = OnCreate(uMsg, wParam, lParam);
		if (FAILED(hr))
			return -1;
		return 0;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_SETCURSOR:
		if (OnSetCursor(hWnd, uMsg, wParam, lParam))
			return TRUE;
		break;
	case WM_LBUTTONDOWN:
		if (OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_MOUSEMOVE:
		if (OnMouseMove(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONUP:
		if (OnLButtonUp(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_MOVE:
		OnMove(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		OnClose(m_hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		OnDestroy(m_hWnd, uMsg, wParam, lParam);
		break;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPU64:
		OnBreakCpu64(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPUDISK:
		OnBreakCpuDisk(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_VICRASTER:
		OnBreakVic(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_ENTERMENULOOP:
		m_pAppCommand->SoundOff();
		return 0;
	case WM_EXITMENULOOP:
		m_pAppCommand->SoundOn();
		return 0;
	case WM_ENTERSIZEMOVE:
		m_pAppCommand->SoundOff();
		return 0;
	case WM_EXITSIZEMOVE:
		m_pAppCommand->SoundOn();
		return 0;
	}
	return ::DefFrameProc(m_hWnd, hWndMDIClient, uMsg, wParam, lParam);
}

void CMDIDebuggerFrame::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CMDIDebuggerFrame::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CMDIDebuggerFrame::OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

HRESULT CMDIDebuggerFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{
		hs = m_pAppCommand->EsShowDevelopment.Advise((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pAppCommand->EsTrace.Advise((CMDIDebuggerFrame_EventSink_OnTrace *)this);
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


HRESULT CMDIDebuggerFrame::InitFonts()
{
	CloseFonts();
	m_hFont = G::CreateMonitorFont();
	if (m_hFont == 0)
		return E_FAIL;
	return S_OK;
}

void CMDIDebuggerFrame::CloseFonts()
{
	if (m_hFont)
	{
		DeleteObject((HGDIOBJ) m_hFont);
		m_hFont=0;
	}
}
