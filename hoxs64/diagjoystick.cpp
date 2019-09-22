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
#include "c64Keys.h"
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
	IDC_CBO_JOY1ENABLEPOV,
	IDC_CBO_JOY1BUTTONKEY1,
	IDC_CBO_JOY1BUTTONKEY2,
	IDC_CBO_JOY1BUTTONKEY3,
	IDC_CBO_JOY1BUTTONKEY4,
	IDC_CBO_JOY1BUTTONKEY5,
	IDC_CBO_JOY1KEY1,
	IDC_CBO_JOY1KEY2,
	IDC_CBO_JOY1KEY3,
	IDC_CBO_JOY1KEY4,
	IDC_CBO_JOY1KEY5,
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
	IDC_CBO_JOY2ENABLEPOV,
	IDC_CBO_JOY2BUTTONKEY1,
	IDC_CBO_JOY2BUTTONKEY2,
	IDC_CBO_JOY2BUTTONKEY3,
	IDC_CBO_JOY2BUTTONKEY4,
	IDC_CBO_JOY2BUTTONKEY5,
	IDC_CBO_JOY2KEY1,
	IDC_CBO_JOY2KEY2,
	IDC_CBO_JOY2KEY3,
	IDC_CBO_JOY2KEY4,
	IDC_CBO_JOY2KEY5,
};

