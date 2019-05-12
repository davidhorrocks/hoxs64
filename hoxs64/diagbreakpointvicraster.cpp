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
#include "IC64.h"
#include "CDPI.h"
#include "utils.h"
#include "resource.h"
#include "diagbreakpointvicraster.h"


CDiagBreakpointVicRaster::CDiagBreakpointVicRaster(IAppCommand *pIAppCommand, IC64 *c64)
{
	m_iLine = 0;
	m_iCycle = 1;
	m_pIAppCommand = pIAppCommand;
	this->c64 = c64;
	m_bInOnVicCursorChange = false;
	m_bInOnTextChange = false;
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
	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], DBGSYM::MonitorOption::Dec, &cycle);
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

	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE_DEC, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], DBGSYM::MonitorOption::Dec, &line);
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
		SetFocus(GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE_DEC));
		return false;
	}

	bCycleOK = TryGetCycle(m_iCycle);
	if (!bCycleOK)
	{
		this->ShowMessage(this->m_hWnd, MB_OK | MB_ICONWARNING, TEXT("Invalid Raster Cycle"), TEXT("Raster cycle must be in the range %d - %d"), 1, PAL_CLOCKS_PER_LINE);
		SetFocus(GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC));
		return false;
	}
	return true;
}

void CDiagBreakpointVicRaster::InitControls(HWND hWndDlg)
{
HWND hWndEdit;
	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE_DEC, TEXT("0"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE_DEC);
	if (hWndEdit)
	{
		SendMessage(hWndEdit, EM_SETLIMITTEXT, 10, 0);
		SendMessage(hWndEdit, EM_SETSEL, 0, -1);
	}

	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE_HEX, TEXT("0"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE_HEX);
	if (hWndEdit)
	{
		SendMessage(hWndEdit, EM_SETLIMITTEXT, 10, 0);
		SendMessage(hWndEdit, EM_SETSEL, 0, -1);
	}

	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC, TEXT("1"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC);
	if (hWndEdit)
	{
		SendMessage(hWndEdit, EM_SETLIMITTEXT, 10, 0);
		SendMessage(hWndEdit, EM_SETSEL, 0, -1);
	}

	SetDlgItemText(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE_HEX, TEXT("1"));
	hWndEdit = GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE_HEX);
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
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC, buffer);

	_stprintf_s(buffer, _countof(buffer), TEXT("%X"), (int)e.Cycle);
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE_HEX, buffer);

	_stprintf_s(buffer, _countof(buffer), TEXT("%d"), (int)e.Line);
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE_DEC, buffer);

	_stprintf_s(buffer, _countof(buffer), TEXT("%X"), (int)e.Line);
	::SetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE_HEX, buffer);

	//UpdateWindow(::GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC));
	//UpdateWindow(::GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE_DEC));
	m_bInOnVicCursorChange = false;
}

void CDiagBreakpointVicRaster::SetVicCursor()
{
	if (m_bInOnVicCursorChange)
	{
		return;
	}

	if (!TryGetLine(m_iLine))
	{
		m_iLine = 0;
	}

	if (!TryGetCycle(m_iCycle))
	{
		m_iCycle = 1;
	}

	m_pIAppCommand->SetVicCursorPos(m_iCycle, m_iLine);
	m_pIAppCommand->UpdateEmulationDisplay();
}

void CDiagBreakpointVicRaster::UpdateHexText(int textDecId, int textHexId)
{
TCHAR szNumber[20];
	if (m_bInOnVicCursorChange)
	{
		return;
	}

	if (m_bInOnTextChange)
	{
		return;
	}

	HRESULT hr;
	bit16 v = 0;
	int r;
	m_bInOnTextChange = true;
	bool ok = false;
	r = ::GetDlgItemText(this->GetHwnd(), textDecId, szNumber, _countof(szNumber));
	if (r > 0)
	{
		hr = Assembler::TryParseAddress16(szNumber, DBGSYM::MonitorOption::Dec, &v);
		if (SUCCEEDED(hr))
		{
			ok = true;
		}
	}

	if (ok)
	{
		szNumber[0] = 0;
		_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%X"), (unsigned int)v);
		::SetDlgItemText(this->GetHwnd(), textHexId, szNumber);
	}
	else
	{
		::SetDlgItemText(this->GetHwnd(), textHexId, G::EmptyString);
	}

	m_bInOnTextChange = false;
}

void CDiagBreakpointVicRaster::UpdateDecText(int textHexId, int textDecId)
{
TCHAR szNumber[20];
	if (m_bInOnVicCursorChange)
	{
		return;
	}

	if (m_bInOnTextChange)
	{
		return;
	}

	HRESULT hr;
	bit16 v = 0;
	int r;
	m_bInOnTextChange = true;
	bool ok = false;
	r = ::GetDlgItemText(this->GetHwnd(), textHexId, szNumber, _countof(szNumber));
	if (r > 0)
	{
		hr = Assembler::TryParseAddress16(szNumber, DBGSYM::MonitorOption::Hex, &v);
		if (SUCCEEDED(hr))
		{
			ok = true;
		}
	}

	if (ok)
	{
		szNumber[0] = 0;
		_sntprintf_s(szNumber, _countof(szNumber), _TRUNCATE, TEXT("%-d"), (unsigned int)v);
		::SetDlgItemText(this->GetHwnd(), textDecId, szNumber);
	}
	else
	{
		::SetDlgItemText(this->GetHwnd(), textDecId, G::EmptyString);
	}

	m_bInOnTextChange = false;
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
					this->c64->GetMon()->GetVic()->SetBreakpointRasterCompare(this->GetRasterLine(), this->GetRasterCycle(), true, 0, 0);
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
		case IDC_TXT_BREAKPOINTRASTERLINE_DEC:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				UpdateHexText(IDC_TXT_BREAKPOINTRASTERLINE_DEC, IDC_TXT_BREAKPOINTRASTERLINE_HEX);
				break;
			}
			
			break;
		case IDC_TXT_BREAKPOINTRASTERCYCLE_DEC:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				UpdateHexText(IDC_TXT_BREAKPOINTRASTERCYCLE_DEC, IDC_TXT_BREAKPOINTRASTERCYCLE_HEX);
				break;
			}
			
			break;
		case IDC_TXT_BREAKPOINTRASTERLINE_HEX:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				UpdateDecText(IDC_TXT_BREAKPOINTRASTERLINE_HEX, IDC_TXT_BREAKPOINTRASTERLINE_DEC);
				break;
			}

			break;
		case IDC_TXT_BREAKPOINTRASTERCYCLE_HEX:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				SetVicCursor();
				UpdateDecText(IDC_TXT_BREAKPOINTRASTERCYCLE_HEX, IDC_TXT_BREAKPOINTRASTERCYCLE_DEC);
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
