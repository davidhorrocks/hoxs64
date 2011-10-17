#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <tchar.h>
#include "defines.h"
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
#include "resource.h"

TCHAR CDisassemblyEditChild::ClassName[] = TEXT("Hoxs64DisassemblyEditChild");

CDisassemblyEditChild::CDisassemblyEditChild()
{
	WIDTH_LEFTBAR2 = WIDTH_LEFTBAR;
	LINE_HEIGHT = 16;

	m_AutoDelete = false;
	m_pParent = NULL;
	m_FirstAddress = 0;
	m_NumLines = 0;
	m_hFont = NULL;
	m_MinSizeDone = false;
	m_pFrontTextBuffer = NULL;
	m_pBackTextBuffer = NULL;
	m_monitorCommand = NULL;
	m_hBmpBreak = NULL;
	m_bHasLastDrawText = false;
	m_bIsFocusedAddress = false;
	m_iFocusedAddress = 0;
	m_bMouseDownOnFocusedAddress = false;
	m_hWndEditText = NULL;
	m_wpOrigEditProc = NULL;
	ZeroMemory(&m_rcLastDrawText, sizeof(m_rcLastDrawText));	
}

CDisassemblyEditChild::~CDisassemblyEditChild()
{
	Cleanup();
}

void CDisassemblyEditChild::Cleanup()
{
	m_hWndEditText = NULL;
	m_wpOrigEditProc = NULL;

	FreeTextBuffer();
	if (m_hBmpBreak)
	{
		DeleteBitmap(m_hBmpBreak);
		m_hBmpBreak=NULL;
	}

}

HRESULT CDisassemblyEditChild::Init(CVirWindow *parent, IMonitorCommand *monitorCommand, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont)
{
HRESULT hr;
	Cleanup();

	this->m_pParent = parent;
	this->m_hFont = hFont;
	this->m_cpu = cpu;
	this->m_monitorCommand = monitorCommand;


	hr = AllocTextBuffer();
	m_mon.Init(cpu, vic);
	m_FirstAddress = GetNearestTopAddress(0);
	m_NumLines = 0;
	return S_OK;
}

HRESULT CDisassemblyEditChild::AllocTextBuffer()
{
	m_pFrontTextBuffer = new AssemblyLineBuffer [MAX_BUFFER_HEIGHT];
	if (m_pFrontTextBuffer == NULL)
		return E_OUTOFMEMORY;
	m_pBackTextBuffer = new AssemblyLineBuffer [MAX_BUFFER_HEIGHT];
	if (m_pBackTextBuffer == NULL)
		return E_OUTOFMEMORY;
	return S_OK;
}

void CDisassemblyEditChild::FreeTextBuffer()
{
	if (m_pFrontTextBuffer != NULL)
	{
		delete[] m_pFrontTextBuffer;
		m_pFrontTextBuffer = NULL;
	}
	if (m_pBackTextBuffer != NULL)
	{
		delete[] m_pBackTextBuffer;
		m_pBackTextBuffer = NULL;
	}
}

void CDisassemblyEditChild::ClearBuffer()
{
	if (m_pFrontTextBuffer != NULL)
	{
		for(int i=0 ; i < MAX_BUFFER_HEIGHT ; i++)
		{
			m_pFrontTextBuffer[i].Clear();
		}
	}
	if (m_pBackTextBuffer != NULL)
	{
		for(int i=0 ; i < MAX_BUFFER_HEIGHT ; i++)
		{
			m_pBackTextBuffer[i].Clear();
		}
	}
}

void CDisassemblyEditChild::InvalidateBuffer()
{
	ClearBuffer();
	if (m_hWnd)
		InvalidateRect(m_hWnd, NULL, TRUE);
}

void CDisassemblyEditChild::FlipBuffer()
{
	AssemblyLineBuffer *t = m_pFrontTextBuffer;
	m_pFrontTextBuffer = m_pBackTextBuffer;
	m_pBackTextBuffer = t;
}

HRESULT CDisassemblyEditChild::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(CDisassemblyEditChild *);
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


HWND CDisassemblyEditChild::Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID)
{
	if (m_hBmpBreak == NULL)
	{
		m_hBmpBreak = (HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_BREAK), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
		if (m_hBmpBreak==NULL)
			return NULL;
	}
	return CVirWindow::Create(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, ctrlID, hInstance);
}

HWND CDisassemblyEditChild::CreateAsmEdit(HWND hWndParent)
{
RECT rect;
	GetClientRect(hWndParent, &rect);
	SetRect(&rect, 0, 0, 10, 10);

	HWND hWnd = CreateWindowEx(0L,
		TEXT("EDIT"),//class name
		NULL,//Title
		WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,//stype
		rect.left,//x
		rect.top,//y
		rect.right - rect.left,//width
		rect.bottom - rect.top,//height
		hWndParent,//Parent
		(HMENU) LongToPtr(ID_EDITDISASSEMBLY),//Menu
		m_hInst,//application instance
		NULL);
	if (hWnd)
	{
		SendMessage(hWnd, WM_SETFONT, (WPARAM)m_hFont, FALSE);
		m_wpOrigEditProc = SubclassChildWindow(hWnd);
	}
	return hWnd;
}