CDiagJoystick::JoyUi::JoyUi(CDX9 *pDX, const struct joyconfig& outerjconfig, int ID, const JoyControlNum& controlNum)
	: pDX(pDX),  jconfig(outerjconfig), ID(ID), controlNum(controlNum)
	, c64buttonFire1(controlNum.cbo_joyfire1, C64JoystickButton::Fire1, &jconfig.fire1ButtonOffsets[0], jconfig.fire1ButtonCount, NULL, NULL, jconfig.fire1AxisCount, NULL, NULL, jconfig.fire1PovCount)
	, c64buttonFire2(controlNum.cbo_joyfire2, C64JoystickButton::Fire2, &jconfig.fire2ButtonOffsets[0], jconfig.fire2ButtonCount, NULL, NULL, jconfig.fire2AxisCount, NULL, NULL, jconfig.fire2PovCount)
	, c64buttonUp(controlNum.cbo_joyup, C64JoystickButton::Up, &jconfig.upButtonOffsets[0], jconfig.upButtonCount, NULL, NULL, jconfig.upAxisCount, NULL, NULL, jconfig.upPovCount)
	, c64buttonDown(controlNum.cbo_joydown, C64JoystickButton::Down, &jconfig.downButtonOffsets[0], jconfig.downButtonCount, NULL, NULL, jconfig.downAxisCount, NULL, NULL, jconfig.downPovCount)
	, c64buttonLeft(controlNum.cbo_joyleft, C64JoystickButton::Left, &jconfig.leftButtonOffsets[0], jconfig.leftButtonCount, NULL, NULL, jconfig.leftAxisCount, NULL, NULL, jconfig.leftPovCount)
	, c64buttonRight(controlNum.cbo_joyright, C64JoystickButton::Right, &jconfig.rightButtonOffsets[0], jconfig.rightButtonCount, NULL, NULL, jconfig.rightAxisCount, NULL, NULL, jconfig.rightPovCount)
	, c64AxisHorizontal(controlNum.cbo_joyh, C64JoystickButton::AxisHorizontal, NULL, jconfig.horizontalAxisButtonCount, &jconfig.dwOfs_X, NULL, jconfig.horizontalAxisAxisCount, NULL, NULL, jconfig.horizontalAxisPovCount)
	, c64AxisVertical(controlNum.cbo_joyv, C64JoystickButton::AxisVertical, NULL, jconfig.verticalAxisButtonCount, &jconfig.dwOfs_Y, NULL, jconfig.verticalAxisAxisCount, NULL, NULL, jconfig.verticalAxisPovCount)
	, c64buttonKey1(controlNum.cbo_joykey1button, C64JoystickButton::ButtonAndAxisKey1, &jconfig.keyNButtonOffsets[0][0], jconfig.keyNButtonCount[0], &jconfig.keyNAxisOffsets[0][0], &jconfig.keyNAxisDirection[0][0], jconfig.keyNAxisCount[0], &jconfig.keyNPovOffsets[0][0], &jconfig.keyNPovDirection[0][0], jconfig.keyNPovCount[0])
	, c64buttonKey2(controlNum.cbo_joykey2button, C64JoystickButton::ButtonAndAxisKey2, &jconfig.keyNButtonOffsets[1][0], jconfig.keyNButtonCount[1], &jconfig.keyNAxisOffsets[1][0], &jconfig.keyNAxisDirection[1][0], jconfig.keyNAxisCount[1], &jconfig.keyNPovOffsets[1][0], &jconfig.keyNPovDirection[1][0], jconfig.keyNPovCount[1])
	, c64buttonKey3(controlNum.cbo_joykey3button, C64JoystickButton::ButtonAndAxisKey3, &jconfig.keyNButtonOffsets[2][0], jconfig.keyNButtonCount[2], &jconfig.keyNAxisOffsets[2][0], &jconfig.keyNAxisDirection[2][0], jconfig.keyNAxisCount[2], &jconfig.keyNPovOffsets[2][0], &jconfig.keyNPovDirection[2][0], jconfig.keyNPovCount[2])
	, c64buttonKey4(controlNum.cbo_joykey4button, C64JoystickButton::ButtonAndAxisKey4, &jconfig.keyNButtonOffsets[3][0], jconfig.keyNButtonCount[3], &jconfig.keyNAxisOffsets[3][0], &jconfig.keyNAxisDirection[3][0], jconfig.keyNAxisCount[3], &jconfig.keyNPovOffsets[3][0], &jconfig.keyNPovDirection[3][0], jconfig.keyNPovCount[3])
	, c64buttonKey5(controlNum.cbo_joykey5button, C64JoystickButton::ButtonAndAxisKey5, &jconfig.keyNButtonOffsets[4][0], jconfig.keyNButtonCount[4], &jconfig.keyNAxisOffsets[4][0], &jconfig.keyNAxisDirection[4][0], jconfig.keyNAxisCount[4], &jconfig.keyNPovOffsets[4][0], &jconfig.keyNPovDirection[4][0], jconfig.keyNPovCount[4])
{
	const int InitialAxisSize = 10;
	axisOptions.reserve(InitialAxisSize);
	buttonOptions.reserve(joyconfig::MAXBUTTONS);
	numButtons = 0;
	numAxis = 0;
	numPov = 0;
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

BOOL CALLBACK EnumDlgJoyPovCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagJoystick::JoyUi *) pvRef)->EnumJoyPov(lpddoi);
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
		case IDC_BTN_CFG_JOY1KEY1:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonKey1);
			}

			break;
		case IDC_BTN_CFG_JOY1KEY2:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonKey2);
			}

			break;
		case IDC_BTN_CFG_JOY1KEY3:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonKey3);
			}

			break;
		case IDC_BTN_CFG_JOY1KEY4:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonKey4);
			}

			break;
		case IDC_BTN_CFG_JOY1KEY5:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy1, this->joy1.c64buttonKey5);
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

		case IDC_BTN_CFG_JOY2KEY1:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonKey1);
			}

			break;
		case IDC_BTN_CFG_JOY2KEY2:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonKey2);
			}

			break;
		case IDC_BTN_CFG_JOY2KEY3:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonKey3);
			}

			break;
		case IDC_BTN_CFG_JOY2KEY4:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonKey4);
			}

			break;
		case IDC_BTN_CFG_JOY2KEY5:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ShowButtonConfig(this->joy2, this->joy2.c64buttonKey5);
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

	unsigned int i;

	//Read the dropdrown selected item.
	joyui.DropdownAutoSelectionChanged(c64joybutton);

	// Append the current list of buttons into a vector to be passed to the dialog.
	vector<ButtonItemData> paramListOfCurrentItems;	
	for (i = 0 ; i <  c64joybutton.buttonCount; i++)
	{
		paramListOfCurrentItems.push_back(ButtonItemData(GameControllerItem::Button, c64joybutton.pButtonOffsets[i], GameControllerItem::DirectionAny));
	}

	// Append the current list of axes into a vector to be passed to the dialog.
	if (c64joybutton.pAxisOffsets != NULL)
	{
		for (i = 0 ; i <  c64joybutton.axisCount; i++)
		{
			GameControllerItem::ControllerAxisDirection direction = GameControllerItem::DirectionAny;
			if (c64joybutton.pAxisDirection != NULL)
			{
				direction = c64joybutton.pAxisDirection[i];
			}

			paramListOfCurrentItems.push_back(ButtonItemData(GameControllerItem::Axis, c64joybutton.pAxisOffsets[i], direction));
		}
	}

	// Append the current list of pov items into a vector to be passed to the dialog.
	if (c64joybutton.pPovOffsets != NULL)
	{
		for (i = 0 ; i <  c64joybutton.povCount; i++)
		{
			GameControllerItem::ControllerAxisDirection direction = GameControllerItem::DirectionAny;
			if (c64joybutton.pPovDirection != NULL)
			{
				direction = c64joybutton.pPovDirection[i];
			}

			paramListOfCurrentItems.push_back(ButtonItemData(GameControllerItem::Pov, c64joybutton.pPovOffsets[i], direction));
		}
	}

	shared_ptr<CDiagButtonSelection> pDiagButtonSelection = make_shared<CDiagButtonSelection>(CDiagButtonSelection(pDX->pDI, spdev->deviceInstance.guidInstance, joyui.ID, c64joybutton.buttonnumber, paramListOfCurrentItems));
	if (pDiagButtonSelection != 0)
	{
		if (IDOK == pDiagButtonSelection->ShowDialog(this->m_hInst, MAKEINTRESOURCE(IDD_JOYBUTTONSELECTION), this->m_hWnd))
		{
			vector<ButtonItemData>::const_iterator iter;
			unsigned int &buttonCount = c64joybutton.buttonCount;
			unsigned int &axisCount = c64joybutton.axisCount;
			unsigned int &povCount = c64joybutton.povCount;
			buttonCount = 0;
			axisCount = 0;
			povCount = 0;
			for (iter = pDiagButtonSelection->resultButtons.cbegin(); iter != pDiagButtonSelection->resultButtons.cend(); iter++)
			{
				switch (iter->itemType)
				{
				case GameControllerItem::Button:
					if (iter->controllerItemOffset >= DIJOFS_BUTTON0 && iter->controllerItemOffset < DIJOFS_BUTTON(joyconfig::MAXBUTTONS))
					{
						c64joybutton.pButtonOffsets[buttonCount] = iter->controllerItemOffset;
						buttonCount++;
					}

					break;
				case GameControllerItem::Axis:
					if (iter->controllerItemOffset <= sizeof(DIJOYSTATE2) - sizeof(DWORD))
					{
						if (c64joybutton.pAxisOffsets != NULL)
						{
							c64joybutton.pAxisOffsets[axisCount] = iter->controllerItemOffset;
						}

						if (c64joybutton.pAxisDirection != NULL)
						{
							c64joybutton.pAxisDirection[axisCount] = iter->axisPovDirection;
						}

						axisCount++;
					}

					break;
				case GameControllerItem::Pov:
					if (iter->controllerItemOffset >= DIJOFS_POV(0) && iter->controllerItemOffset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
					{
						if (c64joybutton.pPovOffsets != NULL)
						{
							c64joybutton.pPovOffsets[povCount] = iter->controllerItemOffset;
						}

						if (c64joybutton.pPovDirection != NULL)
						{
							c64joybutton.pPovDirection[povCount] = iter->axisPovDirection;
						}

						povCount++;
					}

					break;
				}
			}
			
			joyui.SelectJoyButtonAxisDropdownItem(c64joybutton, true);
		}
	}
}

