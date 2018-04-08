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
#include "besttextwidth.h"
#include "diagbuttonselection.h"
#include "diagjoystick.h"


CDiagJoystick::JoyControlNum CDiagJoystick::joy1ControlNum =
{
	IDC_CBO_JOY1DEVICE,
	IDC_CBO_JOY1FIRE, 
	IDC_CBO_JOY1FIRE2, 
	IDC_CBO_JOY1V, 
	IDC_CBO_JOY1H, 
	IDC_CBO_JOY1VREV, 
	IDC_CBO_JOY1HREV, 
	IDC_CBO_JOY1UP, 
	IDC_CBO_JOY1DOWN, 
	IDC_CBO_JOY1LEFT, 
	IDC_CBO_JOY1RIGHT, 
	IDC_CBO_JOY1ENABLE,
	IDC_CBO_JOY1ENABLEPOV
};

CDiagJoystick::JoyControlNum CDiagJoystick::joy2ControlNum =
{
	IDC_CBO_JOY2DEVICE,
	IDC_CBO_JOY2FIRE, 
	IDC_CBO_JOY2FIRE2, 
	IDC_CBO_JOY2V, 
	IDC_CBO_JOY2H, 
	IDC_CBO_JOY2VREV, 
	IDC_CBO_JOY2HREV, 
	IDC_CBO_JOY2UP, 
	IDC_CBO_JOY2DOWN, 
	IDC_CBO_JOY2LEFT, 
	IDC_CBO_JOY2RIGHT, 
	IDC_CBO_JOY2ENABLE,
	IDC_CBO_JOY2ENABLEPOV
};

CDiagJoystick::JoyUi::JoyUi(CDX9 *pDX, const struct joyconfig& outerjconfig, int ID, const JoyControlNum& controlNum)
	: pDX(pDX),  jconfig(outerjconfig), ID(ID), controlNum(controlNum)
	, c64buttonFire1(controlNum.cbo_joyfire1, C64JoystickButton::Fire1, &jconfig.fire1ButtonOffsets[0], jconfig.fire1ButtonCount)
	, c64buttonFire2(controlNum.cbo_joyfire2, C64JoystickButton::Fire2, &jconfig.fire2ButtonOffsets[0], jconfig.fire2ButtonCount)
	, c64buttonUp(controlNum.cbo_joyup, C64JoystickButton::Up, &jconfig.upButtonOffsets[0], jconfig.upButtonCount)
	, c64buttonDown(controlNum.cbo_joydown, C64JoystickButton::Down, &jconfig.downButtonOffsets[0], jconfig.downButtonCount)
	, c64buttonLeft(controlNum.cbo_joyleft, C64JoystickButton::Left, &jconfig.leftButtonOffsets[0], jconfig.leftButtonCount)
	, c64buttonRight(controlNum.cbo_joyright, C64JoystickButton::Right, &jconfig.rightButtonOffsets[0], jconfig.rightButtonCount)
{
	const int InitialAxisSize = 10;
	axisOptions.reserve(InitialAxisSize);
	buttonOptions.reserve(joyconfig::MAXBUTTONS + 5);
	numButtons = 0;
}

CDiagJoystick::CDiagJoystick(CDX9 *pDX, const CConfig *currentCfg)
	: pDX(pDX)
	, currentCfg(currentCfg)
	, newCfg(*currentCfg)
	, joy1(pDX, currentCfg->m_joy1config, 1, joy1ControlNum)
	, joy2(pDX, currentCfg->m_joy2config, 2, joy2ControlNum)
{
	const int InitialDevicesSize = 50;
	devices.reserve(InitialDevicesSize);
}

CDiagJoystick::~CDiagJoystick()
{
}

void CDiagJoystick::Init()
{
	joy1.dialog = this;
	joy2.dialog = this;
}

void CDiagJoystick::loadconfig(const CConfig& cfg)
{
	joy1.loadconfig(cfg.m_joy1config);
	joy2.loadconfig(cfg.m_joy2config);
}

