#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "IC64.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "resource.h"

TCHAR CDisassemblyChild::ClassName[] = TEXT("Hoxs64DisassemblyChild");

CDisassemblyChild::CDisassemblyChild(int cpuid, IC64 *c64, IAppCommand *pAppCommand, HFONT hFont) 
	: 
	DefaultCpu(cpuid, c64)
{
HRESULT hr;

	m_hWndScroll = NULL;
	m_pAppCommand = pAppCommand;
	m_pWinDisassemblyEditChild = shared_ptr<CDisassemblyEditChild>(new CDisassemblyEditChild(cpuid, c64, pAppCommand, hFont));
	if (m_pWinDisassemblyEditChild == NULL)
	{
		throw std::bad_alloc();
	}

	hr = Init();
	if (FAILED(hr))
	{
		throw std::exception("CDisassemblyChild::Init() failed");
	}
}

CDisassemblyChild::~CDisassemblyChild()
{
	Cleanup();
}

void CDisassemblyChild::Cleanup() noexcept
{
	UnadviseEvents();
}

HRESULT CDisassemblyChild::Init()
{
HRESULT hr;
	hr = AdviseEvents();
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT CDisassemblyChild::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;//CS_OWNDC; CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(CDisassemblyChild *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
	{
		return E_FAIL;
	}

	return S_OK;	
};

HWND CDisassemblyChild::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

HWND CDisassemblyChild::CreateScrollBar()
{
RECT rect;
	GetClientRect(m_hWnd, &rect);

	HWND hWnd = CreateWindowEx(0L,
		TEXT("SCROLLBAR"),//class name
		NULL,//Title
		WS_CHILD | SBS_VERT | SBS_RIGHTALIGN,//stype
		rect.left,//x
		rect.top,//y
		rect.right - rect.left,//width
		rect.bottom - rect.top,//height
		m_hWnd,//Parent
		(HMENU) LongToPtr(ID_SCROLLBAR),//Menu
		m_hInst,//application instance
		NULL);
	return hWnd;
}

HRESULT CDisassemblyChild::GetSizeRectEditWindow(RECT &rc)
{
BOOL br;
RECT rcChild;
	br = ::GetClientRect(m_hWnd, &rcChild);
	SetRect(&rc, rcChild.left, rcChild.top, rcChild.right - GetSystemMetrics(SM_CXHTHUMB), rcChild.bottom);
	if (rc.right < rc.left)
	{
		rc.right = rc.left;
	}

	if (rc.bottom < rc.top)
	{
		rc.bottom = rc.top;
	}

	return S_OK;
}

HRESULT CDisassemblyChild::GetSizeRectScrollBar(RECT &rc)
{
BOOL br;
RECT rcChild;
	br = ::GetClientRect(m_hWnd, &rcChild);
	SetRect(&rc, rcChild.right - GetSystemMetrics(SM_CXHTHUMB), rcChild.top, rcChild.right, rcChild.bottom);
	if (rc.right < rc.left)
	{
		rc.right = rc.left;
	}

	if (rc.bottom < rc.top)
	{
		rc.bottom = rc.top;
	}

	return S_OK;
}

HWND CDisassemblyChild::CreateEditWindow(int x, int y, int w, int h)
{
	HWND hWnd = m_pWinDisassemblyEditChild->Create(m_hInst, m_hWnd, NULL, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyChild::ID_DISASSEMBLY);
	return hWnd;
}

HRESULT CDisassemblyChild::OnCreate(HWND hWnd)
{
HRESULT hr;
	try
	{
		m_hWndScroll = CreateScrollBar(); 
		if (m_hWndScroll == NULL)
		{
			return E_FAIL;
		}

		RECT rc;
		ZeroMemory(&rc, sizeof(rc));
		hr = GetSizeRectEditWindow(rc);
		if (FAILED(hr))
		{
			return hr;
		}

		HWND hWndDisAsm = CreateEditWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		if (hWndDisAsm == NULL)
		{
			return E_FAIL;	
		}

		ShowScrollBar(m_hWndScroll, SB_CTL, TRUE);
		return S_OK;
	}
	catch(std::exception&)
	{
	}
	return E_FAIL;
}

void CDisassemblyChild::OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
RECT rc;

	hr = GetSizeRectEditWindow(rc);
	if (FAILED(hr))
	{
		return;
	}

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
	{
		w = 0;
	}

	if (h < 0)
	{
		h = 0;
	}

	SetWindowPos(hWnd, 0, x, y, w, h, SWP_NOREPOSITION | SWP_NOZORDER);
}