template<class T>
void CDiagJoystick::AddToVec(vector<T> &vec, const T *source, unsigned int count)
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
int seldeviceindex;
unsigned int datadeviceindex;
LRESULT lr;

	try
	{
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyv, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyh, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire1, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire2, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyup, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydown, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyleft, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyright, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey1button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey2button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey3button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey4button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey5button, CB_RESETCONTENT, 0, 0);

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
			listButton.clear();
			listAxis.clear();
			listPov.clear();
			GetJoyAxisList(spdev);
			GetJoyButtonList(spdev);
			GetJoyPovList(spdev);

			FillJoyAxis(c64AxisHorizontal, c64AxisVertical, bSetConfig);
			FillJoyButton(bSetConfig);
		}
	}
	catch(std::exception &)
	{
		return;
	}
}

HRESULT CDiagJoystick::JoyUi::GetJoyButtonList(shared_ptr<GameDeviceItem> spDevice)
{
HRESULT hr;
	this->numButtons = 0;
	hr = spDevice->pInputJoy->EnumObjects(EnumDlgJoyButtonCallback, this, DIDFT_BUTTON);
	return hr;
}

HRESULT CDiagJoystick::JoyUi::GetJoyAxisList(shared_ptr<GameDeviceItem> spDevice)
{
HRESULT hr;
	this->numAxis = 0;
	hr = spDevice->pInputJoy->EnumObjects(EnumDlgJoyAxisCallback, this, DIDFT_ABSAXIS);
	return hr;
}

HRESULT CDiagJoystick::JoyUi::GetJoyPovList(shared_ptr<GameDeviceItem> spDevice)
{
HRESULT hr;
	this->numPov = 0;
	hr = spDevice->pInputJoy->EnumObjects(EnumDlgJoyPovCallback, this, DIDFT_POV);
	return hr;
}

void CDiagJoystick::JoyUi::FillJoyButton(bool bSetConfig)
{
	try
	{
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire1, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyfire2, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyup, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joydown, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyleft, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joyright, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey1button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey2button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey3button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey4button, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, this->controlNum.cbo_joykey5button, CB_RESETCONTENT, 0, 0);
		buttonOptions.clear();
		GameControllerItem buttonsNone(GameControllerItem::None, GameControllerItem::DirectionAny);
		GameControllerItem buttonsAll(GameControllerItem::AllButtons, GameControllerItem::DirectionAny);
		GameControllerItem buttonsCustom(GameControllerItem::MultipleButton, GameControllerItem::DirectionAny);
		buttonOptions.push_back(buttonsNone);
		buttonOptions.push_back(buttonsAll);
		vector<GameControllerItem>::const_iterator iter;
		for (iter = listButton.cbegin(); iter != listButton.cend(); iter++)
		{
			buttonOptions.push_back(*iter);
		}

		buttonOptions.push_back(buttonsCustom);

		std::basic_string<TCHAR> buttonname;
		buttonAndAxisOptions.clear();
		buttonAndAxisOptions.push_back(buttonsNone);
		buttonAndAxisOptions.push_back(buttonsAll);
		for (iter = listButton.cbegin(); iter != listButton.cend(); iter++)
		{
			buttonAndAxisOptions.push_back(*iter);
		}

		for (iter = listAxis.cbegin(); iter != listAxis.cend(); iter++)
		{
			GameControllerItem axismin(GameControllerItem::Axis, GameControllerItem::DirectionMin, iter->objectInfo);
			GameControllerItem axismax(GameControllerItem::Axis, GameControllerItem::DirectionMax, iter->objectInfo);
			buttonAndAxisOptions.push_back(axismin);
			buttonAndAxisOptions.push_back(axismax);
		}
		
		for (iter = listPov.cbegin(); iter != listPov.cend(); iter++)
		{
			GameControllerItem povup(GameControllerItem::Pov, GameControllerItem::DirectionUp, iter->objectInfo);
			GameControllerItem povdown(GameControllerItem::Pov, GameControllerItem::DirectionDown, iter->objectInfo);
			GameControllerItem povleft(GameControllerItem::Pov, GameControllerItem::DirectionLeft, iter->objectInfo);
			GameControllerItem povright(GameControllerItem::Pov, GameControllerItem::DirectionRight, iter->objectInfo);
			buttonAndAxisOptions.push_back(povup);
			buttonAndAxisOptions.push_back(povdown);
			buttonAndAxisOptions.push_back(povleft);
			buttonAndAxisOptions.push_back(povright);
		}

		buttonAndAxisOptions.push_back(buttonsCustom);

		// Fill dropdowns
		FillJoyButtonDropdown(this->c64buttonFire1, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonFire2, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonUp, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonDown, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonLeft, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonRight, buttonOptions);
		FillJoyButtonDropdown(this->c64buttonKey1, buttonAndAxisOptions);
		FillJoyButtonDropdown(this->c64buttonKey2, buttonAndAxisOptions);
		FillJoyButtonDropdown(this->c64buttonKey3, buttonAndAxisOptions);
		FillJoyButtonDropdown(this->c64buttonKey4, buttonAndAxisOptions);
		FillJoyButtonDropdown(this->c64buttonKey5, buttonAndAxisOptions);

		// Search the dropdowns and set the current selection.
		SelectJoyButtonAxisDropdownItem(this->c64buttonFire1, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonFire2, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonUp, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonDown, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonLeft, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonRight, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonKey1, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonKey2, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonKey3, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonKey4, bSetConfig);
		SelectJoyButtonAxisDropdownItem(this->c64buttonKey5, bSetConfig);
	}
	catch(std::exception &)
	{
		return;
	}
}

void CDiagJoystick::JoyUi::FillJoyAxisDropdown(C64JoyItem& c64buttonKey, vector<GameControllerItem> &items)
{
unsigned int i;
LRESULT lr;
int ctrlid = c64buttonKey.ctrlid;
	c64buttonKey.pListItems = &items;
	for (i = 0; i < items.size(); i++)
	{
		GameControllerItem& item = items[i];
		LPDIDEVICEOBJECTINSTANCE lpddoi = &item.objectInfo;
		if (item.option == GameControllerItem::Axis)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		}
		else if (item.option == GameControllerItem::None)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT(""));
		}

		if (lr != CB_ERR && lr >= 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_SETITEMDATA, lr, (LPARAM) i);
		}
	}
}

