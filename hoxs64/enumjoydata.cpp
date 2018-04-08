#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
//include <dsound.h>
//include <dmusici.h>
#include <stdio.h>
#pragma warning(disable: 4005 4995)
#include <tchar.h>
#pragma warning(default: 4005 4995)
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 
#include <dxdiag.h>
#include "math.h"
//include "defines.h"
#include "CDPI.h"
//include "bits.h"
#include "util.h"
#pragma warning(disable: 4995)
#include "utils.h"
#pragma warning( default: 4995)
//include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "enumjoydata.h"

EnumJoyData::EnumJoyData()
{
	Reset();
}

void EnumJoyData::Reset()
{
	this->buttonCount = 0;
}

BOOL EnumJoyData::EnumButton(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	if (this->buttonCount >= _countof(buttonOffsets))
	{
		return DIENUM_STOP;
	}

	if (lpddoi->dwOfs + sizeof(BYTE) <= sizeof(DIJOYSTATE))
	{
		this->buttonOffsets[this->buttonCount] = lpddoi->dwOfs;
		this->buttonCount++;
	}

	return DIENUM_CONTINUE;
}

int EnumJoyData::EnumJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	return ((EnumJoyData *) pvRef)->EnumButton(lpddoi);
}
