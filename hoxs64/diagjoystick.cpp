#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <winuser.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "resource.h"
#include "diagjoystick.h"

CDiagJoystick::JoyUi::JoyUi(CDX9 *pDX, const struct joyconfig& jconfig, int ID, int cbo_joydevice, int cbo_joyfire, int cbo_joyv, int cbo_joyh, int cbo_joyenable)
	: pDX(pDX),  jconfig(jconfig), ID(ID), cbo_joydevice(cbo_joydevice), cbo_joyfire(cbo_joyfire), cbo_joyv(cbo_joyv), cbo_joyh(cbo_joyh), cbo_joyenable(cbo_joyenable)
{
	
}

CDiagJoystick::CDiagJoystick(CDX9 *pDX, const CConfig *currentCfg)
	: pDX(pDX)
	, currentCfg(currentCfg)
	, newCfg(*currentCfg)
	, joy1(pDX, currentCfg->m_joy1config, 1, IDC_CBO_JOY1DEVICE, IDC_CBO_JOY1FIRE, IDC_CBO_JOY1V, IDC_CBO_JOY1H, IDC_CBO_JOY1ENABLE)
	, joy2(pDX, currentCfg->m_joy2config, 2, IDC_CBO_JOY2DEVICE, IDC_CBO_JOY2FIRE, IDC_CBO_JOY2V, IDC_CBO_JOY2H, IDC_CBO_JOY2ENABLE)
{
}

CDiagJoystick::~CDiagJoystick()
{
}

void CDiagJoystick::Init()
{
	joy1.dialog = this;
	joy2.dialog = this;
}

void CDiagJoystick::loadconfig(const CConfig *cfg)
{
	if (cfg == NULL)
	{
		return;
	}
	if (cfg->m_joy1config.bEnabled)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLE, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLE, BST_UNCHECKED);
	}

	if (cfg->m_joy2config.bEnabled)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLE, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLE, BST_UNCHECKED);
	}

	if (cfg->m_joy1config.bPovEnabled)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLEPOV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1ENABLEPOV, BST_UNCHECKED);
	}

	if (cfg->m_joy2config.bPovEnabled)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLEPOV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2ENABLEPOV, BST_UNCHECKED);
	}

	if (cfg->m_joy1config.bXReverse)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1HREV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1HREV, BST_UNCHECKED);
	}

	if (cfg->m_joy1config.bYReverse)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1VREV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY1VREV, BST_UNCHECKED);
	}

	if (cfg->m_joy2config.bXReverse)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2HREV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2HREV, BST_UNCHECKED);
	}

	if (cfg->m_joy2config.bYReverse)
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2VREV, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(m_hWnd, IDC_CBO_JOY2VREV, BST_UNCHECKED);
	}
}