void CDiagJoystick::JoyUi::FillJoyButtonDropdown(C64JoyItem& c64buttonKey, vector<GameControllerItem> &items)
{
unsigned int i;
LRESULT lr;
int ctrlid = c64buttonKey.ctrlid;
	c64buttonKey.pListItems = &items;
	for (i = 0; i < items.size(); i++)
	{
		GameControllerItem& item = items[i];
		LPDIDEVICEOBJECTINSTANCE lpddoi = &item.objectInfo;
		if (item.option == GameControllerItem::Button)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) &lpddoi->tszName[0]);
		}
		else if (item.option == GameControllerItem::Axis)
		{
			std::basic_string<TCHAR> name;
			name.clear();
			name.append(&lpddoi->tszName[0]);
			if (item.direction == GameControllerItem::DirectionMin)
			{
				name.append(TEXT(" <"));
			}
			else if (item.direction == GameControllerItem::DirectionMax)
			{
				name.append(TEXT(" >"));
			}

			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) name.c_str());
		}
		else if (item.option == GameControllerItem::Pov)
		{
			std::basic_string<TCHAR> name;
			name.clear();
			name.append(&lpddoi->tszName[0]);
			name.append(TEXT(" "));
			switch (item.direction)
			{
			case GameControllerItem::DirectionUp:
				name.append(TEXT("Up"));
				break;
			case GameControllerItem::DirectionRight:
				name.append(TEXT("Right"));
				break;
			case GameControllerItem::DirectionDown:
				name.append(TEXT("Down"));
				break;
			case GameControllerItem::DirectionLeft:
				name.append(TEXT("Left"));
				break;
			default:
				name.append(TEXT("?"));
				break;
			}

			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) name.c_str());
		}
		else if (item.option == GameControllerItem::AllButtons)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT("All buttons"));
		}
		else if (item.option == GameControllerItem::None)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT(""));
		}
		else if (item.option == GameControllerItem::MultipleButton)
		{
			lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) TEXT("Custom"));
		}

		if (lr != CB_ERR && lr >= 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_SETITEMDATA, lr, (LPARAM) i);
		}
	}
}

