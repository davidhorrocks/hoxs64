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
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "dchelper.h"
#include "resource.h"

#define TOOLBUTTON_WIDTH_96 (16)
#define TOOLBUTTON_HEIGHT_96 (16)
#define MAX_EDIT_GOTO_ADDRESS_CHARS (256)

const TCHAR CDisassemblyFrame::ClassName[] = TEXT("Hoxs64DisassemblyFrame");
const TCHAR CDisassemblyFrame::MenuName[] = TEXT("MENU_CPUDISASSEMBLY");

const ImageInfo CDisassemblyFrame::TB_ImageList[] = 
{
	{IDB_DEBUGGERTRACE, IDB_DEBUGGERTRACEMASK, 0},
	{IDB_DEBUGGERTRACEFRAME, IDB_DEBUGGERTRACEFRAMEMASK, 0},
	{IDB_DEBUGGERSTEPONECLOCK, IDB_DEBUGGERSTEPONECLOCKMASK, 0},
	{IDB_DEBUGGERSTEPIN, IDB_DEBUGGERSTEPINMASK, 0},
	//{IDB_DEBUGGERTRACEINT, IDB_DEBUGGERTRACEINTMASK, 0},
	{IDB_DEBUGGERTRACEINT_64, 0, 0},	
	{IDB_DEBUGGERSTOP, IDB_DEBUGGERSTOPMASK, 0},
	{0, 0, IDI_GREENFIND}
};

const ButtonInfo CDisassemblyFrame::TB_ButtonsStep[] = 
{
	{0, TEXT("Trace"), TEXT("Trace"), BTNS_BUTTON, IDM_STEP_TRACE},
	{1, TEXT("Trace Frame"), TEXT("Trace 1 frame"), BTNS_BUTTON, IDM_STEP_TRACEFRAME},
	{2, TEXT("1 Clock"), TEXT("Step 1 clock"), BTNS_BUTTON, IDM_STEP_ONECLOCK},
	{3, TEXT("1 Instruction"), TEXT("Step 1 instruction"), BTNS_BUTTON, IDM_STEP_ONEINSTRUCTION},
	{4, TEXT("Trace INT"), TEXT("Trace till IRQ/NMI taken"), BTNS_BUTTON, IDM_STEP_TRACEINTERRUPTTAKEN},
	{5, TEXT("Stop"), TEXT("Stop tracing"), BTNS_BUTTON, IDM_STEP_STOP}
};

const ButtonInfo CDisassemblyFrame::TB_ButtonsAddress[] = 
{
	{1, (TCHAR *)-1, TEXT(""), BTNS_SEP, 0},
	{6, TEXT("Find Address"), TEXT("Find address"), BTNS_BUTTON, IDM_VIEW_ADDRESS}
};

CDisassemblyFrame::CDisassemblyFrame(int cpuid, C64 *c64, IMonitorCommand *pMonitorCommand, LPCTSTR pszCaption)
	: 
	DefaultCpu(cpuid, c64),
	m_DisassemblyChild(cpuid, c64, pMonitorCommand),
	m_DisassemblyReg(cpuid, c64, pMonitorCommand)
{
	m_hBmpRebarNotSized = NULL;
	//m_hBmpRebarSized = NULL;
	m_hWndRebar = NULL;
	m_hWndTooBarStep = NULL;
	m_hWndTooBarAddress = NULL;
	m_hImageListToolBarNormal = NULL;
	m_iCurrentControlIndex = 0;

	m_pszCaption = TEXT("Cpu");
	m_pMonitorCommand = pMonitorCommand;
	m_pszCaption = pszCaption;
	m_hWndToolItemAddress = NULL;
	m_wheel_current = 0;
}

CDisassemblyFrame::~CDisassemblyFrame()
{
	Cleanup();
}

