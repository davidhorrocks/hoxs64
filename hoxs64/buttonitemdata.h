#pragma once

class ButtonItemData
{
public:
	static const LONG DefaultMin = -1000;
	static const LONG DefaultMax = 1000;
	static const DWORD POVRightUp = 9000 - 2250;
	static const DWORD POVRightDown = 9000 + 2250;
	static const DWORD POVLeftUp = 27000 + 2250;
	static const DWORD POVLeftDown = 27000 - 2250;
	static const DWORD POVUpLeft = 36000 - 2250;
	static const DWORD POVUpRight = 2250;
	static const DWORD POVDownLeft = 18000 + 2250;
	static const DWORD POVDownRight = 18000 - 2250;
	static const double SensitivityGame;
	static const double SensitivityConfig;
	ButtonItemData();
	ButtonItemData(GameControllerItem::ControllerItemOption itemType, DWORD controllerItemOffset, GameControllerItem::ControllerAxisDirection axisPovDirection);
	HRESULT InitAxis(LPDIRECTINPUTDEVICE7 pJoy);
private:
	void initvars();
public:
	GameControllerItem::ControllerItemOption itemType;
	DWORD controllerItemOffset;
	GameControllerItem::ControllerAxisDirection axisPovDirection;
	bool hasMinMax;
	LONG minAxisValue;
	LONG maxAxisValue;
	LONG minAxisTrigger;
	LONG maxAxisTrigger;

};
