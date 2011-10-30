#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "user_message.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"
#include "filter.h"
#include "sid.h"
#include "sidfile.h"
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "diskinterface.h"
#include "t64.h"
#include "C64.h"

#include "cevent.h"
#include "monitor.h"
#include "edln.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "mdidebuggerframe.h"
#include "resource.h"


const TCHAR CMDIDebuggerFrame::ClassName[] = TEXT("Hoxs64MDIDebuggerFrame");
const TCHAR CMDIDebuggerFrame::MenuName[] = TEXT("MENU_MDI_DEBUGGER");

//TCHAR CMDIDebuggerFrame::ChildClassName[] = TEXT("Hoxs64MDIDebuggerChild");

const ImageInfo CMDIDebuggerFrame::TB_ImageList[] = 
{
	{IDB_DEBUGGERSTEPONECLOCK, IDB_DEBUGGERSTEPONECLOCKMASK},
	{IDB_DEBUGGERSTEPIN, IDB_DEBUGGERSTEPINMASK},
	{IDB_DEBUGGERTRACE, IDB_DEBUGGERTRACEMASK},
	{IDB_DEBUGGERTRACEFRAME, IDB_DEBUGGERTRACEFRAMEMASK},
	{IDB_DEBUGGERSTOP, IDB_DEBUGGERSTOPMASK}
};

const ButtonInfo CMDIDebuggerFrame::TB_StepButtons[] = 
{
	{0, TEXT("1 Clock"), ID_STEP_ONECLOCK},
	{1, TEXT("1 Instruction"), ID_STEP_ONEINSTRUCTION},
	{2, TEXT("Trace"), ID_STEP_TRACE},
	{3, TEXT("Trace Frame"), ID_STEP_TRACEFRAME},
	{4, TEXT("Stop"), ID_STEP_STOP}
};

CMDIDebuggerFrame::CMDIDebuggerFrame()
{
	m_AutoDelete = false;
	m_hwndMDIClient = NULL;
	m_hWndRebar = NULL;
	m_hWndTooBar = NULL;
	m_hImageListToolBarNormal = NULL;

	cfg = NULL;
	appStatus = NULL;
	c64 = NULL;
	m_monitorCommand = NULL;
	m_pParentWindow = NULL;
}

CMDIDebuggerFrame::~CMDIDebuggerFrame()
{
	Cleanup();
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
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG)); 
	wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL)); 
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
	if (!m_monitorCommand)
		return ;
	if (!m_hWnd)
		return ;
	HMENU hMenu = GetMenu(m_hWnd);
	if (!hMenu)
		return ;
	UINT state;
	UINT stateOpp;
	if (m_monitorCommand->IsRunning())
	{
		state = MF_DISABLED | MF_GRAYED;
		stateOpp = MF_ENABLED;
	}
	else
	{
		state = MF_ENABLED;
		stateOpp = MF_DISABLED | MF_GRAYED;
	}
	EnableMenuItem(hMenu, ID_STEP_TRACEFRAME, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_TRACE, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_STOP, MF_BYCOMMAND | stateOpp);
}

HWND CMDIDebuggerFrame::Show(CVirWindow *pParentWindow)
{
int x,y,w,h;

	if(m_hWnd == 0)
	{
		this->m_pParentWindow = pParentWindow;
		POINT pos = {0,0};
		SIZE size= {0,0};
		CConfig::LoadMDIWindowSetting(pos, size);

		x = pos.x;
		y = pos.y;
		w = size.cx;
		h = size.cy;

		HWND hWndParent = NULL;
		HINSTANCE hInstance = NULL;
		if (pParentWindow != NULL)
		{
			hWndParent = pParentWindow->GetHwnd();
			hInstance = pParentWindow->GetHinstance();
		}
		if (hInstance == NULL)
			hInstance = GetModuleHandle(NULL);

		HWND hWnd = this->Create(hInstance, hWndParent, TEXT("C64 Monitor"), 0, 0, 0, 0);
		if(hWnd != 0)
		{
			EnsureWindowPosition(x, y, w, h);			
		}
		
	}
	else
	{
		if (this->m_pParentWindow != pParentWindow)
		{			
			HWND newParent = NULL;
			if (pParentWindow)
				newParent = pParentWindow->GetHwnd();
			::SetParent(m_hWnd, newParent);
			this->m_pParentWindow = pParentWindow;
		}
	}
	if(m_hWnd != 0)
	{
		::ShowWindow(m_hWnd, SW_SHOW);
		::SetForegroundWindow(m_hWnd);
	}
	return m_hWnd;
}


