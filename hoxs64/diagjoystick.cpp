#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include <winuser.h>
#include <commctrl.h>
#include <tchar.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "register.h"
#include "resource.h"
#include "diagjoystick.h"

CDiagJoystick::CDiagJoystick()
{
}

CDiagJoystick::~CDiagJoystick()
{
}

HRESULT CDiagJoystick::Init(CDX9 *pDX, const CConfig *currentCfg)
{
	ClearError();
	CDiagJoystick::currentCfg = currentCfg;

	newCfg = *currentCfg;

	CDiagJoystick::pDX = pDX;
	return S_OK;
}

void CDiagJoystick::loadconfig(const CConfig *cfg)
{
	if (cfg == NULL)
		return;
	if (cfg->m_joy1config.bEnabled)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLE, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLE, BST_UNCHECKED);

	if (cfg->m_joy2config.bEnabled)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLE, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLE, BST_UNCHECKED);


	if (cfg->m_joy1config.bXReverse)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1HREV, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1HREV, BST_UNCHECKED);

	if (cfg->m_joy1config.bYReverse)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1VREV, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1VREV, BST_UNCHECKED);

	if (cfg->m_joy2config.bXReverse)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2HREV, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2HREV, BST_UNCHECKED);

	if (cfg->m_joy2config.bYReverse)
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2VREV, BST_CHECKED);
	else
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2VREV, BST_UNCHECKED);
}

void CDiagJoystick::saveconfig(CConfig *cfg)
{
LRESULT lr;


	cfg->m_joy1config.bValid = FALSE;
	cfg->m_joy2config.bValid = FALSE;

	cfg->m_joy1config.dwOfs_X = 0;
	cfg->m_joy1config.dwOfs_Y = 0;
	cfg->m_joy1config.dwOfs_firebutton = DIJOFS_BUTTON0;

	cfg->m_joy2config.dwOfs_X = 0;
	cfg->m_joy2config.dwOfs_Y = 0;
	cfg->m_joy2config.dwOfs_firebutton = DIJOFS_BUTTON0;

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETCURSEL, 0, 0);
	if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevArray.Count())
			{
				cfg->m_joy1config.joystickID = DevArray[(ULONG)lr].guidInstance;
				cfg->m_joy1config.bValid = TRUE;
			}
		}
	}
	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevArray.Count())
			{
				cfg->m_joy2config.joystickID = DevArray[(ULONG)lr].guidInstance;
				cfg->m_joy2config.bValid = TRUE;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevAxis1Array.Count())
			{
				cfg->m_joy1config.dwOfs_Y = DevAxis1Array[(ULONG)lr].dwOfs;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevAxis1Array.Count())
			{
				cfg->m_joy1config.dwOfs_X = DevAxis1Array[(ULONG)lr].dwOfs;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevButton1Array.Count())
			{
				cfg->m_joy1config.dwOfs_firebutton = DevButton1Array[(ULONG)lr].dwOfs;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevAxis2Array.Count())
			{
				cfg->m_joy2config.dwOfs_Y = DevAxis2Array[(ULONG)lr].dwOfs;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevAxis2Array.Count())
			{
				cfg->m_joy2config.dwOfs_X = DevAxis2Array[(ULONG)lr].dwOfs;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < DevButton2Array.Count())
			{
				cfg->m_joy2config.dwOfs_firebutton = DevButton2Array[(ULONG)lr].dwOfs;
			}
		}
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1ENABLE))
		cfg->m_joy1config.bEnabled = TRUE;
	else
		cfg->m_joy1config.bEnabled = FALSE;

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2ENABLE))
		cfg->m_joy2config.bEnabled = TRUE;
	else
		cfg->m_joy2config.bEnabled = FALSE;

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1VREV))
		cfg->m_joy1config.bYReverse = TRUE;
	else
		cfg->m_joy1config.bYReverse = FALSE;

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2VREV))
		cfg->m_joy2config.bYReverse = TRUE;
	else
		cfg->m_joy2config.bYReverse = FALSE;

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1HREV))
		cfg->m_joy1config.bXReverse = TRUE;
	else
		cfg->m_joy1config.bXReverse = FALSE;

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2HREV))
		cfg->m_joy2config.bXReverse = TRUE;
	else
		cfg->m_joy2config.bXReverse = FALSE;
}

BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumDevices(lpddi);
}

BOOL CALLBACK EnumDlgJoy1AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumJoy1Axis(lpddoi);
}

BOOL CALLBACK EnumDlgJoy2AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumJoy2Axis(lpddoi);
}

BOOL CALLBACK EnumDlgJoy1ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumJoy1Button(lpddoi);
}

BOOL CALLBACK EnumDlgJoy2ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumJoy2Button(lpddoi);
}

HRESULT CDiagJoystick::FillDevices()
{
HWND hWnd1, hWnd2;
RECT rc1,rc2;
const int resize_value=10;
HRESULT r;

	DevArray.Clear();
	r = DevArray.Resize(resize_value);
	if (FAILED(r))
		return r;


	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_RESETCONTENT, 0, 0);

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETDROPPEDWIDTH, 250, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_SETDROPPEDWIDTH, 250, 0);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1DEVICE);
	if (!hWnd1)
		return E_FAIL;
	hWnd2 = GetDlgItem(m_hWnd, IDC_CBO_JOY2DEVICE);
	if (!hWnd2)
		return E_FAIL;

	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	if(!GetWindowRect(hWnd2, &rc2))
		return E_FAIL;
	SetWindowPos(hWnd2,0,0,0,rc2.right - rc2.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);


	
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1FIRE);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2FIRE);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1V);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1H);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);


	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2V);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2H);
	if (!hWnd1)
		return E_FAIL;
	if(!GetWindowRect(hWnd1, &rc1))
		return E_FAIL;
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1DEVICE);
	if (hWnd1)
		SetForegroundWindow(hWnd1);

	r = pDX->EnumDevices(DIDEVTYPE_JOYSTICK, EnumDlgJoyCallback, this, DIEDFL_ATTACHEDONLY);	
	return r;
}

BOOL CDiagJoystick::EnumDevices(LPCDIDEVICEINSTANCE lpddi)
{
LRESULT lr;
HRESULT hr;
unsigned int i;

	hr = DevArray.Append((const DIDEVICEINSTANCE)*lpddi, &i);
	if(FAILED(hr))
		return DIENUM_STOP;

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_ADDSTRING, 0, (LPARAM) &lpddi->tszProductName[0]);
	if (lr >= 0)
	{
		SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETITEMDATA, lr, (LPARAM) i);
		if (newCfg.m_joy1config.bValid)
			if (newCfg.m_joy1config.joystickID == lpddi->guidInstance)
			{
				SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETCURSEL, lr, 0);
				FillJoy1Axis(TRUE);
				FillJoy1Button(TRUE);
			}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_ADDSTRING, 0, (LPARAM) &lpddi->tszProductName[0]);
	if (lr >= 0)
	{
		SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_SETITEMDATA, lr, (LPARAM) i);
		if (newCfg.m_joy2config.bValid)
			if (newCfg.m_joy2config.joystickID == lpddi->guidInstance)
			{
				SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_SETCURSEL, lr, 0);
				FillJoy2Axis(TRUE);
				FillJoy2Button(TRUE);
			}
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		FillDevices();
		loadconfig(&newCfg);
		return TRUE;
	case WM_ACTIVATE:
		if( WA_INACTIVE != wParam )
		{
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			saveconfig(&newCfg);
			EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDC_CBO_JOY1DEVICE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillJoy1Axis(FALSE);
				FillJoy1Button(FALSE);
				return TRUE;
			}
			break;
		case IDC_CBO_JOY2DEVICE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillJoy2Axis(FALSE);
				FillJoy2Button(FALSE);
				return TRUE;
			}
			break;
		case IDC_CBO_JOY1ENABLE:
			if (BN_CLICKED == HIWORD(wParam))
			{
				if (IsDlgButtonChecked(hWndDlg, IDC_CBO_JOY1ENABLE) == BST_UNCHECKED)
				{
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETCURSEL, -1, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_RESETCONTENT, 0, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_RESETCONTENT, 0, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_RESETCONTENT, 0, 0);
				}
				return TRUE;
			}
			break;
		case IDC_CBO_JOY2ENABLE:
			if (BN_CLICKED == HIWORD(wParam))
			{
				if (IsDlgButtonChecked(hWndDlg, IDC_CBO_JOY2ENABLE) == BST_UNCHECKED)
				{
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_SETCURSEL, -1, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_RESETCONTENT, 0, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_RESETCONTENT, 0, 0);
					SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_RESETCONTENT, 0, 0);
				}
				return TRUE;
			}
			break;
		}
	case WM_DESTROY:
		return TRUE;
	}
	return FALSE;
}

