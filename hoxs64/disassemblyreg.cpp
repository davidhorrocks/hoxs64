#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
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

#include "assert.h"
#include "cevent.h"
#include "monitor.h"
#include "edln.h"
#include "disassemblyreg.h"
#include "resource.h"

TCHAR CDisassemblyReg::ClassName[] = TEXT("Hoxs64DisassemblyReg");

CDisassemblyReg::CDisassemblyReg()
{
	m_AutoDelete = false;
	m_pParent = NULL;
	m_hFont = NULL;
	m_monitorCommand = NULL;
	m_MinSizeDone = false;
	m_vic = NULL;
	m_cpu = NULL;
	m_hdc = NULL;
}

CDisassemblyReg::~CDisassemblyReg()
{
	Cleanup();
}

HRESULT CDisassemblyReg::Init(CVirWindow *parent, IMonitorCommand *monitorCommand, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont)
{
	Cleanup();
	HRESULT hr;
	m_pParent = parent;
	this->m_cpu = cpu;
	this->m_hFont = hFont;
	this->m_monitorCommand = monitorCommand;
	this->m_vic = vic;
	hr = this->m_mon.Init(cpu, vic);
	if (FAILED(hr))
		return hr;

	return hr;
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
		return E_FAIL;
	return S_OK;	
}

HWND CDisassemblyReg::Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID)
{
	return CVirWindow::Create(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, ctrlID, hInstance);
}

void CDisassemblyReg::GetMinWindowSize(int &w, int &h)
{
BOOL br;
	if (m_MinSizeDone)
	{
		w = m_MinSizeW;
		h = m_MinSizeH;
		return;
	}
	else
	{
		w = PADDING_LEFT + PADDING_RIGHT + GetSystemMetrics(SM_CXFRAME) * 2;
		h = MARGIN_TOP + PADDING_TOP + PADDING_BOTTOM;
		int cpuid = 0;
		if (m_cpu)
			cpuid = m_cpu->GetCpuId();
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
						br = GetTextMetrics(hdc, &tm);
						if (br)
						{							

							if (cpuid==0)
							{
								TCHAR s[]= TEXT("ABCDXABXABXABXNV-BDIZCXABXABXABXLINEXCYC");
								int slen = lstrlen(s);
								SIZE sizeText;
								BOOL br = GetTextExtentExPoint(hdc, s, slen, 0, NULL, NULL, &sizeText);
								if (br)
								{
									w += sizeText.cx;
								}
							}
							else
							{
								TCHAR s[]= TEXT("ABCDXABXABXABXNV-BDIZCXAB");
								int slen = lstrlen(s);
								SIZE sizeText;
								BOOL br = GetTextExtentExPoint(hdc, s, slen, 0, NULL, NULL, &sizeText);
								if (br)
								{
									w += sizeText.cx;
								}
							}
							h += tm.tmHeight * 2;
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

HRESULT CDisassemblyReg::OnCreate(HWND hWnd)
{
HRESULT hr;
	m_hdc = GetDC(hWnd);
	if (m_hdc == 0)
		return E_FAIL;
	int x = PADDING_LEFT;
	int y = MARGIN_TOP + PADDING_TOP;

	hr = m_RegBuffer.Init(hWnd, m_hdc, m_hFont, x, y, m_cpu->GetCpuId());
	if (FAILED(hr))
		return hr;

	hr = AdviseEvents();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

bool CDisassemblyReg::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_monitorCommand->IsRunning())
		return true;
	m_RegBuffer.ProcessLButtonDown(wParam, lParam);
	if (hWnd != ::GetFocus())
	{
		::SetFocus(hWnd);
	}
	return true;
}

bool CDisassemblyReg::OnChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_monitorCommand->IsRunning())
		return true;
	return m_RegBuffer.ProcessChar(wParam, lParam);
}

