#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include "gamedeviceitem.h"

GameDeviceItem::GameDeviceItem(LPDIRECTINPUT7 pDI, LPCDIDEVICEINSTANCE lpddi)
	:pDI(pDI), deviceInstance(*lpddi)
{
	pInputJoy = NULL;
	hrStatus = E_POINTER;
}

GameDeviceItem::~GameDeviceItem()
{
	CloseDevice();
}


HRESULT GameDeviceItem::OpenDevice(REFGUID refguid)
{
	CloseDevice();
	LPDIRECTINPUTDEVICE7 p = NULL;
	hrStatus = pDI->CreateDeviceEx(refguid, IID_IDirectInputDevice7, (LPVOID *)&p, NULL);
	if (SUCCEEDED(hrStatus))
	{
		this->pInputJoy = p;
		hrStatus = p->SetDataFormat(&c_dfDIJoystick);
	}

	return hrStatus;
}

void GameDeviceItem::CloseDevice()
{
	if (this->pInputJoy)
	{
		this->pInputJoy->Release();
		this->pInputJoy = NULL;
		hrStatus = E_POINTER;
	}
}

