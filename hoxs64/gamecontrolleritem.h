#pragma once

struct GameControllerItem
{
	typedef enum tagControllerItemOption
	{
		Button,
		Axis
	} ControllerItemOption;

	GameControllerItem();

	ControllerItemOption option;
	DIDEVICEOBJECTINSTANCE objectInfo;
};
