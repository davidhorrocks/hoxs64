#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "register.h"
#include "diagnewblankdisk.h"
#include "resource.h"


CDiagNewBlankDisk::CDiagNewBlankDisk()
{
}

CDiagNewBlankDisk::~CDiagNewBlankDisk()
{
}

HRESULT CDiagNewBlankDisk::Init()
{
	ClearError();
	return S_OK;
}

BOOL CDiagNewBlankDisk::DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
BOOL br;

	switch (message) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_DISKNAME, TEXT("HOXS64"));
		SetDlgItemText(hwndDlg, IDC_BYTEID1, TEXT("54"));
		SetDlgItemText(hwndDlg, IDC_BYTEID2, TEXT("52"));
		SendDlgItemMessage(hwndDlg, IDC_DISKNAME, EM_SETLIMITTEXT, DISKNAMELENGTH, 0);
		SendDlgItemMessage(hwndDlg, IDC_BYTEID1, EM_SETLIMITTEXT, 3, 0);
		SendDlgItemMessage(hwndDlg, IDC_BYTEID2, EM_SETLIMITTEXT, 3, 0);
		CheckDlgButton(hwndDlg, IDC_CHK_ALIGNTRACKS, BST_CHECKED);
		CheckRadioButton(hwndDlg, IDC_RAD_TRACKS35, IDC_RAD_TRACKS35, IDC_RAD_TRACKS35);
		return TRUE;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDOK: 
			diskname[0] = TEXT('\0');
			GetDlgItemText(hwndDlg, IDC_DISKNAME, &diskname[0], _countof(diskname));
			id1 = GetDlgItemInt(hwndDlg, IDC_BYTEID1, &br, FALSE);
			id2 = GetDlgItemInt(hwndDlg, IDC_BYTEID2, &br, FALSE);
			bAlignTracks = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ALIGNTRACKS) != BST_UNCHECKED) ? true : false;
			numberOfTracks = (IsDlgButtonChecked(hwndDlg, IDC_RAD_TRACKS35) != BST_UNCHECKED) ? 35 : 40;
			EndDialog(hwndDlg, wParam); 
			return TRUE; 
		case IDCANCEL: 
			EndDialog(hwndDlg, wParam); 
			return TRUE; 
		} 
	} 
	return FALSE; 
} 