void CDiagJoystick::JoyUi::SelectJoyButtonAxisDropdownItem(C64JoyItem& c64joybutton, bool bSetConfig)
{
int selindex;
unsigned int i;
bool bGotdefaultTrigger;
unsigned int defaultTrigger;
LRESULT lr;
GameControllerItem::ControllerItemOption defaultTriggerOption = GameControllerItem::Button;
DWORD defaultTriggerOffset = DIJOFS_BUTTON0;
vector<GameControllerItem> *buttonAxisOptions = c64joybutton.pListItems;
	if (buttonAxisOptions == NULL)
	{
		return;
	}

	switch(c64joybutton.buttonnumber)
	{
	case C64JoystickButton::Fire1:
		defaultTriggerOption = GameControllerItem::Button;
		defaultTriggerOffset = DIJOFS_BUTTON0;		
		break;
	case C64JoystickButton::Fire2:
		defaultTriggerOption = GameControllerItem::None;
		defaultTriggerOffset = 0;
		break;
	case C64JoystickButton::ButtonAndAxisKey1:
	case C64JoystickButton::ButtonAndAxisKey2:
	case C64JoystickButton::ButtonAndAxisKey3:
	case C64JoystickButton::ButtonAndAxisKey4:
	case C64JoystickButton::ButtonAndAxisKey5:
		defaultTriggerOption = GameControllerItem::None;
		defaultTriggerOffset = 0;
		break;
	case C64JoystickButton::AxisHorizontal:
		defaultTriggerOption = GameControllerItem::Axis;
		defaultTriggerOffset = DIJOFS_X;
		break;
	case C64JoystickButton::AxisVertical:
		defaultTriggerOption = GameControllerItem::Axis;
		defaultTriggerOffset = DIJOFS_Y;
		break;
	default:
		defaultTriggerOption = GameControllerItem::None;
		defaultTriggerOffset = 0;
		buttonAxisOptions = &this->buttonAndAxisOptions;
	}

	bGotdefaultTrigger = false;
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
				if (lr >= 0 && (size_t)lr < buttonAxisOptions->size())
				{
					GameControllerItem& item = (*buttonAxisOptions)[(size_t)lr];
					if (!bGotdefaultTrigger)
					{
						if (item.option == defaultTriggerOption && item.objectInfo.dwOfs == defaultTriggerOffset)
						{
							defaultTrigger = i;
							bGotdefaultTrigger = true;
						}
					}
					
					if (!found)
					{
						if (item.option == GameControllerItem::Button)
						{
							if (c64joybutton.pButtonOffsets != NULL)
							{
								if (c64joybutton.buttonCount == 1 && c64joybutton.axisCount == 0 && c64joybutton.povCount == 0 && item.objectInfo.dwOfs == c64joybutton.pButtonOffsets[0])
								{
									match = true;
								}
							}
						}
						else if (item.option == GameControllerItem::Axis)
						{
							GameControllerItem::ControllerAxisDirection direction = GameControllerItem::DirectionAny;
							if (c64joybutton.pAxisDirection != NULL)
							{
								direction = c64joybutton.pAxisDirection[0];
							}

							if (c64joybutton.pAxisOffsets != NULL)
							{
								if (c64joybutton.buttonCount == 0 && c64joybutton.axisCount == 1 && c64joybutton.povCount == 0 && item.objectInfo.dwOfs == c64joybutton.pAxisOffsets[0] && item.direction == direction)
								{
									match = true;
								}
							}
						}
						else if (item.option == GameControllerItem::Pov)
						{
							GameControllerItem::ControllerAxisDirection direction = GameControllerItem::DirectionAny;
							if (c64joybutton.pPovDirection != NULL)
							{
								direction = c64joybutton.pPovDirection[0];
							}

							if (c64joybutton.pPovOffsets != NULL)
							{
								if (c64joybutton.buttonCount == 0 && c64joybutton.axisCount == 0 && c64joybutton.povCount == 1 && item.objectInfo.dwOfs == c64joybutton.pPovOffsets[0] && item.direction == direction)
								{
									match = true;
								}
							}
						}
						else if (item.option == GameControllerItem::AllButtons)
						{
							if (c64joybutton.buttonCount >= numButtons && c64joybutton.axisCount == 0 && c64joybutton.povCount == 0)
							{
								match = true;
							}
						}
						else if (item.option == GameControllerItem::MultipleButton)
						{
							if (c64joybutton.buttonCount + c64joybutton.axisCount + c64joybutton.povCount > 1)
							{
								match = true;
							}
						}
						else if (item.option == GameControllerItem::None)
						{
							if (c64joybutton.buttonCount == 0 && c64joybutton.axisCount == 0 && c64joybutton.povCount == 0)
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

	if (selindex < 0 && bGotdefaultTrigger)
	{
		selindex = defaultTrigger;
	}

	if (selindex >= 0)
	{
		SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_SETCURSEL, selindex, 0);
	}
}

void CDiagJoystick::JoyUi::FillJoyAxis(C64JoyItem& horizontal, C64JoyItem& vertical, bool bSetConfig)
{
	try
	{
		SendDlgItemMessage(dialog->m_hWnd, vertical.ctrlid, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(dialog->m_hWnd, horizontal.ctrlid, CB_RESETCONTENT, 0, 0);
		axisOptions.clear();
		GameControllerItem axisNone(GameControllerItem::None);
		axisOptions.push_back(axisNone);
		vector<GameControllerItem>::const_iterator iter;
		for (iter = listAxis.cbegin(); iter != listAxis.cend(); iter++)
		{
			axisOptions.push_back(*iter);
		}

		// Fill dropdowns
		FillJoyAxisDropdown(horizontal, axisOptions);
		FillJoyAxisDropdown(vertical, axisOptions);

		// Search the dropdowns and set the current selection.
		SelectJoyButtonAxisDropdownItem(horizontal, bSetConfig);
		SelectJoyButtonAxisDropdownItem(vertical, bSetConfig);
	}
	catch(std::exception&)
	{
	}
}

void CDiagJoystick::JoyUi::FillJoyButtonKeys(const joyconfig& cfg)
{
	FillJoyButtonKeyDropdown(controlNum.cbo_joykey1, cfg.isValidKeyNoAssign[0], cfg.keyNoAssign[0]);
	FillJoyButtonKeyDropdown(controlNum.cbo_joykey2, cfg.isValidKeyNoAssign[1], cfg.keyNoAssign[1]);
	FillJoyButtonKeyDropdown(controlNum.cbo_joykey3, cfg.isValidKeyNoAssign[2], cfg.keyNoAssign[2]);
	FillJoyButtonKeyDropdown(controlNum.cbo_joykey4, cfg.isValidKeyNoAssign[3], cfg.keyNoAssign[3]);
	FillJoyButtonKeyDropdown(controlNum.cbo_joykey5, cfg.isValidKeyNoAssign[4], cfg.keyNoAssign[4]);
}

void CDiagJoystick::JoyUi::FillJoyButtonKeyDropdown(int ctrlid, bool isValid, bit8 keyValue)
{
	unsigned int i;
	SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_RESETCONTENT, 0, 0);
	C64Keys::C64Key keyList[] = 
	{
		C64Keys::C64K_NONE,
		C64Keys::C64K_0,
		C64Keys::C64K_1,
		C64Keys::C64K_2,
		C64Keys::C64K_3,
		C64Keys::C64K_4,
		C64Keys::C64K_5,
		C64Keys::C64K_6,
		C64Keys::C64K_7,
		C64Keys::C64K_8,
		C64Keys::C64K_9,
		C64Keys::C64K_A,
		C64Keys::C64K_B,
		C64Keys::C64K_C,
		C64Keys::C64K_D,
		C64Keys::C64K_E,
		C64Keys::C64K_F,
		C64Keys::C64K_G,
		C64Keys::C64K_H,
		C64Keys::C64K_I,
		C64Keys::C64K_J,
		C64Keys::C64K_K,
		C64Keys::C64K_L,
		C64Keys::C64K_M,
		C64Keys::C64K_N,
		C64Keys::C64K_O,
		C64Keys::C64K_P,
		C64Keys::C64K_Q,
		C64Keys::C64K_R,
		C64Keys::C64K_S,
		C64Keys::C64K_T,
		C64Keys::C64K_U,
		C64Keys::C64K_V,
		C64Keys::C64K_W,
		C64Keys::C64K_X,
		C64Keys::C64K_Y,
		C64Keys::C64K_Z,
		C64Keys::C64K_SPACE,
		C64Keys::C64K_PLUS,
		C64Keys::C64K_MINUS,
		C64Keys::C64K_ASTERISK,
		C64Keys::C64K_SLASH,
		C64Keys::C64K_EQUAL,
		C64Keys::C64K_POUND,
		C64Keys::C64K_AT,
		C64Keys::C64K_DOT,
		C64Keys::C64K_COMMA,
		C64Keys::C64K_COLON,
		C64Keys::C64K_SEMICOLON,
		C64Keys::C64K_ARROWLEFT,
		C64Keys::C64K_ARROWUP,
		C64Keys::C64K_RETURN,
		C64Keys::C64K_COMMODORE,
		C64Keys::C64K_CONTROL,
		C64Keys::C64K_LEFTSHIFT,
		C64Keys::C64K_RIGHTSHIFT,
		C64Keys::C64K_HOME,
		C64Keys::C64K_STOP,
		C64Keys::C64K_DEL,
		C64Keys::C64K_CURSORUP,
		C64Keys::C64K_CURSORDOWN,
		C64Keys::C64K_CURSORLEFT,
		C64Keys::C64K_CURSORRIGHT,
		C64Keys::C64K_F1,
		C64Keys::C64K_F2,
		C64Keys::C64K_F3,
		C64Keys::C64K_F4,
		C64Keys::C64K_F5,
		C64Keys::C64K_F6,
		C64Keys::C64K_F7,
		C64Keys::C64K_F8,
		C64Keys::C64K_RESTORE,
	};

	LRESULT lr;
	for (i = 0; i < _countof(keyList); i++)
	{
		C64Keys::C64Key key = keyList[i];
		lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_ADDSTRING, 0, (LPARAM) C64Keys::GetName(key));
		if (lr != CB_ERR && lr >= 0)
		{
			SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_SETITEMDATA, lr, (LPARAM)(unsigned int)key);
		}
	}

	SelectJoyButtonKeyDropdown(ctrlid, isValid, keyValue);
}