void CDisassemblyChild::OnSizeScrollBar(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
RECT rc;

	hr = GetSizeRectScrollBar(rc);
	if (FAILED(hr))
	{
		return;
	}

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
	{
		w = 0;
	}

	if (h < 0)
	{
		h = 0;
	}

	MoveWindow(hWnd, x, y, w, h, TRUE);
}


void CDisassemblyChild::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
	{
		return;
	}

	if (hWnd != this->m_hWnd)
	{
		return;
	}

	int w = LOWORD(lParam);
	int h = HIWORD(lParam);
	OnSizeScrollBar(m_hWndScroll, w, h);

	HWND hWndDisassemblyEditChild = this->m_pWinDisassemblyEditChild->GetHwnd();
	if (hWndDisassemblyEditChild!=NULL)
	{
		OnSizeDisassembly(hWndDisassemblyEditChild, w, h);
	}

	bit16 address = this->m_pWinDisassemblyEditChild->GetTopAddress();
	this->SetAddressScrollPos(address);
}

void CDisassemblyChild::SetAddressScrollPos(int pos)
{
	bit16 topAddress = m_pWinDisassemblyEditChild->GetTopAddress();
	bit16 bottomAddress = m_pWinDisassemblyEditChild->GetBottomAddress(-1);
	int page = abs((int)(bit16s)(bottomAddress - topAddress));
	if (page <=0)
	{
		page = 1;
	}

	SCROLLINFO scrollinfo;
	ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
	scrollinfo.cbSize=sizeof(SCROLLINFO);
	scrollinfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
	scrollinfo.nMax=0xffff;
	scrollinfo.nMin=0x0000;
	scrollinfo.nPage=page;

	if ((bit16s)pos < 0 && (bit16s)pos>-32)
	{
		scrollinfo.nPos=0;
	}
	else
	{
		scrollinfo.nPos=(pos) & 0xffff;
	}

	SetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo, TRUE);

}

int CDisassemblyChild::GetNumberOfLines()
{
	return m_pWinDisassemblyEditChild->GetNumberOfLines();
}

void CDisassemblyChild::SetHome()
{
	CPUState cpustate;
	this->GetCpu()->GetCpuState(cpustate);
	this->SetTopAddress(cpustate.PC_CurrentOpcode, true);
}

bit16 CDisassemblyChild::GetTopAddress()
{
	return m_pWinDisassemblyEditChild->GetTopAddress();
}

bit16 CDisassemblyChild::GetNthAddress(bit16 startaddress, int linenumber)
{
	return m_pWinDisassemblyEditChild->GetNthAddress(startaddress, linenumber);
}

void CDisassemblyChild::SetTopAddress(bit16 address, bool bSetScrollBarPage)
{
	m_pWinDisassemblyEditChild->SetTopAddress(address);
	if (bSetScrollBarPage)
	{
		SetAddressScrollPos((int)address);
	}
}

void CDisassemblyChild::UpdateDisplay(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address)
{
	if (pcmode == DBGSYM::SetDisassemblyAddress::SetTopAddress)
	{
		m_pWinDisassemblyEditChild->SetTopAddress(address);
	}

	m_pWinDisassemblyEditChild->UpdateDisplay(pcmode, address);
	bit16 currentaddress = m_pWinDisassemblyEditChild->GetTopAddress();
	SetAddressScrollPos((int)currentaddress);
}

void CDisassemblyChild::InvalidateBuffer()
{
	m_pWinDisassemblyEditChild->InvalidateBuffer();
}

HRESULT CDisassemblyChild::SaveEditing()
{
	return m_pWinDisassemblyEditChild->SaveAsmEditing();
}

void CDisassemblyChild::CancelEditing()
{
	m_pWinDisassemblyEditChild->CancelAsmEditing();
}