void CDiagJoystick::FillJoy1Button(BOOL bSetConfig)
{
HRESULT hr;
LRESULT i,j,k;
const int resize_value=10;
HRESULT r;

	bJoy1AxisSetConfig = bSetConfig;
	bGotDefault1Fire = FALSE;

	DevButton1Array.Clear();
	r = DevButton1Array.Resize(resize_value);
	if (FAILED(r))
		return;

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_RESETCONTENT, 0, 0);

	i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;
	i= SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETITEMDATA, i, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;

	if (DevArray.Count() <= (ULONG)i)
		return;

	hr = pDX->CreateDeviceJoy(0, DevArray[(ULONG)i].guidInstance);
	if (FAILED(hr)) return;

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr)) return;

	pDX->EnumObjectsJoy(0, EnumDlgJoy1ButtonCallback, this, DIDFT_BUTTON);

	if (!bSetConfig && bGotDefault1Fire)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
					if (k == default1Fire)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_SETCURSEL, j, 0);
			}
		}
	}
}

void CDiagJoystick::FillJoy2Button(BOOL bSetConfig)
{
HRESULT hr;
LRESULT i,j,k;
const int resize_value=10;
HRESULT r;

	bJoy1AxisSetConfig = bSetConfig;
	bGotDefault2Fire = FALSE;

	DevButton2Array.Clear();
	r = DevButton2Array.Resize(resize_value);
	if (FAILED(r))
		return;

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_RESETCONTENT, 0, 0);

	i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;
	i= SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETITEMDATA, i, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;

	if (DevArray.Count() <= (ULONG)i)
		return;

	hr = pDX->CreateDeviceJoy(0, DevArray[(ULONG)i].guidInstance);
	if (FAILED(hr)) return;

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr)) return;

	pDX->EnumObjectsJoy(0, EnumDlgJoy2ButtonCallback, this, DIDFT_BUTTON);

	if (!bSetConfig && bGotDefault2Fire)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k < MAXLONG)
					if (k == default2Fire)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_SETCURSEL, j, 0);
			}
		}
	}
}

void CDiagJoystick::FillJoy1Axis(BOOL bSetConfig)
{
HRESULT hr;
LRESULT i,j,k;
const int resize_value=10;
HRESULT r;

	bJoy1AxisSetConfig = bSetConfig;

	bGotDefault1X = FALSE;
	bGotDefault1Y = FALSE;

	DevAxis1Array.Clear();
	r = DevAxis1Array.Resize(resize_value);
	if (FAILED(r))
		return;

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_RESETCONTENT, 0, 0);

	i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;
	i= SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_GETITEMDATA, i, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;

	if (DevArray.Count() <= (ULONG)i)
		return;

	hr = pDX->CreateDeviceJoy(0, DevArray[(ULONG)i].guidInstance);
	if (FAILED(hr)) return;

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr)) return;

	pDX->EnumObjectsJoy(0, EnumDlgJoy1AxisCallback, this, DIDFT_ABSAXIS);

	if (!bSetConfig && bGotDefault1X)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
					if (k == default1X)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_SETCURSEL, j, 0);
			}
		}
	}
	if (!bSetConfig && bGotDefault1Y)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
					if (k == default1Y)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_SETCURSEL, j, 0);
			}
		}
	}
}

