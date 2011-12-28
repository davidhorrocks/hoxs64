#include <windows.h>
#include <windowsx.h>
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
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "resource.h"

TCHAR CDisassemblyChild::ClassName[] = TEXT("Hoxs64DisassemblyChild");

CDisassemblyChild::CDisassemblyChild()
{
	m_AutoDelete = false;
	m_pParent = NULL;
	m_hWndScroll = NULL;
	m_pMon = NULL;
}

CDisassemblyChild::~CDisassemblyChild()
{
	Cleanup();
}

void CDisassemblyChild::Cleanup()
{
	UnadviseEvents();
}

HRESULT CDisassemblyChild::Init(CVirWindow *parent, IMonitorCommand *monitorCommand, Monitor *pMon, HFONT hFont)
{
HRESULT hr;
	m_pParent = parent;
	m_pMon = pMon;
	m_monitorCommand = monitorCommand;
	hr = m_DisassemblyEditChild.Init(parent, monitorCommand, pMon, hFont);
	if (FAILED(hr))
		return hr;

	hr = AdviseEvents();
	if (FAILED(hr))
		return hr;
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
		return E_FAIL;
	return S_OK;	
};

HWND CDisassemblyChild::Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID)
{
	return CVirWindow::Create(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, ctrlID, hInstance);
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
		rc.right = rc.left;
	if (rc.bottom < rc.top)
		rc.bottom = rc.top;
	return S_OK;
}

HRESULT CDisassemblyChild::GetSizeRectScrollBar(RECT &rc)
{
BOOL br;
RECT rcChild;
	br = ::GetClientRect(m_hWnd, &rcChild);
	SetRect(&rc, rcChild.right - GetSystemMetrics(SM_CXHTHUMB), rcChild.top, rcChild.right, rcChild.bottom);
	if (rc.right < rc.left)
		rc.right = rc.left;
	if (rc.bottom < rc.top)
		rc.bottom = rc.top;
	return S_OK;
}

HWND CDisassemblyChild::CreateEditWindow(int x, int y, int w, int h)
{
	HWND hWnd = m_DisassemblyEditChild.Create(m_hInst, m_hWnd, x, y, w, h, (HMENU)(INT_PTR)CDisassemblyChild::ID_DISASSEMBLY);
	return hWnd;
}

HRESULT CDisassemblyChild::OnCreate(HWND hWnd)
{
HRESULT hr;
	m_hWndScroll = CreateScrollBar(); 
	if (m_hWndScroll == NULL)
		return E_FAIL;

	RECT rc;
	ZeroMemory(&rc, sizeof(rc));
	hr = GetSizeRectEditWindow(rc);
	if (FAILED(hr))
		return hr;
	HWND hWndDisAsm = CreateEditWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	if (hWndDisAsm == NULL)
		return E_FAIL;	

	ShowScrollBar(m_hWndScroll, SB_CTL, TRUE);
	return S_OK;
}

void CDisassemblyChild::OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
RECT rc;

	hr = GetSizeRectEditWindow(rc);
	if (FAILED(hr))
		return;

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;			
	SetWindowPos(hWnd, 0, x, y, w, h, SWP_NOREPOSITION | SWP_NOZORDER);
}

void CDisassemblyChild::OnSizeScrollBar(HWND hWnd, int widthParent, int heightParent)
{
HRESULT hr;
RECT rc;

	hr = GetSizeRectScrollBar(rc);
	if (FAILED(hr))
		return;

	LONG x,y,w,h;
	G::RectToWH(rc, x, y, w, h);
	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;					
	MoveWindow(hWnd, x, y, w, h, TRUE);
}


void CDisassemblyChild::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
		return;
	if (hWnd != this->m_hWnd)
		return;
	int w = LOWORD(lParam);
	int h = HIWORD(lParam);
	OnSizeScrollBar(m_hWndScroll, w, h);

	HWND hWndDisassemblyEditChild = this->m_DisassemblyEditChild.GetHwnd();
	if (hWndDisassemblyEditChild!=NULL)
		OnSizeDisassembly(hWndDisassemblyEditChild, w, h);

	bit16 address = this->m_DisassemblyEditChild.GetTopAddress();
	this->SetAddressScrollPos(address, true);
}

void CDisassemblyChild::SetAddressScrollPos(int pos, bool bUpdatePageSize)
{

	bit16 topAddress = m_DisassemblyEditChild.GetTopAddress();
	bit16 bottomAddress = m_DisassemblyEditChild.GetBottomAddress(-1);
	int page = abs((int)(bit16s)(bottomAddress - topAddress));
	if (page <=0)
		page = 1;

	SCROLLINFO scrollinfo;
	ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
	scrollinfo.cbSize=sizeof(SCROLLINFO);
	scrollinfo.fMask  = SIF_RANGE | SIF_POS;
	if (bUpdatePageSize)
		scrollinfo.fMask |= SIF_PAGE;
	scrollinfo.nMax=0xffff;
	scrollinfo.nMin=0x0000;
	scrollinfo.nPage=page;

	if ((bit16s)pos < 0 && (bit16s)pos>-32)
		scrollinfo.nPos=0;
	else
		scrollinfo.nPos=(pos) & 0xffff;
	SetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo, TRUE);

}

