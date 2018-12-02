#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <windowsx.h>
#include <winuser.h>
#include "dx_version.h"
#include "boost2005.h"
#include <stdio.h>
#include "servicerelease.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "diagbuttonselection.h"
#include "resource.h"

#define IDT_TIMER1 1001
#define TIMER1_DELAY 80

CDiagButtonSelection::CDiagButtonSelection()
	: c64JoystickNumber(0), pDI(NULL), pJoy(NULL), c64button(C64JoystickButton::None)
{
	ZeroMemory(&deviceId, sizeof(deviceId));	
	initvars();
	inputDeviceFormat = &c_dfDIJoystick;
	sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
}

CDiagButtonSelection::CDiagButtonSelection(LPDIRECTINPUT7 pDI, GUID deviceId, int c64JoystickNumber, C64JoystickButton::C64JoystickButtonNumber c64button, std::vector<DWORD> &buttonOffsets)
	: deviceId(deviceId), c64JoystickNumber(c64JoystickNumber), pDI(pDI), pJoy(NULL), c64button(c64button)
{
	initvars();
	std::vector<DWORD>::iterator iter;
	for (iter = buttonOffsets.begin(); iter != buttonOffsets.end(); iter++)
	{
		DWORD d = *iter;
		shared_ptr<ButtonItemData> p;
		p = make_shared<ButtonItemData>(d);
		p->buttonOffset = d;
		currentButtonOffsets.push_back(p);
	}
}

CDiagButtonSelection::~CDiagButtonSelection()
{
	if (pJoy)
	{
		pJoy->Release();
		pJoy = NULL;
	}
}

BOOL CDiagButtonSelection::DialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		init();
		SetTimer(hWndDlg, IDT_TIMER1, TIMER1_DELAY, NULL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			UpdateResult();
			EndDialog(hWndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, LOWORD(wParam));
			return TRUE;
		case IDC_BTN_REMOVE:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ClearSelectedButton();				
			}

			break;
		case IDC_BTN_CLEARALL:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				ClearAllButtons();
			}

			break;
		}

		break;
	case WM_TIMER:
		if (wParam == IDT_TIMER1)
		{
			PollJoystick();
		}

		break;
	case WM_DESTROY:
		KillTimer(hWndDlg, IDT_TIMER1);
		return TRUE;
	}

	return FALSE;
}

void CDiagButtonSelection::UpdateResult()
{
	resultButtonOffsets.clear();
	std::vector<shared_ptr<ButtonItemData>>::const_iterator iter;
	for (iter = currentButtonOffsets.cbegin(); iter != currentButtonOffsets.cend(); iter++)
	{
		resultButtonOffsets.push_back((*iter)->buttonOffset);
	}
}

void CDiagButtonSelection::ClearAllButtons()
{
	if (hwndListBox)
	{
		SendMessage(hwndListBox, LB_RESETCONTENT, 0, 0);
		currentButtonOffsets.clear();
	}				
}

void CDiagButtonSelection::ClearSelectedButton()
{
LRESULT lr;
	if (hwndListBox)
	{
		lr = SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
		if (lr != LB_ERR && lr >= 0)
		{
			unsigned int currentIndex = (unsigned int)lr;
			if (currentIndex >= 0)
			{
				lr = SendMessage(hwndListBox, LB_GETITEMDATA , currentIndex, 0);
				if (lr != LB_ERR)
				{
					ButtonItemData *p = (ButtonItemData *)lr;				
				
					lr = SendMessage(hwndListBox, LB_DELETESTRING, currentIndex, 0);
					if (lr != LB_ERR)
					{
						std::vector<shared_ptr<ButtonItemData>> tempBuffer;
						std::vector<shared_ptr<ButtonItemData>>::iterator iter;
						for (iter = currentButtonOffsets.begin(); iter != currentButtonOffsets.end(); iter++)
						{
							if ((*iter)->buttonOffset != p->buttonOffset)
							{
								tempBuffer.push_back(*iter);
							}
						}

						currentButtonOffsets = tempBuffer;
					}

					if (lr > 0)
					{
						if (currentIndex >= (unsigned int)lr)
						{
							currentIndex = (unsigned int)lr - 1;
						}
					}

					SendMessage(hwndListBox, LB_SETCURSEL, currentIndex, 0);
				}
			}
		}
	}				
}

