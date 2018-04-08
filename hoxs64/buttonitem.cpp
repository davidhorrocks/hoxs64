#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include "buttonitem.h"

ButtonItem::ButtonItem(ButtonItemOption option)
		: option(option)
	{
		ZeroMemory(&objectInfo, sizeof(objectInfo));
	}

ButtonItem::ButtonItem(ButtonItemOption option, const DIDEVICEOBJECTINSTANCE& objectInfo)
		: option(option)
		, objectInfo(objectInfo)
	{
	}