HRESULT CDisassemblyFrame::Init(CVirWindow *parent)
{
HRESULT hr;
	m_pParentWindow = parent;

	hr = InitFonts();
	if (FAILED(hr))
		return hr;
	hr = m_DisassemblyChild.Init(this, this->m_monitor_font);
	if (FAILED(hr))
		return hr;

	hr = m_DisassemblyReg.Init(this, this->m_monitor_font);
	if (FAILED(hr))
		return hr;
	
	hr = AdviseEvents();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void CDisassemblyFrame::Cleanup()
{
	UnadviseEvents();
	if (m_hImageListToolBarNormal != NULL)
	{
		ImageList_Destroy(m_hImageListToolBarNormal);
		m_hImageListToolBarNormal = NULL;
	}
	m_hWndToolItemAddress = NULL;

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
}

HRESULT CDisassemblyFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{
		hs = m_pMonitorCommand->EsResume.Advise((CDisassemblyFrame_EventSink_OnResume *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsTrace.Advise((CDisassemblyFrame_EventSink_OnTrace *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsTraceFrame.Advise((CDisassemblyFrame_EventSink_OnTraceFrame *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsExecuteC64Clock.Advise((CDisassemblyFrame_EventSink_OnExecuteC64Clock *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsExecuteDiskClock.Advise((CDisassemblyFrame_EventSink_OnExecuteDiskClock *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsExecuteC64Instruction.Advise((CDisassemblyFrame_EventSink_OnExecuteC64Instruction *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsExecuteDiskInstruction.Advise((CDisassemblyFrame_EventSink_OnExecuteDiskInstruction *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_pMonitorCommand->EsShowDevelopment.Advise((CDisassemblyFrame_EventSink_OnShowDevelopment *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hr = S_OK;
	} while (false);
	return hr;
}

void CDisassemblyFrame::UnadviseEvents()
{
	((CDisassemblyFrame_EventSink_OnResume *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnTrace *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnTraceFrame *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnExecuteC64Clock *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnExecuteDiskClock *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnExecuteC64Instruction *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnExecuteDiskInstruction *)this)->UnadviseAll();
	((CDisassemblyFrame_EventSink_OnShowDevelopment *)this)->UnadviseAll();

}
#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY (0)
#endif
HRESULT CDisassemblyFrame::InitFonts()
{
	CloseFonts();
	LPTSTR lstFontName[] = { TEXT("Consolas"), TEXT("Lucida"), TEXT("Courier"), TEXT("")};
	for (int i =0; m_monitor_font == 0 && i < _countof(lstFontName); i++)
	{
		m_monitor_font = CreateFont(
			m_dpi.ScaleY(m_dpi.PointsToPixels(12)),
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			ANSI_CHARSET,
			OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			FIXED_PITCH | FF_DONTCARE,
			lstFontName[i]);
	}
	if (m_monitor_font == 0)
	{
		return SetError(E_FAIL, TEXT("Cannot open a fixed pitch true type font."));
	}
	return S_OK;
}

void CDisassemblyFrame::CloseFonts()
{
	if (m_monitor_font)
	{
		DeleteObject((HGDIOBJ) m_monitor_font);
		m_monitor_font=0;
	}
}


HRESULT CDisassemblyFrame::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_OWNDC;//CS_OWNDC; CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(CDisassemblyFrame *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_CHIP1));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MenuName;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
};


HWND CDisassemblyFrame::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_OVERLAPPED | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, hWndParent, hMenu, hInstance);	
}

HIMAGELIST CDisassemblyFrame::CreateImageListStepNormal(HWND hWnd)
{
	int tool_dx = m_dpi.ScaleX(TOOLBUTTON_WIDTH_96);
	int tool_dy = m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96);
	return G::CreateImageListNormal(m_hInst, hWnd, tool_dx, tool_dy, TB_ImageList, _countof(TB_ImageList));
}

HRESULT CDisassemblyFrame::Show()
{
WINDOWPLACEMENT wp;
int x,y,w,h;
BOOL br;
HWND hWnd;

	if (this->m_pParentWindow == NULL)
		return E_FAIL;
	
	hWnd = this->GetHwnd();
	if (hWnd == NULL)
	{
		HWND hWndParent = m_pParentWindow->GetHwnd();
		HINSTANCE hInstance = m_pParentWindow->GetHinstance();	

		ZeroMemory(&wp, sizeof(wp));
		wp.length = sizeof(wp);

		br = GetWindowPlacement(hWndParent, &wp);
		if (!br)
			return E_FAIL;

		hWnd = this->Create(hInstance, hWndParent, m_pszCaption, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL);
		if (hWnd == NULL)
			return E_FAIL;
		GetMinWindowSize(w, h);

		RECT rcDesk;
		G::GetWorkArea(rcDesk);

		int gap = (rcDesk.bottom - rcDesk.top) / 10;
		int id=this->GetCpuId();
		if (id == CPUID_MAIN)
		{
			x = wp.rcNormalPosition.left-w;
			if (wp.rcNormalPosition.left - rcDesk.left >= w)
				y = rcDesk.top + gap;
			else
				y = wp.rcNormalPosition.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
			w = w;
			h = rcDesk.bottom - rcDesk.top -  2*gap;
		}
		else
		{
			x = wp.rcNormalPosition.right;
			if (rcDesk.right - wp.rcNormalPosition.right>= w)
				y = rcDesk.top + gap;
			else
				y = wp.rcNormalPosition.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
			w = w;
			h = rcDesk.bottom - rcDesk.top -  2*gap;
		}
		SetWindowPos(hWnd, HWND_TOP, x, y, w, h, SWP_NOZORDER);
		G::EnsureWindowPosition(hWnd);
	}
	else
	{
		WINDOWPLACEMENT wp;
		if (GetWindowPlacement(hWnd, &wp))
		{
			x = wp.rcNormalPosition.left;
			y = wp.rcNormalPosition.top;
			w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			SetWindowPos(hWnd, HWND_TOP, x, y, w, h, SWP_NOZORDER);
			G::EnsureWindowPosition(hWnd);
		}
	}
	SetMenuState();
	::ShowWindow(hWnd, SW_SHOW);
	::SetForegroundWindow(hWnd);
	return S_OK;
}

void CDisassemblyFrame::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
		return;
	if (hWnd != this->m_hWnd)
		return;
	int w = LOWORD(lParam);
	int h = HIWORD(lParam);
	OnSizeToolBar(m_hWndRebar, w, h);

	HWND hWndDisassemblyChild = this->m_DisassemblyChild.GetHwnd();
	if (hWndDisassemblyChild!=NULL)
		OnSizeDisassembly(hWndDisassemblyChild, w, h);

	HWND hWndReg = this->m_DisassemblyReg.GetHwnd();
	if (hWndReg!=NULL)
		OnSizeRegisters(hWndReg, w, h);

}

void CDisassemblyFrame::OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int w,h;
MINMAXINFO *pMinMax = (MINMAXINFO *)lParam;
	this->GetMinWindowSize(w, h);
	pMinMax->ptMinTrackSize.x = w;
	pMinMax->ptMinTrackSize.y = h;
	//pMinMax->ptMaxTrackSize.x = GetSystemMetrics(SM_CXFULLSCREEN);
	//pMinMax->ptMaxTrackSize.y = GetSystemMetrics(SM_CYFULLSCREEN);
}

void CDisassemblyFrame::OnSizeToolBar(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
RECT rc;

	hr = GetSizeRectToolBar(rc);
	if (FAILED(hr))
		return;

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;	
	SetWindowPos(hWnd, 0, 0, 0, w, h, SWP_NOREPOSITION | SWP_NOZORDER);
}

HRESULT CDisassemblyFrame::GetSizeRectToolBar(RECT &rc)
{
BOOL br;
RECT rcFrameClient;

	if (m_hWndRebar==NULL)
		return E_FAIL;
	br = GetClientRect(this->m_hWnd, &rcFrameClient);
	if (br == FALSE)
		return E_FAIL;

	int heightRebar = (int)SendMessage(this->m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
	if (heightRebar < 0)
		heightRebar = 0;

	SetRect(&rc, rcFrameClient.left, rcFrameClient.top, rcFrameClient.right, rcFrameClient.top + heightRebar);	
	if (rc.right < rc.left)
		rc.right = rc.left;
	if (rc.bottom < rc.top)
		rc.bottom = rc.top;
	return S_OK;
}

HRESULT CDisassemblyFrame::GetSizeRectDisassembly(RECT &rc)
{
BOOL br;
RECT rcFrameClient;
RECT rcReg;

	br = GetClientRect(this->m_hWnd, &rcFrameClient);
	if (br == FALSE)
		return E_FAIL;

	HWND hWndReg = m_DisassemblyReg.GetHwnd();
	if (hWndReg==NULL)
		return E_FAIL;
	br = GetWindowRect(hWndReg, &rcReg);
	if (br == FALSE)
		return E_FAIL;

	int heightReg = rcReg.bottom - rcReg.top;
	if (heightReg < 0)
		heightReg = 0;

	int heightRebar = 0;
	if (m_hWndRebar)
		heightRebar = (int)SendMessage(this->m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
	if (heightRebar < 0)
		heightRebar = 0;

	LONG x,y,w,h;
	x = rcFrameClient.left;
	y = rcFrameClient.top + heightRebar + heightReg;
	w = rcFrameClient.right - rcFrameClient.left;
	h = rcFrameClient.bottom - (heightRebar + heightReg);
	if (w<0)
		w=0;
	if (h<0)
		h=0;

	SetRect(&rc, x, y, x+w, y+h);

	return S_OK;
}

HRESULT CDisassemblyFrame::GetSizeRectReg(RECT &rc)
{
BOOL br;
RECT rcFrameClient;

	br = GetClientRect(m_hWnd, &rcFrameClient);
	if (br == FALSE)
		return E_FAIL;

	int heightRebar = 0;
	if (m_hWndRebar)
		heightRebar = (int)SendMessage(this->m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
	if (heightRebar < 0)
		heightRebar = 0;

	int minwidth_reg, minheight_reg;
	m_DisassemblyReg.GetMinWindowSize(minwidth_reg, minheight_reg);

	LONG x,y,w,h;
	x = rcFrameClient.left;
	y = rcFrameClient.top + heightRebar;
	w = rcFrameClient.right - rcFrameClient.left;
	h = minheight_reg;

	if (w<0)
		w=0;
	if (h<0)
		h=0;

	SetRect(&rc, x, y, x+w, y+h);

	return S_OK;
}

void CDisassemblyFrame::OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
	RECT rc;
	hr = GetSizeRectDisassembly(rc);
	if (FAILED(hr))
		return;

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;	
	MoveWindow(hWnd, x, y, w, h, FALSE);
}

void CDisassemblyFrame::OnSizeRegisters(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
	RECT rc;
	hr = GetSizeRectReg(rc);
	if (FAILED(hr))
		return;

	LONG x, y, w, h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;	
	MoveWindow(hWnd, x, y, w, h, FALSE);
}

HRESULT CDisassemblyFrame::OnCreate(HWND hWnd)
{
HRESULT hr;
ButtonInfo l_TB_ButtonsAddress[_countof(TB_ButtonsAddress)];
RECT rcToolItem;
	
	HDC hdc = GetDC(hWnd);
	if (!hdc)
		return E_FAIL;

	DcHelper dch(hdc);

	memcpy(l_TB_ButtonsAddress, TB_ButtonsAddress, sizeof(l_TB_ButtonsAddress));

	m_hBmpRebarNotSized = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_REBARBKGND1));

	m_hImageListToolBarNormal = CreateImageListStepNormal(hWnd);
	if (m_hImageListToolBarNormal == NULL)
		return E_FAIL;
	m_hWndTooBarStep = G::CreateToolBar(m_hInst, hWnd, ID_TOOLBAR, m_hImageListToolBarNormal, TB_ButtonsStep, _countof(TB_ButtonsStep), m_dpi.ScaleX(TOOLBUTTON_WIDTH_96), m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96));
	if (m_hWndTooBarStep == NULL)
		return E_FAIL;

	m_pToolItemAddress = CreateToolItemAddress(hWnd);
	if (!m_pToolItemAddress)
		return E_FAIL;
	m_hWndToolItemAddress = m_pToolItemAddress->GetHwnd();
	m_pToolItemAddress->SetInterface(this);
	GetWindowRect(m_hWndToolItemAddress, &rcToolItem);
	l_TB_ButtonsAddress[0].ImageIndex = rcToolItem.right - rcToolItem.left;
	l_TB_ButtonsAddress[0].Style = BTNS_SEP;

	m_hWndTooBarAddress = G::CreateToolBar(m_hInst, hWnd, ID_TOOLBAR, m_hImageListToolBarNormal, l_TB_ButtonsAddress, _countof(l_TB_ButtonsAddress), m_dpi.ScaleX(TOOLBUTTON_WIDTH_96), m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96));
	if (m_hWndTooBarAddress == NULL)
		return E_FAIL;
	
	SetParent(m_hWndToolItemAddress, m_hWndTooBarAddress);

	m_hWndRebar = G::CreateRebar(m_hInst, hWnd, m_hWndTooBarStep, ID_RERBAR);
	if (m_hWndRebar == NULL)
		return E_FAIL;

	hr = RebarAddAddressBar(m_hWndRebar, m_hWndTooBarAddress);
	if (FAILED(hr))
		return hr;

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

	RECT rc;
	GetClientRect(hWnd, &rc);

	HWND hWndReg = CreateDisassemblyReg(0, 0, 0, 0);
	if (hWndReg == NULL)
		return E_FAIL;

	HWND hWndDis = CreateDisassemblyChild(0, 0, 0, 0);
	if (hWndDis == NULL)
		return E_FAIL;

	SetHome();

	LONG w = rc.right - rc.left;
	LONG h = rc.bottom - rc.top;
	OnSizeRegisters(hWndReg, w, h);
	OnSizeDisassembly(hWndDis, w, h);
	return S_OK;
}

HRESULT CDisassemblyFrame::RebarAddAddressBar(HWND hWndRebar, HWND hWndToolbar)
{
RECT rcClient;
REBARBANDINFO rbBand;
BOOL br;
DWORD_PTR dwBtnSize;

	br = GetClientRect(m_hWnd, &rcClient);
	if (!br)
		return E_FAIL;

	// Get the height of the toolbar.
	dwBtnSize = SendMessage(hWndToolbar, TB_GETBUTTONSIZE, 0,0);
	int butSizeX = HIWORD(dwBtnSize);
	int butSizeY = HIWORD(dwBtnSize);

	::ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
	rbBand.fMask  = RBBIM_COLORS | RBBIM_STYLE | RBBIM_BACKGROUND | RBBIM_CHILD  | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_TEXT;
	rbBand.fStyle = RBBS_BREAK | RBBS_CHILDEDGE /*| RBBS_FIXEDBMP*/ | RBBS_GRIPPERALWAYS;
	rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbBand.clrBack = GetSysColor(COLOR_BTNFACE);

	rbBand.lpText     = TEXT("Address");
	rbBand.hwndChild  = hWndToolbar;
	rbBand.cxMinChild = butSizeX;
	rbBand.cyMinChild = butSizeY;
	rbBand.cx         = rcClient.right - rcClient.left;

	int iBandNext = (int)SendMessage(hWndRebar, RB_GETBANDCOUNT, (WPARAM)-1, (LPARAM)&rbBand);
	SendMessage(hWndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
	return S_OK;
}

CToolItemAddress *CDisassemblyFrame::CreateToolItemAddress(HWND hWndParent)
{
HRESULT hr;
SIZE sizeText;

	CToolItemAddress *p = new CToolItemAddress();
	if (!p)
		return NULL;
	hr = p->Init(this->m_monitor_font);
	if (FAILED(hr))
	{
		delete p;
		return NULL;
	}
	hr = p->GetDefaultTextBoxSize(hWndParent, sizeText);
	if (FAILED(hr))
	{
		delete p;
		return NULL;
	}
	HWND hwnd = p->Create(m_hInst, hWndParent, NULL, 0, 0, sizeText.cx, sizeText.cy, (HMENU)IDC_TOI_GOTOADDRESS);
	if (!hwnd)
	{
		delete p;
		return NULL;
	}
	p->m_AutoDelete = true;
	return p;
}

void CDisassemblyFrame::SetHome()
{
	this->m_DisassemblyChild.SetHome();
}

HWND CDisassemblyFrame::CreateDisassemblyReg(int x, int y, int w, int h)
{	
	HWND hWnd = m_DisassemblyReg.Create(m_hInst, m_hWnd, NULL, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyFrame::ID_DISASSEMBLEYREG);
	return hWnd;
}

HWND CDisassemblyFrame::CreateDisassemblyChild(int x, int y, int w, int h)
{	
	HWND hWnd = m_DisassemblyChild.Create(m_hInst, m_hWnd, NULL, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyFrame::ID_DISASSEMBLEY);
	return hWnd;
}

bool CDisassemblyFrame::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_UP: 
		SendMessage(m_DisassemblyChild.GetHwnd(), WM_VSCROLL, SB_LINEUP, 0L);
		break; 
	case VK_DOWN: 
		SendMessage(m_DisassemblyChild.GetHwnd(), WM_VSCROLL, SB_LINEDOWN, 0L);
		break; 
	case VK_NEXT: 
		SendMessage(m_DisassemblyChild.GetHwnd(), WM_VSCROLL, SB_PAGEDOWN, 0L);
		break; 
	case VK_PRIOR: 
		SendMessage(m_DisassemblyChild.GetHwnd(), WM_VSCROLL, SB_PAGEUP, 0L);
		break; 
	case VK_HOME:
		SetHome();
		UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
		break; 
	default:
		return false;
	}
	return true;
}

void CDisassemblyFrame::UpdateDisplay(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address)
{
	if (IsWindow(m_hWnd))
	{
		this->m_DisassemblyReg.UpdateDisplay();
		this->m_DisassemblyChild.UpdateDisplay(pcmode, address);
	}
}

void CDisassemblyFrame::CancelEditing()
{
	m_DisassemblyChild.CancelEditing();
	m_DisassemblyReg.CancelEditing();
}

void CDisassemblyFrame::OnResume(void *sender, EventArgs& e)
{
	this->GetCpu()->ClearBreakOnInterruptTaken();
}

void CDisassemblyFrame::OnTrace(void *sender, EventArgs& e)
{
	if (IsWindow(this->m_hWnd))
	{
		m_DisassemblyReg.InvalidateBuffer();
		m_DisassemblyChild.InvalidateBuffer();
		this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
		SetMenuState();
	}
}

void CDisassemblyFrame::OnTraceFrame(void *sender, EventArgs& e)
{
	if (IsWindow(this->m_hWnd))
	{
		this->SetHome();
		this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
	}
}

void CDisassemblyFrame::OnExecuteC64Clock(void *sender, EventArgs& e)
{
	this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CDisassemblyFrame::OnExecuteDiskClock(void *sender, EventArgs& e)
{
	this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CDisassemblyFrame::OnExecuteC64Instruction(void *sender, EventArgs& e)
{
	this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CDisassemblyFrame::OnExecuteDiskInstruction(void *sender, EventArgs& e)
{
	this->UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
}

void CDisassemblyFrame::OnShowDevelopment(void *sender, EventArgs& e)
{
	this->GetCpu()->ClearBreakOnInterruptTaken();
	if (IsWindow(this->m_hWnd))
	{
		SetHome();
		UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
		SetMenuState();
	}
}

bool CDisassemblyFrame::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CDisassemblyFrame::SetMenuState()
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
	bool bIsRunning = m_pMonitorCommand->IsRunning();
	if (bIsRunning)
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
	EnableMenuItem(hMenu, IDM_STEP_ONECLOCK, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_ONEINSTRUCTION, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_TRACE, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_TRACEFRAME, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, IDM_STEP_TRACEINTERRUPTTAKEN, MF_BYCOMMAND | state);
	
	EnableMenuItem(hMenu, IDM_STEP_STOP, MF_BYCOMMAND | stateOpp);
	if (m_hWndTooBarStep!=NULL)
	{
		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_ONECLOCK, stateTb);
		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_ONEINSTRUCTION, stateTb);
		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_TRACE, stateTb);
		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_TRACEFRAME, stateTb);
		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_TRACEINTERRUPTTAKEN, stateTb);

		SendMessage(m_hWndTooBarStep, TB_SETSTATE, IDM_STEP_STOP, stateTbOpp);
	}
	if (m_hWndTooBarStep!=NULL)
	{
		SendMessage(m_hWndTooBarAddress, TB_SETSTATE, IDM_VIEW_ADDRESS, stateTb);
	}
	if (m_hWndToolItemAddress!=NULL)
	{
		EnableWindow(m_hWndToolItemAddress, (BOOL)!bIsRunning);
	}
}

bool CDisassemblyFrame::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!lParam) 
		return false;

	NMHDR *pn = (NMHDR *)lParam;
	if (!pn->hwndFrom)
		return false;
	if (pn->hwndFrom == this->m_hWndRebar)
	{
		if (pn->code == RBN_HEIGHTCHANGE)
			return OnReBarHeightChange((LPNMHDR)lParam);
	}
	else if (pn->hwndFrom == this->m_hWndTooBarStep)
	{
		switch (pn->code)
		{
		case TBN_GETINFOTIP:
			return OnToolBarInfo((LPNMTBGETINFOTIP)lParam);
		}
	}
	return false;
}

bool CDisassemblyFrame::OnEnterGotoAddress(LPTSTR pszAddress)
{
Assembler as;
bit16 v = 0;
HRESULT hr;

	hr = as.ParseAddress16(pszAddress, &v);
	if (SUCCEEDED(hr))
	{
		UpdateDisplay(DBGSYM::SetDisassemblyAddress::SetTopAddress, v);
		MessageBeep(MB_ICONASTERISK);
		return true;
	}
	MessageBeep(MB_OK);
	return false;
}

bool CDisassemblyFrame::OnReBarHeightChange(LPNMHDR notify)
{
	if (notify->hwndFrom == this->m_hWndRebar)
	{
		Refresh();
	}
	return false;
}

bool CDisassemblyFrame::OnToolBarInfo(LPNMTBGETINFOTIP info)
{
	if (info->hdr.hwndFrom == this->m_hWndTooBarStep)
	{
		int id = info->iItem;
		for (int i = 0; i < _countof(TB_ButtonsStep); i++)
		{
			if (id == TB_ButtonsStep[i].CommandId)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TB_ButtonsStep[i].ToolTipText, _TRUNCATE);
				return true;
			}
		}
	}
	return false;
}