bool CDisassemblyEditChild::IsEditing()
{
	if (this->m_hWndEditText!=NULL)
	{
		if (GetFocus() == m_hWndEditText)
		{
			return true;
		}
	}
	return false;
}

void CDisassemblyEditChild::CancelAsmEditing()
{
	bool bHadFocus = IsEditing();
	this->HideEditMnemonic();
	if (bHadFocus && m_hWnd != NULL)
		SetFocus(m_hWnd);
}

HRESULT CDisassemblyEditChild::SaveAsmEditing()
{
	this->HideEditMnemonic();
	return S_OK;
}


HRESULT CDisassemblyEditChild::OnCreate(HWND hWnd)
{
	m_hWndEditText = CreateAsmEdit(hWnd);
	return S_OK;
}

void CDisassemblyEditChild::OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == m_hWnd)
	{
		SendMessage(::GetParent(hWnd), WM_COMMAND, wParam, lParam);
	}
}

LRESULT CDisassemblyEditChild::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
PAINTSTRUCT ps;
HDC hdc;
BOOL br;
	switch (uMsg) 
	{
	case WM_CREATE:
		hr = OnCreate(hWnd);
		if (FAILED(hr))
			return -1;
		return 0;
	case WM_PAINT:
		br = GetUpdateRect(hWnd, NULL, FALSE);
		if (!br)
			break;
		hdc = BeginPaint (hWnd, &ps);
		if (hdc != NULL)
		{
			DrawDisplay(hWnd, hdc);
		}
		EndPaint (hWnd, &ps);
		break;
	case WM_SIZE:
		if (wParam==SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		if (wParam==SIZE_MINIMIZED)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		UpdateDisplay(false);
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
	case WM_LBUTTONUP:
		if (!OnLButtonUp(hWnd, uMsg, wParam, lParam))
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
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
	//case WM_HSCROLL:
	//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	//case WM_VSCROLL:
	//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SETFOCUS:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_CLOSE:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CDisassemblyEditChild::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == NULL)
		return 0;
	if (hWnd == this->m_hWndEditText)
	{
		switch(uMsg)
		{
		case WM_CHAR:
			if (wParam == VK_ESCAPE)
			{				
				return 0;
			}
			else if (wParam == VK_RETURN)
			{				
				return 0;
			}
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				CancelAsmEditing();
				return 0;
			}
			else if (wParam == VK_RETURN)
			{
				SaveAsmEditing();
				return 0;
			}
			break;
		}
		if (m_wpOrigEditProc)
		{
			return ::CallWindowProc(m_wpOrigEditProc, hWnd, uMsg, wParam, lParam);
		}
	}
	return 0;
}

void CDisassemblyEditChild::DrawDisplay(HWND hWnd, HDC hdc)
{
int prevMapMode = 0;
HFONT prevFont = NULL;
HBRUSH prevBrush = NULL;
HBRUSH bshWhite;
	bshWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
	if (bshWhite)
	{
		prevMapMode = SetMapMode(hdc, MM_TEXT);
		if (prevMapMode)
		{
			prevFont = (HFONT)SelectObject(hdc, m_hFont);		
			if (prevFont)
			{				
				prevBrush = (HBRUSH)SelectObject(hdc, bshWhite);		
				if (prevBrush)
				{	
					DrawDisplay2(hWnd, hdc);
				}
			}
		}
	}

	if (prevBrush)
		SelectObject(hdc, prevBrush);
	if (prevFont)
		SelectObject(hdc, prevFont);
	if (prevMapMode)
		SetMapMode(hdc, prevMapMode);
}

int CDisassemblyEditChild::GetNumberOfLines(RECT& rc, int lineHeight)
{
	int num = 0;
	if (lineHeight > 0)
		num = (rc.bottom - rc.top - MARGIN_TOP - PADDING_TOP) / lineHeight;
	if (num <= 0)
		num = 1;
	else
		num++;
	if (num > MAX_BUFFER_HEIGHT)
		num = MAX_BUFFER_HEIGHT;
	return num;
}

void CDisassemblyEditChild::UpdateDisplay(bool bSeekPC)
{
	UpdateBuffer(bSeekPC);
	InvalidateRectChanges();
	UpdateWindow(m_hWnd);
}

void CDisassemblyEditChild::GetRect_Bar(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);		
	prc->right = prc->left + WIDTH_LEFTBAR;
}

