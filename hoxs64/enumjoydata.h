#pragma once
#include <windows.h>
#include "dx_version.h"
#include "hconfig.h"

class EnumJoyData
{
public:
	EnumJoyData();

	int buttonCount;
	DWORD buttonOffsets[joyconfig::MAXBUTTONS];
	BOOL EnumButton(LPCDIDEVICEOBJECTINSTANCE lpddoi);
	void Reset();
	static BOOL CALLBACK EnumJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
};