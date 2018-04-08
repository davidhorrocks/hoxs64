#pragma once

class GameDeviceItem
{
public:
	GameDeviceItem(LPDIRECTINPUT7, LPCDIDEVICEINSTANCE);
	~GameDeviceItem();

	HRESULT hrStatus;

	HRESULT OpenDevice(REFGUID refguid);
	void CloseDevice();
public:
	LPDIRECTINPUT7 pDI;
	LPDIRECTINPUTDEVICE7 pInputJoy;
	DIDEVICEINSTANCE deviceInstance;
};
