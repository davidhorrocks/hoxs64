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
#include "resource.h"
#include "diagbreakpointvicraster.h"


CDiagBreakpointVicRaster::CDiagBreakpointVicRaster()
{
}

CDiagBreakpointVicRaster::~CDiagBreakpointVicRaster()
{
}

int CDiagBreakpointVicRaster::GetRasterLine()
{
	return S_OK;
}

int CDiagBreakpointVicRaster::GetRasterCycle()
{
	return S_OK;
}

HRESULT CDiagBreakpointVicRaster::SavePosition()
{
TCHAR buffer[30];
int c;
	c = ::GetDlgItemText(this->m_hWnd, IDC_TXT_BREAKPOINTRASTERLINE, &buffer[0], _countof(buffer));
	if (c>0)
	{

	}
	return E_FAIL;
}

BOOL CDiagBreakpointVicRaster::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		//FillDevices();
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//saveconfig(&newCfg);
			EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, wParam);
			return TRUE;
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