void CDiagJoystick::saveconfig(CConfig *cfg)
{
	joy1.saveconfig(&cfg->m_joy1config);
	joy2.saveconfig(&cfg->m_joy2config);
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

void CDiagJoystick::OpenDevices()
{
size_t i;
	for (i = 0; i < devices.size(); i++)
	{
		devices[i]->OpenDevice(devices[i]->deviceInstance.guidInstance);
	}
}

HRESULT CDiagJoystick::FillDevices()
{
HRESULT r;


	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY1DEVICE, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hWnd, IDC_CBO_JOY2DEVICE, CB_RESETCONTENT, 0, 0);
	devices.clear();
	r = pDX->pDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumDlgJoyCallback, this, DIEDFL_ATTACHEDONLY);
	if (SUCCEEDED(r))
	{
		OpenDevices();
		joy1.FillDeviceSelection();
		joy2.FillDeviceSelection();
	}

	return r;
}

BOOL CDiagJoystick::EnumDevices(LPCDIDEVICEINSTANCE lpddi)
{
	try
	{
		shared_ptr<GameDeviceItem> sp = make_shared<GameDeviceItem>(GameDeviceItem(pDX->pDI, lpddi));
		devices.push_back(sp);
		return DIENUM_CONTINUE;
	}
	catch(std::exception &)
	{
		return DIENUM_STOP;
	}
}

bool CDiagJoystick::InitDialog(HWND hWndDlg)
{
	defaultFont = CreateFont(
	8,
	0,
	0,
	0,
	FW_NORMAL,
	FALSE,
	FALSE,
	FALSE,
	ANSI_CHARSET,
	OUT_TT_ONLY_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	FIXED_PITCH | FF_DONTCARE,
	TEXT("MS Shell Dlg"));
	if (defaultFont)
	{
		G::ArrangeOKCancel(hWndDlg);
		FillDevices();
		loadconfig(newCfg);
		return true;
	}
	else
	{
		return false;
	}
}

void CDiagJoystick::CleanDialog()
{
	if (defaultFont)
	{
		DeleteObject(defaultFont);
		defaultFont = NULL;
	}
}

BOOL CDiagJoystick::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		return InitDialog(hWndDlg) ? TRUE : FALSE;
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
				joy1.DeviceChanged(false);
				return TRUE;
			}

			break;
		case IDC_CBO_JOY2DEVICE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				joy2.DeviceChanged(false);
				return TRUE;
			}

			break;
		case IDC_BTN_CFG_JOY1FIRE1:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonFire1);
			}

			break;
		case IDC_BTN_CFG_JOY1FIRE2:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonFire2);
			}

			break;
		case IDC_BTN_CFG_JOY1UP:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonUp);
			}

			break;
		case IDC_BTN_CFG_JOY1DOWN:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonDown);
			}

			break;
		case IDC_BTN_CFG_JOY1LEFT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonLeft);
			}

			break;
		case IDC_BTN_CFG_JOY1RIGHT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonRight);
			}

			break;
		case IDC_BTN_CFG_JOY2FIRE1:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonFire1);
			}

			break;
		case IDC_BTN_CFG_JOY2FIRE2:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonFire2);
			}

			break;
		case IDC_BTN_CFG_JOY2UP:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonUp);
			}

			break;
		case IDC_BTN_CFG_JOY2DOWN:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonDown);
			}

			break;
		case IDC_BTN_CFG_JOY2LEFT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonLeft);
			}

			break;
		case IDC_BTN_CFG_JOY2RIGHT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonRight);
			}

			break;
		}

		break;
	case WM_DESTROY:
		CleanDialog();
		return 0;
	}

	return FALSE;
}

