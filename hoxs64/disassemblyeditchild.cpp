#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "CDPI.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "C64.h"
#include "dchelper.h"
#include "assembler.h"
#include "disassemblyeditchild.h"
#include "resource.h"

#define WIDTH_LEFTBAR_96 (16)
#define PADDING_LEFT_96 (4)
#define PADDING_RIGHT_96 (4)
#define PADDING_TOP_96 (4)
#define PADDING_BOTTOM_96 (4)
#define MARGIN_TOP_96 (5)
#define MAX_EDIT_CHARS (256)

TCHAR CDisassemblyEditChild::ClassName[] = TEXT("Hoxs64DisassemblyEditChild");



CDisassemblyEditChild::CDisassemblyEditChild(int cpuid, C64 *c64, IMonitorCommand *pMonitorCommand) 
	: DefaultCpu(cpuid, c64)
{
	WIDTH_LEFTBAR2 = 0;
	LINE_HEIGHT = 0;
	m_hFont = NULL;
	m_pParent = NULL;
	m_FirstAddress = 0;
	m_NumLines = 0;
	m_MinSizeDone = false;
	m_pFrontTextBuffer = NULL;
	m_pBackTextBuffer = NULL;
	m_hBmpBreak = NULL;
	m_bHasLastDrawText = false;
	m_bIsFocusedAddress = false;
	m_iFocusedAddress = 0;
	m_bMouseDownOnFocusedAddress = false;
	m_hWndEditText = NULL;
	m_wpOrigEditProc = NULL;
	m_CurrentEditLineBuffer = NULL;
	ZeroMemory(&m_rcLastDrawText, sizeof(m_rcLastDrawText));

	m_pMonitorCommand = pMonitorCommand;
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

HRESULT CDisassemblyEditChild::Init(CVirWindow *parent, HFONT hFont)
{
HRESULT hr;
HSink hs;
	Cleanup();

	m_pParent = parent;
	m_hFont = hFont;

	hr = AllocTextBuffer();
	m_FirstAddress = GetNearestTopAddress(0);
	m_NumLines = 0;
	m_CurrentEditLineBuffer = NULL;

	hs = m_pMonitorCommand->EsBreakpointC64ExecuteChanged.Advise(this);
	if (!hs)
		return E_FAIL;
	hs = m_pMonitorCommand->EsBreakpointDiskExecuteChanged.Advise(this);
	if (!hs)
		return E_FAIL;
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


HWND CDisassemblyEditChild::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	if (m_hBmpBreak == NULL)
	{
		m_hBmpBreak = (HBITMAP) LoadImage(hInstance, MAKEINTRESOURCE(IDB_BREAK), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
		if (m_hBmpBreak==NULL)
			return NULL;
	}
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
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
Assembler as;
TCHAR szText[MAX_EDIT_CHARS+1];
int i;
HRESULT hr;
bit8 acode[256];
bit16 address;

	if (!IsEditing())
		return S_OK;
	if (GetDlgItemText(m_hWnd, GetDlgCtrlID(m_hWndEditText), szText, _countof(szText)) != 0)
	{
		if (m_CurrentEditLineBuffer != NULL)
		{
			address = m_CurrentEditLineBuffer->Address;
			hr = as.AssembleText(address, szText, acode, _countof(acode), &i);
			if (SUCCEEDED(hr))
			{
				for (int j = 0; j<i; j++)
				{
					this->GetCpu()->MonWriteByte(address+j, acode[j], -1);
				}
				this->UpdateBuffer(DBGSYM::None, 0);
				this->InvalidateRectChanges();
				::UpdateWindow(m_hWnd);
				return S_OK;
			}
		}
	}	
	return E_FAIL;
}

HRESULT CDisassemblyEditChild::UpdateMetrics()
{
TEXTMETRIC tm;
SIZE sz;
	HDC hdc = GetDC(m_hWnd);
	if (!hdc)
		return E_FAIL;
	if (!GetTextMetrics(hdc, &tm))
		return E_FAIL;
	LPCTSTR szSampleProgramCounterPointer = TEXT("I>0");
	if (!::GetTextExtentExPoint(hdc, szSampleProgramCounterPointer, lstrlen(szSampleProgramCounterPointer), 0, NULL, NULL, &sz))
	{
		return E_FAIL;
	}
	WIDTH_LEFTBAR2 = sz.cx + m_dpi.ScaleX(1);
	LINE_HEIGHT =  tm.tmHeight;

	int min_w = m_dpi.ScaleX(WIDTH_LEFTBAR_96);
	int min_h = GetSystemMetrics(SM_CYVTHUMB) * 2 + GetSystemMetrics(SM_CYVTHUMB);

	min_w += WIDTH_LEFTBAR2 + m_dpi.ScaleX(PADDING_LEFT_96) + m_dpi.ScaleX(PADDING_RIGHT_96);			

	TCHAR s[]= TEXT("$ABCDxxABxABxABxxLDA $ABCD,X");
	int slen = lstrlen(s);
	SIZE sizeText;
	BOOL br = GetTextExtentExPoint(hdc, s, slen, 0, NULL, NULL, &sizeText);
	if (!br)
		return E_FAIL;
	min_w += sizeText.cx;
	min_h += sizeText.cy * 5;
	m_MinSizeW = min_w;
	m_MinSizeH = min_h;							
	m_MinSizeDone = true;


	LPCTSTR szSampleAddress = TEXT("$ABCDxx");
	LPCTSTR szSampleBytes = TEXT("ABxABxABxx");
	xcol_Address = m_dpi.ScaleX(WIDTH_LEFTBAR_96) + WIDTH_LEFTBAR2 + m_dpi.ScaleX(PADDING_LEFT_96);
	if (!::GetTextExtentExPoint(hdc, szSampleAddress,lstrlen(szSampleAddress),0,NULL,NULL,&sz))
		return E_FAIL;
	xcol_Bytes = xcol_Address + sz.cx;
	if (!::GetTextExtentExPoint(hdc, szSampleBytes,lstrlen(szSampleBytes),0,NULL,NULL,&sz))
		return E_FAIL;
	xcol_Mnemonic = xcol_Bytes + sz.cx;

	return S_OK;
}

HRESULT CDisassemblyEditChild::OnCreate(HWND hWnd)
{
HDC hdc = GetDC(m_hWnd);
DcHelper dch(hdc);
HRESULT hr;
	dch.UseMapMode(MM_TEXT);
	dch.UseFont(m_hFont);
	//prevent dc restore
	dch.m_hdc = NULL;
	m_bIsFocused = false;
	hr = UpdateMetrics();
	if (FAILED(hr))
		return hr;
	m_hWndEditText = CreateAsmEdit(hWnd);
	if (m_hWndEditText == NULL)
		return E_FAIL;
	return S_OK;
}

void CDisassemblyEditChild::OnDestroy(HWND hWnd)
{
	if (m_wpOrigEditProc!=NULL && m_hWndEditText!=NULL)
	{
		SubclassChildWindow(m_hWndEditText, m_wpOrigEditProc);
		m_wpOrigEditProc= NULL;
	}
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
	case WM_DESTROY:
		OnDestroy(hWnd);
		break;
	case WM_PAINT:
		br = GetUpdateRect(hWnd, NULL, FALSE);
		if (br)
		{
			hdc = BeginPaint (hWnd, &ps);
			if (hdc != NULL)
			{
				DrawDisplay(hWnd, hdc);
				EndPaint (hWnd, &ps);
			}
		}
		return 0;
	case WM_SIZE:
		if (wParam==SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
			return 0;
		if (wParam==SIZE_MINIMIZED)
			return 0;
		UpdateDisplay(DBGSYM::None, 0);
		return 0;
	case WM_KEYDOWN:
		if (OnKeyDown(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_CHAR:
		if (OnChar(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDOWN:
		if (OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONUP:
		if (OnLButtonUp(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		break;
	case WM_NOTIFY:
		OnNotify(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_KILLFOCUS:
		m_bIsFocused = false;
		InvalidateFocus();
		UpdateWindow(m_hWnd);
		break;
	case WM_SETFOCUS:
		m_bIsFocused = true;
		InvalidateFocus();
		UpdateWindow(m_hWnd);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CDisassemblyEditChild::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
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
				hr = SaveAsmEditing();
				if (SUCCEEDED(hr))
				{
					this->HideEditMnemonic();
					MoveNextLine();
					InvalidateRectChanges();
					if (m_hWnd != NULL)
						SetFocus(m_hWnd);
					UpdateWindow(m_hWnd);
					return 0;
				}
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
		num = (rc.bottom - rc.top - m_dpi.ScaleY(MARGIN_TOP_96) - m_dpi.ScaleY(PADDING_TOP_96)) / lineHeight;
	if (num <= 0)
		num = 1;
	else
		num++;
	if (num > MAX_BUFFER_HEIGHT)
		num = MAX_BUFFER_HEIGHT;
	return num;
}

void CDisassemblyEditChild::UpdateDisplay(DBGSYM::DisassemblyPCUpdateMode pcmode, bit16 address)
{
	UpdateBuffer(pcmode, address);
	InvalidateRectChanges();
	UpdateWindow(m_hWnd);
}

void CDisassemblyEditChild::GetRect_Bar(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);		
	prc->right = prc->left + m_dpi.ScaleX(WIDTH_LEFTBAR_96);
}

void CDisassemblyEditChild::GetRect_Status(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);
	prc->left += m_dpi.ScaleX(WIDTH_LEFTBAR_96);
	prc->right = prc->left + WIDTH_LEFTBAR2;
}

void CDisassemblyEditChild::GetRect_Edit(const RECT& rcClient, LPRECT prc)
{
	CopyRect(prc, &rcClient);
	prc->left = m_dpi.ScaleX(WIDTH_LEFTBAR_96) + WIDTH_LEFTBAR2;
}

void CDisassemblyEditChild::InvalidateFocus()
{
	RECT rcLine;
	RECT rcClient;
	RECT rcChange;
	if (GetClientRect(m_hWnd, &rcClient))
	{
		int y = m_dpi.ScaleY(MARGIN_TOP_96) + m_dpi.ScaleY(PADDING_TOP_96);
		int x = rcClient.left;
		for (int i=0; i < m_NumLines; i++, y+=LINE_HEIGHT)
		{
			bool bIsChanged = false;
			AssemblyLineBuffer &front = m_pFrontTextBuffer[i];
			if (front.IsFocused)
				bIsChanged = true;
			if (bIsChanged)
			{				
				SetRect(&rcLine, x, y, rcClient.right, y+LINE_HEIGHT);
				if (IntersectRect(&rcChange, &rcClient, &rcLine))
					InvalidateRect(m_hWnd, &rcChange, TRUE);
			}
		}
	}
}

void CDisassemblyEditChild::InvalidateRectChanges()
{
	RECT rcLine;
	RECT rcClient;
	RECT rcChange;
	if (m_bHasLastDrawText)
	{
		//Make sure we overwite previous text messages.
		InvalidateRect(m_hWnd, &m_rcLastDrawText, TRUE);
		m_bHasLastDrawText = false;
	}
	if (GetClientRect(m_hWnd, &rcClient))
	{
		int y = m_dpi.ScaleY(MARGIN_TOP_96) + m_dpi.ScaleY(PADDING_TOP_96);
		int x = rcClient.left;
		for (int i=0; i < m_NumLines; i++, y+=LINE_HEIGHT)
		{
			bool bIsChanged = false;
			AssemblyLineBuffer &front = m_pFrontTextBuffer[i];
			AssemblyLineBuffer &back = m_pBackTextBuffer[i];
			if (!front.IsEqual(back) || !front.IsValid)
				bIsChanged = true;
			if ((m_bIsFocused && !front.IsFocused) || (!m_bIsFocused && front.IsFocused))
				bIsChanged = true;
			if (bIsChanged)
			{				
				SetRect(&rcLine, x, y, rcClient.right, y+LINE_HEIGHT);
				if (IntersectRect(&rcChange, &rcClient, &rcLine))
					InvalidateRect(m_hWnd, &rcChange, TRUE);
			}
		}
		if (y < rcClient.bottom)
		{
			//Make sure then very bottom of the window is updated.
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

	if (m_pMonitorCommand->IsRunning())
		return false;
	bool bWasWindowFocused = (hWnd == GetFocus());
	if (hWnd)
	{
		::SetFocus(hWnd);
	}
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

			if (this->GetCpuId() == CPUID_MAIN)
			{
				if (this->m_pMonitorCommand->IsBreakpointC64Execute(address))
					this->m_pMonitorCommand->DeleteBreakpointC64Execute(address);
				else
					this->m_pMonitorCommand->SetBreakpointC64Execute(MT_DEFAULT, address, 1);				
			}
			else if (this->GetCpuId() == CPUID_DISK)
			{
				if (this->m_pMonitorCommand->IsBreakpointDiskExecute(address))
					this->m_pMonitorCommand->DeleteBreakpointDiskExecute(address);
				else
					this->m_pMonitorCommand->SetBreakpointDiskExecute(address, 1);
			}
		}
	}
	else if (PtInRect(&rcEdit, pt))
	{
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
					if (prevAddress == address && bWasWindowFocused)
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

	return true;
}

bool CDisassemblyEditChild::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AssemblyLineBuffer *pAlb;
	RECT rcClient;
	RECT rcEdit;

	if (m_pMonitorCommand->IsRunning())
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

void CDisassemblyEditChild::OnEditFocusedMnemonic()
{
	for (int i=0 ; i < m_NumLines ; i++)
	{
		AssemblyLineBuffer *pAlb = &this->m_pFrontTextBuffer[i];
		if (pAlb->IsFocused)
		{
			ShowEditMnemonic(pAlb);
		}
	}
}

void CDisassemblyEditChild::ShowEditMnemonic(AssemblyLineBuffer *pAlb)
{
RECT rcEdit;
	if (m_hWndEditText==NULL)
		return;
	CopyRect(&rcEdit, &pAlb->MnemonicRect);
	m_CurrentEditLineBuffer = pAlb;
	InflateRect(&rcEdit, 2 * ::GetSystemMetrics(SM_CYBORDER), 2 * ::GetSystemMetrics(SM_CXBORDER));
	rcEdit.right-= 2 *::GetSystemMetrics(SM_CXBORDER);
	SetDlgItemText(m_hWnd, GetDlgCtrlID(m_hWndEditText), pAlb->MnemonicText);
	SetWindowPos(m_hWndEditText, HWND_TOP, rcEdit.left, rcEdit.top, rcEdit.right - rcEdit.left, rcEdit.bottom - rcEdit.top, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
	SendMessage(m_hWndEditText, EM_SETLIMITTEXT, MAX_EDIT_CHARS, 0); 
	if (pAlb->GetIsReadOnly())
	{
		SendMessage(m_hWndEditText, EM_SETREADONLY, TRUE, 0);
	}
	else
	{
		SendMessage(m_hWndEditText, EM_SETREADONLY, FALSE, 0);
	}
	SendMessage(m_hWndEditText, EM_SETSEL, 0, -1);
	SetFocus(m_hWndEditText);
}

void CDisassemblyEditChild::HideEditMnemonic()
{
	m_CurrentEditLineBuffer = NULL;
	if (m_hWndEditText==NULL)
		return;
	SetWindowPos(m_hWndEditText, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE);
}

void CDisassemblyEditChild::MoveNextLine()
{
AssemblyLineBuffer *pAlb;

	for (int i = 0; i < m_NumLines; i++)
	{
		pAlb = &this->m_pFrontTextBuffer[i];
		if (pAlb->IsFocused)
		{
			bit16 nextAddress = (pAlb->Address + pAlb->InstructionSize) & 0xffff;
			UpdateBuffer(m_pFrontTextBuffer, m_NumLines, nextAddress);
			SetFocusedAddress(nextAddress);
			break;
		}
	}
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
	if (wParam == VK_F2)
	{
		this->OnEditFocusedMnemonic();
	}
	else
	{
		SendMessage(::GetParent(hWnd), uMsg, wParam, lParam);
	}
	return true;
}

bool CDisassemblyEditChild::OnChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//SendMessage(::GetParent(hWnd), uMsg, wParam, lParam);
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
	this->GetCpu()->GetCpuState(cpustate);
	this->SetTopAddress(cpustate.PC_CurrentOpcode);
}

int CDisassemblyEditChild::GetLineFromYPos(int y)
{
	if (LINE_HEIGHT==0)
		return -1;
	int k = (y - m_dpi.ScaleY(MARGIN_TOP_96) - m_dpi.ScaleY(PADDING_TOP_96)) / LINE_HEIGHT;
	return k;
}

void CDisassemblyEditChild::DrawDisplay2(HWND hWnd, HDC hdc)
{
RECT rcClient;
RECT rcBarBreak;
RECT rcBarStatus;
RECT rcEdit;
RECT rcEditRow;
BOOL br;
HBRUSH bshBarBreak;
HBRUSH bshBarStatus;
HBRUSH bshEdit;
TEXTMETRIC tm;

	CPUState cpustate;
	this->GetCpu()->GetCpuState(cpustate);

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

	bool bUnavailable = m_pMonitorCommand->IsRunning();

	br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		if (tm.tmHeight > 0 && rcClient.bottom > rcClient.top && m_pFrontTextBuffer != NULL && m_pBackTextBuffer != NULL)
		{
			int x_status = m_dpi.ScaleX(WIDTH_LEFTBAR_96);
			int x,y;
			int slen;
			y = m_dpi.ScaleY(MARGIN_TOP_96) + m_dpi.ScaleY(PADDING_TOP_96);
			SIZE sizeText;
			BOOL brTextExtent;

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
							int iWidthImageBreak = m_dpi.ScaleX(bmpBreak.bmWidth);
							int iHeightImageBreak = m_dpi.ScaleY(bmpBreak.bmHeight);
							int iTopPosImageBreak = (LINE_HEIGHT - iHeightImageBreak) / 2;
							int iLeftPosImageBreak = (WIDTH_LEFTBAR2 - iWidthImageBreak) / 2;
							for (int i = 0; i < m_NumLines; i++, y += LINE_HEIGHT)
							{
								AssemblyLineBuffer albFront = m_pFrontTextBuffer[i];
								int sys_back_colour;
								if (albFront.GetIsReadOnly())
								{
									sys_back_colour = COLOR_3DLIGHT;
								}
								else
								{
									sys_back_colour = COLOR_WINDOW;
								}

								SetRect(&rcEditRow, xcol_Address, y, rcClient.right, y + LINE_HEIGHT);
								::SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
								::SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
								if (this->GetCpu()->IsBreakPoint(albFront.Address))
								{
									SelectObject(hdc, bshBarBreak);
									DWORD dwROP = MERGECOPY;
									//BitBlt(hdc, 0, y, bmpBreak.bmWidth, bmpBreak.bmHeight, hMemDC, 0, 0, dwROP);
									StretchBlt(hdc, 0, y, iWidthImageBreak, iHeightImageBreak, hMemDC, 0, 0, bmpBreak.bmWidth, bmpBreak.bmHeight, dwROP);
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

								::FillRect(hdc, &rcEditRow, ::GetSysColorBrush(sys_back_colour));
								::SetBkColor(hdc, GetSysColor(sys_back_colour));
								//Draw the address text
								x = xcol_Address;
								slen = (int)_tcsnlen(albFront.AddressText, _countof(albFront.AddressText));
								if (slen > 0)
								{
									brTextExtent = GetTextExtentExPoint(hdc, albFront.AddressText, slen, 0, NULL, NULL, &sizeText);
									SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.AddressText, slen, NULL);
								}

								//Draw the data bytes text
								x = xcol_Bytes;
								slen = (int)_tcsnlen(albFront.BytesText, _countof(albFront.BytesText));
								if (slen > 0)
								{
									brTextExtent = GetTextExtentExPoint(hdc, albFront.BytesText, slen, 0, NULL, NULL, &sizeText);
									SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.BytesText, slen, NULL);
								}

								::SetBkColor(hdc, GetSysColor(sys_back_colour));
								//Draw the mnemonic text
								x = xcol_Mnemonic;
								SetRect(&albFront.MnemonicRect, xcol_Mnemonic, y, this->m_MinSizeW, y + LINE_HEIGHT);
								slen = (int)_tcsnlen(albFront.MnemonicText, _countof(albFront.MnemonicText));
								if (slen > 0)
								{
									if (albFront.IsUnDoc)
										SetTextColor(hdc, RGB(2, 134, 172));
									else
										SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
									brTextExtent = GetTextExtentExPoint(hdc, albFront.MnemonicText, slen, 0, NULL, NULL, &sizeText);
									ExtTextOut(hdc, x, y, ETO_NUMERICSLATIN | ETO_OPAQUE, NULL, albFront.MnemonicText, slen, NULL);
								}
								::SetBkColor(hdc, GetSysColor(sys_back_colour));
								if (albFront.IsFocused && m_bIsFocused)
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
				::SetRect(&rcText, m_dpi.ScaleX(WIDTH_LEFTBAR_96) + WIDTH_LEFTBAR2 + m_dpi.ScaleX(PADDING_LEFT_96), m_dpi.ScaleY(MARGIN_TOP_96) + m_dpi.ScaleY(PADDING_TOP_96), rcClient.right - m_dpi.ScaleX(PADDING_RIGHT_96), rcClient.bottom);
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

void CDisassemblyEditChild::UpdateBuffer(DBGSYM::DisassemblyPCUpdateMode pcmode, bit16 address)
{
int iEnsureAddress = -1;
	if (pcmode == DBGSYM::EnsurePCVisible)
	{
		CPUState cpustate;
		this->GetCpu()->GetCpuState(cpustate);
		iEnsureAddress = cpustate.PC_CurrentOpcode;
	}
	else if (pcmode == DBGSYM::EnsureAddressVisible)
	{
		iEnsureAddress = address;
	}
	else if (pcmode == DBGSYM::None)
	{
		iEnsureAddress = -1;
	}
	UpdateBuffer(m_pFrontTextBuffer, m_NumLines, iEnsureAddress);
}

void CDisassemblyEditChild::UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, int numLines, int iEnsureAddress)
{
RECT rc;
	ZeroMemory(&rc, sizeof(rc));
	if (GetClientRect(m_hWnd, &rc))
	{
		m_NumLines = GetNumberOfLines(rc, LINE_HEIGHT);
		int pcline=-1;
		bit16 address = this->m_FirstAddress;
		UpdateBuffer(assemblyLineBuffer, address, 0, numLines);
		if (iEnsureAddress >=0 && iEnsureAddress <= 0xffff)
		{
			for (int i = 0; i < m_NumLines; i++)
			{
				if (iEnsureAddress == m_pFrontTextBuffer[i].Address)
				{
					pcline = i;
					break;
				}
			}
			if (pcline < 0 || pcline > m_NumLines - 5)
			{
				address = iEnsureAddress;
				SetTopAddress(address);
				UpdateBuffer(assemblyLineBuffer, address, 0, numLines);
			}
		}
	}
}

void CDisassemblyEditChild::UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, bit16 address, int startLine, int numLines)
{
bit16 currentAddress = address;
CPUState cpustate;
	
	this->GetCpu()->GetCpuState(cpustate);

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
		}

		if (currentAddress == this->m_iFocusedAddress)
		{
			if (this->m_bIsFocusedAddress)
			{
				buffer.IsFocused = true;
			}
		}

		if (this->GetCpu()->IsBreakPoint(currentAddress))
			buffer.IsBreak = true;
		else
			buffer.IsBreak = false;

		int instructionSize = c64->mon.DisassembleOneInstruction(this->GetCpu(), currentAddress, -1, buffer.AddressText, _countof(buffer.AddressText), buffer.BytesText, _countof(buffer.BytesText), buffer.MnemonicText, _countof(buffer.MnemonicText), buffer.IsUnDoc);
		buffer.AddressReadAccessType = this->GetCpu()->GetCpuMmuReadMemoryType(currentAddress, -1);
		buffer.Address = currentAddress;
		buffer.InstructionSize = (bit8)instructionSize;
		buffer.IsValid = true;
		m_pFrontTextBuffer[currentLine] = buffer;
		currentAddress += instructionSize;
	}

}

bit16 CDisassemblyEditChild::GetNearestTopAddress(bit16 address)
{
bool isUndoc = false;
bit16 currentAddress = address - 32;
bit16 nextAddress;
	while((bit16s)(currentAddress - address) < 0)
	{
		int instructionLength = c64->mon.DisassembleOneInstruction(this->GetCpu(), currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
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
		int instructionLength = c64->mon.DisassembleOneInstruction(this->GetCpu(), currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
		currentAddress+=instructionLength;
	}
	
	return currentAddress;
}

bit16 CDisassemblyEditChild::GetNextAddress()
{
bool isUndoc = false;
bit16 currentAddress = m_FirstAddress;
bit16 nextAddress;
	int instructionLength = c64->mon.DisassembleOneInstruction(this->GetCpu(), currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
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
		int instructionLength = c64->mon.DisassembleOneInstruction(this->GetCpu(), currentAddress, -1, NULL, 0, NULL, 0, NULL, 0, isUndoc);
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
	w = m_MinSizeW;
	h = m_MinSizeH;
}

void CDisassemblyEditChild::OnBreakpointC64ExecuteChanged(void *sender, BreakpointC64ExecuteChangedEventArgs& e)
{
	UpdateDisplay(DBGSYM::None, 0);
}

void CDisassemblyEditChild::OnBreakpointDiskExecuteChanged(void *sender, BreakpointDiskExecuteChangedEventArgs& e)
{
	UpdateDisplay(DBGSYM::None, 0);
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

bool CDisassemblyEditChild::AssemblyLineBuffer::GetIsReadOnly()
{
	return (AddressReadAccessType == MT_KERNAL || AddressReadAccessType == MT_BASIC || AddressReadAccessType == MT_CHARGEN || AddressReadAccessType == MT_NOTCONNECTED);
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