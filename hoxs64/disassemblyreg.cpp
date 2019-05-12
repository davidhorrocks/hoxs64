#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "IC64.h"
#include "edln.h"
#include "disassemblyreg.h"
#include "resource.h"

TCHAR CDisassemblyReg::ClassName[] = TEXT("Hoxs64DisassemblyReg");

CDisassemblyReg::CDisassemblyReg(int cpuid, IC64 *c64, IAppCommand *pAppCommand, HFONT hFont) 
	: DefaultCpu(cpuid, c64)
{
	m_hFont = NULL;
	m_MinSizeDone = false;
	m_hdc = NULL;
	m_pAppCommand = pAppCommand;
	m_hFont = hFont;
}

CDisassemblyReg::~CDisassemblyReg()
{
	Cleanup();
}

void CDisassemblyReg::Cleanup()
{
	UnadviseEvents();
}

HRESULT CDisassemblyReg::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_OWNDC;//; CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(CDisassemblyReg *);
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
}

HWND CDisassemblyReg::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

HRESULT CDisassemblyReg::UpdateMetrics()
{
	m_MinSizeDone = false;
	DBGSYM::MonitorOption::Radix radix = this->c64->GetMon()->Get_Radix();
	return this->UpdateMetrics(radix);
}

HRESULT CDisassemblyReg::UpdateMetrics(DBGSYM::MonitorOption::Radix radix)
{
HRESULT hr = E_FAIL;
	BOOL br;
	int w = PADDING_LEFT + PADDING_RIGHT + GetSystemMetrics(SM_CXFRAME) * 2;
	int h = MARGIN_TOP + PADDING_TOP + PADDING_BOTTOM;
	int cpuid = this->GetCpuId();
	if (m_hWnd != NULL && m_hFont != NULL)
	{
		int prevMapMode = SetMapMode(this->m_hdc, MM_TEXT);
		if (prevMapMode)
		{
			HFONT hFontPrev = (HFONT)SelectObject(this->m_hdc, m_hFont);
			if (hFontPrev)
			{
				TEXTMETRIC tm;
				br = GetTextMetrics(this->m_hdc, &tm);
				if (br)
				{							

					if (cpuid==CPUID_MAIN)
					{
						TCHAR *s;
						//if (radix == DBGSYM::MonitorOption::Hex)
						//{
						//	s = TEXT("ABCD AB AB AB NV-BDIZC AB AB AB LINE CYC");
						//}
						//else
						{
							s = TEXT("65535 255 255 255 NV-BDIZC 255 255 255 LINExCYC");
						}

						int slen = lstrlen(s);
						SIZE sizeText;
						BOOL br = GetTextExtentExPoint(this->m_hdc, s, slen, 0, NULL, NULL, &sizeText);
						if (br)
						{
							w += sizeText.cx;
						}
					}
					else
					{
						TCHAR *s;
						//if (radix == DBGSYM::MonitorOption::Hex)
						//{
						//	s = TEXT("ABCD AB AB AB NV-BDIZC 42.5");
						//}
						//else
						{
							s = TEXT("65535 255 255 255 NV-BDIZC 42.5");
						}

						int slen = lstrlen(s);
						SIZE sizeText;
						BOOL br = GetTextExtentExPoint(this->m_hdc, s, slen, 0, NULL, NULL, &sizeText);
						if (br)
						{
							w += sizeText.cx;
						}
					}

					h += tm.tmHeight * 2;
					m_MinSizeW = w;
					m_MinSizeH = h;
					m_MinSizeDone = true;
					this->UpdateLayout();
					hr = S_OK;
				}

				SelectObject(this->m_hdc, hFontPrev);
			}

			SetMapMode(this->m_hdc, prevMapMode);
		}
	}

	return hr;
}

void CDisassemblyReg::GetMinWindowSize(int &w, int &h)
{
	if (!m_MinSizeDone)
	{
		this->UpdateMetrics();
	}

	w = m_MinSizeW;
	h = m_MinSizeH;
}

void CDisassemblyReg::SetRadix(DBGSYM::MonitorOption::Radix radix)
{
	this->radix = radix;
	this->m_MinSizeDone = false;
}