void CDisassemblyFrame::Refresh()
{
RECT rc;
HRESULT hr;

	if (!m_hWnd)
		return;
	HWND hWndReg = m_DisassemblyReg.GetHwnd();
	if (hWndReg == NULL)
		return;
	HWND hWndDis = m_DisassemblyChild.GetHwnd();
	if (hWndDis == NULL)
		return;

LONG x, y, w, h;

	//Get the child register window desired location.
	hr = GetSizeRectReg(rc);
	if (FAILED(hr))
		return;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;	
	//Move the child register window desired location.
	SetWindowPos(hWndReg, HWND_NOTOPMOST, x, y, w, h, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS);

	//Get the child disassembly window desired location.
	hr = this->GetSizeRectDisassembly(rc);
	if (FAILED(hr))
		return;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;	
	//Move the child disassembly window to the new location.
	SetWindowPos(hWndDis, HWND_NOTOPMOST, x, y, w, h, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS);

	UpdateWindow(m_hWnd);
}

bool CDisassemblyFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;

	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case IDM_STEP_ONECLOCK:
		if (!m_pMonitorCommand)
			return true;
		if (m_pMonitorCommand->IsRunning())
			return true;
		CancelEditing();
		if (this->GetCpuId() == CPUID_MAIN)
			m_pMonitorCommand->ExecuteC64Clock();
		else
			m_pMonitorCommand->ExecuteDiskClock();
		m_pMonitorCommand->UpdateApplication();
		return true;
	case IDM_STEP_ONEINSTRUCTION:
		if (!m_pMonitorCommand)
			return true;
		if (m_pMonitorCommand->IsRunning())
			return true;
		CancelEditing();
		if (this->GetCpuId() == CPUID_MAIN)
			m_pMonitorCommand->ExecuteC64Instruction();
		else
			m_pMonitorCommand->ExecuteDiskInstruction();
		m_pMonitorCommand->UpdateApplication();
		return true;
	case IDM_STEP_TRACEFRAME:
		if (!m_pMonitorCommand)
			return true;
		if (m_pMonitorCommand->IsRunning())
			return true;
		CancelEditing();
		m_pMonitorCommand->TraceFrame();
		m_pMonitorCommand->UpdateApplication();
		return true;
	case IDM_STEP_TRACE:
		if (!m_pMonitorCommand)
			return true;
		if (m_pMonitorCommand->IsRunning())
			return true;
		CancelEditing();
		m_pMonitorCommand->Trace();
		return true;
	case IDM_STEP_TRACEINTERRUPTTAKEN:
		if (!m_pMonitorCommand)
			return true;
		if (m_pMonitorCommand->IsRunning())
			return true;
		this->GetCpu()->SetBreakOnInterruptTaken();
		m_pMonitorCommand->Trace();
		return true;
	case IDM_FILE_MONITOR:
	case IDM_STEP_STOP:
		if (!m_pMonitorCommand)
			return true;
		CancelEditing();
		m_pMonitorCommand->ShowDevelopment();
		::SetForegroundWindow(m_hWnd);
		return true;
	case IDM_VIEW_ADDRESS:
		if (!m_pMonitorCommand)
			return true;		
		OnEnterGotoAddress();
		return true;
	default:
		return true;
	}
}