void CDisassemblyEditChild::GetRect_Status(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);
	prc->left += WIDTH_LEFTBAR;
	prc->right = prc->left + WIDTH_LEFTBAR2;
}

void CDisassemblyEditChild::GetRect_Edit(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);
	prc->left = WIDTH_LEFTBAR + WIDTH_LEFTBAR2;
}

void CDisassemblyEditChild::InvalidateRectChanges()
{
	RECT rcLine;
	RECT rcClient;
	RECT rcChange;
	if (m_bHasLastDrawText)
	{
		InvalidateRect(m_hWnd, &m_rcLastDrawText, TRUE);
		m_bHasLastDrawText = false;
	}
	if (GetClientRect(m_hWnd, &rcClient))
	{
		int y = MARGIN_TOP + PADDING_TOP;
		int x = rcClient.left;
		for (int i=0; i < m_NumLines; i++, y+=LINE_HEIGHT)
		{
			AssemblyLineBuffer &front = m_pFrontTextBuffer[i];
			AssemblyLineBuffer &back = m_pBackTextBuffer[i];
			if (!front.IsEqual(back) || !front.IsValid)
			{				
				SetRect(&rcLine, x, y, rcClient.right, y+LINE_HEIGHT);
				if (IntersectRect(&rcChange, &rcClient, &rcLine))
					InvalidateRect(m_hWnd, &rcChange, TRUE);
			}
		}
		if (y < rcClient.bottom)
		{
			SetRect(&rcChange, x, y, rcClient.right, rcClient.bottom);
			InvalidateRect(m_hWnd, &rcChange, TRUE);
		}
	}
}

bool CDisassemblyEditChild::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
RECT rcClient;
RECT rcBarBreak;
RECT rcEdit;
AssemblyLineBuffer *pAlb;
bit16 address;
bit16 prevAddress;
bool bHasPrevAddress;

	m_bMouseDownOnFocusedAddress = false;

	if (m_monitorCommand->IsRunning())
		return false;
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam);

	GetClientRect(hWnd, &rcClient);

	GetRect_Bar(rcClient, &rcBarBreak);
	GetRect_Edit(rcClient, &rcEdit);

	POINT pt = {xPos,yPos};
	if (PtInRect(&rcBarBreak, pt))
	{
		//Mouse down in breakpoint bar
		int iline = GetLineFromYPos(yPos);
		if (iline >= 0 && iline < this->m_NumLines - 1)
		{
			pAlb = &this->m_pFrontTextBuffer[iline];
			address = pAlb->Address;
			if (m_cpu->IsBreakPoint(address))
			{
				m_cpu->ClearBreakPoint(address);
				pAlb->IsBreak = false;
			}
			else
			{
				m_cpu->SetExecute(address, 1);
				pAlb->IsBreak = true;
			}

			InvalidateRectChanges();
			UpdateWindow(m_hWnd);
		}
	}
	else if (PtInRect(&rcEdit, pt))
	{
		if (GetFocus() == this->m_hWndEditText)
		{
			int xx=0;
		}
		//Mouse down in edit text
		int iline = GetLineFromYPos(yPos);
		if (iline >= 0 && iline < this->m_NumLines - 1)
		{
			pAlb = &this->m_pFrontTextBuffer[iline];
			address = pAlb->Address;
			if (!IsEditing())
			{
				bHasPrevAddress = this->GetFocusedAddress(&prevAddress);
				if (bHasPrevAddress)
				{
					if (prevAddress == address)
					{
						m_bMouseDownOnFocusedAddress = true;
					}
				}
			}
			SetFocusedAddress(address);
		}
		else
		{
			ClearFocusedAddress();
		}
		InvalidateRectChanges();
		UpdateWindow(m_hWnd);
	}
	else
	{
		ClearFocusedAddress();
		InvalidateRectChanges();
		UpdateWindow(m_hWnd);
	}

	if (hWnd)
	{
		::SetFocus(hWnd);
	}
	return true;
}

bool CDisassemblyEditChild::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AssemblyLineBuffer *pAlb;
	RECT rcClient;
	RECT rcEdit;

	if (m_monitorCommand->IsRunning())
		return false;
	if (::GetFocus() != hWnd)
		return true;

	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam);
	POINT pt = {xPos,yPos};

	GetClientRect(hWnd, &rcClient);
	GetRect_Edit(rcClient, &rcEdit);

	if (PtInRect(&rcEdit, pt) && m_bMouseDownOnFocusedAddress)
	{
		int iline = GetLineFromYPos(yPos);
		if (iline >= 0 && iline < this->m_NumLines - 1)
		{
			pAlb = &this->m_pFrontTextBuffer[iline];
			if (pAlb->IsFocused)
			{
				ShowEditMnemonic(pAlb);
			}
		}
	}
	return true;
}