void CDiagJoystick::FillJoy2Axis(BOOL bSetConfig)
{
HRESULT hr;
LRESULT i,j,k;
const int resize_value=10;
HRESULT r;

	bJoy2AxisSetConfig = bSetConfig;

	bGotDefault2X = FALSE;
	bGotDefault2Y = FALSE;

	DevAxis2Array.Clear();
	r = DevAxis2Array.Resize(resize_value);
	if (FAILED(r))
		return;

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_RESETCONTENT, 0, 0);

	i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;
	i= SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_GETITEMDATA, i, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
		return;

	if (DevArray.Count() <= (ULONG)i)
		return;

	hr = pDX->CreateDeviceJoy(0, DevArray[(ULONG)i].guidInstance);
	if (FAILED(hr)) return;

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr)) return;

	pDX->EnumObjectsJoy(0, EnumDlgJoy2AxisCallback, this, DIDFT_ABSAXIS);

	if (!bSetConfig && bGotDefault2X)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
					if (k == default2X)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_SETCURSEL, j, 0);
			}
		}
	}
	if (!bSetConfig && bGotDefault2Y)
	{
		i = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >=0 && k < MAXLONG)
					if (k == default2Y)
						SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_SETCURSEL, j, 0);
			}
		}
	}
}

BOOL CDiagJoystick::EnumJoy1Button(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
LRESULT lr;
unsigned int i;

	hr = DevButton1Array.Append((const DIDEVICEOBJECTINSTANCE)*lpddoi, &i);
	if(FAILED(hr))
		return DIENUM_CONTINUE;


	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy1AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy1config.dwOfs_firebutton)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_SETCURSEL, lr, 0);
		}
	}

	
	if (bGotDefault1Fire == FALSE)
		default1Fire = i;
	else
		if (DevButton1Array[default1Fire].dwOfs > lpddoi->dwOfs)
			default1Fire = i;
	bGotDefault1Fire = TRUE;
		
	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::EnumJoy2Button(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
LRESULT lr;
unsigned int i;

	hr = DevButton2Array.Append((const DIDEVICEOBJECTINSTANCE)*lpddoi, &i);
	if(FAILED(hr))
		return DIENUM_CONTINUE;


	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy2AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy2config.dwOfs_firebutton)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_SETCURSEL, lr, 0);
		}
	}
		
	if (bGotDefault2Fire == FALSE)
		default2Fire = i;
	else
		if (DevButton2Array[default2Fire].dwOfs > lpddoi->dwOfs)
			default2Fire = i;
	bGotDefault2Fire = TRUE;

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::EnumJoy1Axis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
LRESULT lr;
unsigned int i;

	hr = DevAxis1Array.Append((const DIDEVICEOBJECTINSTANCE)*lpddoi, &i);
	if(FAILED(hr))
		return DIENUM_CONTINUE;


	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy1AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy1config.dwOfs_Y)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_SETCURSEL, lr, 0);
		}
	}
	
	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy1AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy1config.dwOfs_X)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_SETCURSEL, lr, 0);
		}
	}
	
	if (bGotDefault1X == FALSE)
	{
		if (DevAxis1Array[i].guidType == GUID_XAxis)
		{
			default1X = i;
			bGotDefault1X = TRUE;
		}
	}
	if (bGotDefault1Y == FALSE)
	{
		if (DevAxis1Array[i].guidType == GUID_YAxis)
		{
			default1Y = i;
			bGotDefault1Y = TRUE;
		}
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::EnumJoy2Axis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
LRESULT lr;
unsigned int i;

	hr = DevAxis2Array.Append((const DIDEVICEOBJECTINSTANCE)*lpddoi, &i);
	if(FAILED(hr))
		return DIENUM_STOP;

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy2AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy2config.dwOfs_Y)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_SETCURSEL, lr, 0);
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
		return DIENUM_STOP;
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoy2AxisSetConfig)
	{
		if (lpddoi->dwOfs == newCfg.m_joy2config.dwOfs_X)
		{
			SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_SETCURSEL, lr, 0);
		}
	}

	if (bGotDefault2X == FALSE)
	{
		if (DevAxis2Array[i].guidType == GUID_XAxis)
		{
			default2X = i;
			bGotDefault2X = TRUE;
		}
	}
	if (bGotDefault2Y == FALSE)
	{
		if (DevAxis2Array[i].guidType == GUID_YAxis)
		{
			default2Y = i;
			bGotDefault2Y = TRUE;
		}
	}

	return DIENUM_CONTINUE;
}
