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
#include "register.h"
#include "errormsg.h"
#include "cevent.h"
#include "monitor.h"
#include "edln.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "resource.h"

#define TOOLBUTTON_WIDTH_96 (16)
#define TOOLBUTTON_HEIGHT_96 (16)

const TCHAR CDisassemblyFrame::ClassName[] = TEXT("Hoxs64DisassemblyFrame");
const TCHAR CDisassemblyFrame::MenuName[] = TEXT("MENU_CPUDISASSEMBLY");

const ImageInfo CDisassemblyFrame::TB_ImageList[] = 
{
	{IDB_DEBUGGERTRACE, IDB_DEBUGGERTRACEMASK},
	{IDB_DEBUGGERTRACEFRAME, IDB_DEBUGGERTRACEFRAMEMASK},
	{IDB_DEBUGGERSTEPONECLOCK, IDB_DEBUGGERSTEPONECLOCKMASK},
	{IDB_DEBUGGERSTEPIN, IDB_DEBUGGERSTEPINMASK},
	{IDB_DEBUGGERTRACEINT, IDB_DEBUGGERTRACEINTMASK},
	{IDB_DEBUGGERSTOP, IDB_DEBUGGERSTOPMASK},
};

#define TOOLBUTTON_COUNT (_countof(TB_StepButtons))

const ButtonInfo CDisassemblyFrame::TB_StepButtons[] = 
{
	{0, TEXT("Trace"), ID_STEP_TRACE},
	{1, TEXT("Trace Frame"), ID_STEP_TRACEFRAME},
	{2, TEXT("1 Clock"), ID_STEP_ONECLOCK},
	{3, TEXT("1 Instruction"), ID_STEP_ONEINSTRUCTION},
	{4, TEXT("Trace INT"), ID_STEP_TRACEINTERRUPTTAKEN},
	{5, TEXT("Stop"), ID_STEP_STOP}
};

CDisassemblyFrame::CDisassemblyFrame()
{
	m_AutoDelete = false;
	m_hWndRebar = NULL;
	m_hWndTooBar = NULL;
	m_hImageListToolBarNormal = NULL;
	m_pParentWindow = NULL;
	m_pszCaption = TEXT("Cpu");
	m_pMon = NULL;
	m_iCurrentControlIndex = 0;
}

CDisassemblyFrame::~CDisassemblyFrame()
{
	Cleanup();
}

HRESULT CDisassemblyFrame::Init(CVirWindow *parent, IMonitorCommand *monitorCommand, Monitor *pMon, LPCTSTR pszCaption)
{
HRESULT hr;
	m_pParentWindow = parent;
	m_pMon = pMon;
	if(monitorCommand==NULL)
		return E_POINTER;
	m_monitorCommand = monitorCommand;
	if (pszCaption!=NULL)
		m_pszCaption = pszCaption;
	hr = InitFonts();
	if (FAILED(hr))
		return hr;
	hr = m_DisassemblyChild.Init(this, monitorCommand, pMon, this->m_monitor_font);
	if (FAILED(hr))
		return hr;

	hr = m_DisassemblyReg.Init(this, monitorCommand, pMon, this->m_monitor_font);
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
}