void CDisassemblyEditChild::ShowEditMnemonic(AssemblyLineBuffer *pAlb)
{
RECT rcEdit;
	if (m_hWndEditText==NULL)
		return;
	CopyRect(&rcEdit, &pAlb->MnemonicRect);
	//SM_CXEDGE,SM_CXBORDER
	InflateRect(&rcEdit, 2 * ::GetSystemMetrics(SM_CYBORDER), 2 * ::GetSystemMetrics(SM_CXBORDER));
	rcEdit.right-= 2 *::GetSystemMetrics(SM_CXBORDER);
	SetWindowPos(m_hWndEditText, HWND_TOP, rcEdit.left, rcEdit.top, rcEdit.right - rcEdit.left, rcEdit.bottom - rcEdit.top, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
	SetFocus(m_hWndEditText);
}

void CDisassemblyEditChild::HideEditMnemonic()
{
	if (m_hWndEditText==NULL)
		return;
	SetWindowPos(m_hWndEditText, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE);
}

bool CDisassemblyEditChild::GetFocusedAddress(bit16 *address)
{
	if (!m_bIsFocusedAddress)
		return false;
	if (address!=NULL)
	{
		*address = m_iFocusedAddress;
	}
	return true;
}

void CDisassemblyEditChild::SetFocusedAddress(bit16 address)
{
AssemblyLineBuffer *pAlb;
bool bFound = false;

	for (int i = 0; i < m_NumLines; i++)
	{
		pAlb = &this->m_pFrontTextBuffer[i];
		if (pAlb->Address == address && !bFound)
		{
			pAlb->IsFocused = true;
			bFound = true;
		}
		else
		{
			pAlb->IsFocused = false;
		}
	}
	if (bFound)
	{
		this->m_bIsFocusedAddress = true;
		this->m_iFocusedAddress = address;
	}
	else
	{
		this->m_bIsFocusedAddress = false;
	}
}

void CDisassemblyEditChild::ClearFocusedAddress()
{
	this->m_bIsFocusedAddress = false;
	for (int i = 0; i < m_NumLines; i++)
	{
		this->m_pFrontTextBuffer[i].IsFocused = false;
	}
}

bool CDisassemblyEditChild::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SendMessage(::GetParent(hWnd), uMsg, wParam, lParam);
	return true;
}

bool CDisassemblyEditChild::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != m_hWnd)
		return false;

	if ((HWND)lParam == this->m_hWndEditText)
	{
		switch (HIWORD(wParam))
		{
		case EN_KILLFOCUS:
			this->HideEditMnemonic();
			break;
		}
	}
	if (lParam == NULL)
		SendMessage(::GetParent(hWnd), WM_COMMAND, wParam, lParam);
	return true;
}

bool CDisassemblyEditChild::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LPNMHDR pn;
	if (lParam == NULL)
		return 0;
	pn = (LPNMHDR)lParam;
	if (pn->hwndFrom == this->m_hWndEditText)
	{
		//switch (pn->code)
		//{
		//case NM_KILLFOCUS:
		//	this->HideEditMnemonic();
		//	break;
		//}
	}
	return 0;
}

void CDisassemblyEditChild::SetHome()
{
	CPUState cpustate;
	this->m_cpu->GetCpuState(cpustate);
	this->SetTopAddress(cpustate.PC_CurrentOpcode);
}

int CDisassemblyEditChild::GetLineFromYPos(int y)
{
	if (LINE_HEIGHT==0)
		return -1;
	int k = (y - MARGIN_TOP - PADDING_TOP) / LINE_HEIGHT;
	return k;
}