HRESULT CDisassemblyReg::UpdateLayout()
{
	bool isHex = (this->radix == DBGSYM::MonitorOption::Hex);
	this->PC.SetStyle(EdLn::Address, true, true, 0, isHex);
	this->A.SetStyle(EdLn::Byte, true, true, 0, isHex);
	this->X.SetStyle(EdLn::Byte, true, true, 0, isHex);
	this->Y.SetStyle(EdLn::Byte, true, true, 0, isHex);
	this->SR.SetStyle(EdLn::CpuFlags, true, true, 0, isHex);
	this->SP.SetStyle(EdLn::Byte, true, true, 0, isHex);
	this->Ddr.SetStyle(EdLn::Byte, this->cpuid == CPUID_MAIN, true, 0, isHex);
	this->Data.SetStyle(EdLn::Byte, this->cpuid == CPUID_MAIN, true, 0, isHex);
	this->VicLine.SetStyle(EdLn::Number, this->cpuid == CPUID_MAIN, false, 3, isHex);
	this->VicCycle.SetStyle(EdLn::Number, this->cpuid == CPUID_MAIN, false, 2, isHex);
	this->DiskTrack.SetStyle(EdLn::DiskTrack, this->cpuid == CPUID_DISK, false, 0, isHex);
	this->ArrangeControls();
	return S_OK;
}

HRESULT CDisassemblyReg::ArrangeControls()
{
int w=0;
int h=0;
TEXTMETRIC tm;	
RECT rcAll;
HRESULT hr;

	SetRectEmpty(&rcAll);
	BOOL br = GetTextMetrics(this->m_hdc, &tm);
	if (!br)
	{
		return E_FAIL;
	}

	int x = this->xpos;
	int y = this->ypos;

	// PC
	this->PC.SetPos(x, y);
	hr = this->PC.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// A
	x = rcAll.right + 2*tm.tmAveCharWidth;
	this->A.SetPos(x, y);
	hr = this->A.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// X
	x = rcAll.right + tm.tmAveCharWidth;
	this->X.SetPos(x, y);
	hr = this->X.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// Y
	x = rcAll.right + tm.tmAveCharWidth;
	this->Y.SetPos(x, y);
	hr = this->Y.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// SR
	x = rcAll.right + tm.tmAveCharWidth;
	this->SR.SetPos(x, y);
	hr = this->SR.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// SP
	x = rcAll.right + tm.tmAveCharWidth;
	this->SP.SetPos(x, y);
	hr = this->SP.GetRects(m_hdc, NULL, NULL, &rcAll);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	if (cpuid == CPUID_MAIN)
	{
		// DDR
		x = rcAll.right + tm.tmAveCharWidth;
		this->Ddr.SetPos(x, y);
		hr = this->Ddr.GetRects(m_hdc, NULL, NULL, &rcAll);
		if (FAILED(hr))
		{
			return E_FAIL;
		}

		// Data
		x = rcAll.right + tm.tmAveCharWidth;
		this->Data.SetPos(x, y);
		hr = this->Data.GetRects(m_hdc, NULL, NULL, &rcAll);
		if (FAILED(hr))
		{
			return E_FAIL;
		}

		// VIC Line
		x = rcAll.right + tm.tmAveCharWidth;
		this->VicLine.SetPos(x, y);
		hr = this->VicLine.GetRects(m_hdc, NULL, NULL, &rcAll);
		if (FAILED(hr))
		{
			return E_FAIL;
		}

		// VIC Cycle
		x = rcAll.right + tm.tmAveCharWidth;
		this->VicCycle.SetPos(x, y);
		hr = this->VicCycle.GetRects(m_hdc, NULL, NULL, &rcAll);
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}
	else if (cpuid == CPUID_DISK)
	{
		// Disk Track
		x = rcAll.right + tm.tmAveCharWidth;
		this->DiskTrack.SetPos(x, y);
		hr = this->DiskTrack.GetRects(m_hdc, NULL, NULL, &rcAll);
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}

	for(unsigned int i = 0; i < this->Controls.Count(); i++)
	{
		EdLn* p = this->Controls[i];
		hr = p->CreateDefaultHitRegion(m_hdc);
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CDisassemblyReg::OnCreate(HWND hWnd)
{
HRESULT hr;
	m_hdc = GetDC(hWnd);
	if (m_hdc == 0)
	{
		return E_FAIL;
	}

	if (SetMapMode(m_hdc, MM_TEXT) == 0)
	{
		return E_FAIL;
	}

	if (SelectObject(m_hdc, m_hFont) == NULL)
	{
		return E_FAIL;
	}

	int x = PADDING_LEFT;
	int y = MARGIN_TOP + PADDING_TOP;
	DBGSYM::MonitorOption::Radix radix = this->c64->GetMon()->Get_Radix();
	int cpuid = this->GetCpu()->GetCpuId();
	hr = this->Init(x, y, cpuid, radix);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = AdviseEvents();
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

bool CDisassemblyReg::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pAppCommand->IsRunning())
	{
		return true;
	}

	this->ProcessLButtonDown(wParam, lParam);
	if (hWnd != ::GetFocus())
	{
		::SetFocus(hWnd);
	}

	return true;
}

bool CDisassemblyReg::OnChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pAppCommand->IsRunning())
	{
		return true;
	}

	return this->ProcessChar(wParam, lParam);
}

