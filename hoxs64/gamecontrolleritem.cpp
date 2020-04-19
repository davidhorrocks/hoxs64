#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <windowsx.h>
#include <winuser.h>
#include "dx_version.h"
#include <stdio.h>
#include "servicerelease.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "diagbuttonselection.h"
#include "resource.h"

GameControllerItem::GameControllerItem(ControllerItemOption option)
	: option(option)
	, direction(DirectionAny)
{
	ZeroMemory(&objectInfo, sizeof(objectInfo));
}

GameControllerItem::GameControllerItem(ControllerItemOption option, ControllerAxisDirection direction)
	: option(option)
	, direction(direction)
{
	ZeroMemory(&objectInfo, sizeof(objectInfo));
}

GameControllerItem::GameControllerItem(ControllerItemOption option, ControllerAxisDirection direction, const DIDEVICEOBJECTINSTANCE& objectInfo)
	: option(option)
	, direction(direction)
	, objectInfo(objectInfo)
{
}