void CDiagJoystick::saveconfig(CConfig *cfg)
{
LRESULT lr;
DWORD dwOffset;

	cfg->m_joy1config.bValid = false;
	cfg->m_joy2config.bValid = false;

	cfg->m_joy1config.bPovEnabled = true;
	cfg->m_joy1config.dwOfs_X = 0;
	cfg->m_joy1config.dwOfs_Y = 0;
	cfg->m_joy1config.dwOfs_firebutton = DIJOFS_BUTTON0;

	cfg->m_joy2config.bPovEnabled = true;
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
				cfg->m_joy1config.bValid = true;
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
				cfg->m_joy2config.bValid = true;
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1V, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy1.DevAxisArray.Count())
			{
				dwOffset = joy1.DevAxisArray[(ULONG)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE))
				{
					cfg->m_joy1config.dwOfs_Y = dwOffset;
				}
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1H, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy1.DevAxisArray.Count())
			{
				dwOffset = joy1.DevAxisArray[(ULONG)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE))
				{
					cfg->m_joy1config.dwOfs_X = dwOffset;
				}
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1FIRE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy1.DevButtonArray.Count())
			{
				ButtonItem& item = joy1.DevButtonArray[(ULONG)lr];
				if (item.option == ButtonItem::SingleButton)
				{
					dwOffset = item.objectInfo.dwOfs;
					if (dwOffset + sizeof(BYTE) <= sizeof(DIJOYSTATE))
					{
						cfg->m_joy1config.dwOfs_firebutton = dwOffset;
						cfg->m_joy1config.firemask = 0;
					}
				}
				else if (item.option == ButtonItem::AllButtons)
				{
					cfg->m_joy1config.firemask = (DWORD)-1;
				}
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2V, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy2.DevAxisArray.Count())
			{
				dwOffset = joy2.DevAxisArray[(ULONG)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE2))
				{
					cfg->m_joy2config.dwOfs_Y = dwOffset;
				}
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2H, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy2.DevAxisArray.Count())
			{
				dwOffset = joy2.DevAxisArray[(ULONG)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE))
				{
					cfg->m_joy2config.dwOfs_X = dwOffset;
				}
			}
		}
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETCURSEL, 0, 0);
	if (lr >= 0 && lr < MAXLONG)
	{
		lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2FIRE, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
		{		
			if ((ULONG)lr < joy2.DevButtonArray.Count())
			{
				ButtonItem& item = joy2.DevButtonArray[(ULONG)lr];
				if (item.option == ButtonItem::SingleButton)
				{
					dwOffset = item.objectInfo.dwOfs;
					if (dwOffset + sizeof(BYTE) <= sizeof(DIJOYSTATE))
					{
						cfg->m_joy2config.dwOfs_firebutton = dwOffset;
						cfg->m_joy2config.firemask = 0;
					}
				}
				else if (item.option == ButtonItem::AllButtons)
				{
					cfg->m_joy2config.firemask = (DWORD)-1;
				}
			}
		}
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1ENABLE))
	{
		cfg->m_joy1config.bEnabled = true;
	}
	else
	{
		cfg->m_joy1config.bEnabled = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2ENABLE))
	{
		cfg->m_joy2config.bEnabled = true;
	}
	else
	{
		cfg->m_joy2config.bEnabled = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1ENABLEPOV))
	{
		cfg->m_joy1config.bPovEnabled = true;
	}
	else
	{
		cfg->m_joy1config.bPovEnabled = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2ENABLEPOV))
	{
		cfg->m_joy2config.bPovEnabled = true;
	}
	else
	{
		cfg->m_joy2config.bPovEnabled = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1VREV))
	{
		cfg->m_joy1config.bYReverse = true;
	}
	else
	{
		cfg->m_joy1config.bYReverse = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2VREV))
	{
		cfg->m_joy2config.bYReverse = true;
	}
	else
	{
		cfg->m_joy2config.bYReverse = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY1HREV))
	{
		cfg->m_joy1config.bXReverse = true;
	}
	else
	{
		cfg->m_joy1config.bXReverse = false;
	}

	if (IsDlgButtonChecked(m_hWnd, IDC_CBO_JOY2HREV))
	{
		cfg->m_joy2config.bXReverse = true;
	}
	else
	{
		cfg->m_joy2config.bXReverse = false;
	}
}

BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	return ((CDiagJoystick *) pvRef)->EnumDevices(lpddi);
}

BOOL CALLBACK EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick::JoyUi *) pvRef)->EnumJoyAxis(lpddoi);
}