void CDisassemblyEditChild::DrawDisplay2(HWND hWnd, HDC hdc)
{
RECT rcClient;
RECT rcBarBreak;
RECT rcBarStatus;
RECT rcEdit;
BOOL br;
HBRUSH bshBarBreak;
HBRUSH bshBarStatus;
HBRUSH bshEdit;
TEXTMETRIC tm;

	CPUState cpustate;
	m_cpu->GetCpuState(cpustate);

	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

	SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

	br = GetClientRect(hWnd, &rcClient);
	if (!br)
		return;

	GetRect_Bar(rcClient, &rcBarBreak);
	GetRect_Status(rcClient, &rcBarStatus);
	GetRect_Edit(rcClient, &rcEdit);

	bshBarBreak = GetSysColorBrush(COLOR_3DFACE);
	if (bshBarBreak != NULL)
	{
		SelectObject(hdc, bshBarBreak);
		FillRect(hdc, &rcBarBreak, bshBarBreak);
	}
	bshBarStatus = GetSysColorBrush (COLOR_WINDOW);
	bshEdit = GetSysColorBrush (COLOR_WINDOW);

	FillRect(hdc, &rcEdit, bshEdit);

	HPEN oldpen = NULL;
	if (bshBarStatus != NULL)
	{
		SelectObject(hdc, bshBarStatus);
		FillRect(hdc, &rcBarStatus, bshBarStatus);
		
		HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
		if (pen)
		{
			oldpen = (HPEN)SelectObject(hdc, pen);
			if (oldpen)
			{
				MoveToEx(hdc, rcBarStatus.right - 1, rcBarStatus.top, NULL);
				LineTo(hdc, rcBarStatus.right - 1, rcBarStatus.bottom);
				SelectObject(hdc, oldpen);
			}
			DeleteObject(pen);
		}
	}

	bool bUnavailable = m_monitorCommand->IsRunning();

	br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		if (tm.tmHeight > 0 && rcClient.bottom > rcClient.top && m_pFrontTextBuffer != NULL && m_pBackTextBuffer != NULL)
		{
			int x_status = WIDTH_LEFTBAR;
			int x,y;
			int slen;
			y = MARGIN_TOP + PADDING_TOP;
			SIZE sizeText;
			BOOL brTextExtent;
			int lineHeight = tm.tmHeight;

			LONG width_intflag = 0;
			TCHAR szIntText[] = TEXT("I");
			slen = (int)_tcsnlen(szIntText, _countof(szIntText));
			brTextExtent = GetTextExtentExPoint(hdc, szIntText, slen, 0, NULL, NULL, &sizeText);
			if (brTextExtent)
			{
				width_intflag = sizeText.cx;
			}
			else
			{
				LONG width_intflag = tm.tmAveCharWidth * slen;
			}

			SIZE sz;
			const int defwidth = 10;
			LPCTSTR szSampleAddress = TEXT("$ABCDxx");
			LPCTSTR szSampleBytes = TEXT("ABxABxABxx");
			int xcol_Address = WIDTH_LEFTBAR + WIDTH_LEFTBAR2 + PADDING_LEFT;
			int xcol_Bytes = xcol_Address+defwidth*lstrlen(szSampleAddress);
			int xcol_Mnemonic = xcol_Bytes+defwidth*lstrlen(szSampleBytes);
			if (::GetTextExtentExPoint(hdc, szSampleAddress,lstrlen(szSampleAddress),0,NULL,NULL,&sz))
			{							
				xcol_Bytes = xcol_Address + sz.cx;
			}
			if (::GetTextExtentExPoint(hdc, szSampleBytes,lstrlen(szSampleBytes),0,NULL,NULL,&sz))
			{							
				xcol_Mnemonic = xcol_Bytes + sz.cx;
			}

			if (!bUnavailable)
			{
				BITMAP bmpBreak;
				HDC hMemDC = CreateCompatibleDC(hdc);
				if (hMemDC)
				{
					HBITMAP hbmpPrev = 0;
					if (GetObject(m_hBmpBreak, sizeof(BITMAP), (BITMAP *)&bmpBreak))
					{
						hbmpPrev = (HBITMAP)SelectObject(hMemDC, m_hBmpBreak);
						if (hbmpPrev)
						{
							for (int i = 0; i < m_NumLines; i++, y += lineHeight)
							{
								AssemblyLineBuffer albFront = m_pFrontTextBuffer[i];
								SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

								if (m_cpu->IsBreakPoint(albFront.Address))
								{
									SelectObject(hdc, bshBarBreak);
									DWORD dwROP = MERGECOPY;
									BitBlt(hdc, 0, y, bmpBreak.bmWidth, bmpBreak.bmHeight, hMemDC, 0, 0, dwROP);
									SelectObject(hdc, bshBarStatus);
								}

								if (albFront.Address == cpustate.PC_CurrentOpcode)
								{
									if (cpustate.IsInterruptInstruction)
									{
										SetTextColor(hdc, RGB(0xff,02,70));
										ExtTextOut(hdc, x_status, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, szIntText, lstrlen(szIntText), NULL);
									}									
									TCHAR szArrowText[3] = TEXT(">0");
									if (cpustate.cycle >= 0 && cpustate.cycle <= 9)
										szArrowText[1] = TEXT('0') + ((abs(cpustate.cycle) % 10));
									else
										szArrowText[1] = TEXT('#');
									SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									ExtTextOut(hdc, x_status + width_intflag, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, szArrowText, lstrlen(szArrowText), NULL);
								}

								//Draw the address text
								x = xcol_Address;
								slen = (int)_tcsnlen(albFront.AddressText, _countof(albFront.AddressText));
								if (slen > 0)
								{
									brTextExtent = GetTextExtentExPoint(hdc, albFront.AddressText, slen, 0, NULL, NULL, &sizeText);
									SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.AddressText, slen, NULL);
									if (brTextExtent)
										x = x + sizeText.cx + 1 * tm.tmAveCharWidth;
									else
										x = x + (slen + 1) * tm.tmAveCharWidth;
								}
								else
								{
									x = x + (Monitor::BUFSIZEADDRESSTEXT) * tm.tmAveCharWidth;
								}

								//Draw the data bytes text
								x = xcol_Bytes;
								slen = (int)_tcsnlen(albFront.BytesText, _countof(albFront.BytesText));
								if (slen > 0)
								{
									brTextExtent = GetTextExtentExPoint(hdc, albFront.BytesText, slen, 0, NULL, NULL, &sizeText);
									SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.BytesText, slen, NULL);
									if (brTextExtent)
										x = x + sizeText.cx + 1 * tm.tmAveCharWidth;
									else
										x = x + (slen + 1) * tm.tmAveCharWidth;
								}
								else
								{
									x = x + (Monitor::BUFSIZEINSTRUCTIONBYTESTEXT) * tm.tmAveCharWidth;
								}

								if (slen < Monitor::BUFSIZEINSTRUCTIONBYTESTEXT - 1)
									x = x + (Monitor::BUFSIZEINSTRUCTIONBYTESTEXT - slen - 1) * tm.tmMaxCharWidth;

								//Draw the mnemonic text
								x = xcol_Mnemonic;
								SetRect(&albFront.MnemonicRect, xcol_Mnemonic, y, rcClient.right, y + lineHeight);
								slen = (int)_tcsnlen(albFront.MnemonicText, _countof(albFront.MnemonicText));
								if (slen > 0)
								{
									if (albFront.IsUnDoc)
										SetTextColor(hdc, RGB(2, 134, 172));
									else
										SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									brTextExtent = GetTextExtentExPoint(hdc, albFront.MnemonicText, slen, 0, NULL, NULL, &sizeText);
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.MnemonicText, slen, NULL);
									if (brTextExtent)
										x = x + sizeText.cx + 1 * tm.tmAveCharWidth;
									else
										x = x + (slen + 1) * tm.tmAveCharWidth;
								}
								else
								{
									x = x + (Monitor::BUFSIZEINSTRUCTIONBYTESTEXT) * tm.tmAveCharWidth;
								}
								if (albFront.IsFocused)
								{
									::DrawFocusRect(hdc, &albFront.MnemonicRect);
								}
								albFront.WantUpdate = false;
								m_pFrontTextBuffer[i] = albFront;
								m_pBackTextBuffer[i] = albFront;
				
							}
							SelectObject(hMemDC, hbmpPrev);						
						}
					}		
					DeleteDC(hMemDC);
				}
			}
			else
			{
				TCHAR *tTitle;
				RECT rcText;
				::SetRect(&rcText, WIDTH_LEFTBAR + WIDTH_LEFTBAR2 + PADDING_LEFT, MARGIN_TOP + PADDING_TOP, rcClient.right - PADDING_RIGHT, rcClient.bottom);
				if (rcClient.right > rcClient.left && rcClient.bottom > rcClient.top)
				{
					tTitle = TEXT("CPU disassembly window unavailable during trace.");
					slen = (int)_tcslen(tTitle);
					
					if (DrawText(hdc, tTitle, slen, &rcText, DT_TOP | DT_WORDBREAK | DT_CALCRECT))
					{
						m_bHasLastDrawText = true;
						::CopyRect(&m_rcLastDrawText, &rcText);
						DrawText(hdc, tTitle, slen, &rcText, DT_TOP | DT_WORDBREAK);
					}
				}
			}
		}
	}

}