void CDiagButtonSelection::PollJoystick()
{
HRESULT hr;
DIJOYSTATE2  js;

	if (pJoy)
	{
		ZeroMemory(&js, sizeof(js));
		pJoy->Poll();
		hr = pJoy->GetDeviceState(this->sizeOfInputDeviceFormat, &js);
		if(hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			hr = pJoy->Acquire();
			if (SUCCEEDED(hr))
			{
				ZeroMemory(&js, sizeof(js));
				hr = pJoy->GetDeviceState(this->sizeOfInputDeviceFormat, &js);
			}
		}

		if (SUCCEEDED(hr))
		{
			unsigned int b;
			for (b = 0; b < joyconfig::MAXBUTTONS; b++)
			{
				if (js.rgbButtons[b] & 0x80)
				{
					AmendList(b);
				}
			}			
		}
	}
}

void CDiagButtonSelection::AmendList(unsigned int buttonNumber)
{
LRESULT lr;
	try
	{
		if (buttonNumber < joyconfig::MAXBUTTONS)
		{
			bool found = false;
			bool ok = true;
			DWORD offset = DIJOFS_BUTTON(buttonNumber);
			unsigned int count = (unsigned int)SendMessage(hwndListBox, LB_GETCOUNT , 0, 0);
			if (count != LB_ERR)
			{
				for(unsigned int i=0; i < count; i++)
				{
					lr = SendMessage(hwndListBox, LB_GETITEMDATA, i, 0);
					if (lr != LB_ERR)
					{
						if (((ButtonItemData *)lr)->buttonOffset == offset)
						{
							SendMessage(hwndListBox, LB_SETCURSEL, i, 0);						
							found = true;
							break;
						}
					}				
					else
					{
						ok = false;
						break;
					}
				}
			}
			else
			{
				ok = false;
			}

			if (ok && !found)
			{
				deviceButtonName.clear();
				shared_ptr<ButtonItemData> p = make_shared<ButtonItemData>(offset);
				currentButtonOffsets.push_back(p);
				if (GetButtonNameFromOffset(deviceButtonName, offset))
				{
					int pos = (int)SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM) deviceButtonName.c_str());
					if (pos >= 0)
					{
						SendMessage(hwndListBox, LB_SETITEMDATA, pos, (LPARAM) p.get());
					}
				}
			}
		}
	}
	catch(std::exception &)
	{
	}
}

void CDiagButtonSelection::initvars()
{
	hwndDeviceName = 0;
	hwndMappedName = 0;
	hwndListBox = 0;
}

