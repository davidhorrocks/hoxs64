#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "errormsg.h"

#include "assert.h"

#include "monitor.h"
#include "disassemblyreg.h"
#include "resource.h"

TCHAR CDisassemblyReg::ClassName[] = TEXT("Hoxs64DisassemblyReg");

CDisassemblyReg::CDisassemblyReg()
{
	m_AutoDelete = false;
	m_pParent = NULL;
	m_hFont = NULL;
	m_monitorEvent = NULL;
	m_MinSizeDone = false;
	m_vic = NULL;
	m_cpu = NULL;
}

CDisassemblyReg::~CDisassemblyReg()
{
}

HRESULT CDisassemblyReg::Init(CVirWindow *parent, IMonitorEvent *monitorEvent, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont)
{
	HRESULT hr;
	m_pParent = parent;
	this->m_cpu = cpu;
	this->m_hFont = hFont;
	this->m_monitorEvent = monitorEvent;
	this->m_vic = vic;
	hr = this->m_mon.Init(cpu, vic);
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
	return S_OK;
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
		if (wParam==SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		if (wParam==SIZE_MINIMIZED)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		UpdateDisplay();
		return 0;
	case WM_CLOSE:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
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
			int x,y,yTitle,tx;
			int slen;			
			if (!m_monitorEvent->IsRunning(NULL))
			{
				UpdateBuffer(m_TextBuffer);

				yTitle = MARGIN_TOP + PADDING_TOP;
				y = yTitle + tm.tmHeight;
				x = PADDING_LEFT;


				SIZE sz;
				const int defwidth = 10;
				LPCTSTR szSampleWord = TEXT("$ABCDX");
				LPCTSTR szSampleByte = TEXT("ABX");
				LPCTSTR szSample3Bytes = TEXT("ABCX");
				LPCTSTR szSampleFlags = TEXT("NV-BDIZCX");
				LPCTSTR szSampleLine = TEXT("LINEX");
				int sp_word = (lstrlen(szSampleWord)) * tm.tmAveCharWidth;
				int sp_byte = (lstrlen(szSampleByte)) * tm.tmAveCharWidth;
				int sp_3bytes = (lstrlen(szSample3Bytes)) * tm.tmAveCharWidth;
				int sp_flags = (lstrlen(szSampleFlags)) * tm.tmAveCharWidth;
				int sp_line = (lstrlen(szSampleLine)) * tm.tmAveCharWidth;
				if (::GetTextExtentExPoint(hdc, szSampleWord, lstrlen(szSampleWord),0,NULL,NULL,&sz))
				{							
					sp_word = sz.cx;
				}
				if (::GetTextExtentExPoint(hdc, szSampleByte, lstrlen(szSampleByte),0,NULL,NULL,&sz))
				{							
					sp_byte = sz.cx;
				}
				if (::GetTextExtentExPoint(hdc, szSample3Bytes, lstrlen(szSample3Bytes),0,NULL,NULL,&sz))
				{							
					sp_3bytes = sz.cx;
				}
				if (::GetTextExtentExPoint(hdc, szSampleFlags, lstrlen(szSampleFlags),0,NULL,NULL,&sz))
				{							
					sp_flags = sz.cx;
				}
				if (::GetTextExtentExPoint(hdc, szSampleLine, lstrlen(szSampleLine),0,NULL,NULL,&sz))
				{							
					sp_line = sz.cx;
				}
				int xcol_PC = PADDING_LEFT;
				int xcol_A = xcol_PC+sp_word;
				int xcol_X = xcol_A+sp_byte;
				int xcol_Y = xcol_X+sp_byte;
				int xcol_Flags = xcol_Y+sp_byte;
				int xcol_SP = xcol_Flags+sp_flags;
				int xcol_00 = xcol_SP+sp_byte;
				int xcol_01 = xcol_00+sp_byte;
				int xcol_Line = xcol_01+sp_byte;
				int xcol_Cyc = xcol_Line+sp_line;

				TCHAR *tTitle;
				tTitle = TEXT("PC");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_PC, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.PC_Text, _countof(m_TextBuffer.PC_Text));
				G::DrawDefText(hdc, xcol_PC, y, m_TextBuffer.PC_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				tTitle = TEXT("A");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_A, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.A_Text, _countof(m_TextBuffer.A_Text));
				G::DrawDefText(hdc, xcol_A, y, m_TextBuffer.A_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				tTitle = TEXT("X");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_X, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.X_Text, _countof(m_TextBuffer.X_Text));
				G::DrawDefText(hdc, xcol_X, y, m_TextBuffer.X_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				tTitle = TEXT("Y");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_Y, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.Y_Text, _countof(m_TextBuffer.Y_Text));
				G::DrawDefText(hdc, xcol_Y, y, m_TextBuffer.Y_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				tTitle = TEXT("NV-BDIZC");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_Flags, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.SR_Text, _countof(m_TextBuffer.SR_Text));
				G::DrawDefText(hdc, xcol_Flags, y, m_TextBuffer.SR_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				tTitle = TEXT("SP");
				slen = (int)_tcslen(tTitle);
				G::DrawDefText(hdc, xcol_SP, yTitle, tTitle, slen, NULL, NULL);
				slen = (int)_tcsnlen(m_TextBuffer.SP_Text, _countof(m_TextBuffer.SP_Text));
				G::DrawDefText(hdc, xcol_SP, y, m_TextBuffer.SP_Text, slen, &x, NULL);
				x = x + tm.tmAveCharWidth * 1;

				if (m_cpu->GetCpuId()==0)
				{
					tTitle = TEXT("00");
					slen = (int)_tcslen(tTitle);
					G::DrawDefText(hdc, xcol_00, yTitle, tTitle, slen, NULL, NULL);
					slen = (int)_tcsnlen(m_TextBuffer.Ddr_Text, _countof(m_TextBuffer.Ddr_Text));
					G::DrawDefText(hdc, xcol_00, y, m_TextBuffer.Ddr_Text, slen, &x, NULL);
					x = x + tm.tmAveCharWidth * 1;

					tTitle = TEXT("01");
					slen = (int)_tcslen(tTitle);
					G::DrawDefText(hdc, xcol_01, yTitle, tTitle, slen, NULL, NULL);
					slen = (int)_tcsnlen(m_TextBuffer.Data_Text, _countof(m_TextBuffer.Data_Text));
					G::DrawDefText(hdc, xcol_01, y, m_TextBuffer.Data_Text, slen, &x, NULL);
					x = x + tm.tmAveCharWidth * 1;

					RECT rcText;

					tTitle = TEXT("LINE");
					slen = (int)_tcslen(tTitle);
					G::DrawDefText(hdc, xcol_Line, yTitle, tTitle, slen, &tx, NULL);

					SetRect(&rcText, xcol_Line, y, tx, y+tm.tmHeight+1);

					slen = (int)_tcsnlen(m_TextBuffer.VicLine_Text, _countof(m_TextBuffer.VicLine_Text));
					::DrawText(hdc, m_TextBuffer.VicLine_Text, slen, &rcText, DT_TOP | DT_RIGHT | DT_SINGLELINE);
					//G::DrawDefText(hdc, xcol_Line+tm.tmAveCharWidth, y, m_TextBuffer.VicLine_Text, slen, &tx, NULL);

					tTitle = TEXT("CYC");
					slen = (int)_tcslen(tTitle);
					G::DrawDefText(hdc, xcol_Cyc, yTitle, tTitle, slen, &tx, NULL);
					SetRect(&rcText, xcol_Cyc, y, tx, y+tm.tmHeight+1);
					slen = (int)_tcsnlen(m_TextBuffer.VicCycle_Text, _countof(m_TextBuffer.VicCycle_Text));
					::DrawText(hdc, m_TextBuffer.VicCycle_Text, slen, &rcText, DT_TOP | DT_RIGHT | DT_SINGLELINE);
					//G::DrawDefText(hdc, xcol_Cyc+tm.tmAveCharWidth, y, m_TextBuffer.VicCycle_Text, slen, &tx, NULL);
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
		}
	}

}

void CDisassemblyReg::InvalidateBuffer()
{
	m_TextBuffer.ClearBuffer();
	if (!m_hWnd)
		InvalidateRect(m_hWnd, NULL, TRUE);
}

void CDisassemblyReg::UpdateDisplay()
{
	UpdateBuffer(m_TextBuffer);
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);

}

void CDisassemblyReg::UpdateBuffer(RegLineBuffer& b)
{
	b.ClearBuffer();
	m_mon.GetCpuRegisters(b.PC_Text, _countof(b.PC_Text), b.A_Text, _countof(b.A_Text), b.X_Text, _countof(b.X_Text), b.Y_Text, _countof(b.Y_Text), b.SR_Text, _countof(b.SR_Text), b.SP_Text, _countof(b.SP_Text), b.Ddr_Text, _countof(b.Ddr_Text), b.Data_Text, _countof(b.Data_Text));
	m_mon.GetVicRegisters(b.VicLine_Text, _countof(b.VicLine_Text), b.VicCycle_Text, _countof(b.VicCycle_Text));
}

void CDisassemblyReg::RegLineBuffer::ClearBuffer()
{
	PC_Text[0]=0;
	A_Text[0]=0;
	X_Text[0]=0;
	Y_Text[0]=0;
	SR_Text[0]=0;
	SP_Text[0]=0;
	Ddr_Text[0]=0;
	Data_Text[0]=0;
	Output_Text[0]=0;
	Input_Text[0]=0;
	Mmu_Text[0]=0;
	VicLine_Text[0];
	VicCycle_Text[0];
}