HRESULT CDisassemblyFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{
		hs = m_monitorCommand->EsResume.Advise((CDisassemblyFrame_EventSink_OnResume *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsTrace.Advise((CDisassemblyFrame_EventSink_OnTrace *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsTraceFrame.Advise((CDisassemblyFrame_EventSink_OnTraceFrame *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsExecuteC64Clock.Advise((CDisassemblyFrame_EventSink_OnExecuteC64Clock *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsExecuteDiskClock.Advise((CDisassemblyFrame_EventSink_OnExecuteDiskClock *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsExecuteC64Instruction.Advise((CDisassemblyFrame_EventSink_OnExecuteC64Instruction *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsExecuteDiskInstruction.Advise((CDisassemblyFrame_EventSink_OnExecuteDiskInstruction *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsShowDevelopment.Advise((CDisassemblyFrame_EventSink_OnShowDevelopment *)this);
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

HRESULT CDisassemblyFrame::InitFonts()
{
	CloseFonts();
	//m_monitor_font = GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf); 
	//m_monitor_font = (HFONT)GetStockObject(ANSI_FIXED_FONT); 
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
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MenuName;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
};


HWND CDisassemblyFrame::Create(HINSTANCE hInstance, HWND parent, const TCHAR title[], int x,int y, int w, int h)
{
	return CVirWindow::Create(0L, ClassName, title, WS_OVERLAPPED | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, parent, NULL, hInstance);
	//return CVirWindow::Create(WS_EX_MDICHILD, ClassName, title, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX  | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, parent, NULL, hInstance);
	
}

HWND CDisassemblyFrame::CreateRebar(HWND hwndTB)
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
		rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP;
		rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
		rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
		rbBand.hbmBack = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_REBARBKGND1));   
   
		// Get the height of the toolbar.
		dwBtnSize = SendMessage(hwndTB, TB_GETBUTTONSIZE, 0,0);

		// Set values unique to the band with the toolbar.
		rbBand.lpText     = TEXT("Tool Bar");
		rbBand.hwndChild  = hwndTB;
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize);
		rbBand.cx         = 250;

		// Add the band that has the toolbar.
		SendMessage(hWndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);		

		return hWndRB;
}

HIMAGELIST CDisassemblyFrame::CreateImageListNormal(HWND hWnd)
{
HIMAGELIST hImageList = NULL;
int r;
bool fail = false;

	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		HDC hMemDC_Dest = CreateCompatibleDC(hdc);
		if (hMemDC_Dest)
		{
			HDC hMemDC_Src = CreateCompatibleDC(hdc);
			if (hMemDC_Src)
			{
				int tool_dx = m_dpi.ScaleX(TOOLBUTTON_WIDTH_96);
				int tool_dy = m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96);
				hImageList = ImageList_Create(tool_dx, tool_dy, ILC_MASK, TOOLBUTTON_COUNT, 0);
				//hImageList = ImageList_Create((TOOLBUTTON_WIDTH_96), (TOOLBUTTON_HEIGHT_96), ILC_MASK, TOOLBUTTON_COUNT, 0);
				if (hImageList)
				{
					HBITMAP hbmpImage = NULL;
					HBITMAP hbmpMask = NULL;
					HBITMAP hBmpImageSz = NULL;
					HBITMAP hBmpMaskSz = NULL;
					for(int i = 0; i < _countof(TB_ImageList); i++)
					{
						hbmpImage = LoadBitmap(m_hInst, MAKEINTRESOURCE(TB_ImageList[i].BitmapImageResourceId));
						if (hbmpImage == NULL)
						{
							fail = true;
							break;
						}
						hbmpMask = LoadBitmap(m_hInst, MAKEINTRESOURCE(TB_ImageList[i].BitmapMaskResourceId));

						BITMAP bitmapImage;
						r = GetObject(hbmpImage, sizeof(BITMAP), &bitmapImage);
						if (!r)
						{
							fail = true;
							break;
						}
						BITMAP bitmapMask;
						r = GetObject(hbmpMask, sizeof(BITMAP), &bitmapMask);
						if (!r)
						{
							fail = true;
							break;
						}
						hBmpImageSz = CreateCompatibleBitmap(hdc, tool_dx, tool_dy);
						if (!hBmpImageSz)
						{
							fail = true;
							break;
						}
						hBmpMaskSz = CreateCompatibleBitmap(hdc, tool_dx, tool_dy);
						if (!hBmpMaskSz)
						{
							fail = true;
							break;
						}

						bool bOK = false;
						HBITMAP hOld_BmpDest = (HBITMAP)SelectObject(hMemDC_Dest, hBmpImageSz);
						HBITMAP hOld_BmpSrc = (HBITMAP)SelectObject(hMemDC_Src, hbmpImage);
						if (hOld_BmpDest && hOld_BmpSrc)
						{
							StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapImage.bmWidth, bitmapImage.bmHeight, SRCCOPY);

							HBITMAP hOld_BmpDest2 = (HBITMAP)SelectObject(hMemDC_Dest, hBmpMaskSz);
							HBITMAP hOld_BmpSrc2 = (HBITMAP)SelectObject(hMemDC_Src, hbmpMask);
							if (hOld_BmpDest2 && hOld_BmpSrc2)
							{
								StretchBlt(hMemDC_Dest, 0, 0, tool_dx, tool_dy, hMemDC_Src, 0, 0, bitmapMask.bmWidth, bitmapMask.bmHeight, SRCCOPY);
								bOK = true;
							}
						}
						if (hOld_BmpDest)
							SelectObject(hMemDC_Dest, hOld_BmpDest);
						if (hOld_BmpSrc)
							SelectObject(hMemDC_Src, hOld_BmpSrc);
						if (!bOK)
						{
							fail = true;
							break;
						}
						r = ImageList_Add(hImageList, hBmpImageSz, hBmpMaskSz);
						if (r < 0)
						{
							fail = true;
							break;
						}
						DeleteObject(hBmpImageSz);
						DeleteObject(hBmpMaskSz);
						hBmpImageSz = NULL;
						hBmpMaskSz = NULL;
						DeleteObject(hbmpImage);
						DeleteObject(hbmpMask);
						hbmpImage = NULL;
						hbmpMask = NULL;
					}
					if (hBmpImageSz != NULL)
					{
						DeleteObject(hBmpImageSz);
						hBmpImageSz = NULL;
					}
					if (hBmpMaskSz != NULL)
					{
						DeleteObject(hBmpMaskSz);
						hBmpMaskSz = NULL;
					}
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
					if (fail)
					{
						if (hImageList != NULL)
						{
							ImageList_Destroy(hImageList);
							hImageList = NULL;
						}
					}
				}
				DeleteDC(hMemDC_Src);
			}
			DeleteDC(hMemDC_Dest);
		}
	}
	return hImageList;
}

HWND CDisassemblyFrame::CreateToolBar(HIMAGELIST hImageListToolBarNormal)
{
HWND hwndTB = NULL;
int i;
LRESULT lr = 0;

	hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPTSTR) NULL, 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS |	WS_CHILD | 	TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NODIVIDER
		, 0, 0, 0, 0, 
		m_hWnd, 
		(HMENU) LongToPtr(ID_TOOLBAR), 
		m_hInst, 
		NULL
	); 

	lr = SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

	lr = SendMessage(hwndTB, TB_SETBUTTONSIZE, 0, MAKELONG(m_dpi.ScaleX(TOOLBUTTON_WIDTH_96), m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96)));

	lr = SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hImageListToolBarNormal);

	//lr = SendMessage(hwndTB, TB_ADDSTRING, 0,  (LPARAM)TEXT("TEST A\0TESTB\0"));


	TBBUTTON tbArray[TOOLBUTTON_COUNT];
	for (i=0; i<_countof(tbArray); i++)
	{
		ZeroMemory(&tbArray[i], sizeof(TBBUTTON));		
		tbArray[i].iBitmap = TB_StepButtons[i].ImageIndex;
		tbArray[i].idCommand = TB_StepButtons[i].CommandId;
		tbArray[i].fsStyle = BTNS_BUTTON;
		tbArray[i].fsState = TBSTATE_ENABLED;
		tbArray[i].iString = -1;
	}
	
	lr = SendMessage(hwndTB, TB_ADDBUTTONS, TOOLBUTTON_COUNT, (LPARAM)tbArray);
	
	return hwndTB;
}


void CDisassemblyFrame::EnsureWindowPosition(int x, int y, int w, int h)
{
RECT rcMain,rcWorkArea;
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
		G::GetWorkArea(rcWorkArea);
	}

	int minw,minh;
	//Check minimum size.
	GetMinWindowSize(minw, minh);
	int workarea_w = (rcWorkArea.right-rcWorkArea.left);
	int workarea_h = (rcWorkArea.bottom-rcWorkArea.top);
	if (bWantDefaultSizeAndPos)
	{

		//Try for half workspace width and half workspace height.
		w = workarea_w/2;
		h = workarea_h/2;
		//Honour minimum size
		if (w<minw)
			w=minw;
		if (h<minh)
			h=minh;

		//Center the window
		x = (workarea_w - w) / 2;
		y = (workarea_h - h) / 2;

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

HRESULT CDisassemblyFrame::Show()
{
	return Show(false);
}

HRESULT CDisassemblyFrame::Show(bool bSeekPC)
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

		hWnd = this->Create(hInstance, hWndParent, m_pszCaption, 0, 0, 0, 0);
		if (hWnd == NULL)
			return E_FAIL;
		GetMinWindowSize(w, h);

		RECT rcDesk;
		G::GetWorkArea(rcDesk);

		int gap = (rcDesk.bottom - rcDesk.top) / 10;
		int id=0;
		if (this->m_pMon->GetCpu())
			id = m_pMon->GetCpu()->GetCpuId();
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
		EnsureWindowPosition(x, y, w, h);
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
			EnsureWindowPosition(x, y, w, h);
		}
	}
	SetMenuState();
	UpdateDisplay(bSeekPC);
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
	m_hImageListToolBarNormal = CreateImageListNormal(hWnd);
	if (m_hImageListToolBarNormal == NULL)
		return E_FAIL;
	m_hWndTooBar = CreateToolBar(m_hImageListToolBarNormal);
	if (m_hWndTooBar == NULL)
		return E_FAIL;
	m_hWndRebar = CreateRebar(m_hWndTooBar);
	//if (m_hWndRebar == NULL)
	//	return E_FAIL;

	int heightRebar = 0;
	if (m_hWndRebar)
	{
		 heightRebar = (int)SendMessage(this->m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
	}
	if (heightRebar < 0)
		heightRebar = 0;

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

void CDisassemblyFrame::SetHome()
{
	this->m_DisassemblyChild.SetHome();;
}

HWND CDisassemblyFrame::CreateDisassemblyReg(int x, int y, int w, int h)
{	
	HWND hWnd = m_DisassemblyReg.Create(m_hInst, m_hWnd, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyFrame::ID_DISASSEMBLEYREG);
	return hWnd;
}

HWND CDisassemblyFrame::CreateDisassemblyChild(int x, int y, int w, int h)
{	
	HWND hWnd = m_DisassemblyChild.Create(m_hInst, m_hWnd, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyFrame::ID_DISASSEMBLEY);
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
		UpdateDisplay(true);
		break; 
	default:
		return false;
	}
	return true;
}

void CDisassemblyFrame::UpdateDisplay(bool bSeekPC)
{
	if (IsWindow(m_hWnd))
	{
		this->m_DisassemblyReg.UpdateDisplay();
		this->m_DisassemblyChild.UpdateDisplay(bSeekPC);
	}
}

void CDisassemblyFrame::CancelEditing()
{
	m_DisassemblyChild.CancelEditing();
	m_DisassemblyReg.CancelEditing();
}

void CDisassemblyFrame::OnResume(void *sender, EventArgs& e)
{
	m_pMon->GetCpu()->ClearBreakOnInterruptTaken();
}

void CDisassemblyFrame::OnTrace(void *sender, EventArgs& e)
{
	if (IsWindow(this->m_hWnd))
	{
		m_DisassemblyReg.InvalidateBuffer();
		m_DisassemblyChild.InvalidateBuffer();
		this->UpdateDisplay(false);
		SetMenuState();
	}
}

void CDisassemblyFrame::OnTraceFrame(void *sender, EventArgs& e)
{
	if (IsWindow(this->m_hWnd))
	{
		this->SetHome();
		this->UpdateDisplay(true);
	}
}

void CDisassemblyFrame::OnExecuteC64Clock(void *sender, EventArgs& e)
{
	this->UpdateDisplay(true);
}

void CDisassemblyFrame::OnExecuteDiskClock(void *sender, EventArgs& e)
{
	this->UpdateDisplay(true);
}

void CDisassemblyFrame::OnExecuteC64Instruction(void *sender, EventArgs& e)
{
	this->UpdateDisplay(true);
}

void CDisassemblyFrame::OnExecuteDiskInstruction(void *sender, EventArgs& e)
{
	this->UpdateDisplay(true);
}

void CDisassemblyFrame::OnShowDevelopment(void *sender, EventArgs& e)
{
	m_pMon->GetCpu()->ClearBreakOnInterruptTaken();
	if (IsWindow(this->m_hWnd))
	{
		SetHome();
		UpdateDisplay(true);
		SetMenuState();
	}
}

bool CDisassemblyFrame::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CDisassemblyFrame::SetMenuState()
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
	UINT stateTb;
	UINT stateTbOpp;
	//
	if (m_monitorCommand->IsRunning())
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
	EnableMenuItem(hMenu, ID_STEP_ONECLOCK, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_ONEINSTRUCTION, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_TRACE, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_TRACEFRAME, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_TRACEINTERRUPTTAKEN, MF_BYCOMMAND | state);
	
	EnableMenuItem(hMenu, ID_STEP_STOP, MF_BYCOMMAND | stateOpp);
	if (m_hWndTooBar!=NULL)
	{
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_ONECLOCK, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_ONEINSTRUCTION, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_TRACE, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_TRACEFRAME, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_TRACEINTERRUPTTAKEN, stateTb);

		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_STOP, stateTbOpp);
	}
}