void CDiagJoystick::ShowButtonConfig(JoyUi &joyui, C64JoyItem& c64joybutton)
{
LRESULT lr;
int seldeviceindex;
unsigned int datadeviceindex;
	lr = SendDlgItemMessage(m_hWnd, joyui.controlNum.cbo_joydevice, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		return;
	}
	
	seldeviceindex = (int)lr;
	lr = SendDlgItemMessage(m_hWnd, joyui.controlNum.cbo_joydevice, CB_GETITEMDATA, (WPARAM)seldeviceindex, 0);
	if (lr == CB_ERR || lr < 0)
	{
		return;
	}

	datadeviceindex = (unsigned int)lr;
	if (this->devices.size() <= (size_t)datadeviceindex)
	{
		return;
	}

	shared_ptr<GameDeviceItem> spdev = this->devices[(size_t)datadeviceindex];
	if (FAILED(spdev->hrStatus))
	{
		return;
	}

	joyui.ButtonSelectionChanged(c64joybutton);
	std::vector<DWORD> vecButtonOffsets;
	AddToVec<DWORD>(vecButtonOffsets, c64joybutton.pButtonOffsets, c64joybutton.buttonCount);
	shared_ptr<CDiagButtonSelection> pDiagButtonSelection = make_shared<CDiagButtonSelection>(CDiagButtonSelection(pDX->pDI, spdev->deviceInstance.guidInstance, joyui.ID, c64joybutton.buttonnumber, vecButtonOffsets));
	if (pDiagButtonSelection != 0)
	{
		if (IDOK == pDiagButtonSelection->ShowDialog(this->m_hInst, MAKEINTRESOURCE(IDD_JOYBUTTONSELECTION), this->m_hWnd))
		{
			std::vector<DWORD>::const_iterator iter;
			unsigned int &i = c64joybutton.buttonCount;
			i = 0;
			for (iter = pDiagButtonSelection->resultButtonOffsets.cbegin(); iter != pDiagButtonSelection->resultButtonOffsets.cend(); iter++)
			{
				if (*iter >= DIJOFS_BUTTON0 && *iter <= DIJOFS_BUTTON31)
				{
					c64joybutton.pButtonOffsets[i] = *iter;
					i++;
				}
			}

			joyui.SelectJoyButtonDropdownItem(c64joybutton, true);
		}
	}
}

template<class T>
void CDiagJoystick::AddToVec(std::vector<T> &vec, const T *source, unsigned int count)
{
	if (source != NULL)
	{
		for (unsigned int i = 0 ; i < count; i++, source++)
		{
			vec.push_back(*source);
		}
	}
}

void CDiagJoystick::JoyUi::FillDeviceSelection()
{
LRESULT lr;
size_t i;
HDC hdcControl = NULL;
HWND hWndDevice = NULL;
	try
	{
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_RESETCONTENT, 0, 0);
		hWndDevice = GetDlgItem(dialog->m_hWnd, this->controlNum.cbo_joydevice);
		if (!hWndDevice)
		{
			return;
		}

		hdcControl = GetDC(hWndDevice);
		tw.SetDC(hdcControl);
		tw.SetFont(this->dialog->defaultFont);
		tw.Reset();
		int dpiX = this->m_dpi.GetDPIX();
		int dpiY = this->m_dpi.GetDPIY();

		bool found = false;
		for (i = 0; i < dialog->devices.size(); i++)
		{
			shared_ptr<GameDeviceItem> item = dialog->devices[i];
			LPCDIDEVICEINSTANCE lpddi = &item->deviceInstance;
			tmpDeviceName.clear();
			tmpDeviceName.append(lpddi->tszProductName);
			if (FAILED(item->hrStatus))
			{
				if (tmpDeviceName.length() > 0)
				{
					tmpDeviceName.append(TEXT(" "));
				}

				tmpDeviceName.append(G::GetLastWin32ErrorString(item->hrStatus));
			}

			tw.GetWidth(tmpDeviceName.c_str());		
			lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_ADDSTRING, 0, (LPARAM) tmpDeviceName.c_str());
			if (lr >= 0)
			{
				SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_SETITEMDATA, lr, (LPARAM) i);
				if (!found && jconfig.IsValidId)
				{
					if (jconfig.joystickID == lpddi->guidInstance)
					{
						found = true;
						SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_SETCURSEL, lr, 0);
						this->DeviceChanged(true);
					}
				}
			}
		}

		if (tw.maxWidth > 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(dialog->m_hWnd), 0);
		}
	}
	catch(std::exception &)
	{
	}

	tw.Clean();
	if (hdcControl != NULL)
	{
		ReleaseDC(hWndDevice, hdcControl);
		hdcControl = NULL;
	}
}

void CDiagJoystick::JoyUi::DeviceChanged(bool bSetConfig)
{
	FillJoyAxis(bSetConfig);
	FillJoyButton(bSetConfig);
}