void CDisassemblyEditChild::UpdateBuffer(bool bEnsurePC)
{
	UpdateBuffer(m_pFrontTextBuffer, m_NumLines, bEnsurePC);
}

void CDisassemblyEditChild::UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, int numLines, bool bEnsurePC)
{
RECT rc;
	ZeroMemory(&rc, sizeof(rc));
	if (GetClientRect(m_hWnd, &rc))
	{
		m_NumLines = GetNumberOfLines(rc, LINE_HEIGHT);
		int pcline=-1;
		bit16 address = this->m_FirstAddress;
		UpdateBuffer(assemblyLineBuffer, address, 0, numLines, pcline);
		if (bEnsurePC)
		{
			if (pcline < 0 || pcline > m_NumLines - 5)
			{
				CPUState cpustate;
				m_cpu->GetCpuState(cpustate);
				address = cpustate.PC_CurrentOpcode;
				this->SetTopAddress(address);
				UpdateBuffer(assemblyLineBuffer, address, 0, numLines, pcline);
			}
		}
	}
}

void CDisassemblyEditChild::UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, bit16 address, int startLine, int numLines, int& lineOfPC)
{
bit16 currentAddress = address;
CPUState cpustate;
	
	m_cpu->GetCpuState(cpustate);

	bit16s pcdiff_start = (bit16s)((cpustate.PC_CurrentOpcode - address) & 0xffff);	
	bool pcfound = false;
	int currentLine = startLine;
	for (currentLine = startLine; currentLine < startLine + numLines && currentLine < MAX_BUFFER_HEIGHT; currentLine++)
	{
		AssemblyLineBuffer buffer = AssemblyLineBuffer();

		CopyRect(&buffer.MnemonicRect, &m_pFrontTextBuffer[currentLine].MnemonicRect);

		if (!pcfound)
		{
			if (currentAddress == cpustate.PC_CurrentOpcode)
			{
				pcfound = true;
			}
			else
			{
				bit16s pcdiff_current = (bit16s)((cpustate.PC_CurrentOpcode - currentAddress) & 0xffff);	
				if (pcdiff_start > 0 && pcdiff_current < 0)
				{
					currentAddress = cpustate.PC_CurrentOpcode;
					pcfound = true;
				}
			}
		}

		if (currentAddress == cpustate.PC_CurrentOpcode)
		{
			buffer.IsPC = true;
			buffer.InstructionCycle = cpustate.cycle;
			lineOfPC = currentLine;
		}

		if (currentAddress == this->m_iFocusedAddress)
		{
			if (this->m_bIsFocusedAddress)
			{
				buffer.IsFocused = true;
			}
		}

		if (m_cpu->IsBreakPoint(currentAddress))
			buffer.IsBreak = true;
		else
			buffer.IsBreak = false;

		int r = m_mon.DisassembleOneInstruction(currentAddress, -1, buffer.AddressText, _countof(buffer.AddressText), buffer.BytesText, _countof(buffer.BytesText), buffer.MnemonicText, _countof(buffer.MnemonicText), buffer.IsUnDoc);
		buffer.Address = currentAddress;
		buffer.IsValid = true;
		m_pFrontTextBuffer[currentLine] = buffer;
		currentAddress += r;
	}

}

