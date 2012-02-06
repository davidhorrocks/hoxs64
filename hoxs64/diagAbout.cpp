#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <windowsx.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include "servicerelease.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "diagabout.h"
#include "resource.h"


CDiagAbout::CDiagAbout()
{
}

CDiagAbout::~CDiagAbout()
{
}


HRESULT CDiagAbout::Init(VS_FIXEDFILEINFO *pVinfo)
{
	m_pVinfo = pVinfo;
	return S_OK;
}

BOOL CDiagAbout::DialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
TCHAR textTotal[300];
TCHAR textTemp[300];
	switch (message) 
	{ 
	case WM_INITDIALOG:
		textTotal[0] = 0;
		textTemp[0] = 0;
#if defined(_WIN64)
		_tcsncpy_s(textTotal, _countof(textTotal), TEXT("64 bit version:"), _TRUNCATE);
#elif _WIN32_WINNT >= 0x0400
		_tcsncpy_s(textTotal, _countof(textTotal), TEXT("32 bit version:"), _TRUNCATE);
#else
		_tcsncpy_s(textTotal, _countof(textTotal), TEXT("Win 98 version:"), _TRUNCATE);
#endif
		if (m_pVinfo)
		{
			int len = lstrlen(textTotal);
#if defined(SERVICERELEASE) && (SERVICERELEASE > 0)
		_sntprintf_s(textTemp, _countof(textTemp), _TRUNCATE, TEXT("    V %d.%d.%d.%d SR %d")
			, (int)(m_pVinfo->dwProductVersionMS>>16 & 0xFF)
			, (int)(m_pVinfo->dwProductVersionMS & 0xFF)
			, (int)(m_pVinfo->dwProductVersionLS>>16 & 0xFF)
			, (int)(m_pVinfo->dwProductVersionLS & 0xFF)
			, (int)SERVICERELEASE);
#else
			_sntprintf_s(textTemp, _countof(textTemp), _TRUNCATE, TEXT("    V %d.%d.%d.%d")
				,m_pVinfo->dwProductVersionMS>>16 & 0xFF
				,m_pVinfo->dwProductVersionMS & 0xFF
				,m_pVinfo->dwProductVersionLS>>16 & 0xFF
				,m_pVinfo->dwProductVersionLS & 0xFF);

#endif
			_tcscat_s(textTotal, _countof(textTotal), textTemp);
		}
		else
		{
			_tcscat_s(textTotal, _countof(textTotal), TEXT(" UNKNOWN"));
		}
		SetDlgItemText(hWndDlg, IDC_VERSION,textTotal);
		return TRUE;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDOK: 
			EndDialog(hWndDlg, wParam); 
			return TRUE; 
		case IDCANCEL: 
			EndDialog(hWndDlg, wParam); 
			return TRUE; 
		} 
	} 
	return FALSE; 
}