void CDiagJoystick::JoyUi::FillJoyButton(bool bSetConfig)
{
HRESULT hr;
int seldeviceindex;
unsigned int datadeviceindex;
LRESULT lr;

	try
	{
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire1, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire2, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyup, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydown, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyleft, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyright, CB_RESETCONTENT, 0, 0);
		buttonOptions.clear();
		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_GETCURSEL, 0, 0);
		if (lr == CB_ERR || lr < 0)
		{
			return;
		}
	
		seldeviceindex = (int)lr;
		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_GETITEMDATA, (WPARAM)seldeviceindex, 0);
		if (lr == CB_ERR || lr < 0)
		{
			return;
		}

		datadeviceindex = (unsigned int)lr;
		if (dialog->devices.size() <= (size_t)datadeviceindex)
		{
			return;
		}

		shared_ptr<GameDeviceItem> spdev = dialog->devices[(size_t)datadeviceindex];
		if (SUCCEEDED(spdev->hrStatus))
		{
			ButtonItem buttonsNone(ButtonItem::None);
			ButtonItem buttonsAll(ButtonItem::AllButtons);
			ButtonItem buttonsCustom(ButtonItem::MultipleButton);
			buttonOptions.push_back(buttonsNone);
			buttonOptions.push_back(buttonsAll);
			buttonOptions.push_back(buttonsCustom);

			// Fill buttonOptions vector
			numButtons = 0;
			hr = spdev->pInputJoy->EnumObjects(EnumDlgJoyButtonCallback, this, DIDFT_BUTTON);
			if (SUCCEEDED(hr))
			{
				// Fill dropdown
				FillJoyButtonDropdown(this->controlNum.cbo_joyfire1);
				FillJoyButtonDropdown(this->controlNum.cbo_joyfire2);
				FillJoyButtonDropdown(this->controlNum.cbo_joyup);
				FillJoyButtonDropdown(this->controlNum.cbo_joydown);
				FillJoyButtonDropdown(this->controlNum.cbo_joyleft);
				FillJoyButtonDropdown(this->controlNum.cbo_joyright);

				// Search the drop down and set the current selection.
				SelectJoyButtonDropdownItem(this->c64buttonFire1, bSetConfig);
				SelectJoyButtonDropdownItem(this->c64buttonFire2, bSetConfig);
				SelectJoyButtonDropdownItem(this->c64buttonUp, bSetConfig);
				SelectJoyButtonDropdownItem(this->c64buttonDown, bSetConfig);
				SelectJoyButtonDropdownItem(this->c64buttonLeft, bSetConfig);
				SelectJoyButtonDropdownItem(this->c64buttonRight, bSetConfig);
			}
		}
	}
	catch(std::exception &)
	{
		return;
	}
}

void CDiagJoystick::JoyUi::FillJoyButtonDropdown(int ctrlid)
{
unsigned int i;
LRESULT lr;
	for (i = 0; i < buttonOptions.size(); i++)
	{
		ButtonItem& item = buttonOptions[i];
		LPDIDEVICEOBJECTINSTANCE lpddoi = &item.objectInfo;
		if (item.option == ButtonItem::SingleButton)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		}
		else if (item.option == ButtonItem::AllButtons)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT("All buttons"));
		}
		else if (item.option == ButtonItem::None)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT(""));
		}
		else if (item.option == ButtonItem::MultipleButton)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT("Custom"));
		}

		if (lr != CB_ERR && lr >= 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_SETITEMDATA, lr, (LPARAM) i);
		}
	}
}

