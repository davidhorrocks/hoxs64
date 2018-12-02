#pragma once

class GameDeviceItem
{
public:
	GameDeviceItem(LPDIRECTINPUT7, LPCDIDEVICEINSTANCE);
	~GameDeviceItem();

	HRESULT OpenDevice(REFGUID refguid);
	void CloseDevice();

	HRESULT hrStatus;
	LPDIRECTINPUT7 pDI;
	LPDIRECTINPUTDEVICE7 pInputJoy;
	DIDEVICEINSTANCE deviceInstance;
	LPCDIDATAFORMAT inputDeviceFormat;
	DWORD sizeOfInputDeviceFormat;
};