bool CDisassemblyReg::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pAppCommand->IsRunning())
	{
		return true;
	}

	EdLn *p = this->GetSelectedControl();
	if (p != NULL)
	{
		return this->ProcessKeyDown(wParam, lParam);
	}
	else if (wParam == VK_TAB)
	{
		p = this->GetTabFirstControl();
		if (p != NULL)
		{
			this->StartEditing(p, 0);
		}

		return true;
	}
	else
	{
		SendMessage(GetParent(m_hWnd), uMsg, wParam, lParam);
		return true;
	}
}

bool CDisassemblyReg::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == m_hWnd)
	{
		SendMessage(::GetParent(hWnd), WM_COMMAND, wParam, lParam);
		return true;
	}

	return false;
}

void CDisassemblyReg::OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == m_hWnd)
	{
		SendMessage(::GetParent(hWnd), WM_COMMAND, wParam, lParam);
	}
}

LRESULT CDisassemblyReg::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		{
			return -1;
		}

		return 0;
	case WM_PAINT:
		br = GetUpdateRect(hWnd, NULL, FALSE);
		if (!br)
		{
			break;
		}

		hdc = BeginPaint (hWnd, &ps);
		if (hdc != NULL)
		{
			DrawDisplay(hWnd, hdc);
		}

		EndPaint (hWnd, &ps);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		if (wParam == SIZE_MINIMIZED)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		UpdateDisplay();
		return 0;
	case WM_LBUTTONDOWN:
		if (!OnLButtonDown(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	case WM_CHAR:
		if (!OnChar(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	case WM_KEYDOWN:
		if (!OnKeyDown(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	case WM_SETFOCUS:
		CreateCaret(hWnd, NULL, this->TextMetric.tmAveCharWidth, this->TextMetric.tmHeight);
		this->StartCaretCount();
		this->UpdateCaret();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KILLFOCUS:
		::HideCaret(hWnd);
		::DestroyCaret();
		this->ClearCaretCount();
		this->SaveEditing();
		this->CancelEditing();
		this->UpdateDisplay();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
		else
		{
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		}
	case WM_HSCROLL:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_VSCROLL:
		OnVScroll(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void CDisassemblyReg::DrawDisplay(HWND hWnd, HDC hdc)
{
int prevMapMode = 0;
HFONT prevFont = NULL;
HBRUSH prevBrush = NULL;
HBRUSH bshWhite;
UINT textAlign;

	textAlign = GetTextAlign(hdc);
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

	if (textAlign != GDI_ERROR)
	{
		SetTextAlign(hdc, textAlign);
	}

	if (prevBrush)
	{
		SelectObject(hdc, prevBrush);
	}

	if (prevFont)
	{
		SelectObject(hdc, prevFont);
	}

	if (prevMapMode)
	{
		SetMapMode(hdc, prevMapMode);
	}
}

void CDisassemblyReg::DrawDisplay2(HWND hWnd, HDC hdc)
{
RECT rc;
BOOL br;
TEXTMETRIC tm;
int x,y;
int slen;

	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

	SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

	br = GetClientRect(hWnd, &rc);
	if (!br)
	{
		return;
	}

	br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		if (tm.tmHeight > 0 && rc.bottom > rc.top)
		{
			if (!m_pAppCommand->IsRunning())
			{
				::HideCaret(hWnd);
				UpdateBuffer();

				int w=0;
				int h=0;
				x = PADDING_LEFT;
				y = MARGIN_TOP + PADDING_TOP;
				
				this->PC.Draw(hdc);				
				this->A.Draw(hdc);
				this->X.Draw(hdc);
				this->Y.Draw(hdc);
				this->SR.Draw(hdc);
				this->SP.Draw(hdc);

				if (this->GetCpuId()==CPUID_MAIN)
				{
					this->Ddr.Draw(hdc);
					this->Data.Draw(hdc);
					this->VicLine.Draw(hdc);
					this->VicCycle.Draw(hdc);
				}
				else if (this->GetCpuId()==CPUID_DISK)
				{
					this->DiskTrack.Draw(hdc);
				}

				::ShowCaret(hWnd);
			}
			else
			{
				y = MARGIN_TOP + PADDING_TOP;
				x = PADDING_LEFT;

				TCHAR *tTitle;
				RECT rcText;
				::SetRect(&rcText, x, y, rc.right - PADDING_RIGHT, rc.bottom);
				if (rc.right > rc.left && rc.bottom > rc.top)
				{
					tTitle = TEXT("CPU register window unavailable during trace.");
					slen = (int)_tcslen(tTitle);
					if (DrawText(hdc, tTitle, slen, &rcText, DT_TOP | DT_WORDBREAK | DT_CALCRECT))
					{
						LONG k = ((rc.bottom - rc.top) / tm.tmHeight) * tm.tmHeight;
						rc.bottom = rc.top + k;
						DrawText(hdc, tTitle, slen, &rcText, DT_TOP | DT_WORDBREAK);
					}
				}
			}
		}
	}

}

void CDisassemblyReg::InvalidateBuffer()
{
	if (!m_hWnd)
	{
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

void CDisassemblyReg::UpdateDisplay()
{
	UpdateBuffer();
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);
}

void CDisassemblyReg::UpdateBuffer()
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	if (!this->PC.IsChanged)
	{
		this->PC.SetEditValue(state.PC_CurrentOpcode);
	}

	if (!this->A.IsChanged)
	{
		this->A.SetEditValue(state.A);
	}

	if (!this->X.IsChanged)
	{
		this->X.SetEditValue(state.X);
	}

	if (!this->Y.IsChanged)
	{
		this->Y.SetEditValue(state.Y);
	}

	if (!this->SR.IsChanged)
	{
		this->SR.SetEditValue(state.Flags);
	}

	if (!this->SP.IsChanged)
	{
		this->SP.SetEditValue(state.SP);
	}

	if (this->GetCpuId() == CPUID_MAIN)
	{
		if (!this->Ddr.IsChanged)
		{
			this->Ddr.SetEditValue(state.PortDdr);
		}

		if (!this->Data.IsChanged)
		{
			this->Data.SetEditValue(state.PortDataStored);
		}

		bit16 line = this->c64->GetMon()->GetVic()->GetNextRasterLine();
		bit8 cycle = this->c64->GetMon()->GetVic()->GetNextRasterCycle();
		this->VicLine.SetEditValue(line);
		this->VicCycle.SetEditValue(cycle);
	}
	else if (this->GetCpuId() == CPUID_DISK)
	{
		this->DiskTrack.SetEditValue(this->c64->GetMon()->GetDisk()->GetHalfTrackIndex());
	}
}

void CDisassemblyReg::SelectControl(EdLn *control)
{
	EdLn *p = this->GetSelectedControl();
	if (p != NULL)
	{
		if (p != control)
		{
			this->SaveEditing();
			this->CancelEditing();
			this->UpdateCaret();
			this->UpdateDisplay();
		}

		p->IsSelected = false;
	}

	if (control != NULL)
	{
		control->IsSelected = true;
		this->CurrentControlID = control->GetControlID();
	}
	else
	{
		this->CurrentControlID = 0;
	}
}

EdLn *CDisassemblyReg::GetSelectedControl()
{
	return this->GetControlByID(this->CurrentControlID);
}

EdLn *CDisassemblyReg::GetControlByID(int id)
{
	unsigned int i;
	if (id > 0)
	{
		for (i = 0; i < (int)Controls.Count() ; i++)
		{
			EdLn *p = this->Controls[i];
			if (p != NULL)
			{
				if (p->GetControlID() == id)
				{
					return p;
				}
			}
		}
	}

	return NULL;
}

EdLn *CDisassemblyReg::GetTabFirstControl()
{
unsigned int i;

	int firstTabNumber = -1;
	int firstIndex = 0;
	for (i = 0; i < Controls.Count() ; i++)
	{
		EdLn *p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex >= 0)
		{
			if (firstTabNumber > p->m_iTabIndex || firstTabNumber < 0)
			{
				firstTabNumber = p->m_iTabIndex;
				firstIndex = i;
			}
		}
	}

	if (firstTabNumber >=0 && firstTabNumber < (int)this->Controls.Count())
	{
		return this->Controls[firstIndex];
	}
	else
	{
		return NULL;
	}
}

EdLn *CDisassemblyReg::GetTabLastControl()
{
unsigned int i;

	int lastTabNumber = -1;
	int lastIndex = 0;
	for (i = 0; i < (int)Controls.Count() ; i++)
	{
		EdLn *p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex >= 0)
		{
			if (lastTabNumber < p->m_iTabIndex || lastTabNumber < 0)
			{
				lastTabNumber = p->m_iTabIndex;
				lastIndex = i;
			}
		}
	}

	if (lastTabNumber >=0 && lastTabNumber < (int)this->Controls.Count())
	{
		return this->Controls[lastIndex];
	}
	else
	{
		return NULL;
	}
}

EdLn * CDisassemblyReg::GetTabNextControl()
{
unsigned int i;

	EdLn *p = this->GetControlByID(this->CurrentControlID);
	if (p == NULL)
	{
		return this->GetTabFirstControl();
	}

	int tabCurrentNumber = p->m_iTabIndex;
	int firstTabNumber = -1;
	int lastTabNumber = -1;
	int nextTabNumber = -1;
	int firstIndex = 0;
	int lastIndex = 0;
	int nextIndex = 0;
	for (i = 0; i < this->Controls.Count(); i++)
	{
		p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex >= 0)
		{
			if (firstTabNumber < 0 || (firstTabNumber >= 0 && firstTabNumber > p->m_iTabIndex))
			{
				firstTabNumber = p->m_iTabIndex;
				firstIndex = i;
			}

			if (lastTabNumber < 0 || (lastTabNumber >= 0 && lastTabNumber < p->m_iTabIndex))
			{
				lastTabNumber = p->m_iTabIndex;
				lastIndex = i;
			}

			if ((nextTabNumber < 0 && p->m_iTabIndex > tabCurrentNumber) || (nextTabNumber >= 0 && p->m_iTabIndex > tabCurrentNumber && p->m_iTabIndex < nextTabNumber))
			{
				nextTabNumber = p->m_iTabIndex;
				nextIndex = i;
			}
		}
	}

	if (tabCurrentNumber >= lastTabNumber || nextTabNumber < 0)
	{
		nextTabNumber = firstTabNumber;
		nextIndex = firstIndex;
	}

	if (nextIndex >= 0 && nextIndex < (int)this->Controls.Count())
	{
		return this->Controls[nextIndex];
	}
	else
	{
		return this->GetTabFirstControl();
	}
}

EdLn *CDisassemblyReg::GetTabPreviousControl()
{
unsigned int i;

	EdLn *p = this->GetControlByID(this->CurrentControlID);
	if (p == NULL)
	{
		return this->GetTabFirstControl();
	}

	int tabCurrentNumber = p->m_iTabIndex;
	int firstTabNumber = -1;
	int lastTabNumber = -1;
	int nextTabNumber = -1;
	int firstIndex = 0;
	int lastIndex = 0;
	int nextIndex = 0;
	for (i = 0; i < (int)this->Controls.Count() ; i++)
	{
		p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex >= 0)
		{
			if (firstTabNumber < 0 || (firstTabNumber >= 0 && firstTabNumber > p->m_iTabIndex))
			{
				firstTabNumber = p->m_iTabIndex;
				firstIndex = i;
			}

			if (lastTabNumber < 0 || (lastTabNumber >= 0 && lastTabNumber < p->m_iTabIndex))
			{
				lastTabNumber = p->m_iTabIndex;
				lastIndex = i;
			}

			if ((nextTabNumber < 0 && p->m_iTabIndex < tabCurrentNumber) || (nextTabNumber >= 0 && p->m_iTabIndex < tabCurrentNumber && p->m_iTabIndex > nextTabNumber))
			{
				nextTabNumber = p->m_iTabIndex;
				nextIndex = i;
			}
		}
	}

	if (tabCurrentNumber <= firstTabNumber || nextTabNumber < 0)
	{
		nextTabNumber = lastTabNumber;
		nextIndex = lastIndex;
	}

	if (nextIndex >= 0 && nextIndex < (int)this->Controls.Count())
	{
		return this->Controls[nextIndex];
	}
	else
	{
		return this->GetTabLastControl();
	}
}

void CDisassemblyReg::UpdateCaret()
{
	if (m_iShowCaretCount < 0)
	{
		return;
	}

	EdLn *p = NULL;
	if (this->IsEditing)
	{
		p = this->GetSelectedControl();;
		if (p != NULL)
		{
			p->UpdateCaretPosition(this->m_hdc);
		}
	}

	if (p != NULL && this->m_hWnd == GetFocus())
	{
		if (m_iShowCaretCount == 0)
		{
			m_iShowCaretCount++;
			::ShowCaret(this->m_hWnd);
		}
	}
	else
	{
		if (m_iShowCaretCount > 0)
		{
			m_iShowCaretCount--;
			::HideCaret(this->m_hWnd);
		}
	}
}

void CDisassemblyReg::ClearCaretCount()
{
	m_iShowCaretCount = -1;
}

void CDisassemblyReg::StartCaretCount()
{
	m_iShowCaretCount = 0;
}

bool CDisassemblyReg::ProcessChar(WPARAM wParam, LPARAM lParam)
{
	if (this->CurrentControlID <= 0)
	{
		return false;
	}

	EdLn *p = this->GetSelectedControl();
	if (p != NULL)
	{
		if (p->GetIsEditable() && p->CanCharEdit((TCHAR)wParam))
		{
			if (!this->IsEditing)
			{
				this->StartEditing(p, 0);
			}

			p->CharEdit((TCHAR)wParam);
			return true;
		}
	}

	return false;
}

bool CDisassemblyReg::ProcessKeyDown(WPARAM wParam, LPARAM lParam)
{
	EdLn *p = this->GetSelectedControl();
	if (p != NULL)
	{
		if (p->GetIsEditable())
		{
			p->KeyDown((int)wParam);
		}
	}

	return true;
}

bool CDisassemblyReg::ProcessLButtonDown(WPARAM wParam, LPARAM lParam)
{
int x;
int y;
HRESULT hr;
	x = GET_X_LPARAM(lParam);
	y = GET_Y_LPARAM(lParam);
	bool bFound = false;
	int charIndex = 0;
	HWND hWnd = this->GetHwnd();
	for(unsigned int i = 0; i < this->Controls.Count(); i++)
	{
		EdLn *p = this->Controls[i];
		if (p->IsHitEdit(x, y) && p->GetIsEditable() && p->GetIsVisible())
		{
			bFound = true;
			hr = p->GetCharIndex(this->m_hdc, x, y, &charIndex, NULL);
			if (FAILED(hr))
			{
				break;
			}

			this->StartEditing(p, charIndex);
			break;
		}
	}

	if (!bFound)
	{
		this->CancelEditing();
		this->UpdateCaret();
		this->UpdateDisplay();
	}

	return true;
}

void CDisassemblyReg::StartEditing(EdLn *control, int insertionPoint)
{
	if (control != NULL)
	{
		this->SelectControl(control);
		if (insertionPoint >= 0)
		{
			control->SetInsertionPoint(insertionPoint);
		}

		this->IsEditing = true;
		this->UpdateCaret();
	}
}

bool CDisassemblyReg::IsChanged()
{
unsigned int i;
	for (i = 0; i < this->Controls.Count(); i++)
	{
		if (this->Controls[i]->IsChanged)
		{
			return true;
		}		
	}

	return false;
}

HRESULT CDisassemblyReg::Init(int x, int y, int cpuid, DBGSYM::MonitorOption::Radix radix)
{
HRESULT hr;

	this->cpuid = cpuid;
	this->xpos = x;
	this->ypos = y;
	this->radix = radix;
	ZeroMemory(&this->TextMetric, sizeof(this->TextMetric));
	::GetTextMetrics(m_hdc, &this->TextMetric);    
	hr = PC.Init(m_hWnd, CTRLID_PC, 1, m_hFont, TEXT("PC"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = A.Init(m_hWnd, CTRLID_A, 2, m_hFont, TEXT("A"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = X.Init(m_hWnd, CTRLID_X, 3, m_hFont, TEXT("X"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = Y.Init(m_hWnd, CTRLID_Y, 4, m_hFont, TEXT("Y"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = SR.Init(m_hWnd, CTRLID_SR, 5, m_hFont, TEXT("NV-BDIZC"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = SP.Init(m_hWnd, CTRLID_SP, 6, m_hFont, TEXT("SP"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = Ddr.Init(m_hWnd, CTRLID_DDR, 7, m_hFont, TEXT("00"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = Data.Init(m_hWnd, CTRLID_DATA, 8, m_hFont, TEXT("01"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = VicLine.Init(m_hWnd, CTRLID_VICLINE, 9, m_hFont, TEXT("LINE"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = VicCycle.Init(m_hWnd, CTRLID_VICCYCLE, 10, m_hFont, TEXT("CYC"));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = DiskTrack.Init(m_hWnd, CTRLID_DISKTRACK, 11, m_hFont, TEXT("TRACK"));
	if (FAILED(hr))
	{
		return hr;	
	}

	EdLn* ctrls[] = {&PC, &A, &X, &Y, &SR, &SP, &Ddr, &Data, &VicLine, &VicCycle, &DiskTrack};
	Controls.Clear();
	hr = Controls.Resize(_countof(ctrls));
	if (FAILED(hr))
	{
		return hr;
	}

	for(int i = 0; i < _countof(ctrls); i++)
	{
		EdLn* p = ctrls[i];
		Controls.Append(p);
	}

	this->CurrentControlID = 0;
	this->IsEditing = false;
	this->SetRadix(radix);
	//this->UpdateMetrics(radix);
	return S_OK;
}

HRESULT CDisassemblyReg::UpdateDisplay(EdLn *control, int value)
{	
	RECT rcEdit;
	HRESULT hr;

	control->IsChanged = false;
	hr = control->GetRects(this->m_hdc, NULL, &rcEdit, NULL);
	if (SUCCEEDED(hr))
	{
		control->SetEditValue(value);
		InvalidateRect(this->m_hWnd, &rcEdit, TRUE);
		UpdateWindow(this->m_hWnd);
	}

	return hr;
}

HRESULT CDisassemblyReg::AdviseEvents()
{
	HSink hs;
	for (unsigned int i=0; i < this->Controls.Count(); i++)
	{
		EdLn *p = this->Controls[i];
		hs = p->EsOnTextChanged.Advise((CDisassemblyReg_EventSink_OnTextChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = p->EsOnTabControl.Advise((CDisassemblyReg_EventSink_OnTabControl *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = p->EsOnSaveControl.Advise((CDisassemblyReg_EventSink_OnSaveControl *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = p->EsOnCancelControl.Advise((CDisassemblyReg_EventSink_OnCancelControl *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = p->EsOnCancelControl.Advise((CDisassemblyReg_EventSink_OnCancelControl *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}
	}
	
	if (this->cpuid == CPUID_MAIN)
	{
		hs = this->m_pAppCommand->EsCpuC64RegPCChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegPCChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegAChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegAChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegXChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegXChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegYChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegYChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegSRChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegSRChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegSPChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegSPChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegDdrChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegDdrChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuC64RegDataChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegDataChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}
	}
	else
	{
		hs = this->m_pAppCommand->EsCpuDiskRegPCChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegPCChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuDiskRegAChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegAChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuDiskRegXChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegXChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuDiskRegYChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegYChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuDiskRegSRChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegSRChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}

		hs = this->m_pAppCommand->EsCpuDiskRegSPChanged.Advise((CDisassemblyReg_EventSink_OnCpuRegSPChanged *)this);
		if (hs == NULL)
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

void CDisassemblyReg::UnadviseEvents()
{
	((CDisassemblyReg_EventSink_OnTextChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnTabControl *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCancelControl *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnSaveControl *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegPCChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegAChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegXChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegYChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegSRChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegSPChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegDdrChanged *)this)->UnadviseAll();
	((CDisassemblyReg_EventSink_OnCpuRegDataChanged *)this)->UnadviseAll();
}

void CDisassemblyReg::OnCancelControl(void *sender, EdLnCancelControlEventArgs& e)
{
	this->CancelEditing();
	this->UpdateCaret();
	this->UpdateDisplay();
}

void CDisassemblyReg::OnSaveControl(void *sender, EdLnSaveControlEventArgs& e)
{
	if (this->SaveEditing(e.pEdLnControl))
	{
		this->MoveTab(true);
		e.IsCancelled = false;
	}
	else
	{
		e.IsCancelled = true;
	}
}

void CDisassemblyReg::OnTabControl(void *sender, EdLnTabControlEventArgs& e)
{
	if (!this->SaveEditing(e.pEdLnControl))
	{
		this->CancelEditing();
		this->UpdateCaret();
		this->UpdateDisplay();
	}

	this->MoveTab(e.IsNext);
	e.IsCancelled = false;
}

void CDisassemblyReg::OnTextChanged(void *sender, EdLnTextChangedEventArgs& e)
{
}

void CDisassemblyReg::OnCpuRegPCChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->PC, state.PC);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegAChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->A, state.A);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegXChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->X, state.X);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegYChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->Y, state.Y);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegSRChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->SR, state.Flags);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegSPChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->SP, state.SP);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegDdrChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->Ddr, state.PortDdr);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::OnCpuRegDataChanged(void *sender, EventArgs& e)
{
	CPUState state;
	this->GetCpu()->GetCpuState(state);
	HRESULT hr = this->UpdateDisplay(&this->Data, state.PortDataStored);
	if (FAILED(hr))
	{
		this->UpdateDisplay();
	}
}

void CDisassemblyReg::CancelEditing()
{
	unsigned int i;
	for (i = 0; i < (int)Controls.Count() ; i++)
	{
		this->Controls[i]->IsChanged = false;	
	}

	this->IsEditing = false;
	//this->UpdateCaret();
	//this->UpdateDisplay();
}

void CDisassemblyReg::SaveEditing()
{
	for(unsigned int i = 0; i < this->Controls.Count(); i++)
	{
		EdLn *p = this->Controls[i];
		if (p->IsChanged)
		{
			if (this->SaveEditing(p))
			{
				p->IsChanged = false;
			}
		}		
	}
}

bool CDisassemblyReg::SaveEditing(EdLn *pEdLnControl)
{
int v = 0;
HRESULT hr = E_FAIL;
bit16 address;
bit8 dataByte;
CPUState cpustate;
EventArgs args;

	if (pEdLnControl == NULL)
	{
		return false;
	}

	int id = pEdLnControl->GetControlID();
	if (id == this->CTRLID_PC)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			address = (bit16)v;
			this->GetCpu()->GetCpuState(cpustate);
			if (cpustate.PC_CurrentOpcode != address)
			{
				this->GetCpu()->SetPC(address);
				if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
				{
					this->m_pAppCommand->EsCpuC64RegPCChanged.Raise(this, args);
				}
				else
				{
					this->m_pAppCommand->EsCpuDiskRegPCChanged.Raise(this, args);
				}
			}
		}
	}
	else if (id == this->CTRLID_A)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetA(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegAChanged.Raise(this, args);
			}
			else
			{
				this->m_pAppCommand->EsCpuDiskRegAChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_X)
	{
		hr = pEdLnControl->GetEditValue(v);		
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetX(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegXChanged.Raise(this, args);
			}
			else
			{
				this->m_pAppCommand->EsCpuDiskRegXChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_Y)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetY(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegYChanged.Raise(this, args);
			}
			else
			{
				this->m_pAppCommand->EsCpuDiskRegYChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_SR)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetSR(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegSRChanged.Raise(this, args);
			}
			else
			{
				this->m_pAppCommand->EsCpuDiskRegSRChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_SP)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetSP(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegSPChanged.Raise(this, args);
			}
			else
			{
				this->m_pAppCommand->EsCpuDiskRegSPChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_DDR)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetDdr(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegDdrChanged.Raise(this, args);
			}
		}
	}
	else if (id == this->CTRLID_DATA)
	{
		hr = pEdLnControl->GetEditValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->GetCpu()->SetData(dataByte);
			if (this->GetCpu()->GetCpuId() == CPUID_MAIN)
			{
				this->m_pAppCommand->EsCpuC64RegDataChanged.Raise(this, args);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		pEdLnControl->IsChanged = false;
		return true;
	}
	else
	{
		return false;
	}	
}


void CDisassemblyReg::MoveTab(bool isNext)
{
EdLn *p;
	if (isNext)
	{
		p = this->GetTabNextControl();
	}
	else
	{
		p = this->GetTabPreviousControl();
	}

	if (p != NULL)
	{
		this->StartEditing(p, 0);
	}
	else
	{
		if (isNext)
		{
			p = this->GetTabFirstControl();
		}
		else
		{
			p = this->GetTabLastControl();
		}

		if (p != NULL)
		{
			this->StartEditing(p, 0);
		}
	}
}