void CDiagJoystick::JoyUi::SelectJoyButtonDropdownItem(C64JoyItem& c64joybutton, bool bSetConfig)
{
int selindex;
unsigned int i;
bool bGotDefaultFire;
unsigned int defaultFire;
LRESULT lr;
ButtonItem::ButtonItemOption defaultFireOption = ButtonItem::SingleButton;
DWORD defaultFireOffset = DIJOFS_BUTTON0;

	if (c64joybutton.buttonnumber == C64JoystickButton::Fire1)
	{
		defaultFireOption = ButtonItem::SingleButton;
		defaultFireOffset = DIJOFS_BUTTON0;
	}
	else
	{
		defaultFireOption = ButtonItem::None;
		defaultFireOffset = 0;
	}

	bGotDefaultFire = false;
	selindex = -1;	
	lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETCOUNT, 0, 0);
	if (lr != CB_ERR && lr >= 0)
	{
		unsigned int count = (unsigned int)lr;
		bool found = false;
		for (i = 0; i < count; i++)
		{
			bool match = false;
			lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETITEMDATA, i, 0);
			if (lr != CB_ERR)
			{
				if (lr >= 0 && (size_t)lr < buttonOptions.size())
				{
					ButtonItem& item = buttonOptions[(size_t)lr];
					if (!bGotDefaultFire)
					{
						if (item.option == defaultFireOption && item.objectInfo.dwOfs == defaultFireOffset)
						{
							defaultFire = i;
							bGotDefaultFire = true;
						}
					}
					
					if (!found)
					{
						if (item.option == ButtonItem::SingleButton)
						{
							if (c64joybutton.buttonCount== 1 && item.objectInfo.dwOfs == c64joybutton.pButtonOffsets[0])
							{
								match = true;
							}
						}
						else if (item.option == ButtonItem::AllButtons)
						{
							if (c64joybutton.buttonCount >= numButtons)
							{
								match = true;
							}
						}
						else if (item.option == ButtonItem::MultipleButton)
						{
							if (c64joybutton.buttonCount > 1 && c64joybutton.buttonCount < numButtons)
							{
								match = true;
							}
						}
						else if (item.option == ButtonItem::None)
						{
							if (c64joybutton.buttonCount == 0)
							{
								match = true;
							}
						}

						if (match)
						{
							found = true;
							if (bSetConfig)
							{
								selindex = i;
							}
						}
					}
				}
			}
		}
	}

	if (selindex < 0 && bGotDefaultFire)
	{
		selindex = defaultFire;
	}

	if (selindex >= 0)
	{
		SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_SETCURSEL, selindex, 0);
	}
}

void CDiagJoystick::JoyUi::FillJoyAxis(bool bSetConfig)
{
HRESULT hr;
unsigned int i;
LRESULT lr;
unsigned int count;

	try
	{
		bJoyAxisSetConfig = bSetConfig;
		bGotDefaultX = false;
		bGotDefaultY = false;
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_RESETCONTENT, 0, 0);
		axisOptions.clear();
		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_GETCURSEL, 0, 0);
		if (lr == CB_ERR || lr < 0)
		{
			return;
		}

		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydevice, CB_GETITEMDATA, (WPARAM)lr, 0);
		if (lr == CB_ERR || lr < 0)
		{
			return;
		}

		if (dialog->devices.size() <= (size_t)lr)
		{
			return;
		}

		shared_ptr<GameDeviceItem> spdev = dialog->devices[(size_t)lr];
		if (SUCCEEDED(spdev->hrStatus))
		{
			hr = spdev->pInputJoy->EnumObjects(EnumDlgJoyAxisCallback, this, DIDFT_ABSAXIS);
			if (SUCCEEDED(hr))
			{
				if (!bSetConfig && bGotDefaultX)
				{
					lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_GETCOUNT, 0, 0);
					if (lr != CB_ERR && lr >= 0)
					{
						count = (unsigned int)lr;
						for (i = 0; i < count; i++)
						{
							lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_GETITEMDATA, i, 0);
							if (lr != CB_ERR && lr >= 0)
							{
								if (lr == defaultX)
								{
									SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_SETCURSEL, i, 0);
								}
							}
						}
					}
				}

				if (!bSetConfig && bGotDefaultY)
				{
					lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_GETCOUNT, 0, 0);
					if (lr != CB_ERR && lr >= 0)
					{
						count = (unsigned int)lr;
						for (i = 0; i < count; i++)
						{
							lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_GETITEMDATA, i, 0);
							if (lr != CB_ERR && lr >= 0)
							{
								if (lr == defaultY)
								{
									SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_SETCURSEL, i, 0);
								}
							}
						}
					}
				}
			}
		}
	}
	catch(std::exception&)
	{
	}
}