bool CDisassemblyFrame::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!lParam) 
		return false;

	NMHDR *pn = (NMHDR *)lParam;
	if (!pn->hwndFrom)
		return false;
	switch (pn->code)
	{
	case TBN_GETINFOTIP:
		return OnToolBarInfo((LPNMTBGETINFOTIP)lParam);
	}
	return false;
}

bool CDisassemblyFrame::OnToolBarInfo(LPNMTBGETINFOTIP info)
{
	if (info->hdr.hwndFrom == this->m_hWndTooBar)
	{
		int id = info->iItem;
		if (info->pszText)
		{
			if (id == ID_STEP_ONECLOCK)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Step 1 clock"), _TRUNCATE);
			}
			else if (id == ID_STEP_ONEINSTRUCTION)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Step 1 instruction"), _TRUNCATE);
			}
			else if (id == ID_STEP_TRACE)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Trace"), _TRUNCATE);
			}
			else if (id == ID_STEP_TRACEFRAME)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Trace 1 frame"), _TRUNCATE);
			}
			else if (id == ID_STEP_TRACEINTERRUPTTAKEN)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Trace till IRQ/NMI taken"), _TRUNCATE);
			}
			else if (id == ID_STEP_STOP)
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("Stop tracing"), _TRUNCATE);
			}
			else 
			{
				_tcsncpy_s(info->pszText, info->cchTextMax, TEXT("?"), _TRUNCATE);
			}
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool CDisassemblyFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case ID_STEP_ONECLOCK:
		if (!m_monitorCommand)
			return true;
		if (m_monitorCommand->IsRunning())
			return true;
		CancelEditing();
		if (m_pMon->GetCpu()->GetCpuId() == CPUID_MAIN)
			m_monitorCommand->ExecuteC64Clock();
		else
			m_monitorCommand->ExecuteDiskClock();
		m_monitorCommand->UpdateApplication();
		return true;
	case ID_STEP_ONEINSTRUCTION:
		if (!m_monitorCommand)
			return true;
		if (m_monitorCommand->IsRunning())
			return true;
		CancelEditing();
		if (m_pMon->GetCpu()->GetCpuId() == CPUID_MAIN)
			m_monitorCommand->ExecuteC64Instruction();
		else
			m_monitorCommand->ExecuteDiskInstruction();
		m_monitorCommand->UpdateApplication();
		return true;
	case ID_STEP_TRACEFRAME:
		if (!m_monitorCommand)
			return true;
		if (m_monitorCommand->IsRunning())
			return true;
		CancelEditing();
		m_monitorCommand->TraceFrame();
		m_monitorCommand->UpdateApplication();
		return true;
	case ID_STEP_TRACE:
		if (!m_monitorCommand)
			return true;
		if (m_monitorCommand->IsRunning())
			return true;
		CancelEditing();
		m_monitorCommand->Trace();
		return true;
	case ID_STEP_TRACEINTERRUPTTAKEN:
		if (!m_monitorCommand)
			return true;
		if (m_monitorCommand->IsRunning())
			return true;
		m_pMon->GetCpu()->SetBreakOnInterruptTaken();
		m_monitorCommand->Trace();
		return true;
	case ID_FILE_MONITOR:
	case ID_STEP_STOP:
		if (!m_monitorCommand)
			return true;
		CancelEditing();
		m_monitorCommand->ShowDevelopment();
		::SetForegroundWindow(m_hWnd);
		return true;
	default:
		return true;
	}
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
		if (!OnKeyDown(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		else
			return 0;
	case WM_LBUTTONDOWN:
		if (!OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		else
			return 0;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_NOTIFY:
		if (OnNotify(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);		
	case WM_DESTROY:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);

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
	//case WM_SETFOCUS:
	//	CreateCaret(hWnd, NULL, 10, 10);
	//	SetCaretPos(20, 50);
	//	ShowCaret(hWnd);
	//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	//case WM_KILLFOCUS:
	//	DestroyCaret();
	//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;
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