void CDiagJoystick::JoyUi::SelectJoyButtonKeyDropdown(int ctrlid, bool isValid, bit8 keyValue)
{
	int selindex = 0;	
	bool found = false;
	if (isValid)
	{
		LRESULT lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_GETCOUNT, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			unsigned int i;
			unsigned int count = (unsigned int)lr;
			for (i = 0; i < count; i++)
			{
				bool match = false;
				lr = SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_GETITEMDATA, i, 0);
				if (lr != CB_ERR)
				{
					if ((bit8)(lr & 0xff) == keyValue)
					{
						found = true;
						selindex = (int)i;
						break;
					}
				}
			}
		}
	}

	SendDlgItemMessage(dialog->m_hWnd, ctrlid, CB_SETCURSEL, selindex, 0);
}

BOOL CDiagJoystick::JoyUi::EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
GameControllerItem item(GameControllerItem::Button, GameControllerItem::DirectionAny, (DIDEVICEOBJECTINSTANCE)*lpddoi);

	try
	{
		listButton.push_back(item);
		numButtons++;
		if (numButtons >= joyconfig::MAXBUTTONS)
		{
			return DIENUM_STOP;
		}
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagJoystick::JoyUi::EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
GameControllerItem item(GameControllerItem::Axis, GameControllerItem::DirectionAny, (DIDEVICEOBJECTINSTANCE)*lpddoi);

	try
	{
		listAxis.push_back(item);
		numAxis++;
		if (numAxis >= joyconfig::MAXAXIS)
		{
			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;

	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}
}

BOOL CDiagJoystick::JoyUi::EnumJoyPov(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
GameControllerItem item(GameControllerItem::Pov, GameControllerItem::DirectionAny, (DIDEVICEOBJECTINSTANCE)*lpddoi);

	try
	{
		listPov.push_back(item);
		numPov++;
		if (numPov >= joyconfig::MAXPOV)
		{
			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;

	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}
}

void CDiagJoystick::JoyUi::ButtonAndAxisSelectionChanged(C64JoyItem& c64joyitem)
{
	DeviceItemSelectionChanged(c64joyitem, this->buttonAndAxisOptions, GameControllerItem::ObjectTypeFilterButtonAndAxisAndPov);
}

void CDiagJoystick::JoyUi::ButtonSelectionChanged(C64JoyItem& c64joyitem)
{
	DeviceItemSelectionChanged(c64joyitem, this->buttonOptions, GameControllerItem::ObjectTypeFilterButton);
}

void CDiagJoystick::JoyUi::AxisSelectionChanged(C64JoyItem& c64joyitem)
{
	DeviceItemSelectionChanged(c64joyitem, this->axisOptions, GameControllerItem::ObjectTypeFilterAxis);
}

void CDiagJoystick::JoyUi::DropdownAutoSelectionChanged(C64JoyItem& c64joyitem)
{
	switch (c64joyitem.buttonnumber)
	{
		case C64JoystickButton::AxisHorizontal:
		case C64JoystickButton::AxisVertical:
			AxisSelectionChanged(c64joyitem);
			break;
		case C64JoystickButton::Fire1:
		case C64JoystickButton::Fire2:
		case C64JoystickButton::Up:
		case C64JoystickButton::Down:
		case C64JoystickButton::Left:
		case C64JoystickButton::Right:
			ButtonSelectionChanged(c64joyitem);
			break;
		case C64JoystickButton::ButtonAndAxisKey1:
		case C64JoystickButton::ButtonAndAxisKey2:
		case C64JoystickButton::ButtonAndAxisKey3:
		case C64JoystickButton::ButtonAndAxisKey4:
		case C64JoystickButton::ButtonAndAxisKey5:
			ButtonAndAxisSelectionChanged(c64joyitem);
			break;
	}
}

void CDiagJoystick::JoyUi::DeviceItemSelectionChanged(C64JoyItem& c64joybutton, vector<GameControllerItem> &buttonAxisOptions, GameControllerItem::ObjectTypeFilter filter)
{
LRESULT lr;
unsigned int i;
unsigned int j;
DWORD dwOffset;

	unsigned int oldbuttoncount = c64joybutton.buttonCount;
	unsigned int oldaxiscount = c64joybutton.axisCount;
	unsigned int oldpovCount = c64joybutton.povCount;
	lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETCURSEL, 0, 0);
	if (lr >= 0)
	{
		lr = SendDlgItemMessage(dialog->m_hWnd, c64joybutton.ctrlid, CB_GETITEMDATA, lr, 0);
		if (lr != CB_ERR && lr >= 0)
		{		
			if ((size_t)lr < buttonAxisOptions.size())
			{
				GameControllerItem &item = buttonAxisOptions[(size_t)lr];
				if (item.option == GameControllerItem::Button)
				{
					if (filter & GameControllerItem::ObjectTypeFilterButton)
					{
						dwOffset = item.objectInfo.dwOfs;
						if (dwOffset  <= sizeof(DIJOYSTATE2) - sizeof(BYTE))
						{
							c64joybutton.axisCount = 0;
							c64joybutton.buttonCount = 1;
							c64joybutton.povCount = 0;
							c64joybutton.pButtonOffsets[0] = dwOffset;
						}
					}
				}
				else if (item.option == GameControllerItem::Axis)
				{
					if (filter & GameControllerItem::ObjectTypeFilterAxis)
					{
						dwOffset = item.objectInfo.dwOfs;
						if (dwOffset <= sizeof(DIJOYSTATE2) - sizeof(LONG))
						{
							c64joybutton.buttonCount = 0;
							c64joybutton.axisCount = 0;
							c64joybutton.povCount = 0;
							if (c64joybutton.pAxisOffsets != NULL)
							{
								c64joybutton.pAxisOffsets[0] = dwOffset;
								c64joybutton.axisCount = 1;
							}

							if (c64joybutton.pAxisDirection != NULL)
							{
								c64joybutton.pAxisDirection[0] = item.direction;
							}
						}
					}
				}
				else if (item.option == GameControllerItem::Pov)
				{
					if (filter & GameControllerItem::ObjectTypeFilterPov)
					{
						dwOffset = item.objectInfo.dwOfs;
						if (dwOffset >= DIJOFS_POV(0) && dwOffset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
						{
							c64joybutton.buttonCount = 0;
							c64joybutton.axisCount = 0;
							c64joybutton.povCount = 0;
							if (c64joybutton.pPovOffsets != NULL)
							{
								c64joybutton.pPovOffsets[0] = dwOffset;
								c64joybutton.povCount = 1;
							}

							if (c64joybutton.pPovDirection != NULL)
							{
								c64joybutton.pPovDirection[0] = item.direction;
							}
						}
					}
				}
				else if (item.option == GameControllerItem::AllButtons)
				{
					if (filter & GameControllerItem::ObjectTypeFilterButton)
					{
						for (i = 0, j = 0; i < joyconfig::MAXBUTTONS && j < buttonAxisOptions.size(); j++)
						{
							const GameControllerItem& item = buttonAxisOptions[j];
							if (item.option == GameControllerItem::Button)
							{
								dwOffset = item.objectInfo.dwOfs;
								if (dwOffset  <= sizeof(DIJOYSTATE2) - sizeof(BYTE))
								{
									c64joybutton.pButtonOffsets[i++] = dwOffset;
								}
							}						
						}

						c64joybutton.axisCount = 0;
						c64joybutton.buttonCount = i;
					}
				}
				else if (item.option == GameControllerItem::MultipleButton)
				{
					c64joybutton.buttonCount = oldbuttoncount;
					c64joybutton.axisCount = oldaxiscount;
					c64joybutton.povCount = oldpovCount;
				}
				else if (item.option == GameControllerItem::None)
				{
					c64joybutton.buttonCount = 0;
					c64joybutton.axisCount = 0;
					c64joybutton.povCount = 0;
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
		CheckDlgButton(hWnd, this->controlNum.chk_joyenable, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyenable, BST_UNCHECKED);
	}

	if (this->jconfig.isPovEnabled)
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyenablepov, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyenablepov, BST_UNCHECKED);
	}

	if (this->jconfig.isXReverse)
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyhrev, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyhrev, BST_UNCHECKED);
	}

	if (this->jconfig.isYReverse)
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyvrev, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, this->controlNum.chk_joyvrev, BST_UNCHECKED);
	}

	FillJoyButtonKeys(cfg);
}

