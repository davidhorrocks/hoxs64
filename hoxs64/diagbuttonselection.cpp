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

CDiagButtonSelection::CDiagButtonSelection(LPDIRECTINPUT7 pDI, GUID deviceId, int c64JoystickNumber, C64JoystickButton::C64JoystickButtonNumber c64button, vector<ButtonItemData> &controllerItemOffsets)
	: deviceId(deviceId), c64JoystickNumber(c64JoystickNumber), pDI(pDI), pJoy(NULL), c64button(c64button)
{
	initvars();
	vector<ButtonItemData>::iterator iter;
	for (iter = controllerItemOffsets.begin(); iter != controllerItemOffsets.end(); iter++)
	{
		shared_ptr<ButtonItemData> p = make_shared<ButtonItemData>(*iter);
		listCurrentControllerItem.push_back(p);
	}
}

CDiagButtonSelection::~CDiagButtonSelection()
{
	if (pJoy)
	{
		pJoy->Release();
		pJoy = NULL;
	}

	if (pAnsiStringBuffer)
	{
		free(pAnsiStringBuffer);
		pAnsiStringBuffer = NULL;
		lenAnsiStringBuffer = 0;
	}
}

BOOL CDiagButtonSelection::DialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		if (SUCCEEDED(init()))
		{
			SetTimer(hWndDlg, IDT_TIMER1, TIMER1_DELAY, NULL);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
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
	resultButtons.clear();
	vector<shared_ptr<ButtonItemData>>::const_iterator iter;
	for (iter = listCurrentControllerItem.cbegin(); iter != listCurrentControllerItem.cend(); iter++)
	{
		resultButtons.push_back(*(iter->get()));
	}
}

