#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "dx_version.h"
#include "CDPI.h"
#include "utils.h"
#include "gamedeviceitem.h"

GameDeviceItem::GameDeviceItem(LPDIRECTINPUT7 pDI, LPCDIDEVICEINSTANCE lpddi)
	:pDI(pDI), deviceInstance(*lpddi)
{
	this->pInputJoy = NULL;
	this->hrStatus = E_POINTER;
	this->inputDeviceFormat = &c_dfDIJoystick;
	this->sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
}

GameDeviceItem::~GameDeviceItem()
{
	CloseDevice();
}


HRESULT GameDeviceItem::OpenDevice(REFGUID refguid)
{
HRESULT hr;
DIDEVCAPS dicaps;
	CloseDevice();
	LPDIRECTINPUTDEVICE7 p = NULL;
	hr = pDI->CreateDeviceEx(refguid, IID_IDirectInputDevice7, (LPVOID *)&p, NULL);
	if (SUCCEEDED(hr))
	{
		ZeroMemory(&dicaps, sizeof(dicaps));
		dicaps.dwSize = sizeof(dicaps);
		hr = p->GetCapabilities(&dicaps);
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

			this->pInputJoy = p;
			hr = p->SetDataFormat(this->inputDeviceFormat);
		}
	}

	this->hrStatus = hr;
	return hr;
}

void GameDeviceItem::CloseDevice()
{
	if (this->pInputJoy)
	{
		this->pInputJoy->Release();
		this->pInputJoy = NULL;
		this->hrStatus = E_POINTER;
	}
}

