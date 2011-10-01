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
}

HRESULT CDisassemblyReg::Init(CVirWindow *parent, IMonitorCommand *monitorCommand, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont)
{
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

	hr = m_RegBuffer.Init(m_hdc, m_hFont, x, y, m_cpu->GetCpuId());
	if (FAILED(hr))
		return hr;
	return S_OK;
}

bool CDisassemblyReg::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int x;
int y;
	x = GET_X_LPARAM(lParam);
	y = GET_Y_LPARAM(lParam);
	int c = (int)m_RegBuffer.Controls.Count();
	bool bFound = false;
	for(int i = 0; i<c ; i++)
	{
		EdLn *p = m_RegBuffer.Controls[i];
		if (p->IsHitAll(x, y))
		{
			bFound = true;
			m_RegBuffer.SelectControl(i);
			m_RegBuffer.UpdateCaret(hWnd);
		}
	}
	if (hWnd != ::GetFocus())
	{
		::SetFocus(hWnd);
	}
	else
	{

	}
	return true;
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
		if (OnLButtonDown(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SETFOCUS:
		CreateCaret(hWnd, NULL, this->m_RegBuffer.TextMetric.tmAveCharWidth, this->m_RegBuffer.TextMetric.tmHeight);
		m_RegBuffer.UpdateCaret(hWnd);
		//SetCaretPos(0, 0);
		//ShowCaret(hWnd);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KILLFOCUS:
		m_RegBuffer.ClearCaret(hWnd);
		DestroyCaret();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
			::HideCaret(hWnd);
			if (!m_monitorCommand->IsRunning())
			{
				UpdateBuffer(m_RegBuffer);


				int w=0;
				int h=0;
				x = PADDING_LEFT;
				y = MARGIN_TOP + PADDING_TOP;
				
				//this->m_RegBuffer.PC.SetPos(x, y);
				this->m_RegBuffer.PC.Draw(hdc);
				//this->m_RegBuffer.PC.GetMinWindowSize(hdc, w, h);
				
				//x += w + 2*tm.tmAveCharWidth;
				//this->m_RegBuffer.A.SetPos(x, y);
				this->m_RegBuffer.A.Draw(hdc);
				//this->m_RegBuffer.A.GetMinWindowSize(hdc, w, h);

				//x += w + tm.tmAveCharWidth;
				//this->m_RegBuffer.X.SetPos(x, y);
				this->m_RegBuffer.X.Draw(hdc);
				//this->m_RegBuffer.X.GetMinWindowSize(hdc, w, h);

				//x += w + tm.tmAveCharWidth;
				//this->m_RegBuffer.Y.SetPos(x, y);
				this->m_RegBuffer.Y.Draw(hdc);
				//this->m_RegBuffer.Y.GetMinWindowSize(hdc, w, h);

				//x += w + tm.tmAveCharWidth;
				//this->m_RegBuffer.SR.SetPos(x, y);
				this->m_RegBuffer.SR.Draw(hdc);
				//this->m_RegBuffer.SR.GetMinWindowSize(hdc, w, h);

				//x += w + tm.tmAveCharWidth;
				//this->m_RegBuffer.SP.SetPos(x, y);
				this->m_RegBuffer.SP.Draw(hdc);
				//this->m_RegBuffer.SP.GetMinWindowSize(hdc, w, h);

				if (m_cpu->GetCpuId()==0)
				{
					//x += w + tm.tmAveCharWidth;
					//this->m_RegBuffer.Ddr.SetPos(x, y);
					this->m_RegBuffer.Ddr.Draw(hdc);
					//this->m_RegBuffer.Ddr.GetMinWindowSize(hdc, w, h);

					//x += w + tm.tmAveCharWidth;
					//this->m_RegBuffer.Data.SetPos(x, y);
					this->m_RegBuffer.Data.Draw(hdc);
					//this->m_RegBuffer.Data.GetMinWindowSize(hdc, w, h);

					//x += w + tm.tmAveCharWidth;
					//this->m_RegBuffer.VicLine.SetPos(x, y);
					this->m_RegBuffer.VicLine.Draw(hdc);
					//this->m_RegBuffer.VicLine.GetMinWindowSize(hdc, w, h);

					//x += w + tm.tmAveCharWidth;
					//this->m_RegBuffer.VicCycle.SetPos(x, y);
					this->m_RegBuffer.VicCycle.Draw(hdc);
					//this->m_RegBuffer.VicCycle.GetMinWindowSize(hdc, w, h);
				}
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
			::ShowCaret(hWnd);
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
	if (i>=0 && i<this->Controls.Count())
	{
		this->Controls[i]->IsFocused = true;
		this->CurrentControlIndex = i;
	}
}

void CDisassemblyReg::RegLineBuffer::DeSelectControl(int i)
{
	if (CurrentControlIndex >= 0 && CurrentControlIndex < this->Controls.Count())
		this->Controls[CurrentControlIndex]->IsFocused = false;
}

void CDisassemblyReg::RegLineBuffer::UpdateCaret(HWND hWnd)
{
bool bFound = false;
	if (hWnd != GetFocus())
		return;
	for (int i=0; i<this->Controls.Count(); i++)
	{
		EdLn *t = this->Controls[i];
		if (t->IsFocused)
		{
			bFound = true;
			::SetCaretPos(t->GetXPos(), t->GetYPos());
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

HRESULT CDisassemblyReg::RegLineBuffer::Init(HDC hdc, HFONT hFont, int x, int y, int cpuid)
{
HRESULT hr;
int prevMapMode = 0;
HFONT prevFont = NULL;

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
	hr = PC.Init(hFont, TEXT("PC"), EdLn::HexAddress, true, 4);
	if (FAILED(hr))
		return hr;
	hr = A.Init(hFont, TEXT("A"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = X.Init(hFont, TEXT("X"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Y.Init(hFont, TEXT("Y"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = SR.Init(hFont, TEXT("NV-BDIZC"), EdLn::CpuFlags, true, 8);
	if (FAILED(hr))
		return hr;
	hr = SP.Init(hFont, TEXT("SP"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Ddr.Init(hFont, TEXT("00"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = Data.Init(hFont, TEXT("01"), EdLn::HexByte, true, 2);
	if (FAILED(hr))
		return hr;
	hr = VicLine.Init(hFont, TEXT("LINE"), EdLn::Hex, false, 3);
	if (FAILED(hr))
		return hr;
	hr = VicCycle.Init(hFont, TEXT("CYC"), EdLn::Dec, false, 2);
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