HWND CMDIDebuggerFrame::Create(HINSTANCE hInstance, HWND parent, const TCHAR title[], int x,int y, int w, int h)
{
	return CVirWindow::Create(0L, ClassName, title, WS_OVERLAPPED | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, parent, NULL, hInstance);
}

HRESULT CMDIDebuggerFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	hr = CreateMDIToolBars();
	if (FAILED(hr))
		return hr;
	HWND hWnd = CreateMDIClientWindow(IDC_MAIN_MDI, IDM_WINDOWCHILD);
	if (hWnd==NULL)
		return E_FAIL;
	return S_OK;
}

void CMDIDebuggerFrame::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//CConfig::SaveMDIWindowSetting(m_hWnd);

	if (this->m_pParentWindow)
	{
		HWND hWndParent = m_pParentWindow->GetHwnd();
		if (hWndParent)
		{
			::SetForegroundWindow(hWndParent);
		}
	}
}

void CMDIDebuggerFrame::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (::IsWindow(m_hWnd))
		CConfig::SaveMDIWindowSetting(m_hWnd);
}

void CMDIDebuggerFrame::OnMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

void CMDIDebuggerFrame::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
BOOL br;
	if (hWnd != this->m_hWnd)
		return;
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return;
	

	if (wParam != SIZE_MAXIMIZED || wParam != SIZE_MINIMIZED)
	{
		int w = LOWORD(lParam);
		int h = HIWORD(lParam);
		RECT rcAbsRebar;
		int heightRebar = 0;
		if (m_hWndRebar!=NULL)
		{
			br = GetWindowRect(m_hWndRebar, &rcAbsRebar);
			if (br == FALSE)
				return;
			heightRebar = rcAbsRebar.bottom - rcAbsRebar.top;

			SetWindowPos(m_hWndRebar, 0, 0, 0, w, heightRebar, SWP_NOREPOSITION | SWP_NOZORDER);
		}

		MoveWindow(m_hwndMDIClient, 0, heightRebar, w, h - heightRebar, TRUE);
	}

}

HRESULT CMDIDebuggerFrame::CreateMDIToolBars()
{
	if (m_hImageListToolBarNormal == NULL)
	{
		m_hImageListToolBarNormal = CreateImageListNormal(TB_ImageList, _countof(TB_ImageList), TOOLBUTTON_WIDTH, TOOLBUTTON_HEIGHT);
		if (m_hImageListToolBarNormal == NULL)
			return E_FAIL;
	}
	m_hWndTooBar = CreateToolBar(m_hImageListToolBarNormal, TB_StepButtons, _countof(TB_StepButtons), TOOLBUTTON_WIDTH, TOOLBUTTON_HEIGHT);
	if (m_hWndTooBar == NULL)
		return E_FAIL;
	//m_hWndRebar = CreateRebar(m_hWndTooBar);
	//if (m_hWndRebar == NULL)
	//	return E_FAIL;
	return S_OK;
}