BOOL CDiagJoystick::JoyUi::EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
ButtonItem item(ButtonItem::SingleButton, (DIDEVICEOBJECTINSTANCE)*lpddoi);

	try
	{
		buttonOptions.push_back(item);
		numButtons++;
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::JoyUi::EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
LRESULT lr;
unsigned int lastIndex;
ButtonItem item(ButtonItem::SingleAxis, *lpddoi);

	try
	{
		axisOptions.push_back(item);
		lastIndex = (unsigned int)axisOptions.size() - 1;
		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		if (lr < 0)
		{
			return DIENUM_STOP;
		}

		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_SETITEMDATA, lr, (LPARAM) lastIndex);
		if (bJoyAxisSetConfig)
		{
			if (lpddoi->dwOfs == this->jconfig.dwOfs_Y)
			{
				SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_SETCURSEL, lr, 0);
			}
		}
	
		lr = SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		if (lr < 0)
		{
			return DIENUM_STOP;
		}

		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_SETITEMDATA, lr, (LPARAM) lastIndex);
		if (bJoyAxisSetConfig)
		{
			if (lpddoi->dwOfs == this->jconfig.dwOfs_X)
			{
				SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_SETCURSEL, lr, 0);
			}
		}
	
		if (!bGotDefaultX)
		{
			if (axisOptions[lastIndex].objectInfo.guidType == GUID_XAxis)
			{
				defaultX = lastIndex;
				bGotDefaultX = true;
			}
		}

		if (!bGotDefaultY)
		{
			if (axisOptions[lastIndex].objectInfo.guidType == GUID_YAxis)
			{
				defaultY = lastIndex;
				bGotDefaultY = true;
			}
		}

		return DIENUM_CONTINUE;

	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}
}

void CDiagJoystick::JoyUi::ButtonSelectionChanged(C64JoyItem& c64joybutton)
{
LRESULT lr;
unsigned int i;
unsigned int j;

	unsigned int oldcount = c64joybutton.buttonCount;
	lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETCURSEL, 0, 0);
	if (lr >= 0)
	{
		lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0)
		{		
			if ((size_t)lr < this->buttonOptions.size())
			{
				ButtonItem &item = this->buttonOptions[(size_t)lr];
				if (item.option == ButtonItem::SingleButton)
				{
					c64joybutton.buttonCount = 1;
					c64joybutton.pButtonOffsets[0] = item.objectInfo.dwOfs;
				}
				else if (item.option == ButtonItem::AllButtons)
				{
					for (i = 0, j = 0; i < joyconfig::MAXBUTTONS && j < this->buttonOptions.size(); j++)
					{
						const ButtonItem& item = this->buttonOptions[j];
						if (item.option == ButtonItem::SingleButton)
						{
							DWORD dwOffset = item.objectInfo.dwOfs;
							if (dwOffset + sizeof(BYTE) <= sizeof(DIJOYSTATE))
							{
								c64joybutton.pButtonOffsets[i++] = dwOffset;
							}
						}						
					}

					c64joybutton.buttonCount = i;
				}
				else if (item.option == ButtonItem::MultipleButton)
				{
					c64joybutton.buttonCount = oldcount;
				}
				else if (item.option == ButtonItem::None)
				{
					c64joybutton.buttonCount = 0;
				}
			}
		}
	}
}

void CDiagJoystick::JoyUi::loadconfig(const joyconfig& cfg)
{
	HWND hWnd = this->dialog->m_hWnd;
	this->jconfig = cfg;
	if (this->jconfig.IsEnabled)
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyenable, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyenable, BST_UNCHECKED);
	}

	if (this->jconfig.isPovEnabled)
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyenablepov, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyenablepov, BST_UNCHECKED);
	}

	if (this->jconfig.isXReverse)
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyhrev, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyhrev, BST_UNCHECKED);
	}

	if (this->jconfig.isYReverse)
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyvrev, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.cbo_joyvrev, BST_UNCHECKED);
	}
}