void CDisassemblyChild::OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
SCROLLINFO scrollinfo;
BOOL br;
bit16 address, nearestAdress, bottomAddress;
int page;
int pos;

	try
	{
		address = m_pWinDisassemblyEditChild->GetTopAddress();
		switch(LOWORD (wParam)) 
		{
		case SB_TOP:
			address = 0;
			nearestAdress = m_pWinDisassemblyEditChild->GetNearestTopAddress(address);
			SetTopAddress(nearestAdress, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);		
			break;
		case SB_BOTTOM:
			address=0xffc0;
			nearestAdress = m_pWinDisassemblyEditChild->GetNearestTopAddress(address);
			SetTopAddress(nearestAdress, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_PAGEUP:
			page = this->GetNumberOfLines();
			page -= 2;
			if (page < 0)
			{
				page = 1;
			}

			address = this->GetTopAddress();
			address = this->GetNthAddress(address, - page);
			nearestAdress = m_pWinDisassemblyEditChild->GetNearestTopAddress(address);
			SetTopAddress(nearestAdress, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_PAGEDOWN:
			bottomAddress = m_pWinDisassemblyEditChild->GetBottomAddress(-1);
			SetTopAddress(bottomAddress, true);		
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_LINEUP:
			address = m_pWinDisassemblyEditChild->GetPrevAddress();
			SetTopAddress(address, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_LINEDOWN:
			address = m_pWinDisassemblyEditChild->GetNextAddress();
			SetTopAddress(address, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_THUMBPOSITION:
			ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
			scrollinfo.cbSize=sizeof(SCROLLINFO);
			scrollinfo.fMask  = SIF_ALL;
			br = GetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo);
			if (!br)
			{
				return;
			}

			pos = scrollinfo.nTrackPos;
			address = (bit16)((unsigned int)pos & 0xffff);
			nearestAdress = m_pWinDisassemblyEditChild->GetNearestTopAddress(address);
			SetTopAddress(nearestAdress, true);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		case SB_THUMBTRACK:
			ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
			scrollinfo.cbSize=sizeof(SCROLLINFO);
			scrollinfo.fMask  = SIF_ALL;
			br = GetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo);
			if (!br)
			{
				return;
			}

			pos = scrollinfo.nTrackPos;
			address = (bit16)((unsigned int)pos & 0xffff);
			nearestAdress = m_pWinDisassemblyEditChild->GetNearestTopAddress(address);
			SetTopAddress(nearestAdress, false);
			this->SaveEditing();
			this->CancelEditing();
			m_pWinDisassemblyEditChild->UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
			break;
		default:
			return;
		}
	}
	catch(...)
	{
	}
}

bool CDisassemblyChild::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CDisassemblyChild::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (wParam)
		{
		case VK_UP: 
			SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0L);
			return true; 
		case VK_DOWN: 
			SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0L);
			return true;
		case VK_NEXT: 
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
			return true;
		case VK_PRIOR: 
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0L);
			return true;
		case VK_HOME:
			SetHome();
			UpdateDisplay(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
			return true;
		}
	}
	catch(std::exception&)
	{
	}

	return false;
}

bool CDisassemblyChild::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

bool CDisassemblyChild::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == m_hWnd)
	{
		SendMessage(::GetParent(hWnd), WM_COMMAND, wParam, lParam);
		return true;
	}

	return false;
}

LRESULT CDisassemblyChild::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	switch (uMsg) 
	{
	case WM_CREATE:
		hr = OnCreate(hWnd);
		if (FAILED(hr))
		{
			return -1;
		}

		return 0;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_VSCROLL:
		OnVScroll(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_KEYDOWN:
		if (OnKeyDown(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}

		break;
	case WM_LBUTTONDOWN:
		if (OnLButtonDown(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}

		break;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}

		break;
	case WM_NOTIFY:
		if (OnNotify(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HRESULT CDisassemblyChild::UpdateMetrics()
{
HRESULT hr;
	hr = m_pWinDisassemblyEditChild->UpdateMetrics();
	int w2,h2;
	int w = GetSystemMetrics(SM_CXVSCROLL);
	int h = 0;
	m_pWinDisassemblyEditChild->GetMinWindowSize(w2, h2);
	w += w2;
	h += h2;
	m_MinSizeW = w;
	m_MinSizeH = h;
	return hr;
}

void CDisassemblyChild::SetRadix(DBGSYM::MonitorOption::Radix radix)
{
	m_pWinDisassemblyEditChild->SetRadix(radix);
}

void CDisassemblyChild::GetMinWindowSize(int &w, int &h)
{
	w = m_MinSizeW;
	h = m_MinSizeH;
}

void CDisassemblyChild::OnCpuRegPCChanged(void *sender, EventArgs& e)
{
	UpdateDisplay(DBGSYM::SetDisassemblyAddress::None, 0);
}

HRESULT CDisassemblyChild::AdviseEvents()
{
	HRESULT hr;
	HSink hs = nullptr;
	hr = S_OK;
	do
	{
		int cpuid = this->GetCpu()->GetCpuId();
		if (cpuid == CPUID_MAIN)
		{
			hs = this->m_pAppCommand->EsCpuC64RegPCChanged.Advise((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this);
		}
		else if (cpuid == CPUID_DISK)
		{
			hs = this->m_pAppCommand->EsCpuDiskRegPCChanged.Advise((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this);
		}

		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}

		hr = S_OK;
	} while (false);

	return hr;
}

void CDisassemblyChild::UnadviseEvents() noexcept
{
	((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this)->UnadviseAll();
}