BOOL CALLBACK EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick::JoyUi *) pvRef)->EnumJoyButton(lpddoi);
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
	{
		return r;
	}

	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETDROPPEDWIDTH, 250, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_SETDROPPEDWIDTH, 250, 0);

	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1DEVICE);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	hWnd2 = GetDlgItem(m_hWnd, IDC_CBO_JOY2DEVICE);
	if (!hWnd2)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd2, &rc2))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd2,0,0,0,rc2.right - rc2.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);	
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1FIRE);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2FIRE);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1V);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1H);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2V);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY2H);
	if (!hWnd1)
	{
		return E_FAIL;
	}

	if(!GetWindowRect(hWnd1, &rc1))
	{
		return E_FAIL;
	}

	SetWindowPos(hWnd1,0,0,0,rc1.right - rc1.left,100,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	hWnd1 = GetDlgItem(m_hWnd, IDC_CBO_JOY1DEVICE);
	if (hWnd1)
	{
		SetForegroundWindow(hWnd1);
	}

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
	{
		return DIENUM_STOP;
	}

	lr = SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_ADDSTRING, 0, (LPARAM) &lpddi->tszProductName[0]);
	if (lr >= 0)
	{
		SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETITEMDATA, lr, (LPARAM) i);
		if (newCfg.m_joy1config.bValid)
			if (newCfg.m_joy1config.joystickID == lpddi->guidInstance)
			{
				SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_SETCURSEL, lr, 0);
				joy1.FillJoyAxis(true);
				joy1.FillJoyButton(true);
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
				joy2.FillJoyAxis(true);
				joy2.FillJoyButton(true);
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
				joy1.FillJoyAxis(false);
				joy1.FillJoyButton(false);
				return TRUE;
			}
			break;
		case IDC_CBO_JOY2DEVICE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				joy2.FillJoyAxis(false);
				joy2.FillJoyButton(false);
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

void CDiagJoystick::JoyUi::FillJoyButton(bool bSetConfig)
{
HRESULT hr;
int seldeviceindex;
int selindex;
unsigned int datadeviceindex;
unsigned int i;
unsigned int lastindexadded;
const int resize_value=10;
LRESULT lr;

	bJoyAxisSetConfig = bSetConfig;
	bGotDefaultFire = false;
	DevButtonArray.Clear();
	hr = DevButtonArray.Resize(resize_value);
	if (FAILED(hr))
	{
		return;
	}

	SendDlgItemMessage(dialog->m_hWnd, cbo_joyfire, CB_RESETCONTENT, 0, 0);
	lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joydevice, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0 || lr >= MAXLONG)
	{
		return;
	}
	
	seldeviceindex = (int)lr;
	lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joydevice, CB_GETITEMDATA, seldeviceindex, 0);
	if (lr == CB_ERR || lr < 0 || lr >= MAXLONG)
	{
		return;
	}

	datadeviceindex = (unsigned int)lr;
	if (dialog->DevArray.Count() <= (ULONG)datadeviceindex)
	{
		return;
	}

	hr = pDX->CreateDeviceJoy(0, dialog->DevArray[(ULONG)datadeviceindex].guidInstance);
	if (FAILED(hr)) 
	{
		return;
	}

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr)) 
	{
		return;
	}

	ButtonItem allbuttons(ButtonItem::AllButtons);
	hr = DevButtonArray.Append(allbuttons, &lastindexadded);
	if (FAILED(hr)) 
	{
		return;
	}

	selindex = -1;
	pDX->EnumObjectsJoy(0, EnumDlgJoyButtonCallback, this, DIDFT_BUTTON);
	for (i = 0; i < DevButtonArray.Count(); i++)
	{
		ButtonItem& item = DevButtonArray[i];
		LPDIDEVICEOBJECTINSTANCE lpddoi = &item.objectInfo;
		if (item.option == ButtonItem::SingleButton)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joyfire, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		}
		else
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joyfire, CB_ADDSTRING, 0, (LPARAM) TEXT("All buttons"));
		}

		if (lr >= 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, cbo_joyfire, CB_SETITEMDATA, lr, (LPARAM) i);
			if (item.option == ButtonItem::SingleButton)
			{
				if (!bGotDefaultFire)
				{
					defaultFire = i;
				}
				else
				{
					// Choose a default fire with the lowest offset.
					if (DevButtonArray[defaultFire].objectInfo.dwOfs > lpddoi->dwOfs)
					{
						defaultFire = i;
					}
				}

				if (bSetConfig)
				{
					if (this->jconfig.firemask == 0 && lpddoi->dwOfs == this->jconfig.dwOfs_firebutton)
					{
						selindex = (int)lr;
					}
				}
				else
				{
					if (bGotDefaultFire)
					{
						selindex = defaultFire;
					}
				}

				bGotDefaultFire = true;
			}
			else if (item.option == ButtonItem::AllButtons)
			{
				if (bSetConfig)
				{
					if (this->jconfig.firemask != 0)
					{
						selindex = (int)lr;
					}
				}
			}
		}
	}

	if (selindex >= 0)
	{
		SendDlgItemMessage(dialog->m_hWnd, cbo_joyfire, CB_SETCURSEL, selindex, 0);
	}
}