bit16 CDisassemblyEditChild::GetNearestTopAddress(bit16 address)
{
bool isUndoc = false;
bit16 currentAddress = address - 32;
bit16 nextAddress;
	while((bit16s)(currentAddress - address) < 0)
	{
		int instructionLength = m_mon.DisassembleOneInstruction(currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
		if (instructionLength<=0)
		{
			nextAddress = currentAddress+1;
		}
		else
		{
			nextAddress = currentAddress + instructionLength;
		}
		if (nextAddress == address)
			return nextAddress;
		if ((bit16s)(nextAddress - address) > 0)
			return currentAddress;

		currentAddress=nextAddress;			
	}
	return currentAddress;
}

void CDisassemblyEditChild::SetTopAddress(bit16 address)
{
	this->m_FirstAddress = address;
}

bit16 CDisassemblyEditChild::GetTopAddress()
{
	return this->m_FirstAddress;
}

bit16 CDisassemblyEditChild::GetBottomAddress()
{
bool isUndoc = false;
	if (m_NumLines <= 1)
		return m_FirstAddress;
	else
		return GetNthAddress(m_FirstAddress, m_NumLines - 1);
}

bit16 CDisassemblyEditChild::GetBottomAddress(int offset)
{
bool isUndoc = false;
	if (m_NumLines+offset <= 1)
		return m_FirstAddress;
	else
		return GetNthAddress(m_FirstAddress, m_NumLines + offset - 1);
}

bit16 CDisassemblyEditChild::GetNthAddress(bit16 startaddress, int linenumber)
{
bool isUndoc = false;
	if (linenumber <= 0)
		return startaddress;
	bit16 currentAddress = startaddress;
	for (int i=0 ; i<linenumber ; i++)
	{
		int instructionLength = m_mon.DisassembleOneInstruction(currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
		currentAddress+=instructionLength;
	}
	
	return currentAddress;
}

bit16 CDisassemblyEditChild::GetNextAddress()
{
bool isUndoc = false;
bit16 currentAddress = m_FirstAddress;
bit16 nextAddress;
	int instructionLength = m_mon.DisassembleOneInstruction(currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
	if (instructionLength<=0)
	{
		nextAddress = currentAddress+1;
	}
	else
	{
		nextAddress = currentAddress + instructionLength;
	}
	return nextAddress;
}

bit16 CDisassemblyEditChild::GetPrevAddress()
{
bool isUndoc = false;
bit16 currentAddress = m_FirstAddress - 32;
bit16 nextAddress;
	while((bit16s)(currentAddress-m_FirstAddress) < 0)
	{
		int instructionLength = m_mon.DisassembleOneInstruction(currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
		if (instructionLength<=0)
		{
			nextAddress = currentAddress+1;
		}
		else
		{
			nextAddress = currentAddress + instructionLength;
		}
		if ((bit16s)(nextAddress - m_FirstAddress) >= 0)
			return currentAddress;

		currentAddress=nextAddress;			
	}
	return currentAddress;
}

void CDisassemblyEditChild::GetMinWindowSize(int &w, int &h)
{
	if (m_MinSizeDone)
	{
		w = m_MinSizeW;
		h = m_MinSizeH;
		return;
	}
	else
	{
		w = WIDTH_LEFTBAR;
		h = GetSystemMetrics(SM_CYVTHUMB) * 2 + GetSystemMetrics(SM_CYVTHUMB);

		if (m_hWnd != NULL && m_hFont != NULL)
		{
			HDC hdc = GetDC(m_hWnd);
			if (hdc)
			{
				int prevMapMode = SetMapMode(hdc, MM_TEXT);
				if (prevMapMode)
				{
					HFONT hFontPrev = (HFONT)SelectObject(hdc, m_hFont);
					if (hFontPrev)
					{
						TEXTMETRIC tm;
						SIZE sz;
						LPCTSTR szSampleProgramCounterPointer = TEXT("I>0");
						if (GetTextMetrics(hdc, &tm))
						{
							LPCTSTR szSampleProgramCounterPointer = TEXT("I>0");
							if (::GetTextExtentExPoint(hdc, szSampleProgramCounterPointer, lstrlen(szSampleProgramCounterPointer), 0, NULL, NULL, &sz))
							{							
								WIDTH_LEFTBAR2 = sz.cx + 1;
							}
							LINE_HEIGHT = tm.tmHeight;
						}
						w += WIDTH_LEFTBAR2 + PADDING_LEFT + PADDING_RIGHT;						
						TCHAR s[]= TEXT("$ABCDxxABxABxABxxLDA $ABCD,X");
						int slen = lstrlen(s);
						SIZE sizeText;
						BOOL br = GetTextExtentExPoint(hdc, s, slen, 0, NULL, NULL, &sizeText);
						if (br)
						{
							w += sizeText.cx;
							h += sizeText.cy * 5;
							m_MinSizeW = w;
							m_MinSizeH = h;							
							m_MinSizeDone = true;
						}

						SelectObject(hdc, hFontPrev);
					}
					SetMapMode(hdc, prevMapMode);
				}
				ReleaseDC(m_hWnd, hdc);
			}
		}
	}
}

CDisassemblyEditChild::AssemblyLineBuffer::AssemblyLineBuffer()
{
	Clear();
}

void CDisassemblyEditChild::AssemblyLineBuffer::Clear()
{
	Address = 0;
	AddressText[0] = 0;
	BytesText[0] = 0;
	MnemonicText[0] = 0;
	IsUnDoc = false;
	IsPC = false;
	IsBreak = false;
	InstructionCycle = 0;
	IsValid = false;
	IsFocused = false;
	WantUpdate = false;
	SetRectEmpty(&MnemonicRect);
}

bool CDisassemblyEditChild::AssemblyLineBuffer::IsEqual(AssemblyLineBuffer& other)
{
	if (IsValid != other.IsValid)
		return false;	
	if (Address != other.Address)
		return false;
	if (IsUnDoc != other.IsUnDoc)
		return false;
	if (IsPC != other.IsPC)
		return false;
	if (IsBreak != other.IsBreak)
		return false;
	if (IsFocused != other.IsFocused)
		return false;
	if (WantUpdate != other.WantUpdate)
		return false;
	if (InstructionCycle != other.InstructionCycle)
		return false;

	if (_tcscmp(AddressText, other.AddressText)!=0)
		return false;

	if (_tcscmp(BytesText, other.BytesText)!=0)
		return false;

	if (_tcscmp(MnemonicText, other.MnemonicText)!=0)
		return false;

	return true;
}

void CDisassemblyEditChild::AssemblyLineBuffer::WriteDisassemblyString(TCHAR *pszBuffer, int cchBuffer)
{
////TCHAR szSpace[]=TEXT("         ");
//	if (pszBuffer!=NULL || cchBuffer <=0)
//		return;
//	TCHAR *p = pszBuffer;
//	p[0] = 0;
//	int len;
//	int sizeRemaining = cchBuffer;
//
//	if (sizeRemaining < 1)
//		return;
//	_tcsncpy_s(p, sizeRemaining, this->AddressText, _TRUNCATE);
//	len = _tcslen(p);
//	sizeRemaining = sizeRemaining - len;
//	p+=len;
//
//	if (sizeRemaining < 1)
//		return;
//	_tcsncpy_s(p, sizeRemaining, this->BytesText, _TRUNCATE);
//	len = _tcslen(p);
//	sizeRemaining = sizeRemaining - len;
//	p+=len;
//
//	if (sizeRemaining < 1)
//		return;
//	int lenSpace = 
}