void CDiagButtonSelection::ClearAllButtons()
{
	if (hwndListBox)
	{
		SendMessage(hwndListBox, LB_RESETCONTENT, 0, 0);
		listCurrentControllerItem.clear();
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
						vector<shared_ptr<ButtonItemData>> tempBuffer;
						vector<shared_ptr<ButtonItemData>>::iterator iter;
						for (iter = listCurrentControllerItem.begin(); iter != listCurrentControllerItem.end(); iter++)
						{
							if ((*iter)->itemType != p->itemType || (*iter)->controllerItemOffset != p->controllerItemOffset || (*iter)->axisPovDirection != p->axisPovDirection)
							{
								tempBuffer.push_back(*iter);
							}
						}

						listCurrentControllerItem = tempBuffer;
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

void CDiagButtonSelection::InitAllAxes()
{	
	vector<AxisState>::iterator iter;
	for (iter = listAllControllerItem.begin(); iter != listAllControllerItem.end(); iter++)
	{
		AxisState& item = *iter;
		if (item.option == AxisState::Axis)
		{
			if (item.objectInfo.dwOfs <= sizeof(DIJOYSTATE2) - sizeof(DWORD))
			{
				InitAxis(&item);
			}
		}
	}
}

void CDiagButtonSelection::InitAxis(AxisState *axis)
{
DIPROPRANGE diprg;
HRESULT hr;
LONG minValue = 0;
LONG maxValue = 0;
	ZeroMemory(&diprg, sizeof(diprg));
	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow        = DIPH_BYOFFSET; 
	diprg.diph.dwObj        = axis->objectInfo.dwOfs; // Specify the enumerated axis
	diprg.lMin              = ButtonItemData::DefaultMin; 
	diprg.lMax              = ButtonItemData::DefaultMax; 
	hr = pJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (SUCCEEDED(hr))
	{
		minValue = ButtonItemData::DefaultMin; 
		maxValue = ButtonItemData::DefaultMax; 
	}
	else
	{
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYOFFSET; 
		diprg.diph.dwObj        = axis->objectInfo.dwOfs; // Specify the enumerated axis
		hr = pJoy->GetProperty(DIPROP_RANGE, &diprg.diph);
		if (SUCCEEDED(hr))
		{
			minValue = diprg.lMin;
			maxValue = diprg.lMax;
		}
		else
		{
			//SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
		}
	}

	if (SUCCEEDED(hr))
	{
		axis->hasMinMax = true;
		axis->minValue = minValue;
		axis->maxValue = maxValue;
		axis->minActiveValue = (LONG)((double)axis->minValue + ButtonItemData::SensitivityConfig * (double)(axis->maxValue - axis->minValue) / 2.0);
		axis->maxActiveValue = (LONG)((double)axis->maxValue - ButtonItemData::SensitivityConfig * (double)(axis->maxValue - axis->minValue) / 2.0);
	}
}

void CDiagButtonSelection::PollJoystick()
{
HRESULT hr;
DIJOYSTATE2  js;
const unsigned int MAXTRIGGERCOUNT = 1;
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
			// Check for buttons held down.
			vector<AxisState>::iterator iter;
			unsigned int buttonNumber;
			DWORD offset;
			for (buttonNumber = 0; buttonNumber < joyconfig::MAXBUTTONS; buttonNumber++)
			{
				offset = DIJOFS_BUTTON(buttonNumber);
				if (IsValidControllerItemNameOffset(offset) || buttonNumber < joyconfig::MAXV1BUTTONS)
				{
					if (js.rgbButtons[buttonNumber] & 0x80)
					{
						// Button is held down.
						for (iter = listAllControllerItem.begin(); iter != listAllControllerItem.end(); iter++)
						{
							if (iter->option == AxisState::Button && iter->objectInfo.dwOfs == offset)
							{
								AmendList(*iter);
								break;
							}
						}
					}
				}
			}

			// Check for axes movement.
			for (iter = listAllControllerItem.begin(); iter != listAllControllerItem.end(); iter++)
			{
				AxisState& item = *iter;
				if (item.option == AxisState::Axis)
				{
					offset = item.objectInfo.dwOfs;
					if (offset <= sizeof(DIJOYSTATE2) - sizeof(DWORD))
					{
						LONG value = *((LONG *)(((char *)&js) + offset));
						if (item.direction == AxisState::DirectionMin)
						{
							if (item.IsTriggeredMin(value) && item.notMinCount != 0)
							{
								AmendList(*iter);
							}
							else
							{
								if (item.notMinCount < MAXTRIGGERCOUNT)
								{
									item.notMinCount++;
								}
							}
						}

						if (item.direction == AxisState::DirectionMax)
						{
							if (item.IsTriggeredMax(value) && item.notMaxCount != 0)
							{
								AmendList(*iter);
							}
							else
							{
								if (item.notMaxCount < MAXTRIGGERCOUNT)
								{
									item.notMaxCount++;
								}
							}
						}
					}
				}
			}

			// Check for POV movement.
			for (iter = listAllControllerItem.begin(); iter != listAllControllerItem.end(); iter++)
			{
				// Check for axes held centred.
				AxisState& item = *iter;
				if (item.option == AxisState::Pov)
				{
					offset = item.objectInfo.dwOfs;
					
					if (offset >= DIJOFS_POV(0) && offset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
					{
						DWORD pov = *((DWORD *)(((BYTE *)&js) + offset));
						if (LOWORD(pov) != 0xFFFF)
						{
							GameControllerItem::ControllerAxisDirection direction = GameControllerItem::DirectionAny;
							bool moved = false;
							if (pov < ButtonItemData::POVRightUp || pov >= ButtonItemData::POVLeftUp)
							{
								direction = GameControllerItem::DirectionUp;
								moved = true;
							}
							else if (pov >= ButtonItemData::POVRightDown && pov < ButtonItemData::POVLeftDown)
							{
								direction = GameControllerItem::DirectionDown;
								moved = true;
							}
							else if (pov < ButtonItemData::POVUpLeft && pov >= ButtonItemData::POVDownLeft)
							{
								direction = GameControllerItem::DirectionLeft;
								moved = true;
							}
							else if (pov >= ButtonItemData::POVUpRight && pov < ButtonItemData::POVDownRight)
							{
								direction = GameControllerItem::DirectionRight;
								moved = true;
							}

							if (moved && direction == iter->direction)
							{
								AmendList(*iter);
							}
						}
					}
				}
			}
		}
	}
}

void CDiagButtonSelection::AmendList(AxisState item)
{
LRESULT lr;
	try
	{
		bool found = false;
		bool ok = true;
		unsigned int count = (unsigned int)SendMessage(hwndListBox, LB_GETCOUNT , 0, 0);
		if (count != LB_ERR)
		{
			for(unsigned int i=0; i < count; i++)
			{
				lr = SendMessage(hwndListBox, LB_GETITEMDATA, i, 0);
				if (lr != LB_ERR)
				{
					ButtonItemData *dditem = (ButtonItemData *)lr;
					if (dditem != NULL)
					{
						if (item.option == AxisState::Button)
						{
							if (dditem->controllerItemOffset == item.objectInfo.dwOfs)
							{
								SendMessage(hwndListBox, LB_SETCURSEL, i, 0);						
								found = true;
								break;
							}
						}
						else if (item.option == AxisState::Axis)
						{
							if (dditem->controllerItemOffset == item.objectInfo.dwOfs && dditem->axisPovDirection == item.direction)
							{
								SendMessage(hwndListBox, LB_SETCURSEL, i, 0);						
								found = true;
								break;
							}
						}
						else if (item.option == AxisState::Pov)
						{
							if (dditem->controllerItemOffset == item.objectInfo.dwOfs && dditem->axisPovDirection == item.direction)
							{
								SendMessage(hwndListBox, LB_SETCURSEL, i, 0);						
								found = true;
								break;
							}
						}
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
			DWORD offset = item.objectInfo.dwOfs;
			deviceButtonName.clear();
			shared_ptr<ButtonItemData> p = make_shared<ButtonItemData>(ButtonItemData(item.option, offset, item.direction));
			listCurrentControllerItem.push_back(p);				
			item.GetName(deviceButtonName);
			int pos = (int)SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM) deviceButtonName.c_str());
			if (pos >= 0)
			{
				SendMessage(hwndListBox, LB_SETITEMDATA, pos, (LPARAM) p.get());
			}
		}
	}
	catch(std::exception &)
	{
	}
}

void CDiagButtonSelection::initvars()
{	
	inputDeviceFormat = &c_dfDIJoystick;
	sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
	hwndDeviceName = 0;
	hwndMappedName = 0;
	hwndListBox = 0;
	pAnsiStringBuffer = NULL;
	lenAnsiStringBuffer = 0;
	allowButtons = false;
	allowAxes = false;
	allowPov = false;
}

HRESULT CDiagButtonSelection::init()
{
HRESULT hr = E_FAIL;
DIDEVCAPS dicaps;
DIPROPSTRING phName;
TCHAR strnumber[20];
int len;

	try
	{
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
			allowButtons = true;		
			break;
		case C64JoystickButton::Fire2:
			mappedName.append(TEXT("Fire 2"));
			allowButtons = true;
			break;
		case C64JoystickButton::Up:
			mappedName.append(TEXT("Up"));
			allowButtons = true;
			break;
		case C64JoystickButton::Down:
			mappedName.append(TEXT("Down"));
			allowButtons = true;
			break;
		case C64JoystickButton::Left:
			mappedName.append(TEXT("Left"));
			allowButtons = true;
			break;
		case C64JoystickButton::Right:
			mappedName.append(TEXT("Right"));
			allowButtons = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey1:
			mappedName.append(TEXT("Key 1"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey2:
			mappedName.append(TEXT("Key 2"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey3:
			mappedName.append(TEXT("Key 3"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey4:
			mappedName.append(TEXT("Key 4"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey5:
			mappedName.append(TEXT("Key 5"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		case C64JoystickButton::ButtonAndAxisKey6:
			mappedName.append(TEXT("Key 6"));
			allowButtons = true;
			allowAxes = true;
			allowPov = true;
			break;
		}

		std::basic_string<TCHAR> title;
		std::basic_string<TCHAR> andtext;
		andtext.append(TEXT(" And "));
		if (allowButtons)
		{
			if (title.length() != 0)
			{
				title.append(andtext);
			}

			title.append(TEXT("Buttons"));
		}
	
		if (allowAxes)
		{
			if (title.length() != 0)
			{
				title.append(andtext);
			}

			title.append(TEXT("Axes"));
		}
	
		if (allowPov)
		{
			if (title.length() != 0)
			{
				title.append(andtext);
			}

			title.append(TEXT("POV"));
		}

		SetWindowText(this->m_hWnd, title.c_str());
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
					if (this->allowButtons)
					{
						pJoy->EnumObjects(::EnumDlgJoyButtonSelectionCallback, this, DIDFT_BUTTON);
					}

					if (this->allowAxes)
					{					
						pJoy->EnumObjects(::EnumDlgJoyAxisSelectionCallback, this, DIDFT_ABSAXIS);
					}

					if (this->allowAxes)
					{					
						pJoy->EnumObjects(::EnumDlgJoyPovSelectionCallback, this, DIDFT_POV);
					}

					InitAllAxes();
					if (hwndListBox)
					{
						vector<shared_ptr<ButtonItemData>>::iterator iter;
						for (iter = listCurrentControllerItem.begin(); iter != listCurrentControllerItem.end(); iter++)
						{
							shared_ptr<ButtonItemData> currentItem = *iter;
							deviceButtonName.clear();
							DWORD offset = currentItem->controllerItemOffset;
							bool found = false;
							vector<AxisState>::iterator iterAllItem;
							for (iterAllItem = listAllControllerItem.begin(); iterAllItem != listAllControllerItem.end() ; iterAllItem++)
							{
								AxisState& item = *iterAllItem;
								if (currentItem->itemType == GameControllerItem::Button)
								{
									if (currentItem->controllerItemOffset == item.objectInfo.dwOfs)
									{
										found = true;								
										break;
									}
								}
								else if (currentItem->itemType == GameControllerItem::Axis)
								{
									if (currentItem->itemType == item.option && currentItem->axisPovDirection == item.direction && currentItem->controllerItemOffset == item.objectInfo.dwOfs)
									{
										found = true;
										break;
									}
								}
								else if (currentItem->itemType == GameControllerItem::Pov)
								{
									if (currentItem->itemType == item.option && currentItem->axisPovDirection == item.direction && currentItem->controllerItemOffset == item.objectInfo.dwOfs)
									{
										found = true;
										break;
									}
								}
							}

							if (found)
							{
								AxisState& deviceitem = *iterAllItem;
								deviceitem.GetName(deviceButtonName);
								int pos = (int)SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM) deviceButtonName.c_str());
								if (pos >= 0)
								{
									SendMessage(hwndListBox, LB_SETITEMDATA, pos, (LPARAM) currentItem.get());
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
	#ifdef UNICODE
						Edit_SetText(hwndDeviceName, phName.wsz);
	#else

						int maxallowedstringlen = _countof(phName.wsz);
						int lenAnsiBuffer = 0;
						bool stringOK = false;
						if (SUCCEEDED(G::UcToAnsiRequiredBufferLength(phName.wsz, maxallowedstringlen - 1, lenAnsiBuffer)))
						{
							if (lenAnsiBuffer > 0)
							{
								char *p = this->AllocAnsiStringBuffer(lenAnsiBuffer);
								if (p != NULL)
								{
									if (SUCCEEDED(G::UcToAnsi(phName.wsz, p, lenAnsiBuffer)))
									{
										stringOK = true;
										Edit_SetText(hwndDeviceName, p);
									}
								}
							}
						}

						if (!stringOK)
						{
							Edit_SetText(hwndDeviceName, TEXT(""));
						}
	#endif
					}
				}

				hr = pJoy->SetCooperativeLevel(this->m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
			}
		}
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}

	return hr;
}

char *CDiagButtonSelection::AllocAnsiStringBuffer(size_t size)
{
	if (this->lenAnsiStringBuffer < size)
	{
		if (this->pAnsiStringBuffer)
		{
			free(this->pAnsiStringBuffer);
			this->pAnsiStringBuffer = 0;
		}

		this->pAnsiStringBuffer = (char *)malloc(size + 1);
		if (this->pAnsiStringBuffer)
		{
			this->lenAnsiStringBuffer = size;
		}
	}

	return this->pAnsiStringBuffer;
}

bool CDiagButtonSelection::IsValidControllerItemNameOffset(DWORD offset)
{
	vector<AxisState>::iterator iter;
	for (iter = listAllControllerItem.begin(); iter != listAllControllerItem.end(); iter++)
	{
		DIDEVICEOBJECTINSTANCE &obj = iter->objectInfo;
		if (obj.dwOfs == offset)
		{
			return true;
		}
	}

	return false;
}

BOOL CDiagButtonSelection::EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	try
	{
		AxisState item(GameControllerItem::Button, GameControllerItem::DirectionAny, *lpddoi);
		listAllControllerItem.push_back(item);
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagButtonSelection::EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	try
	{
		AxisState itemMin(GameControllerItem::Axis, GameControllerItem::DirectionMin, *lpddoi);
		AxisState itemMax(GameControllerItem::Axis, GameControllerItem::DirectionMax, *lpddoi);
		listAllControllerItem.push_back(itemMin);
		listAllControllerItem.push_back(itemMax);
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CDiagButtonSelection::EnumJoyPov(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	try
	{
		AxisState itemUp(GameControllerItem::Pov, GameControllerItem::DirectionUp, *lpddoi);
		AxisState itemDown(GameControllerItem::Pov, GameControllerItem::DirectionDown, *lpddoi);
		AxisState itemLeft(GameControllerItem::Pov, GameControllerItem::DirectionLeft, *lpddoi);
		AxisState itemRight(GameControllerItem::Pov, GameControllerItem::DirectionRight, *lpddoi);
		listAllControllerItem.push_back(itemUp);
		listAllControllerItem.push_back(itemDown);
		listAllControllerItem.push_back(itemLeft);
		listAllControllerItem.push_back(itemRight);
	}
	catch (std::exception&)
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}
BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagButtonSelection *) pvRef)->EnumJoyButton(lpddoi);
}

BOOL CALLBACK EnumDlgJoyAxisSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagButtonSelection *) pvRef)->EnumJoyAxis(lpddoi);
}

BOOL CALLBACK EnumDlgJoyPovSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((CDiagButtonSelection *) pvRef)->EnumJoyPov(lpddoi);
}

CDiagButtonSelection::AxisState::AxisState()
	: GameControllerItem(GameControllerItem::None)
{
	Init();
}

CDiagButtonSelection::AxisState::AxisState(GameControllerItem::ControllerItemOption option)
	: GameControllerItem(option)
{
	Init();
}

CDiagButtonSelection::AxisState::AxisState(GameControllerItem::ControllerItemOption option, GameControllerItem::ControllerAxisDirection direction)
	: GameControllerItem(option, direction)
{
	Init();
}

CDiagButtonSelection::AxisState::AxisState(GameControllerItem::ControllerItemOption option, GameControllerItem::ControllerAxisDirection direction, const DIDEVICEOBJECTINSTANCE& objectInfo)
	: GameControllerItem(option, direction, objectInfo)
{
	Init();
}

void CDiagButtonSelection::AxisState::Init()
{
	hasMinMax = false;
	maxValue = 0;
	minValue = 0;
	minActiveValue = 0;
	maxActiveValue = 0;
	centredCount = 0;
	notMinCount = 0;
	notMaxCount = 0;
	minCount = 0;
	maxCount = 0;
}


bool CDiagButtonSelection::AxisState::IsTriggeredMin(LONG value)
{
	return value < this->minActiveValue;
}

bool CDiagButtonSelection::AxisState::IsTriggeredMax(LONG value)
{
	return value > this->maxActiveValue;
}

void CDiagButtonSelection::AxisState::GetName(std::basic_string<TCHAR> &name)
{	
	name.clear();
	if (this->objectInfo.dwSize!=0)
	{
		name.append(this->objectInfo.tszName);
		if (this->direction == AxisState::DirectionMin)
		{
			name.append(TEXT(" <"));
		}
		else if (this->direction == AxisState::DirectionMax)
		{
			name.append(TEXT(" >"));
		}
		else if (this->direction == AxisState::DirectionUp)
		{
			name.append(TEXT(" Up"));
		}
		else if (this->direction == AxisState::DirectionDown)
		{
			name.append(TEXT(" Down"));
		}
		else if (this->direction == AxisState::DirectionLeft)
		{
			name.append(TEXT(" Left"));
		}
		else if (this->direction == AxisState::DirectionRight)
		{
			name.append(TEXT(" Right"));
		}
	}
}
