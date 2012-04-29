#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include <winuser.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "assembler.h"
#include "resource.h"
#include "diagbreakpointvicraster.h"


CDiagBreakpointVicRaster::CDiagBreakpointVicRaster()
{
	m_iLine = 0;
	m_iCycle = 1;
}

CDiagBreakpointVicRaster::~CDiagBreakpointVicRaster()
{
}

int CDiagBreakpointVicRaster::GetRasterLine()
{
	return m_iLine;
}

int CDiagBreakpointVicRaster::GetRasterCycle()
{
	return m_iCycle;
}

bool CDiagBreakpointVicRaster::SaveUI()
{
TCHAR buffer[30];
int c;
bit16 line, cycle;
bool bLineOK = false;
bool bCycleOK = false;
HRESULT hr;
	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], &line);
		if (SUCCEEDED(hr))
		{
			if (line >= 0 && line < PALLINESPERFRAME)
			{
				m_iLine = line;
				bLineOK = true;
			}
		}
	}
	if (!bLineOK)
	{
		this->ShowMessage(this->m_hWnd, MB_OK | MB_ICONWARNING, TEXT("Invalid Raster Line"), TEXT("Raster line must be in the range %d - %d"), 0, PALLINESPERFRAME - 1);
		SetFocus(GetDlgItem(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE));
		return false;
	}

	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERCYCLE, &buffer[0], _countof(buffer));
	if (c > 0)
	{
		hr = Assembler::TryParseAddress16(&buffer[0], &cycle);
		if (SUCCEEDED(hr))
		{
			if (cycle >= 1 && cycle <= PALCLOCKSPERLINE)
			{
				m_iCycle = cycle;
				bCycleOK = true;
			}
		}
	}
	if (!bCycleOK)
	{
		this->ShowMessage(this->m_hWnd, MB_OK | MB_ICONWARNING, TEXT("Invalid Raster Cycle"), TEXT("Raster cycle must be in the range %d - %d"), 0, PALCLOCKSPERLINE);
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
}

BOOL CDiagBreakpointVicRaster::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		InitControls(hWndDlg);
		//FillDevices();
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//saveconfig(&newCfg);
			if (SaveUI())
				EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDC_TXT_BREAKPOINTRASTERLINE:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				PostMessage(GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERLINE), EM_SETSEL, 0, -1);
				break;
			}
			break;
		case IDC_TXT_BREAKPOINTRASTERCYCLE:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				PostMessage(GetDlgItem(hWndDlg, IDC_TXT_BREAKPOINTRASTERCYCLE), EM_SETSEL, 0, -1);
				break;
			}
			break;
		//case IDC_CBO_JOY1DEVICE:
		//	switch (HIWORD(wParam))
		//	{
		//	case CBN_SELCHANGE:
		//		FillJoy1Axis(FALSE);
		//		FillJoy1Button(FALSE);
		//		return TRUE;
		//	}
		//	break;
		}
	case WM_DESTROY:
		return TRUE;
	}
	return FALSE;
}
