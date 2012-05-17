#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include <winuser.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "C64.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assembler.h"
#include "resource.h"
#include "diagbreakpointvicraster.h"


CDiagBreakpointVicRaster::CDiagBreakpointVicRaster(IAppCommand *pIAppCommand, C64 *c64)
{
	m_iLine = 0;
	m_iCycle = 1;
	m_pIAppCommand = pIAppCommand;
	this->c64 = c64;
	m_bInOnVicCursorChange = false;
}

CDiagBreakpointVicRaster::~CDiagBreakpointVicRaster()
{
	int x = 0;
	x++;
}

int CDiagBreakpointVicRaster::GetRasterLine()
{
	return m_iLine;
}

int CDiagBreakpointVicRaster::GetRasterCycle()
{
	return m_iCycle;
}

bool CDiagBreakpointVicRaster::TryGetCycle(int& v)
{
TCHAR buffer[30];
int c;
bit16 cycle;
HRESULT hr;

	v = 1;
	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], &cycle);
		if (SUCCEEDED(hr))
		{
			if (cycle >= 1 && cycle <= PAL_CLOCKS_PER_LINE)
			{
				v = cycle;
				return true;
			}
		}
	}
	return false;
}

bool CDiagBreakpointVicRaster::TryGetLine(int& v)
{
TCHAR buffer[30];
int c;
bit16 line;
HRESULT hr;

	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], &line);
		if (SUCCEEDED(hr))
		{
			if (line >= 0 && line < PAL_LINES_PER_FRAME)
			{
				v = line;
				return true;
			}
		}
	}
	return false;
}

bool CDiagBreakpointVicRaster::SaveUI()
{
bool bLineOK = false;
bool bCycleOK = false;

	bLineOK = TryGetLine(m_iLine);
	if (!bLineOK)
	{
		this->ShowMessage(this->m_hWnd, MB_OK | MB_ICONWARNING, TEXT("Invalid Raster Line"), TEXT("Raster line must be in the range %d - %d"), 0, PAL_LINES_PER_FRAME - 1);
		SetFocus(GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE));
		return false;
	}
	bCycleOK = TryGetCycle(m_iCycle);
	if (!bCycleOK)
	{
		this->ShowMessage(this->m_hWnd, MB_OK | MB_ICONWARNING, TEXT("Invalid Raster Cycle"), TEXT("Raster cycle must be in the range %d - %d"), 0, PAL_CLOCKS_PER_LINE);
		SetFocus(GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE));
		return false;
	}
	return true;
}

void CDiagBreakpointVicRaster::InitControls(HWND hWndDlg)
{
HWND hWndEdit;
	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE, TEXT("0"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE);
	if (hWndEdit)
	{
		SendMessage(hWndEdit, EM_SETLIMITTEXT, 10, 0);
		SendMessage(hWndEdit, EM_SETSEL, 0, -1);
	}

	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE, TEXT("1"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE);
	if (hWndEdit)
	{
		SendMessage(hWndEdit, EM_SETLIMITTEXT, 10, 0);
		SendMessage(hWndEdit, EM_SETSEL, 0, -1);
	}
	this->m_pIAppCommand->DisplayVicCursor(true);
	this->m_pIAppCommand->DisplayVicRasterBreakpoints(true);
	this->m_pIAppCommand->UpdateEmulationDisplay();
	this->m_pIAppCommand->EsVicCursorMove.Advise((CDiagBreakpointVicRaster_EventSink_OnVicCursorChange *)this);
}

void CDiagBreakpointVicRaster::OnVicCursorChange(void *sender, VicCursorMoveEventArgs& e)
{
TCHAR buffer[30];
	m_bInOnVicCursorChange = true;
	_stprintf_s(buffer, _countof(buffer), TEXT("%d"), (int)e.Cycle);
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE, buffer);

	_stprintf_s(buffer, _countof(buffer), TEXT("%d"), (int)e.Line);
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE, buffer);

	UpdateWindow(::GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE));
	UpdateWindow(::GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE));
	m_bInOnVicCursorChange = false;
}

void CDiagBreakpointVicRaster::SetVicCursor()
{
	if (m_bInOnVicCursorChange)
		return;
	TryGetLine(m_iLine);
	TryGetCycle(m_iCycle);
	m_pIAppCommand->SetVicCursorPos(m_iCycle, m_iLine);
	m_pIAppCommand->UpdateEmulationDisplay();
}

BOOL CDiagBreakpointVicRaster::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		InitControls(hWndDlg);
		this->m_pIAppCommand->UpdateEmulationDisplay();
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (SaveUI())
			{
				if (m_bIsModeless)
				{
					this->c64->mon.GetVic()->SetBreakpointRasterCompare(this->GetRasterLine(), this->GetRasterCycle(), true, 0, 0);
					this->m_pIAppCommand->ShowDevelopment();
					DestroyWindow(hWndDlg);
				}
				else
				{
					EndDialog(hWndDlg, wParam);
				}
			}
			return TRUE;
		case IDCANCEL:
			if (m_bIsModeless)
			{
				this->m_pIAppCommand->ShowDevelopment();
				DestroyWindow(hWndDlg);
			}
			else
			{
				EndDialog(hWndDlg, wParam);
			}
			return TRUE;
		case IDC_TXT_BREAKPOINTRASTERLINE:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				break;
			}
			break;
		case IDC_TXT_BREAKPOINTRASTERCYCLE:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				break;
			}
			break;
		}
		break;
	case WM_DESTROY:
		CDiagBreakpointVicRaster_EventSink_OnVicCursorChange::UnadviseAll();
		this->m_pIAppCommand->DisplayVicCursor(false);
		this->m_pIAppCommand->DisplayVicRasterBreakpoints(false);
		this->m_pIAppCommand->UpdateEmulationDisplay();
		break;
	}
	return FALSE;
}