void CDiagJoystick::JoyUi::FillJoyAxis(bool bSetConfig)
{
HRESULT hr;
LRESULT i,j,k;
const int resize_value=10;
HRESULT r;

	bJoyAxisSetConfig = bSetConfig;
	bGotDefaultX = false;
	bGotDefaultY = false;
	DevAxisArray.Clear();
	r = DevAxisArray.Resize(resize_value);
	if (FAILED(r))
	{
		return;
	}

	SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_RESETCONTENT, 0, 0);
	i = SendDlgItemMessage(dialog->m_hWnd, cbo_joydevice, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
	{
		return;
	}

	i= SendDlgItemMessage(dialog->m_hWnd, cbo_joydevice, CB_GETITEMDATA, i, 0);
	if (i == CB_ERR || i < 0 || i >= MAXLONG)
	{
		return;
	}

	if (dialog->DevArray.Count() <= (ULONG)i)
	{
		return;
	}

	hr = pDX->CreateDeviceJoy(0, dialog->DevArray[(ULONG)i].guidInstance);
	if (FAILED(hr))
	{
		return;
	}

	hr = pDX->SetDataFormatJoy(0, &c_dfDIJoystick);
	if (FAILED(hr))
	{
		return;
	}

	pDX->EnumObjectsJoy(0, EnumDlgJoyAxisCallback, this, DIDFT_ABSAXIS);
	if (!bSetConfig && bGotDefaultX)
	{
		i = SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
				{
					if (k == defaultX)
					{
						SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_SETCURSEL, j, 0);
					}
				}
			}
		}
	}

	if (!bSetConfig && bGotDefaultY)
	{
		i = SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_GETCOUNT, 0, 0);
		if (i != CB_ERR && i >= 0 && i <= MAXLONG)
		{
			for (j=0 ; j < i ; j++)
			{
				k = SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_GETITEMDATA, j, 0);
				if (k != CB_ERR && k >= 0 && k < MAXLONG)
				{
					if (k == defaultY)
					{
						SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_SETCURSEL, j, 0);
					}
				}
			}
		}
	}
}

BOOL CDiagJoystick::JoyUi::EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
unsigned int i;
ButtonItem item(ButtonItem::SingleButton, (DIDEVICEOBJECTINSTANCE)*lpddoi);

	hr = DevButtonArray.Append(item, &i);
	if(FAILED(hr))
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::JoyUi::EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
HRESULT hr;
LRESULT lr;
unsigned int i;
ButtonItem item(ButtonItem::SingleAxis, *lpddoi);

	hr = DevAxisArray.Append(item, &i);
	if(FAILED(hr))
	{
		return DIENUM_CONTINUE;
	}

	lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
	{
		return DIENUM_STOP;
	}

	SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoyAxisSetConfig)
	{
		if (lpddoi->dwOfs == this->jconfig.dwOfs_Y)
		{
			SendDlgItemMessage(dialog->m_hWnd, cbo_joyv, CB_SETCURSEL, lr, 0);
		}
	}
	
	lr = SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
	if (lr < 0)
	{
		return DIENUM_STOP;
	}

	SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_SETITEMDATA, lr, (LPARAM) i);
	if (bJoyAxisSetConfig)
	{
		if (lpddoi->dwOfs == this->jconfig.dwOfs_X)
		{
			SendDlgItemMessage(dialog->m_hWnd, cbo_joyh, CB_SETCURSEL, lr, 0);
		}
	}
	
	if (!bGotDefaultX)
	{
		if (DevAxisArray[i].objectInfo.guidType == GUID_XAxis)
		{
			defaultX = i;
			bGotDefaultX = true;
		}
	}

	if (!bGotDefaultY)
	{
		if (DevAxisArray[i].objectInfo.guidType == GUID_YAxis)
		{
			defaultY = i;
			bGotDefaultY = true;
		}
	}

	return DIENUM_CONTINUE;
}