HWND CMDIDebuggerFrame::CreateRebar(HWND hwndTB)
{
RECT rect;
BOOL br;
HWND hWndRB = NULL;
REBARINFO     rbi;
REBARBANDINFO rbBand;
DWORD_PTR dwBtnSize;

		br = GetWindowRect(m_hWnd, &rect);
		if (!br)
			return NULL;
		hWndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
			REBARCLASSNAME,//class name
			NULL,//Title
			//CCS_NORESIZE  |
			WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|CCS_NODIVIDER,
			0,0,0,0,
			m_hWnd,//Parent
			(HMENU) LongToPtr(ID_RERBAR),//Menu
			m_hInst,//application instance
			NULL);

		if (hWndRB == NULL)
			return NULL;

		// Initialize and send the REBARINFO structure.
		::ZeroMemory(&rbi, sizeof(REBARINFO));
		rbi.cbSize = sizeof(REBARINFO);  // Required when using this
		// structure.
		rbi.fMask  = 0;
		rbi.himl   = (HIMAGELIST)NULL;
		if(!SendMessage(hWndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
			return NULL;

		// Initialize structure members that both bands will share.
		::ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
		rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
		rbBand.fMask  = RBBIM_COLORS | RBBIM_STYLE | RBBIM_BACKGROUND | RBBIM_CHILD  | RBBIM_CHILDSIZE | RBBIM_SIZE;// | RBBIM_TEXT
		rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP | RBBS_GRIPPERALWAYS;
		rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
		rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
		rbBand.hbmBack = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_REBARBKGND1));   
   
		// Get the height of the toolbar.
		dwBtnSize = SendMessage(hwndTB, TB_GETBUTTONSIZE, 0,0);

		// Set values unique to the band with the toolbar.
		rbBand.lpText     = TEXT("Tool Bar");
		rbBand.hwndChild  = hwndTB;
		rbBand.cyMinChild = HIWORD(dwBtnSize);
		rbBand.cyMaxChild = HIWORD(dwBtnSize);
		rbBand.cyIntegral = 1;
		rbBand.cx         = 250;

		// Add the band that has the toolbar.
		SendMessage(hWndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);		

		return hWndRB;
}


HIMAGELIST CMDIDebuggerFrame::CreateImageListNormal(const ImageInfo imageInfo[], int length, int imageWidth, int imageHeight)
{
HIMAGELIST hImageList = NULL;
HBITMAP hbmpImage = NULL;
HBITMAP hbmpMask = NULL;
int r;
bool fail = false;

	hImageList = ImageList_Create(imageWidth, imageHeight, ILC_MASK, length, 0);
	if (hImageList == NULL)
		return NULL;

	for(int i=0; i < length; i++)
	{
		hbmpImage = LoadBitmap(m_hInst, MAKEINTRESOURCE(imageInfo[i].BitmapImageResourceId));
		if (hbmpImage == NULL)
		{
			fail = true;
			break;
		}
		hbmpMask = LoadBitmap(m_hInst, MAKEINTRESOURCE(imageInfo[i].BitmapMaskResourceId));
		if (hbmpImage == NULL)
		{
			fail = true;
			break;
		}
		r = ImageList_Add(hImageList, hbmpImage, hbmpMask);
		if (r < 0)
		{
			fail = true;
			break;
		}
		DeleteObject(hbmpImage);
		DeleteObject(hbmpMask);
		hbmpImage = NULL;
		hbmpMask = NULL;
	}
	if (fail)
	{
		if (hbmpImage != NULL)
		{
			DeleteObject(hbmpImage);
			hbmpImage = NULL;
		}
		if (hbmpMask != NULL)
		{
			DeleteObject(hbmpMask);
			hbmpMask = NULL;
		}
		if (hImageList != NULL)
		{
			ImageList_Destroy(hImageList);
			hImageList = NULL;
		}
		return NULL;
	}

	return hImageList;

}