void CDiagJoystick::JoyUi::saveconfig(joyconfig *cfg)
{
LRESULT lr;
//DWORD dwOffset;
unsigned int i;
unsigned int j;
int seldeviceindex;
unsigned int datadeviceindex;

	HWND hWnd = this->dialog->m_hWnd;
	lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joydevice, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		return;
	}
	
	seldeviceindex = (int)lr;
	lr = SendDlgItemMessage(hWnd, this->controlNum.cbo_joydevice, CB_GETITEMDATA, (WPARAM)seldeviceindex, 0);
	if (lr == CB_ERR || lr < 0)
	{
		return;
	}

	datadeviceindex = (unsigned int)lr;
	if (this->dialog->devices.size() <= (size_t)datadeviceindex)
	{
		return;
	}

	cfg->IsValidId = false;
	cfg->isPovEnabled = true;
	cfg->dwOfs_X = DIJOFS_X;
	cfg->horizontalAxisAxisCount = 1;
	cfg->dwOfs_Y = DIJOFS_Y;
	cfg->verticalAxisAxisCount = 1;
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

	// Save vertical axis
	this->AxisSelectionChanged(this->c64AxisVertical);
	cfg->dwOfs_Y = this->jconfig.dwOfs_Y;
	cfg->verticalAxisAxisCount = this->jconfig.verticalAxisAxisCount;

	// Save horizontal axis
	this->AxisSelectionChanged(this->c64AxisHorizontal);
	cfg->dwOfs_X = this->jconfig.dwOfs_X;
	cfg->horizontalAxisAxisCount = this->jconfig.horizontalAxisAxisCount;

	// Save button sources for joystick fire1.
	C64JoyItem& item = c64buttonFire1;
	this->ButtonSelectionChanged(this->c64buttonFire1);
	cfg->fire1ButtonCount = this->jconfig.fire1ButtonCount;
	for(i = 0; i < this->jconfig.fire1ButtonCount; i++)
	{
		cfg->fire1ButtonOffsets[i] = this->jconfig.fire1ButtonOffsets[i];
	}

	// Save button sources for joystick fire2.
	this->ButtonSelectionChanged(this->c64buttonFire2);
	cfg->fire2ButtonCount = this->jconfig.fire2ButtonCount;
	for(i = 0; i < this->jconfig.fire2ButtonCount; i++)
	{
		cfg->fire2ButtonOffsets[i] = this->jconfig.fire2ButtonOffsets[i];
	}

	// Save button sources for joystick up.
	this->ButtonSelectionChanged(this->c64buttonUp);
	cfg->upButtonCount = this->jconfig.upButtonCount;
	for(i = 0; i < this->jconfig.upButtonCount; i++)
	{
		cfg->upButtonOffsets[i] = this->jconfig.upButtonOffsets[i];
	}

	// Save button sources for joystick down.
	this->ButtonSelectionChanged(this->c64buttonDown);
	cfg->downButtonCount = this->jconfig.downButtonCount;
	for(i = 0; i < this->jconfig.downButtonCount; i++)
	{
		cfg->downButtonOffsets[i] = this->jconfig.downButtonOffsets[i];
	}

	// Save button sources for joystick left.
	this->ButtonSelectionChanged(this->c64buttonLeft);
	cfg->leftButtonCount = this->jconfig.leftButtonCount;
	for(i = 0; i < this->jconfig.leftButtonCount; i++)
	{
		cfg->leftButtonOffsets[i] = this->jconfig.leftButtonOffsets[i];
	}

	// Save button sources for joystick right.
	this->ButtonSelectionChanged(this->c64buttonRight);
	cfg->rightButtonCount = this->jconfig.rightButtonCount;
	for(i = 0; i < this->jconfig.rightButtonCount; i++)
	{
		cfg->rightButtonOffsets[i] = this->jconfig.rightButtonOffsets[i];
	}

	// Save button and axis sources for keys.
	C64JoyItem* keybuttoncontrols[] = { &this->c64buttonKey1, &this->c64buttonKey2, &this->c64buttonKey3, &this->c64buttonKey4, &this->c64buttonKey5 };
	for(j = 0; j < cfg->MAXKEYMAPS; j++)
	{
		C64JoyItem &item = *keybuttoncontrols[j];
		this->ButtonAndAxisSelectionChanged(item);

		// Save buttons
		cfg->keyNButtonCount[j] = item.buttonCount;
		for(i = 0; i < item.buttonCount; i++)
		{
			if (item.pButtonOffsets != NULL)
			{
				cfg->keyNButtonOffsets[j][i] = item.pButtonOffsets[i];
			}
			else
			{
				cfg->keyNButtonOffsets[j][i] = 0;
			}
		}

		// Save axes
		cfg->keyNAxisCount[j] = item.axisCount;
		for(i = 0; i < item.axisCount; i++)
		{
			// Save axes offsets
			if (item.pAxisOffsets != NULL)
			{
				cfg->keyNAxisOffsets[j][i] = item.pAxisOffsets[i];
			}
			else
			{
				cfg->keyNAxisOffsets[j][i] = 0;
			}

			// Save axes direction
			if (item.pAxisDirection != NULL)
			{
				cfg->keyNAxisDirection[j][i] = item.pAxisDirection[i];
			}
			else
			{
				cfg->keyNAxisDirection[j][i] = GameControllerItem::DirectionAny;
			}

			// Initial axes calibrations.
			ButtonItemData axis(GameControllerItem::Axis, cfg->keyNAxisOffsets[j][i], cfg->keyNAxisDirection[j][i]);
			shared_ptr<GameDeviceItem> spdev = dialog->devices[(size_t)datadeviceindex];
			if (SUCCEEDED(spdev->hrStatus))
			{
				axis.InitAxis(spdev->pInputJoy);
			}			

			cfg->axes[j][i] = axis;
		}

		// Save POV
		cfg->keyNPovCount[j] = item.povCount;
		for(i = 0; i < item.povCount; i++)
		{
			// Save pov offsets
			if (item.pPovOffsets != NULL)
			{
				cfg->keyNPovOffsets[j][i] = item.pPovOffsets[i];
			}
			else
			{
				cfg->keyNPovOffsets[j][i] = 0;
			}

			// Save pov direction
			if (item.pPovDirection != NULL)
			{
				cfg->keyNPovDirection[j][i] = item.pPovDirection[i];
			}
			else
			{
				cfg->keyNPovDirection[j][i] = GameControllerItem::DirectionAny;
			}

			// Initial axes calibrations.
			ButtonItemData povdata(GameControllerItem::Pov, cfg->keyNPovOffsets[j][i], cfg->keyNPovDirection[j][i]);
			cfg->pov[j][i] = povdata;
		}
	}

	// Save target C64 key assignments.
	int c64keycontrols[] = { controlNum.cbo_joykey1, controlNum.cbo_joykey2, controlNum.cbo_joykey3, controlNum.cbo_joykey4, controlNum.cbo_joykey5};
	cfg->enableKeyAssign = false;
	for(i = 0; i < _countof(c64keycontrols); i++)
	{
		if (i < _countof(cfg->isValidKeyNoAssign) && i < _countof(cfg->keyNoAssign))
		{
			cfg->isValidKeyNoAssign[i] = false;
			lr = SendDlgItemMessage(dialog->m_hWnd, c64keycontrols[i], CB_GETCURSEL, 0, 0);
			if (lr != CB_ERR)
			{
				lr = SendDlgItemMessage(dialog->m_hWnd, c64keycontrols[i], CB_GETITEMDATA, (WPARAM)lr, 0);
				if (lr != CB_ERR)
				{
					if (lr != 0)
					{
						cfg->keyNoAssign[i] = (C64Keys::C64Key)(lr & 0xff);
						cfg->isValidKeyNoAssign[i] = true;
						cfg->enableKeyAssign = true;
					}
				}
			}
		}
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.chk_joyenable))
	{
		cfg->IsEnabled = true;
	}
	else
	{
		cfg->IsEnabled = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.chk_joyenablepov))
	{
		cfg->isPovEnabled = true;
	}
	else
	{
		cfg->isPovEnabled = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.chk_joyvrev))
	{
		cfg->isYReverse = true;
	}
	else
	{
		cfg->isYReverse = false;
	}

	if (IsDlgButtonChecked(hWnd, this->controlNum.chk_joyhrev))
	{
		cfg->isXReverse = true;
	}
	else
	{
		cfg->isXReverse = false;
	}
}

CDiagJoystick::C64JoyItem::C64JoyItem(int ctrlid, C64JoystickButton::C64JoystickButtonNumber buttonnumber
	, DWORD * const pButtonOffsets, unsigned int &buttonCount
	, DWORD * const pAxisOffsets, GameControllerItem::ControllerAxisDirection * const pAxisDirection, unsigned int &axisCount
	, DWORD * const pPovOffsets, GameControllerItem::ControllerAxisDirection * const pPovDirection, unsigned int &povCount
	)
	: ctrlid(ctrlid), buttonnumber(buttonnumber)
	, pButtonOffsets(pButtonOffsets), buttonCount(buttonCount)
	, pAxisOffsets(pAxisOffsets), pAxisDirection(pAxisDirection), axisCount(axisCount)
	, pPovOffsets(pPovOffsets), pPovDirection(pPovDirection), povCount(povCount)
{
	pListItems = NULL;
}