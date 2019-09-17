#pragma once
#include <windows.h>
#include <tchar.h>
#include "dx_version.h"
#include "gamedeviceitem.h"

struct GameControllerItem
{
	typedef enum tagControllerItemOption : unsigned int
	{
		None = 0,
		Button,
		Axis,
		MultipleButton,
		AllButtons,
		Pov,
	} ControllerItemOption;

	typedef enum tagControllerAxisDirection : unsigned int
	{
		DirectionMin = 1,
		DirectionMax = 2,
		DirectionAny = 3,
		DirectionUp = 4,
		DirectionRight = 5,
		DirectionDown = 6,
		DirectionLeft = 7,
	} ControllerAxisDirection;


	typedef enum tagObjectTypeFilter : unsigned int
	{
		ObjectTypeFilterButton = 1,
		ObjectTypeFilterAxis = 2,
		ObjectTypeFilterPov = 4,
		ObjectTypeFilterButtonAndAxisAndPov = 7,
	} ObjectTypeFilter;

	GameControllerItem(ControllerItemOption option);
	GameControllerItem(ControllerItemOption option, ControllerAxisDirection direction);
	GameControllerItem(ControllerItemOption option, ControllerAxisDirection direction, const DIDEVICEOBJECTINSTANCE& objectInfo);

	ControllerItemOption option;
	ControllerAxisDirection direction;
	DIDEVICEOBJECTINSTANCE objectInfo;
};

#include "buttonitemdata.h"