HWND CMDIDebuggerFrame::CreateToolBar(HIMAGELIST hImageListToolBarNormal, const ButtonInfo buttonInfo[], int length, int buttonWidth, int buttonHeight)
{
HWND hwndTB = NULL;
int i;
LRESULT lr = 0;

	hwndTB = ::CreateWindowEx(0, TOOLBARCLASSNAME, 	(LPTSTR) NULL, 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS |	WS_CHILD | 	TBSTYLE_FLAT | CCS_NORESIZE | CCS_NODIVIDER // | CCS_ADJUSTABLE
		, 0, 0, 0, 0, 
		m_hWnd, 
		(HMENU) LongToPtr(ID_TOOLBAR), 
		m_hInst, 
		NULL
	); 

	lr = SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

	lr = SendMessage(hwndTB, TB_SETBUTTONSIZE, 0, MAKELONG(buttonWidth, buttonHeight));

	lr = SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hImageListToolBarNormal);

	TBBUTTON* tbArray = new TBBUTTON[length];
	for (i=0; i<length; i++)
	{
		ZeroMemory(&tbArray[i], sizeof(TBBUTTON));		
		tbArray[i].iBitmap = i;
		tbArray[i].idCommand = buttonInfo[i].CommandId;
		tbArray[i].fsStyle = BTNS_BUTTON;
		tbArray[i].fsState = TBSTATE_ENABLED;
		tbArray[i].iString = -1;//(INT_PTR)TB_ImageListIDs[i].ButtonText;
	}
	
	
	lr = SendMessage(hwndTB, TB_ADDBUTTONS, length, (LPARAM)tbArray);
	
	delete[] tbArray;
	return hwndTB;
}

void CMDIDebuggerFrame::ShowDebugCpuC64(bool bSeekPC)
{
	m_debugCpuC64.Show(bSeekPC);
}