void CDisassemblyChild::SetHome()
{
	CPUState cpustate;
	this->m_pMon->GetCpu()->GetCpuState(cpustate);
	this->SetTopAddress(cpustate.PC_CurrentOpcode, true);
}

void CDisassemblyChild::SetTopAddress(bit16 address, bool bSetScrollBarPage)
{
	m_DisassemblyEditChild.SetTopAddress(address);
	SetAddressScrollPos((int)address, bSetScrollBarPage);
}

void CDisassemblyChild::UpdateDisplay(bool bSeekPC)
{
	m_DisassemblyEditChild.UpdateDisplay(bSeekPC);
	bit16 address = m_DisassemblyEditChild.GetTopAddress();
	SetAddressScrollPos((int)address, true);
}

void CDisassemblyChild::InvalidateBuffer()
{
	m_DisassemblyEditChild.InvalidateBuffer();
}

void CDisassemblyChild::CancelEditing()
{
	m_DisassemblyEditChild.CancelAsmEditing();
}

void CDisassemblyChild::OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
SCROLLINFO scrollinfo;
BOOL br;
bit16 address, nearestAdress, bottomAddress;
int page;
int pos;

	address = m_DisassemblyEditChild.GetTopAddress();
	switch(LOWORD (wParam)) 
	{
	case SB_TOP:
		address = 0;
		nearestAdress = m_DisassemblyEditChild.GetNearestTopAddress(address);
		SetTopAddress(nearestAdress, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);		
		break;
	case SB_BOTTOM:
		address=0xffc0;
		nearestAdress = m_DisassemblyEditChild.GetNearestTopAddress(address);
		SetTopAddress(nearestAdress, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_PAGEUP:
		bottomAddress = m_DisassemblyEditChild.GetBottomAddress(-1);
		address = m_DisassemblyEditChild.GetTopAddress();
		//FIXME could do with an improved accuracy for paging up.
		page = abs((int)(bit16s)(bottomAddress - address));
		if (page <=0)
			page = 1;

		address-=page;
		nearestAdress = m_DisassemblyEditChild.GetNearestTopAddress(address);
		SetTopAddress(nearestAdress, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_PAGEDOWN:
		bottomAddress = m_DisassemblyEditChild.GetBottomAddress(-1);
		SetTopAddress(bottomAddress, true);		
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_LINEUP:
		address = m_DisassemblyEditChild.GetPrevAddress();
		SetTopAddress(address, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_LINEDOWN:
		address = m_DisassemblyEditChild.GetNextAddress();
		SetTopAddress(address, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_THUMBPOSITION:
		ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
		scrollinfo.cbSize=sizeof(SCROLLINFO);
		scrollinfo.fMask  = SIF_ALL;
		br = GetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo);
		if (!br)
			return;
		pos = scrollinfo.nTrackPos;
		address = (bit16)((unsigned int)pos & 0xffff);
		nearestAdress = m_DisassemblyEditChild.GetNearestTopAddress(address);
		SetTopAddress(nearestAdress, true);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&scrollinfo, sizeof(SCROLLINFO));
		scrollinfo.cbSize=sizeof(SCROLLINFO);
		scrollinfo.fMask  = SIF_ALL;
		br = GetScrollInfo(m_hWndScroll, SB_CTL, &scrollinfo);
		if (!br)
			return;
		pos = scrollinfo.nTrackPos;
		address = (bit16)((unsigned int)pos & 0xffff);
		nearestAdress = m_DisassemblyEditChild.GetNearestTopAddress(address);
		SetTopAddress(nearestAdress, false);
		CancelEditing();
		m_DisassemblyEditChild.UpdateDisplay(false);
		break;
	default:
		return;
	}
}

bool CDisassemblyChild::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//::SetFocus(hWnd);
	return false;
}

bool CDisassemblyChild::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_UP: 
		SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0L);
		break; 
	case VK_DOWN: 
		SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0L);
		break; 
	case VK_NEXT: 
		SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
		break; 
	case VK_PRIOR: 
		SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0L);
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
			return -1;
		return 0;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_HSCROLL:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_VSCROLL:
		OnVScroll(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_KEYDOWN:
		if (!OnKeyDown(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			return 0;
	case WM_LBUTTONDOWN:
		if (!OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			return 0;
	case WM_COMMAND:
		if (!OnCommand(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			return 0;
	case WM_NOTIFY:
		OnNotify(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void CDisassemblyChild::GetMinWindowSize(int &w, int &h)
{
	int w2,h2;

	w = GetSystemMetrics(SM_CXVSCROLL);
	h = 0;

	m_DisassemblyEditChild.GetMinWindowSize(w2, h2);

	w += w2;
	h += h2;
}

void CDisassemblyChild::OnCpuRegPCChanged(void *sender, EventArgs& e)
{
	UpdateDisplay(false);
}

HRESULT CDisassemblyChild::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{
		if (this->m_pMon->GetCpu()->GetCpuId() == CPUID_MAIN)
			hs = this->m_monitorCommand->EsCpuC64RegPCChanged.Advise((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this);
		else
			hs = this->m_monitorCommand->EsCpuDiskRegPCChanged.Advise((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hr = S_OK;
	} while (false);
	return hr;
}

void CDisassemblyChild::UnadviseEvents()
{
	((CDisassemblyChild_EventSink_OnCpuRegPCChanged *)this)->UnadviseAll();
}