HRESULT CDiagButtonSelection::init()
{
HRESULT hr = E_FAIL;
DIDEVCAPS dicaps;
DIPROPSTRING phName;
TCHAR strnumber[20];
int len;

	hwndDeviceName = ::GetDlgItem(this->m_hWnd, IDC_TXT_DEVICE_NAME);
	hwndMappedName = ::GetDlgItem(this->m_hWnd, IDC_TXT_MAPPED_NAME);
	hwndListBox = ::GetDlgItem(this->m_hWnd, IDC_LIST_BUTTONS);
	mappedName.clear();	
	mappedName.append(TEXT("Joystick"));	
	len = _sntprintf(strnumber, _countof(strnumber) - 1, TEXT("%d"), this->c64JoystickNumber);
	if (len > 0 && len < _countof(strnumber))
	{
		strnumber[len] = TEXT('\0');
		mappedName.append(TEXT(" "));
		mappedName.append(strnumber);
	}	

	mappedName.append(TEXT(" "));
	switch (c64button)
	{
	case C64JoystickButton::Fire1:		
		mappedName.append(TEXT("Fire"));
		break;
	case C64JoystickButton::Fire2:
		mappedName.append(TEXT("Fire 2"));
		break;
	case C64JoystickButton::Up:
		mappedName.append(TEXT("Up"));
		break;
	case C64JoystickButton::Down:
		mappedName.append(TEXT("Down"));
		break;
	case C64JoystickButton::Left:
		mappedName.append(TEXT("Left"));
		break;
	case C64JoystickButton::Right:
		mappedName.append(TEXT("Right"));
		break;
	}
	

	if (hwndMappedName)
	{
		Edit_SetText(hwndMappedName, this->mappedName.c_str());
	}
	
	hr = pDI->CreateDeviceEx(deviceId, IID_IDirectInputDevice7, (LPVOID *)&pJoy, NULL);
	if (SUCCEEDED(hr))
	{
		ZeroMemory(&dicaps, sizeof(dicaps));
		dicaps.dwSize = sizeof(dicaps);
		hr = pJoy->GetCapabilities(&dicaps);
		if (SUCCEEDED(hr))
		{
			if (G::IsLargeGameDevice(dicaps))
			{
				this->inputDeviceFormat = &c_dfDIJoystick2;
				this->sizeOfInputDeviceFormat = sizeof(DIJOYSTATE2);

			}
			else
			{
				this->inputDeviceFormat = &c_dfDIJoystick;
				this->sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
			}

			hr = pJoy->SetDataFormat(this->inputDeviceFormat);
			if (SUCCEEDED(hr))
			{
				hr = pJoy->EnumObjects(::EnumDlgJoyButtonSelectionCallback, this, DIDFT_BUTTON);
				if (hwndListBox)
				{
					std::vector<shared_ptr<ButtonItemData>>::iterator iter;
					for (iter = currentButtonOffsets.begin(); iter != currentButtonOffsets.end(); iter++)
					{
						deviceButtonName.clear();
						DWORD offset = (*iter)->buttonOffset;			
						if (GetButtonNameFromOffset(deviceButtonName, offset))
						{
							int pos = (int)SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM) deviceButtonName.c_str());
							if (pos >= 0)
							{
								SendMessage(hwndListBox, LB_SETITEMDATA, pos, (LPARAM) (*iter).get());
							}
						}
					}
				}
			}

			ZeroMemory(&phName, sizeof(phName));
			phName.diph.dwSize       = sizeof(DIPROPSTRING); 
			phName.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			if (SUCCEEDED(pJoy->GetProperty(DIPROP_INSTANCENAME, &phName.diph)))
			{
				if (hwndDeviceName)
				{
					Edit_SetText(hwndDeviceName, phName.wsz);
				}
			}

			hr = pJoy->SetCooperativeLevel(this->m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		}
	}

	return hr;
}

bool CDiagButtonSelection::GetButtonNameFromOffset(std::basic_string<TCHAR> &name, DWORD offset)
{	
	std::vector<GameControllerItem>::iterator iter;
	if (offset >= DIJOFS_BUTTON0 && offset < DIJOFS_BUTTON(joyconfig::MAXBUTTONS))
	{
		for (iter = buttons.begin(); iter != buttons.end(); iter++)
		{
			DIDEVICEOBJECTINSTANCE &obj = (*iter).objectInfo;
			if (obj.dwOfs == offset)
			{
				name.append(obj.tszName);
				return true;
			}
		}
	}

	name.append(TEXT("?"));
	return true;
}

BOOL CDiagButtonSelection::EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	try
	{
		GameControllerItem item;
		item.option = GameControllerItem::Button;
		item.objectInfo = *lpddoi;
		buttons.push_back(item);
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

CDiagButtonSelection::ButtonItemData::ButtonItemData(DWORD buttonOffset)
	: buttonOffset(buttonOffset)
{
}


CDiagButtonSelection::ButtonItemData::~ButtonItemData()
{
}

BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagButtonSelection *) pvRef)->EnumJoyButton(lpddoi);
}