void CDiagJoystick::JoyUi::saveconfig(joyconfig *cfg)
{
LRESULT lr;
DWORD dwOffset;
unsigned int i;

	HWND hWnd = this->dialog->m_hWnd;
	cfg->IsValidId = false;
	cfg->isPovEnabled = true;
	cfg->dwOfs_X = DIJOFS_X;
	cfg->dwOfs_Y = DIJOFS_Y;
	cfg->isValidXAxis = true;
	cfg->isValidYAxis = true;
	cfg->fire1ButtonOffsets[0] = DIJOFS_BUTTON0;
	cfg->fire1ButtonCount = 1;
	cfg->fire2ButtonCount = 0;	
	lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joydevice, CB_GETCURSEL, 0, 0);
	if (lr != CB_ERR && lr >= 0)
	{
		lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joydevice, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0)
		{		
			if ((size_t)lr < this->dialog->devices.size())
			{
				cfg->joystickID = this->dialog->devices[(size_t)lr]->deviceInstance.guidInstance;
				cfg->IsValidId = true;
			}
		}
	}

	lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joyv, CB_GETCURSEL, 0, 0);
	if (lr >= 0)
	{
		lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joyv, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0)
		{		
			if ((size_t)lr < this->axisOptions.size())
			{
				dwOffset = this->axisOptions[(size_t)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE))
				{
					cfg->dwOfs_Y = dwOffset;
				}
			}
		}
	}

	lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joyh, CB_GETCURSEL, 0, 0);
	if (lr >= 0)
	{
		lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joyh, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0)
		{		
			if ((size_t)lr < this->axisOptions.size())
			{
				dwOffset = this->axisOptions[(size_t)lr].objectInfo.dwOfs;
				if (dwOffset + sizeof(LONG) <= sizeof(DIJOYSTATE))
				{
					cfg->dwOfs_X = dwOffset;
				}
			}
		}
	}

	this->ButtonSelectionChanged(this->c64buttonFire1);
	cfg->fire1ButtonCount = this->jconfig.fire1ButtonCount;
	for(i = 0; i < this->jconfig.fire1ButtonCount; i++)
	{
		cfg->fire1ButtonOffsets[i] = this->jconfig.fire1ButtonOffsets[i];
	}

	this->ButtonSelectionChanged(this->c64buttonFire2);
	cfg->fire2ButtonCount = this->jconfig.fire2ButtonCount;
	for(i = 0; i < this->jconfig.fire2ButtonCount; i++)
	{
		cfg->fire2ButtonOffsets[i] = this->jconfig.fire2ButtonOffsets[i];
	}

	this->ButtonSelectionChanged(this->c64buttonUp);
	cfg->upButtonCount = this->jconfig.upButtonCount;
	for(i = 0; i < this->jconfig.upButtonCount; i++)
	{
		cfg->upButtonOffsets[i] = this->jconfig.upButtonOffsets[i];
	}

	this->ButtonSelectionChanged(this->c64buttonDown);
	cfg->downButtonCount = this->jconfig.downButtonCount;
	for(i = 0; i < this->jconfig.downButtonCount; i++)
	{
		cfg->downButtonOffsets[i] = this->jconfig.downButtonOffsets[i];
	}

	this->ButtonSelectionChanged(this->c64buttonLeft);
	cfg->leftButtonCount = this->jconfig.leftButtonCount;
	for(i = 0; i < this->jconfig.leftButtonCount; i++)
	{
		cfg->leftButtonOffsets[i] = this->jconfig.leftButtonOffsets[i];
	}

	this->ButtonSelectionChanged(this->c64buttonRight);
	cfg->rightButtonCount = this->jconfig.rightButtonCount;
	for(i = 0; i < this->jconfig.rightButtonCount; i++)
	{
		cfg->rightButtonOffsets[i] = this->jconfig.rightButtonOffsets[i];
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.cbo_joyenable))
	{
		cfg->IsEnabled = true;
	}
	else
	{
		cfg->IsEnabled = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.cbo_joyenablepov))
	{
		cfg->isPovEnabled = true;
	}
	else
	{
		cfg->isPovEnabled = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.cbo_joyvrev))
	{
		cfg->isYReverse = true;
	}
	else
	{
		cfg->isYReverse = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.cbo_joyhrev))
	{
		cfg->isXReverse = true;
	}
	else
	{
		cfg->isXReverse = false;
	}
}

bool CDiagJoystick::JoyUi::isReservedButton(const joyconfig *cfg, DWORD dwOffset)
{
	if (dwOffset != 0 && cfg != NULL && cfg->dwOfs_Left == dwOffset || cfg->dwOfs_Right == dwOffset || cfg->dwOfs_Up == dwOffset || cfg->dwOfs_Down == dwOffset)
	{
		return true;
	}
	else
	{
		return false;
	}
}

CDiagJoystick::C64JoyItem::C64JoyItem(int ctrlid, C64JoystickButton::C64JoystickButtonNumber buttonnumber, DWORD *pButtonOffsets, unsigned int &buttonCount)
	: ctrlid(ctrlid), buttonnumber(buttonnumber), pButtonOffsets(pButtonOffsets), buttonCount(buttonCount)
{
}