bool CDisassemblyFrame::OnEnterGotoAddress()
{
	m_tempAddressBuffer[0] = 0;
	m_pToolItemAddress->GetAddressText(0, &m_tempAddressBuffer[0], _countof(m_tempAddressBuffer));
	return OnEnterGotoAddress(m_tempAddressBuffer);
}

void CDisassemblyFrame::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (this->m_pParentWindow)
	{
		HWND hWndParent = m_pParentWindow->GetHwnd();
		if (hWndParent)
		{
			::SetForegroundWindow(hWndParent);
		}
	}
}

void CDisassemblyFrame::OnMouseWheel(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	m_wheel_current += zDelta;
	if (abs(m_wheel_current) < WHEEL_DELTA)
		return;
	int amount = m_wheel_current / WHEEL_DELTA;
	bit16 address = m_DisassemblyChild.GetTopAddress();
	address = m_DisassemblyChild.GetNthAddress(address, - amount);
	m_DisassemblyChild.CancelEditing();
	m_DisassemblyChild.UpdateDisplay(DBGSYM::SetDisassemblyAddress::SetTopAddress, address);
	m_wheel_current = m_wheel_current - amount * WHEEL_DELTA;
}

LRESULT CDisassemblyFrame::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	switch (uMsg) 
	{
	case WM_CREATE:
		hr = OnCreate(hWnd);
		if (FAILED(hr))
			return -1;
		return 0;
	case WM_CLOSE:
		OnClose(hWnd, uMsg, wParam, lParam);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_GETMINMAXINFO:
		OnGetMinMaxSizeInfo(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_KEYDOWN:
		if (OnKeyDown(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDOWN:
		if (OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_NOTIFY:
		if (OnNotify(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_MOUSEWHEEL:
		OnMouseWheel(hWnd, uMsg, wParam, lParam);
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
	}
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
};

void CDisassemblyFrame::GetMinWindowSize(int &w, int &h)
{
	int w1,h1;
	int w2,h2;
	w = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
	h = GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
	
	m_DisassemblyReg.GetMinWindowSize(w1, h1);
	m_DisassemblyChild.GetMinWindowSize(w2, h2);
	UINT heightRebar = 0;
	if (m_hWndRebar)
	{
		heightRebar = (UINT)SendMessage(this->m_hWndRebar, RB_GETBARHEIGHT, 0, 0);
	}
	if (heightRebar<0)
		heightRebar = 0;

	h += h1;
	h += heightRebar + h1 + h2;
	if (w1 > w2)
		w += w1;
	else
		w += w2;
}