void CMDIDebuggerFrame::ShowDebugCpuDisk(bool bSeekPC)
{
	m_debugCpuDisk.Show(bSeekPC);
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

void CMDIDebuggerFrame::EnsureWindowPosition(int x, int y, int w, int h)
{
RECT rcMain,rcWorkArea;
LONG   lRetCode; 
bool bGotWorkArea = false;
bool bWantDefaultSizeAndPos = false;

	if (!m_hWnd)
		return;
	if (w<=0 || h<=0)
	{
		bWantDefaultSizeAndPos = true;
		w=h=1;
	}
		
	SetRect(&rcMain, x, y, x+w, y+h);
	SetRectEmpty(&rcWorkArea);
	if (G::s_pFnMonitorFromRect != NULL && G::s_pFnGetMonitorInfo != NULL)
	{
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		HMONITOR hMonitor = G::s_pFnMonitorFromRect(&rcMain, MONITOR_DEFAULTTOPRIMARY);
		if (G::s_pFnGetMonitorInfo(hMonitor, &mi))
		{
			rcWorkArea = mi.rcMonitor;
			bGotWorkArea = true;
		}
	}

	if (!bGotWorkArea)
	{
		// Get the limits of the 'workarea'
		lRetCode = SystemParametersInfo(
			SPI_GETWORKAREA,  // system parameter to query or set
			sizeof(RECT),
			&rcWorkArea,
			0);
		if (!lRetCode)
		{
			rcWorkArea.left = rcWorkArea.top = 0;
			rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
			rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		}
	}

	int minw,minh;
	//Check minimum size.
	GetMinWindowSize(minw, minh);
	int workarea_w = (rcWorkArea.right-rcWorkArea.left);
	int workarea_h = (rcWorkArea.bottom-rcWorkArea.top);
	if (bWantDefaultSizeAndPos)
	{

		//Try for half workspace width and half workspace height.
		w = workarea_w/4;
		h = workarea_h/8;
		//Honour minimum size
		if (w<minw)
			w=minw;
		if (h<minh)
			h=minh;

		//Center the window
		x = (workarea_w - w) / 2;
		//y = (workarea_h - h) / 2;
		y = (workarea_h) / 16;

		SetRect(&rcMain, x, y, x+w, y+h);
	}
	else
	{
		if (w > workarea_w)
			w = workarea_w;
		if (h > workarea_h)
			h = workarea_h;
		//Honour minimum size
		if (w<minw)
			w=minw;
		if (h<minh)
			h=minh;

		SetRect(&rcMain, x, y, x+w, y+h);
	}
	if (rcMain.right>rcWorkArea.right)
		OffsetRect(&rcMain, rcWorkArea.right - rcMain.right, 0);
	if (rcMain.bottom>rcWorkArea.bottom)
		OffsetRect(&rcMain, 0, rcWorkArea.bottom - rcMain.bottom);

	if (rcMain.left<rcWorkArea.left)
		OffsetRect(&rcMain, rcWorkArea.left - rcMain.left, 0);
	if (rcMain.top<rcWorkArea.top)
		OffsetRect(&rcMain, 0, rcWorkArea.top - rcMain.top);

	SetWindowPos(m_hWnd, 0, rcMain.left,rcMain.top, rcMain.right-rcMain.left, rcMain.bottom-rcMain.top , SWP_NOZORDER);
}

bool CMDIDebuggerFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case ID_DEBUG_CPUC64:
		ShowDebugCpuC64(false);
		return true;
	case ID_DEBUG_CPUDISK:
		ShowDebugCpuDisk(false);
		return true;
	case ID_STEP_TRACEFRAME:
		if (!m_monitorCommand)
			return false;
		if (m_monitorCommand->IsRunning())
			return false;
		m_monitorCommand->TraceFrame();
		m_monitorCommand->UpdateApplication();
		return true;
	case ID_STEP_TRACE:
		if (!m_monitorCommand)
			return false;
		this->m_monitorCommand->Trace();
		return true;
	case ID_FILE_MONITOR:
	case ID_STEP_STOP:
		if (!m_monitorCommand)
			return false;
		this->m_monitorCommand->ShowDevelopment();
		return true;
	default:
		return false;
	}
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
		ShowWindow(m_hwndMDIClient, SW_SHOW); 
		return 0;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_MOVE:
		OnMove(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		if (m_monitorCommand)
			m_monitorCommand->Resume();
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
		m_monitorCommand->SoundOff();
		return 0;
	case WM_EXITMENULOOP:
		m_monitorCommand->SoundOn();
		return 0;
	case WM_ENTERSIZEMOVE:
		m_monitorCommand->SoundOff();
		return 0;
	case WM_EXITSIZEMOVE:
		m_monitorCommand->SoundOn();
		return 0;
	default:
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;	
}

void CMDIDebuggerFrame::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_debugCpuC64.Show(true);
}

void CMDIDebuggerFrame::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_debugCpuDisk.Show(true);
}

HRESULT CMDIDebuggerFrame::Init(IMonitorCommand *monitorCommand, CConfig *cfg, CAppStatus *appStatus, C64 *c64)
{
HRESULT hr;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;
	this->m_monitorCommand = monitorCommand;

	IMonitorCpu *monitorMainCpu = &c64->cpu;
	IMonitorCpu *monitorDiskCpu = &c64->diskdrive.cpu;
	IMonitorVic *monitorVic = &c64->vic;
	IMonitorDisk *monitorDisk = &c64->diskdrive;

	m_monitorC64.Init(CPUID_MAIN, monitorMainCpu, monitorDiskCpu, monitorVic, monitorDisk);
	m_monitorDisk.Init(CPUID_DISK, monitorMainCpu, monitorDiskCpu, monitorVic, monitorDisk);

	do
	{
		hr = m_debugCpuC64.Init(this, monitorCommand, &m_monitorC64, TEXT("C64 - cpu"));
		if (FAILED(hr))
			break;

		hr = m_debugCpuDisk.Init(this, monitorCommand, &m_monitorDisk, TEXT("Disk - cpu"));
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

HRESULT CMDIDebuggerFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{

		hs = m_monitorCommand->EsShowDevelopment.Advise((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsTrace.Advise((CMDIDebuggerFrame_EventSink_OnTrace *)this);
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
