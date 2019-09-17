#include "gamecontrolleritem.h"

const double ButtonItemData::SensitivityGame = 0.6;
const double ButtonItemData::SensitivityConfig = 0.5;

ButtonItemData::ButtonItemData()
	: itemType(GameControllerItem::Button)
	, controllerItemOffset(DIJOFS_BUTTON0)
	, axisPovDirection(GameControllerItem::DirectionAny)
{
	initvars();
}

ButtonItemData::ButtonItemData(GameControllerItem::ControllerItemOption itemType, DWORD controllerItemOffset, GameControllerItem::ControllerAxisDirection axisPovDirection)
	: itemType(itemType)
	, controllerItemOffset(controllerItemOffset)
	, axisPovDirection(axisPovDirection)
{
	initvars();
}

void ButtonItemData::initvars()
{
	minAxisValue = 0;
	maxAxisValue = 0;
	minAxisTrigger = 0;
	maxAxisTrigger = 0;
	hasMinMax = false;
}

HRESULT ButtonItemData::InitAxis(LPDIRECTINPUTDEVICE7 pJoy)
{
DIPROPRANGE diprg;
HRESULT hr;
LONG minValue = 0;
LONG maxValue = 0;
		
	ZeroMemory(&diprg, sizeof(diprg));
	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow        = DIPH_BYOFFSET; 
	diprg.diph.dwObj        = controllerItemOffset; // Specify the enumerated axis
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
		diprg.diph.dwObj        = controllerItemOffset; // Specify the enumerated axis
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
		hasMinMax = true;
		minValue = minValue;
		maxValue = maxValue;
		minAxisTrigger = (LONG)((double)minValue + ButtonItemData::SensitivityGame * (double)(maxValue - minValue) / 2.0);
		maxAxisTrigger = (LONG)((double)maxValue - ButtonItemData::SensitivityGame * (double)(maxValue - minValue) / 2.0);
	}
	else
	{
		hasMinMax = false;
		minValue = 0;
		maxValue = 0;
		minAxisTrigger = 0;
		maxAxisTrigger = 0;
	}

	return hr;
}