bool CDisassemblyReg::OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_monitorCommand->IsRunning())
		return true;
	EdLn *p = m_RegBuffer.GetFocusedControl();
	if (p != NULL)
	{
		return m_RegBuffer.ProcessKeyDown(wParam, lParam);
	}
	else if (wParam == VK_TAB)
	{
		int i = m_RegBuffer.GetTabFirstControlIndex();
		if (i >= 0)
		{
			EdLn *p = this->m_RegBuffer.Controls[i];
			p->Home();
			this->m_RegBuffer.SelectControl(i);
			this->m_RegBuffer.UpdateCaret(m_hWnd, m_hdc);
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
		if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		if (wParam == SIZE_MINIMIZED)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		UpdateDisplay();
		return 0;
	case WM_LBUTTONDOWN:
		if (!OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_CHAR:
		if (!OnChar(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KEYDOWN:
		if (!OnKeyDown(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SETFOCUS:
		CreateCaret(hWnd, NULL, this->m_RegBuffer.TextMetric.tmAveCharWidth, this->m_RegBuffer.TextMetric.tmHeight);
		m_RegBuffer.UpdateCaret(hWnd, m_hdc);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KILLFOCUS:
		m_RegBuffer.ClearCaret(hWnd);
		DestroyCaret();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
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
		return;

	br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		if (tm.tmHeight > 0 && rc.bottom > rc.top)
		{
			if (!m_monitorCommand->IsRunning())
			{
				::HideCaret(hWnd);
				UpdateBuffer(m_RegBuffer);

				int w=0;
				int h=0;
				x = PADDING_LEFT;
				y = MARGIN_TOP + PADDING_TOP;
				
				this->m_RegBuffer.PC.Draw(hdc);				
				this->m_RegBuffer.A.Draw(hdc);
				this->m_RegBuffer.X.Draw(hdc);
				this->m_RegBuffer.Y.Draw(hdc);
				this->m_RegBuffer.SR.Draw(hdc);
				this->m_RegBuffer.SP.Draw(hdc);

				if (m_cpu->GetCpuId()==0)
				{
					this->m_RegBuffer.Ddr.Draw(hdc);
					this->m_RegBuffer.Data.Draw(hdc);
					this->m_RegBuffer.VicLine.Draw(hdc);
					this->m_RegBuffer.VicCycle.Draw(hdc);
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
	m_RegBuffer.ClearBuffer();
	if (!m_hWnd)
		InvalidateRect(m_hWnd, NULL, TRUE);
}

void CDisassemblyReg::UpdateDisplay()
{
	UpdateBuffer(m_RegBuffer);
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);

}

void CDisassemblyReg::UpdateBuffer(RegLineBuffer& b)
{
	b.ClearBuffer();
	//m_mon.GetCpuRegisters(b.PC_Text, _countof(b.PC_Text), b.A_Text, _countof(b.A_Text), b.X_Text, _countof(b.X_Text), b.Y_Text, _countof(b.Y_Text), b.SR_Text, _countof(b.SR_Text), b.SP_Text, _countof(b.SP_Text), b.Ddr_Text, _countof(b.Ddr_Text), b.Data_Text, _countof(b.Data_Text));
	//m_mon.GetVicRegisters(b.VicLine_Text, _countof(b.VicLine_Text), b.VicCycle_Text, _countof(b.VicCycle_Text));

	CPUState state;
	m_mon.GetCpu()->GetCpuState(state);

	b.PC.SetValue(state.PC_CurrentOpcode);
	b.A.SetValue(state.A);
	b.X.SetValue(state.X);
	b.Y.SetValue(state.Y);
	b.SR.SetValue(state.Flags);
	b.SP.SetValue(state.SP);
	if (m_mon.GetCpu()->GetCpuId() == 0)
	{
		b.Ddr.SetValue(state.PortDdr);
		b.Data.SetValue(state.PortDataStored);

		bit16 line = m_mon.GetVic()->GetRasterLine();
		bit8 cycle = m_mon.GetVic()->GetRasterCycle();
		
		b.VicLine.SetValue(line);
		b.VicCycle.SetValue(cycle);
	}
}

void CDisassemblyReg::RegLineBuffer::ClearBuffer()
{
	PC.SetValue(0);
	A.SetValue(0);
	X.SetValue(0);
	Y.SetValue(0);
	SR.SetValue(0);
	SP.SetValue(0);
	Ddr.SetValue(0);
	Data.SetValue(0);
	VicLine.SetValue(0);
	VicCycle.SetValue(0);
}

void CDisassemblyReg::RegLineBuffer::SelectControl(int i)
{
	DeSelectControl(CurrentControlIndex);
	if (i >= 0 && i < this->Controls.Count())
	{
		EdLn *p = this->Controls[i];
		p->IsFocused = true;
		this->CurrentControlIndex = i;
	}
}

void CDisassemblyReg::RegLineBuffer::DeSelectControl(int i)
{
	if (i >= 0 && i < this->Controls.Count())
		this->Controls[i]->IsFocused = false;
}

EdLn *CDisassemblyReg::RegLineBuffer::GetFocusedControl()
{
	if (CurrentControlIndex < 0 || CurrentControlIndex >= Controls.Count())
		return NULL;
	EdLn *p = Controls[CurrentControlIndex];
	if (!p->IsFocused)
		return NULL;
	return p;
}

int CDisassemblyReg::RegLineBuffer::GetTabFirstControlIndex()
{
int i;
	if (Controls.Count()<=0)
	{
		return -1;
	}
	bool bFound1st = false;
	int tabNextIndex;
	int controlIndex = -1;
	for (i = 0; i < Controls.Count() ; i++)
	{
		EdLn *p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible())
		{
			if (!bFound1st)
			{
				bFound1st = true;
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
			else if (p->m_iTabIndex < tabNextIndex)
			{
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
		}
	}
	return controlIndex;
}

int CDisassemblyReg::RegLineBuffer::GetTabLastControlIndex()
{
int i;
	if (Controls.Count()<=0)
	{
		return -1;
	}
	bool bFound1st = false;
	int tabNextIndex;
	int controlIndex = -1;
	for (i = 0; i < Controls.Count() ; i++)
	{
		EdLn *p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible())
		{
			if (!bFound1st)
			{
				bFound1st = true;
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
			else if (p->m_iTabIndex > tabNextIndex)
			{
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
		}
	}
	return controlIndex;
}

int CDisassemblyReg::RegLineBuffer::GetTabNextControlIndex()
{
int i;
	if (Controls.Count()<=0)
	{
		return -1;
	}
	if (CurrentControlIndex < 0 || CurrentControlIndex >= Controls.Count())
		return GetTabFirstControlIndex();

	i = CurrentControlIndex;
	EdLn *p = this->Controls[i];
	int tabCurrentIndex = p->m_iTabIndex;
	int tabNextIndex = tabCurrentIndex;
	int controlIndex = -1;
	bool bFound1st = false;
	for (i = 0; i < Controls.Count() ; i++)
	{
		p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex > tabCurrentIndex)
		{
			if (!bFound1st)
			{
				bFound1st = true;
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
			else if (p->m_iTabIndex < tabNextIndex)
			{
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
		}
	}
	return controlIndex;
}

int CDisassemblyReg::RegLineBuffer::GetTabPreviousControlIndex()
{
int i;
	if (Controls.Count()<=0)
	{
		return -1;
	}
	if (CurrentControlIndex < 0 || CurrentControlIndex >= Controls.Count())
		return GetTabLastControlIndex();

	i = CurrentControlIndex;
	EdLn *p = this->Controls[i];
	int tabCurrentIndex = p->m_iTabIndex;
	int tabNextIndex = tabCurrentIndex;
	int controlIndex = -1;
	bool bFound1st = false;
	for (i = 0; i < Controls.Count() ; i++)
	{
		p = this->Controls[i];
		if (p->GetIsEditable() && p->GetIsVisible() && p->m_iTabIndex < tabCurrentIndex)
		{
			if (!bFound1st)
			{
				bFound1st = true;
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
			else if (p->m_iTabIndex > tabNextIndex)
			{
				tabNextIndex = p->m_iTabIndex;
				controlIndex = i;
			}
		}
	}
	return controlIndex;
}

void CDisassemblyReg::RegLineBuffer::UpdateCaret(HWND hWnd, HDC hdc)
{
bool bFound = false;

	if (hWnd != GetFocus())
		return;
	if (hdc == NULL)
		return;

	for (int i=0; i < this->Controls.Count(); i++)
	{
		EdLn *t = this->Controls[i];
		if (t->IsFocused)
		{
			t->UpdateCaretPosition(hdc);
			bFound = true;
			break;
		}
	}
	if (bFound)
	{
		if (m_iShowCaretCount <= 0)
		{
			m_iShowCaretCount++;
			::ShowCaret(hWnd);
		}
	}
	else
	{
		if (m_iShowCaretCount > 0)
		{
			m_iShowCaretCount--;
			::HideCaret(hWnd);
		}
	}
}

void CDisassemblyReg::RegLineBuffer::ClearCaret(HWND hWnd)
{
	m_iShowCaretCount = 0;
}

bool CDisassemblyReg::RegLineBuffer::ProcessChar(WPARAM wParam, LPARAM lParam)
{
	if (CurrentControlIndex < 0 || CurrentControlIndex > Controls.Count() - 1)
		return false;
	EdLn *t = Controls[CurrentControlIndex];
	
	if (t->IsFocused && t->GetIsEditable())
	{
		t->CharEdit((TCHAR)wParam);
	}
	return true;
}

bool CDisassemblyReg::RegLineBuffer::ProcessKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (CurrentControlIndex < 0 || CurrentControlIndex > Controls.Count() - 1)
		return false;
	EdLn *t = Controls[CurrentControlIndex];
	
	if (t->IsFocused && t->GetIsEditable())
	{
		t->KeyDown((int)wParam);
	}
	
	return true;
}

bool CDisassemblyReg::RegLineBuffer::ProcessLButtonDown(WPARAM wParam, LPARAM lParam)
{
int x;
int y;
HRESULT hr;
	x = GET_X_LPARAM(lParam);
	y = GET_Y_LPARAM(lParam);
	int c = (int)this->Controls.Count();
	bool bFound = false;
	int iCellIndex = 0;
	HWND hWnd = m_hWndParent;
	HDC hdc = GetDC(hWnd);
	if (hdc==NULL)
		return false;
	for(int i = 0; i < c; i++)
	{
		EdLn *p = this->Controls[i];
		if (p->IsHitEdit(x, y) && p->GetIsEditable() && p->GetIsVisible())
		{
			bFound = true;
			hr = p->GetCharIndex(hdc, x, y, &iCellIndex, NULL);
			if (FAILED(hr))
				break;
			this->SelectControl(i);
			p->SetInsertionPoint(iCellIndex);
			this->UpdateCaret(hWnd, hdc);
			break;
		}
	}
	if (!bFound)
	{
		CancelEditing();
	}
	return true;
}

void CDisassemblyReg::RegLineBuffer::CancelEditing()
{
	HWND hWnd = m_hWndParent;
	HDC hdc = GetDC(hWnd);
	this->DeSelectControl(this->CurrentControlIndex);
	if (hdc==NULL)
		return;
	this->UpdateCaret(hWnd, hdc);
}

HRESULT CDisassemblyReg::RegLineBuffer::Init(HWND hWnd, HDC hdc, HFONT hFont, int x, int y, int cpuid)
{
HRESULT hr;
int prevMapMode = 0;
HFONT prevFont = NULL;

	this->m_hWndParent = hWnd;
	ZeroMemory(&TextMetric, sizeof(TextMetric));
	prevMapMode = SetMapMode(hdc, MM_TEXT);
	if (prevMapMode)
	{
		prevFont = (HFONT)SelectObject(hdc, hFont);		
		if (prevFont)
		{				
			::GetTextMetrics(hdc, &TextMetric);
		}
	}
	if (prevFont)
		SelectObject(hdc, prevFont);
	if (prevMapMode)
		SetMapMode(hdc, prevMapMode);

	hr = PC.Init(hWnd, CTRLID_PC, 1, hFont, TEXT("PC"), EdLn::HexAddress, true, true, 4);
	if (FAILED(hr))
		return hr;
	hr = A.Init(hWnd, CTRLID_A, 2, hFont, TEXT("A"), EdLn::HexByte, true, true, 2);
	if (FAILED(hr))
		return hr;
	hr = X.Init(hWnd, CTRLID_X, 3, hFont, TEXT("X"), EdLn::HexByte, true, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Y.Init(hWnd, CTRLID_Y, 4, hFont, TEXT("Y"), EdLn::HexByte, true, true, 2);
	if (FAILED(hr))
		return hr;
	hr = SR.Init(hWnd, CTRLID_SR, 5, hFont, TEXT("NV-BDIZC"), EdLn::CpuFlags, true, true, 8);
	if (FAILED(hr))
		return hr;
	hr = SP.Init(hWnd, CTRLID_SP, 6, hFont, TEXT("SP"), EdLn::HexByte, true, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Ddr.Init(hWnd, CTRLID_DDR, 7, hFont, TEXT("00"), EdLn::HexByte, cpuid == 0, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Data.Init(hWnd, CTRLID_DATA, 8, hFont, TEXT("01"), EdLn::HexByte, cpuid == 0, true, 2);
	if (FAILED(hr))
		return hr;
	hr = VicLine.Init(hWnd, CTRLID_VICLINE, 9, hFont, TEXT("LINE"), EdLn::Hex, cpuid == 0, false, 3);
	if (FAILED(hr))
		return hr;
	hr = VicCycle.Init(hWnd, CTRLID_VICCYCLE, 10, hFont, TEXT("CYC"), EdLn::Dec, cpuid == 0, false, 2);
	if (FAILED(hr))
		return hr;
	
	CurrentControlIndex = 0;

	hr = ArrangeControls(hdc, x, y, cpuid);
	if (FAILED(hr))
		return hr;

	EdLn* ctrls[10] = {&PC, &A, &X, &Y, &SR, &SP, &Ddr, &Data, &VicLine, &VicCycle};
	Controls.Clear();
	hr = Controls.Resize(_countof(ctrls));
	if (FAILED(hr))
		return hr;

	for(int i = 0; i < _countof(ctrls); i++)
	{
		EdLn* p = ctrls[i];
		hr = p->CreateDefaultHitRegion(hdc);
		if (FAILED(hr))
			return hr;
		Controls.Append(p);
	}
	return S_OK;
}

HRESULT CDisassemblyReg::RegLineBuffer::ArrangeControls(HDC hdc, int x, int y, int cpuid)
{
int w=0;
int h=0;
TEXTMETRIC tm;	
RECT rcAll;
	SetRectEmpty(&rcAll);

	BOOL br = GetTextMetrics(hdc, &tm);
	if (!br)
		return E_FAIL;
	this->PC.SetPos(x, y);
	this->PC.GetRects(hdc, NULL, NULL, &rcAll);				
	x = rcAll.right + 2*tm.tmAveCharWidth;
	this->A.SetPos(x, y);
	this->A.GetRects(hdc, NULL, NULL, &rcAll);

	x = rcAll.right + tm.tmAveCharWidth;
	this->X.SetPos(x, y);
	this->X.GetRects(hdc, NULL, NULL, &rcAll);

	x = rcAll.right + tm.tmAveCharWidth;
	this->Y.SetPos(x, y);
	this->Y.GetRects(hdc, NULL, NULL, &rcAll);

	x = rcAll.right + tm.tmAveCharWidth;
	this->SR.SetPos(x, y);
	this->SR.GetRects(hdc, NULL, NULL, &rcAll);

	x = rcAll.right + tm.tmAveCharWidth;
	this->SP.SetPos(x, y);
	this->SP.GetRects(hdc, NULL, NULL, &rcAll);

	if (cpuid==0)
	{
		x = rcAll.right + tm.tmAveCharWidth;
		this->Ddr.SetPos(x, y);
		this->Ddr.GetRects(hdc, NULL, NULL, &rcAll);

		x = rcAll.right + tm.tmAveCharWidth;
		this->Data.SetPos(x, y);
		this->Data.GetRects(hdc, NULL, NULL, &rcAll);

		x = rcAll.right + tm.tmAveCharWidth;
		this->VicLine.SetPos(x, y);
		this->VicLine.GetRects(hdc, NULL, NULL, &rcAll);

		x = rcAll.right + tm.tmAveCharWidth;
		this->VicCycle.SetPos(x, y);
		this->VicCycle.GetRects(hdc, NULL, NULL, &rcAll);
	}
	return S_OK;
}

HRESULT CDisassemblyReg::AdviseEvents()
{
	HSink hs;
	for (int i=0; i<m_RegBuffer.Controls.Count(); i++)
	{
		EdLn *p = m_RegBuffer.Controls[i];
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
		hs = p->EsOnEscControl.Advise((CDisassemblyReg_EventSink_OnEscControl *)this);
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
}

void CDisassemblyReg::CancelEditing()
{
	this->m_RegBuffer.CancelEditing();
}

void CDisassemblyReg::OnTextChanged(void *sender, EdLnTextChangedEventArgs& e)
{
int v = 0;
HRESULT hr;
EdLn *ped = e.pEdLnControl;
bit16 address;
bit8 dataByte;

	if (ped == NULL)
		return ;

	int id = ped->GetControlID();
	if (id == m_RegBuffer.CTRLID_PC)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			EventArgs regPCChangedEventArgs;
			address = (bit16)v;
			this->m_cpu->SetPC(address);
			if (this->m_cpu->GetCpuId() == 0)
			{
				this->m_monitorCommand->EsCpuC64RegPCChanged.Raise(this, regPCChangedEventArgs);
			}
			else
			{
				this->m_monitorCommand->EsCpuDiskRegPCChanged.Raise(this, regPCChangedEventArgs);
			}
		}
	}
	else if (id == m_RegBuffer.CTRLID_A)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetA(dataByte);
		}
	}
	else if (id == m_RegBuffer.CTRLID_X)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetX(dataByte);
		}
	}
	else if (id == m_RegBuffer.CTRLID_Y)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetY(dataByte);
		}
	}
	else if (id == m_RegBuffer.CTRLID_SR)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetSR(dataByte);
		}
	}
	else if (id == m_RegBuffer.CTRLID_SP)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetSP(dataByte);
		}
	}
	else if (id == m_RegBuffer.CTRLID_DDR)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetDdr(dataByte);
			EventArgs regPCChangedEventArgs;
			if (this->m_cpu->GetCpuId() == 0)
			{
				this->m_monitorCommand->EsCpuC64RegPCChanged.Raise(this, regPCChangedEventArgs);
			}
			else
			{
				this->m_monitorCommand->EsCpuDiskRegPCChanged.Raise(this, regPCChangedEventArgs);
			}
		}
	}
	else if (id == m_RegBuffer.CTRLID_DATA)
	{
		hr = ped->GetValue(v);
		if (SUCCEEDED(hr))
		{
			dataByte = (bit8)v;
			this->m_cpu->SetData(dataByte);
			EventArgs regPCChangedEventArgs;
			if (this->m_cpu->GetCpuId() == 0)
			{
				this->m_monitorCommand->EsCpuC64RegPCChanged.Raise(this, regPCChangedEventArgs);
			}
			else
			{
				this->m_monitorCommand->EsCpuDiskRegPCChanged.Raise(this, regPCChangedEventArgs);
			}
		}
	}
}

void CDisassemblyReg::OnTabControl(void *sender, EdLnTabControlEventArgs& e)
{
int i;
	if (e.IsNext)
		i = this->m_RegBuffer.GetTabNextControlIndex();
	else
		i = this->m_RegBuffer.GetTabPreviousControlIndex();
	if (i >= 0)
	{
		HDC hdc = GetDC(m_hWnd);
		if (hdc != NULL)
		{
			EdLn *p = this->m_RegBuffer.Controls[i];
			p->Home();
			this->m_RegBuffer.SelectControl(i);
			this->m_RegBuffer.UpdateCaret(m_hWnd, hdc);
		}
	}
	else
	{
		if (e.IsNext)
			i = this->m_RegBuffer.GetTabFirstControlIndex();
		else
			i = this->m_RegBuffer.GetTabLastControlIndex();
		if (i >= 0)
		{
			HDC hdc = GetDC(m_hWnd);
			if (hdc != NULL)
			{
				EdLn *p = this->m_RegBuffer.Controls[i];
				p->Home();
				this->m_RegBuffer.SelectControl(i);
				this->m_RegBuffer.UpdateCaret(m_hWnd, hdc);
			}
		}
	}
}

void CDisassemblyReg::OnEscControl(void *sender, EventArgs& e)
{
	EdLn *p = (EdLn*)sender;
	p->IsFocused  = false;
	m_RegBuffer.UpdateCaret(m_hWnd, m_hdc);
}