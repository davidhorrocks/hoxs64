#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "c64keys.h"
#include "carray.h"
#include "hconfig.h"

LPCTSTR JoyKeyName::Name[2][JoynLast] =
{
	{
		TEXT(""),
		TEXT("Joy1Enabled"),
		TEXT("Joy1Valid"),
		TEXT("Joy1GUID"),
		TEXT("Joy1IsValidAxisX"),
		TEXT("Joy1IsValidAxisY"),
		TEXT("Joy1AxisX"),
		TEXT("Joy1AxisY"),
		TEXT("Joy1RevX"),
		TEXT("Joy1RevY"),
		TEXT("Joy1POV"),
		TEXT("Joy1Fire"),
		TEXT("Joy1FireMask"),
		TEXT("Joy1FireButtonCount"),
		TEXT("Joy1FireButtonList"),
		TEXT("Joy1Fire2"),
		TEXT("Joy1Fire2Mask"),
		TEXT("Joy1Fire2ButtonCount"),
		TEXT("Joy1Fire2ButtonList"),
		TEXT("Joy1ButtonUp"),
		TEXT("Joy1ButtonUpMask"),
		TEXT("Joy1ButtonUpCount"),
		TEXT("Joy1ButtonUpList"),
		TEXT("Joy1ButtonDown"),
		TEXT("Joy1ButtonDownMask"),
		TEXT("Joy1ButtonDownCount"),
		TEXT("Joy1ButtonDownList"),
		TEXT("Joy1ButtonLeft"),
		TEXT("Joy1ButtonLeftMask"),
		TEXT("Joy1ButtonLeftCount"),
		TEXT("Joy1ButtonLeftList"),
		TEXT("Joy1ButtonRight"),
		TEXT("Joy1ButtonRightMask"),
		TEXT("Joy1ButtonRightCount"),
		TEXT("Joy1ButtonRightList"),
		TEXT("Joy1ButtonKeyNoAssign"),
		TEXT("Joy1ButtonKeyNoAssignIsValid"),
		TEXT("Joy1ButtonKey1"),
		TEXT("Joy1ButtonKey1Mask"),
		TEXT("Joy1ButtonKey1Count"),
		TEXT("Joy1ButtonKey1List"),
		TEXT("Joy1ButtonKey2"),
		TEXT("Joy1ButtonKey2Mask"),
		TEXT("Joy1ButtonKey2Count"),
		TEXT("Joy1ButtonKey2List"),
		TEXT("Joy1ButtonKey3"),
		TEXT("Joy1ButtonKey3Mask"),
		TEXT("Joy1ButtonKey3Count"),
		TEXT("Joy1ButtonKey3List"),
		TEXT("Joy1ButtonKey4"),
		TEXT("Joy1ButtonKey4Mask"),
		TEXT("Joy1ButtonKey4Count"),
		TEXT("Joy1ButtonKey4List"),
		TEXT("Joy1ButtonKey5"),
		TEXT("Joy1ButtonKey5Mask"),
		TEXT("Joy1ButtonKey5Count"),
		TEXT("Joy1ButtonKey5List"),
		TEXT("Joy1ButtonKey6"),
		TEXT("Joy1ButtonKey6Mask"),
		TEXT("Joy1ButtonKey6Count"),
		TEXT("Joy1ButtonKey6List"),
		TEXT("Joy1AxisKey1List"),
		TEXT("Joy1AxisKey1ListDirection"),
		TEXT("Joy1AxisKey1Count"),
		TEXT("Joy1AxisKey2List"),
		TEXT("Joy1AxisKey2ListDirection"),
		TEXT("Joy1AxisKey2Count"),
		TEXT("Joy1AxisKey3List"),
		TEXT("Joy1AxisKey3ListDirection"),
		TEXT("Joy1AxisKey3Count"),
		TEXT("Joy1AxisKey4List"),
		TEXT("Joy1AxisKey4ListDirection"),
		TEXT("Joy1AxisKey4Count"),
		TEXT("Joy1AxisKey5List"),
		TEXT("Joy1AxisKey5ListDirection"),
		TEXT("Joy1AxisKey5Count"),
		TEXT("Joy1AxisKey6List"),
		TEXT("Joy1AxisKey6ListDirection"),
		TEXT("Joy1AxisKey6Count"),
		TEXT("Joy1PovKey1List"),
		TEXT("Joy1PovKey1ListDirection"),
		TEXT("Joy1PovKey1Count"),
		TEXT("Joy1PovKey2List"),
		TEXT("Joy1PovKey2ListDirection"),
		TEXT("Joy1PovKey2Count"),
		TEXT("Joy1PovKey3List"),
		TEXT("Joy1PovKey3ListDirection"),
		TEXT("Joy1PovKey3Count"),
		TEXT("Joy1PovKey4List"),
		TEXT("Joy1PovKey4ListDirection"),
		TEXT("Joy1PovKey4Count"),
		TEXT("Joy1PovKey5List"),
		TEXT("Joy1PovKey5ListDirection"),
		TEXT("Joy1PovKey5Count"),		
		TEXT("Joy1PovKey6List"),
		TEXT("Joy1PovKey6ListDirection"),
		TEXT("Joy1PovKey6Count"),		
	}
	,
	{
		TEXT(""),
		TEXT("Joy2Enabled"),
		TEXT("Joy2Valid"),
		TEXT("Joy2GUID"),
		TEXT("Joy2IsValidAxisX"),
		TEXT("Joy2IsValidAxisY"),
		TEXT("Joy2AxisX"),
		TEXT("Joy2AxisY"),
		TEXT("Joy2RevX"),
		TEXT("Joy2RevY"),
		TEXT("Joy2POV"),
		TEXT("Joy2Fire"),
		TEXT("Joy2FireMask"),
		TEXT("Joy2FireButtonCount"),
		TEXT("Joy2FireButtonList"),
		TEXT("Joy2Fire2"),	
		TEXT("Joy2Fire2Mask"),
		TEXT("Joy2Fire2ButtonCount"),
		TEXT("Joy2Fire2ButtonList"),
		TEXT("Joy2ButtonUp"),
		TEXT("Joy2ButtonUpMask"),
		TEXT("Joy2ButtonUpCount"),
		TEXT("Joy2ButtonUpList"),
		TEXT("Joy2ButtonDown"),
		TEXT("Joy2ButtonDownMask"),
		TEXT("Joy2ButtonDownCount"),
		TEXT("Joy2ButtonDownList"),
		TEXT("Joy2ButtonLeft"),
		TEXT("Joy2ButtonLeftMask"),
		TEXT("Joy2ButtonLeftCount"),
		TEXT("Joy2ButtonLeftList"),
		TEXT("Joy2ButtonRight"),
		TEXT("Joy2ButtonRightMask"),
		TEXT("Joy2ButtonRightCount"),
		TEXT("Joy2ButtonRightList"),
		TEXT("Joy2ButtonKeyNoAssign"),
		TEXT("Joy2ButtonKeyNoAssignIsValid"),
		TEXT("Joy2ButtonKey1"),
		TEXT("Joy2ButtonKey1Mask"),
		TEXT("Joy2ButtonKey1Count"),
		TEXT("Joy2ButtonKey1List"),
		TEXT("Joy2ButtonKey2"),
		TEXT("Joy2ButtonKey2Mask"),
		TEXT("Joy2ButtonKey2Count"),
		TEXT("Joy2ButtonKey2List"),
		TEXT("Joy2ButtonKey3"),
		TEXT("Joy2ButtonKey3Mask"),
		TEXT("Joy2ButtonKey3Count"),
		TEXT("Joy2ButtonKey3List"),
		TEXT("Joy2ButtonKey4"),
		TEXT("Joy2ButtonKey4Mask"),
		TEXT("Joy2ButtonKey4Count"),
		TEXT("Joy2ButtonKey4List"),
		TEXT("Joy2ButtonKey5"),
		TEXT("Joy2ButtonKey5Mask"),
		TEXT("Joy2ButtonKey5Count"),
		TEXT("Joy2ButtonKey5List"),
		TEXT("Joy2ButtonKey6"),
		TEXT("Joy2ButtonKey6Mask"),
		TEXT("Joy2ButtonKey6Count"),
		TEXT("Joy2ButtonKey6List"),
		TEXT("Joy2AxisKey1List"),
		TEXT("Joy2AxisKey1ListDirection"),
		TEXT("Joy2AxisKey1Count"),
		TEXT("Joy2AxisKey2List"),
		TEXT("Joy2AxisKey2ListDirection"),
		TEXT("Joy2AxisKey2Count"),
		TEXT("Joy2AxisKey3List"),
		TEXT("Joy2AxisKey3ListDirection"),
		TEXT("Joy2AxisKey3Count"),
		TEXT("Joy2AxisKey4List"),
		TEXT("Joy2AxisKey4ListDirection"),
		TEXT("Joy2AxisKey4Count"),
		TEXT("Joy2AxisKey5List"),
		TEXT("Joy2AxisKey5ListDirection"),
		TEXT("Joy2AxisKey5Count"),
		TEXT("Joy2AxisKey6List"),
		TEXT("Joy2AxisKey6ListDirection"),
		TEXT("Joy2AxisKey6Count"),
		TEXT("Joy2PovKey1List"),
		TEXT("Joy2PovKey1ListDirection"),
		TEXT("Joy2PovKey1Count"),
		TEXT("Joy2PovKey2List"),
		TEXT("Joy2PovKey2ListDirection"),
		TEXT("Joy2PovKey2Count"),
		TEXT("Joy2PovKey3List"),
		TEXT("Joy2PovKey3ListDirection"),
		TEXT("Joy2PovKey3Count"),
		TEXT("Joy2PovKey4List"),
		TEXT("Joy2PovKey4ListDirection"),
		TEXT("Joy2PovKey4Count"),
		TEXT("Joy2PovKey5List"),
		TEXT("Joy2PovKey5ListDirection"),
		TEXT("Joy2PovKey5Count"),
		TEXT("Joy2PovKey6List"),
		TEXT("Joy2PovKey6ListDirection"),
		TEXT("Joy2PovKey6Count"),
	}
};

JoyKeyName::ButtonKeySet JoyKeyName::Fire1 = 
{
	JoyKeyName::JoynFire1,
	JoyKeyName::JoynFire1Mask,
	JoyKeyName::JoynFire1ButtonCount,
	JoyKeyName::JoynFire1ButtonList
};

JoyKeyName::ButtonKeySet JoyKeyName::Fire2 = 
{
	JoyKeyName::JoynFire2,
	JoyKeyName::JoynFire2Mask,
	JoyKeyName::JoynFire2ButtonCount,
	JoyKeyName::JoynFire2ButtonList
};

JoyKeyName::ButtonKeySet JoyKeyName::Up = 
{
	JoyKeyName::JoynButtonUp,
	JoyKeyName::JoynButtonUpMask,
	JoyKeyName::JoynButtonUpCount,
	JoyKeyName::JoynButtonUpList
};

JoyKeyName::ButtonKeySet JoyKeyName::Down = 
{
	JoyKeyName::JoynButtonDown,
	JoyKeyName::JoynButtonDownMask,
	JoyKeyName::JoynButtonDownCount,
	JoyKeyName::JoynButtonDownList
};

JoyKeyName::ButtonKeySet JoyKeyName::Left = 
{
	JoyKeyName::JoynButtonLeft,
	JoyKeyName::JoynButtonLeftMask,
	JoyKeyName::JoynButtonLeftCount,
	JoyKeyName::JoynButtonLeftList
};

JoyKeyName::ButtonKeySet JoyKeyName::Right = 
{
	JoyKeyName::JoynButtonRight,
	JoyKeyName::JoynButtonRightMask,
	JoyKeyName::JoynButtonRightCount,
	JoyKeyName::JoynButtonRightList
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey1 = 
{
	JoyKeyName::JoynButtonKey1,
	JoyKeyName::JoynButtonKey1Mask,
	JoyKeyName::JoynButtonKey1Count,
	JoyKeyName::JoynButtonKey1List
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey2 = 
{
	JoyKeyName::JoynButtonKey2,
	JoyKeyName::JoynButtonKey2Mask,
	JoyKeyName::JoynButtonKey2Count,
	JoyKeyName::JoynButtonKey2List
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey3 = 
{
	JoyKeyName::JoynButtonKey3,
	JoyKeyName::JoynButtonKey3Mask,
	JoyKeyName::JoynButtonKey3Count,
	JoyKeyName::JoynButtonKey3List
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey4 = 
{
	JoyKeyName::JoynButtonKey4,
	JoyKeyName::JoynButtonKey4Mask,
	JoyKeyName::JoynButtonKey4Count,
	JoyKeyName::JoynButtonKey4List
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey5 = 
{
	JoyKeyName::JoynButtonKey5,
	JoyKeyName::JoynButtonKey5Mask,
	JoyKeyName::JoynButtonKey5Count,
	JoyKeyName::JoynButtonKey5List
};

JoyKeyName::ButtonKeySet JoyKeyName::ButtonKey6 = 
{
	JoyKeyName::JoynButtonKey6,
	JoyKeyName::JoynButtonKey6Mask,
	JoyKeyName::JoynButtonKey6Count,
	JoyKeyName::JoynButtonKey6List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey1 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey1ListDirection,
	JoyKeyName::JoynAxisKey1Count,
	JoyKeyName::JoynAxisKey1List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey2 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey2ListDirection,
	JoyKeyName::JoynAxisKey2Count,
	JoyKeyName::JoynAxisKey2List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey3 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey3ListDirection,
	JoyKeyName::JoynAxisKey3Count,
	JoyKeyName::JoynAxisKey3List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey4 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey4ListDirection,
	JoyKeyName::JoynAxisKey4Count,
	JoyKeyName::JoynAxisKey4List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey5 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey5ListDirection,
	JoyKeyName::JoynAxisKey5Count,
	JoyKeyName::JoynAxisKey5List
};

JoyKeyName::ButtonKeySet JoyKeyName::AxisKey6 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynAxisKey6ListDirection,
	JoyKeyName::JoynAxisKey6Count,
	JoyKeyName::JoynAxisKey6List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey1 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey1ListDirection,
	JoyKeyName::JoynPovKey1Count,
	JoyKeyName::JoynPovKey1List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey2 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey2ListDirection,
	JoyKeyName::JoynPovKey2Count,
	JoyKeyName::JoynPovKey2List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey3 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey3ListDirection,
	JoyKeyName::JoynPovKey3Count,
	JoyKeyName::JoynPovKey3List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey4 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey4ListDirection,
	JoyKeyName::JoynPovKey4Count,
	JoyKeyName::JoynPovKey4List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey5 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey5ListDirection,
	JoyKeyName::JoynPovKey5Count,
	JoyKeyName::JoynPovKey5List
};

JoyKeyName::ButtonKeySet JoyKeyName::PovKey6 = 
{
	JoyKeyName::JoynBlank,
	JoyKeyName::JoynPovKey6ListDirection,
	JoyKeyName::JoynPovKey6Count,
	JoyKeyName::JoynPovKey6List
};

CConfig::CConfig()
{
	ZeroMemory(&m_fullscreenAdapterId, sizeof(m_fullscreenAdapterId));
	ZeroMemory(&m_joy1config.joystickID, sizeof(m_joy1config.joystickID));
	ZeroMemory(&m_joy2config.joystickID, sizeof(m_joy2config.joystickID));
	m_bD1541_Thread_Enable = false;
	m_bAllowOpposingJoystick = false;
	m_bDisableDwmFullscreen = false;
	m_bWindowedCustomSize = false;
	m_bUseBlitStretch = true;
	m_bSwapJoysticks = false;
	m_bCPUFriendly = true;
	m_bAudioClockSync = true;
	m_bSidDigiBoost = false;
	m_bMaxSpeed = false;
	m_bSkipFrames = false;
	m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
	m_CIAMode = HCFG::CM_CIA6526A;
	m_bTimerBbug = false;
	SetPalettePepto();
	m_numberOfExtraSIDs = 0;
	m_Sid2Address = 0;
	m_Sid3Address = 0;
	m_Sid4Address = 0;
	m_Sid5Address = 0;
	m_Sid6Address = 0;
	m_Sid7Address = 0;
	m_Sid8Address = 0;
}

void CConfig::SetCiaNewOldMode(bool isNew)
{
	if (isNew)
	{
		m_CIAMode = HCFG::CM_CIA6526A;
		m_bTimerBbug = false;
	}
	else
	{
		m_CIAMode = HCFG::CM_CIA6526;
		m_bTimerBbug = true;
	}
}

void CConfig::SetRunFast()
{
	m_bSkipFrames = true;
	m_bLimitSpeed = false;
	m_bMaxSpeed = true;
}

void CConfig::SetRunNormal()
{
	m_bSkipFrames = false;
	m_bLimitSpeed = true;
	m_bMaxSpeed = false;
}

#define readregkeyboarditem(n) tempLenValue = lenValue;\
		lRetCode = RegReadStr(hKey1, TEXT(#n), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);\
		if (lRetCode == ERROR_SUCCESS)\
		{\
			m_KeyMap[n] = (unsigned char) _ttol(szValue);\
		}


HRESULT CConfig::LoadCurrentSetting()
{
TCHAR szValue[20];
HKEY  hKey1; 
LONG   lRetCode; 
ULONG tempLenValue;
DWORD type;
DWORD dwValue;
int i;
	
	LoadDefaultSetting();
	const ULONG lenValue = sizeof(szValue);
	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		lRetCode = RegQueryValueEx(hKey1, TEXT("PrefsSaved"), NULL, &type, (PBYTE) NULL, &tempLenValue);
		RegCloseKey(hKey1);
		if (lRetCode != ERROR_SUCCESS)
		{
			return S_OK;
		}
	}
	else
	{
		return S_OK;
	}

	//Patch soft keys to work after version v1.0.7.2
	m_KeyMap[C64Keys::C64K_CURSORUP]= DIK_UP;
	m_KeyMap[C64Keys::C64K_CURSORLEFT]= DIK_LEFT;

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Keyboard"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		readregkeyboarditem(C64Keys::C64K_0);
		readregkeyboarditem(C64Keys::C64K_1);
		readregkeyboarditem(C64Keys::C64K_2);
		readregkeyboarditem(C64Keys::C64K_3);
		readregkeyboarditem(C64Keys::C64K_4);
		readregkeyboarditem(C64Keys::C64K_5);
		readregkeyboarditem(C64Keys::C64K_6);
		readregkeyboarditem(C64Keys::C64K_7);
		readregkeyboarditem(C64Keys::C64K_8);
		readregkeyboarditem(C64Keys::C64K_9);
		readregkeyboarditem(C64Keys::C64K_A);
		readregkeyboarditem(C64Keys::C64K_B);
		readregkeyboarditem(C64Keys::C64K_C);
		readregkeyboarditem(C64Keys::C64K_D);
		readregkeyboarditem(C64Keys::C64K_E);
		readregkeyboarditem(C64Keys::C64K_F);
		readregkeyboarditem(C64Keys::C64K_G);
		readregkeyboarditem(C64Keys::C64K_H);
		readregkeyboarditem(C64Keys::C64K_I);
		readregkeyboarditem(C64Keys::C64K_J);
		readregkeyboarditem(C64Keys::C64K_K);
		readregkeyboarditem(C64Keys::C64K_L);
		readregkeyboarditem(C64Keys::C64K_M);
		readregkeyboarditem(C64Keys::C64K_N);
		readregkeyboarditem(C64Keys::C64K_O);
		readregkeyboarditem(C64Keys::C64K_P);
		readregkeyboarditem(C64Keys::C64K_Q);
		readregkeyboarditem(C64Keys::C64K_R);
		readregkeyboarditem(C64Keys::C64K_S);
		readregkeyboarditem(C64Keys::C64K_T);
		readregkeyboarditem(C64Keys::C64K_U);
		readregkeyboarditem(C64Keys::C64K_V);
		readregkeyboarditem(C64Keys::C64K_W);
		readregkeyboarditem(C64Keys::C64K_X);
		readregkeyboarditem(C64Keys::C64K_Y);
		readregkeyboarditem(C64Keys::C64K_Z);
		readregkeyboarditem(C64Keys::C64K_PLUS);
		readregkeyboarditem(C64Keys::C64K_MINUS);
		readregkeyboarditem(C64Keys::C64K_ASTERISK);
		readregkeyboarditem(C64Keys::C64K_SLASH);
		readregkeyboarditem(C64Keys::C64K_COMMA);
		readregkeyboarditem(C64Keys::C64K_DOT);
		readregkeyboarditem(C64Keys::C64K_ARROWLEFT);
		readregkeyboarditem(C64Keys::C64K_COLON);
		readregkeyboarditem(C64Keys::C64K_SEMICOLON);
		readregkeyboarditem(C64Keys::C64K_CONTROL);
		readregkeyboarditem(C64Keys::C64K_STOP);
		readregkeyboarditem(C64Keys::C64K_COMMODORE);
		readregkeyboarditem(C64Keys::C64K_LEFTSHIFT);
		readregkeyboarditem(C64Keys::C64K_RIGHTSHIFT);
		readregkeyboarditem(C64Keys::C64K_RESTORE);		
		readregkeyboarditem(C64Keys::C64K_HOME);
		readregkeyboarditem(C64Keys::C64K_DEL);
		readregkeyboarditem(C64Keys::C64K_RETURN);
		readregkeyboarditem(C64Keys::C64K_ARROWUP);
		readregkeyboarditem(C64Keys::C64K_POUND);
		readregkeyboarditem(C64Keys::C64K_EQUAL);
		readregkeyboarditem(C64Keys::C64K_CURSORDOWN);
		readregkeyboarditem(C64Keys::C64K_CURSORRIGHT);
		readregkeyboarditem(C64Keys::C64K_CURSORUP);
		readregkeyboarditem(C64Keys::C64K_CURSORLEFT);
		readregkeyboarditem(C64Keys::C64K_SPACE);
		readregkeyboarditem(C64Keys::C64K_AT);
		readregkeyboarditem(C64Keys::C64K_F1);
		readregkeyboarditem(C64Keys::C64K_F2);
		readregkeyboarditem(C64Keys::C64K_F3);
		readregkeyboarditem(C64Keys::C64K_F4);
		readregkeyboarditem(C64Keys::C64K_F5);
		readregkeyboarditem(C64Keys::C64K_F6);
		readregkeyboarditem(C64Keys::C64K_F7);
		readregkeyboarditem(C64Keys::C64K_F8);

		readregkeyboarditem(C64Keys::C64K_JOY1FIRE);
		readregkeyboarditem(C64Keys::C64K_JOY1UP);
		readregkeyboarditem(C64Keys::C64K_JOY1DOWN);
		readregkeyboarditem(C64Keys::C64K_JOY1LEFT);
		readregkeyboarditem(C64Keys::C64K_JOY1RIGHT);
		readregkeyboarditem(C64Keys::C64K_JOY2FIRE);
		readregkeyboarditem(C64Keys::C64K_JOY2UP);
		readregkeyboarditem(C64Keys::C64K_JOY2DOWN);
		readregkeyboarditem(C64Keys::C64K_JOY2LEFT);
		readregkeyboarditem(C64Keys::C64K_JOY2RIGHT);

		readregkeyboarditem(C64Keys::C64K_JOY1FIRE2);
		readregkeyboarditem(C64Keys::C64K_JOY2FIRE2);

		RegCloseKey(hKey1);
	}

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("D1541_Emulation"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bD1541_Emulation_Enable = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SID_Emulation"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSID_Emulation_Enable = _ttol(szValue) != 0;
		}
		
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("LimitSpeed"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bLimitSpeed = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("ShowSpeed"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bShowSpeed = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SkipAltFrames"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSkipFrames = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SIDSampleMode"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSIDResampleMode = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		tempLenValue = lenValue;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SIDStereo"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSIDStereo = dwValue != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SyncMode1"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_syncModeFullscreen = (HCFG::FULLSCREENSYNCMODE) _ttol(szValue);
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SyncMode2"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_syncModeWindowed = (HCFG::FULLSCREENSYNCMODE) _ttol(szValue);
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("DoubleSizedWindow"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bDoubleSizedWindow = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("UseBlitStretch"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bUseBlitStretch = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("UseKeymap"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bUseKeymap = _ttol(szValue) != 0;
		}
		
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SwapJoysticks"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSwapJoysticks = _ttol(szValue) != 0;
		}
		else
		{
			m_bSwapJoysticks = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("CPUFriendly"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bCPUFriendly = _ttol(szValue) != 0;
		}
		else
		{
			m_bCPUFriendly = true;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("AudioClockSync"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bAudioClockSync = _ttol(szValue) != 0;
		}
		else
		{
			m_bAudioClockSync = true;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("SIDDigiBoost"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSidDigiBoost = _ttol(szValue) != 0;
		}
		else
		{
			m_bSidDigiBoost = false;
		}

		lRetCode = G::GetClsidFromRegValue(hKey1, TEXT("FullscreenAdapterId"), &m_fullscreenAdapterId);
		if (FAILED(lRetCode))
		{
			ZeroMemory(&m_fullscreenAdapterId, sizeof(GUID));
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenAdapterNumber"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenAdapterNumber = _ttol(szValue);
		}
		else
		{
			m_fullscreenAdapterNumber = 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenWidth"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenWidth = _ttol(szValue);
		}
		else
		{
			m_fullscreenWidth = 0;
		}
	
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenHeight"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenHeight = _ttol(szValue);
		}
		else
		{
			m_fullscreenHeight = 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenRefresh"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenRefresh = _ttol(szValue);
		}
		else
		{
			m_fullscreenRefresh = 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenFormat"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenFormat = _ttol(szValue);
		}
		else
		{
			m_fullscreenFormat = 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FullscreenStretch"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH) _ttol(szValue);
		}
		else
		{
			m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH)0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("BlitFilter"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_blitFilter = (HCFG::EMUWINDOWFILTER)_ttol(szValue);
		}
		else
		{
			m_blitFilter = (HCFG::EMUWINDOWFILTER)0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("BorderSize"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_borderSize = (HCFG::EMUBORDERSIZE)_ttol(szValue);
		}
		else
		{
			m_borderSize = HCFG::EMUBORDER_TV;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("ShowFloppyLed"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bShowFloppyLed = _ttol(szValue) != 0;
		}
		else
		{
			m_bShowFloppyLed = true;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("FPS"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fps = (HCFG::EMUFPS)_ttol(szValue);
		}
		else
		{
			m_fps = HCFG::EMUFPS_50;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("TrackZeroSensor"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_TrackZeroSensorStyle = (HCFG::ETRACKZEROSENSORSTYLE)_ttol(szValue);
		}
		else
		{
			m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("CIAMode"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_CIAMode = (HCFG::CIAMODE)_ttol(szValue);
		}
		else
		{
			m_CIAMode = HCFG::CM_CIA6526A;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("CIATimerBbug"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			this->m_bTimerBbug = _ttol(szValue) != 0;
		}
		else
		{
			this->m_bTimerBbug = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("NumberOfExtraSidChips"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			this->m_numberOfExtraSIDs = dwValue;
			if (this->m_numberOfExtraSIDs <  0 || this->m_numberOfExtraSIDs > 7)
			{
				this->m_numberOfExtraSIDs = 0;
			}
		}
		else
		{
			this->m_numberOfExtraSIDs = 0;
		}

		LPCTSTR sidAddressName[] = { 
			TEXT("Sid2Address"), 
			TEXT("Sid3Address"), 
			TEXT("Sid4Address"), 
			TEXT("Sid5Address"), 
			TEXT("Sid6Address"), 
			TEXT("Sid7Address"), 
			TEXT("Sid8Address") 
		};

		bit16* sidAddressValue[] = { 
			&this->m_Sid2Address,
			&this->m_Sid3Address,
			&this->m_Sid4Address,
			&this->m_Sid5Address,
			&this->m_Sid6Address,
			&this->m_Sid7Address,
			&this->m_Sid8Address
		};

		for (int i=0; i < _countof(sidAddressName); i++)
		{
			tempLenValue = lenValue;
			lRetCode = RegReadDWordOrStr(hKey1, sidAddressName[i], &dwValue);
			bit16 sidAddress = 0;
			if (lRetCode == ERROR_SUCCESS)
			{
				sidAddress = (bit16)dwValue;
			}
			else
			{
				sidAddress = 0;
			}

			if (!((sidAddress >= 0xD420 && sidAddress <= 0xD7E0) || (sidAddress >= 0xDE00 && sidAddress <= 0xDFE0)))
			{
				sidAddress = 0;
			}			

			*sidAddressValue[i] = sidAddress;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("DiskThreadEnable"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bD1541_Thread_Enable = _ttol(szValue) != 0;
		}
		else
		{
			if (G::IsMultiCore())
			{
				m_bD1541_Thread_Enable = true;
			}
			else
			{
				m_bD1541_Thread_Enable = false;
			}
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("AllowOpposingJoystick"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bAllowOpposingJoystick = _ttol(szValue) != 0;
		}
		else
		{
			m_bAllowOpposingJoystick = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("DisableDwmFullscreen"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bDisableDwmFullscreen = _ttol(szValue) != 0;
		}
		else
		{
			m_bDisableDwmFullscreen = false;
		}
		
		RegCloseKey(hKey1);
	} 

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\VICIIPalette"),
		0, KEY_READ,
		&hKey1);
	if (lRetCode == ERROR_SUCCESS)
	{
		std::basic_string<TCHAR> colorregkeyname;
		for (i = 0; i < VicIIPalette::NumColours; i++)
		{
			colorregkeyname.clear();
			colorregkeyname.append(TEXT("color_"));
			if (i < 0xa)
			{
				colorregkeyname.push_back(TEXT('0') + i);
			}
			else
			{
				colorregkeyname.push_back(TEXT('a') + i - 0xa);
			}

			tempLenValue = sizeof(DWORD);
			type = REG_DWORD;
			lRetCode = RegQueryValueEx(hKey1, colorregkeyname.c_str(), NULL, &type, (PBYTE) &szValue[0], &tempLenValue);
			if (lRetCode == ERROR_SUCCESS && tempLenValue == sizeof(DWORD))
			{
				this->m_colour_palette[i] = (*(DWORD *)szValue);
			}
			else
			{
				this->m_colour_palette[i] = VicIIPalette::Pepto[i];
			}
		}

		RegCloseKey(hKey1);

		// Load joystick 1 setting
		LoadCurrentJoystickSetting(1, this->m_joy1config);

		// Load joystick 2 setting
		LoadCurrentJoystickSetting(2, this->m_joy2config);
	}
	
	return S_OK;
}

HRESULT CConfig::LoadCurrentJoystickSetting(int joystickNumber, struct joyconfig& jconfig)
{
HKEY  hKey1; 
LONG   lRetCode; 
DWORD dw;

	unsigned int joyIndex = joystickNumber - 1;
	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Joystick"),
		0, KEY_READ,
		&hKey1);
	if (lRetCode == ERROR_SUCCESS)
	{
		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynValid], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			jconfig.IsValidId = dw != 0;
		}
		else
		{
			jconfig.IsValidId = false;
		}

		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynEnabled], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			jconfig.IsEnabled = dw != 0;
		}
		else
		{
			jconfig.IsEnabled = false;
		}

		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynPOV], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			jconfig.isPovEnabled = dw != 0;
		}
		else
		{
			jconfig.isPovEnabled = true;
		}

		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevX], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			jconfig.isXReverse = dw != 0;
		}

		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevY], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			jconfig.isYReverse = dw != 0;
		}

		if (jconfig.IsValidId)
		{
			if (SUCCEEDED(G::GetClsidFromRegValue(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynGUID], &jconfig.joystickID)))
			{
				jconfig.IsValidId = true;
			}
			else
			{
				jconfig.IsValidId = false;
			}

			//Joystick X axis
			jconfig.horizontalAxisAxisCount = 0;
			lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisX], &dw);
			if (lRetCode == ERROR_SUCCESS)
			{
				jconfig.isValidXAxis = dw != 0;
			}
			else
			{
				jconfig.isValidXAxis = true;
			}

			lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisX], &dw);
			if (lRetCode == ERROR_SUCCESS)
			{
				if (dw <= (sizeof(DIJOYSTATE2) - sizeof(DWORD)))
				{
					jconfig.dwOfs_X = dw;
					jconfig.horizontalAxisAxisCount = 1;
				}
			}
					
			//Joystick Y axis
			jconfig.verticalAxisAxisCount = 0;
			lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisY], &dw);
			if (lRetCode == ERROR_SUCCESS)
			{
				jconfig.isValidYAxis = dw != 0;
			}
			else
			{
				jconfig.isValidYAxis = true;
			}

			lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisY], &dw);
			if (lRetCode == ERROR_SUCCESS)
			{
				if (dw <= (sizeof(DIJOYSTATE2) - sizeof(DWORD)))
				{
					jconfig.dwOfs_Y = dw;
					jconfig.verticalAxisAxisCount = 1;
				}
			}					

			// Read map of game buttons to C64 joystick.
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Fire1, jconfig.fire1ButtonOffsets, jconfig.fire1ButtonCount);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Fire2, jconfig.fire2ButtonOffsets, jconfig.fire2ButtonCount);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Up, jconfig.upButtonOffsets, jconfig.upButtonCount);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Down, jconfig.downButtonOffsets, jconfig.downButtonCount);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Left, jconfig.leftButtonOffsets, jconfig.leftButtonCount);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Right, jconfig.rightButtonOffsets, jconfig.rightButtonCount);

			// Read map of game buttons to C64 keys.
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey1, jconfig.keyNButtonOffsets[0], jconfig.keyNButtonCount[0]);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey2, jconfig.keyNButtonOffsets[1], jconfig.keyNButtonCount[1]);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey3, jconfig.keyNButtonOffsets[2], jconfig.keyNButtonCount[2]);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey4, jconfig.keyNButtonOffsets[3], jconfig.keyNButtonCount[3]);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey5, jconfig.keyNButtonOffsets[4], jconfig.keyNButtonCount[4]);
			ReadJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey6, jconfig.keyNButtonOffsets[5], jconfig.keyNButtonCount[5]);

			// Read map of game axes to C64 keys.
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey1, jconfig.keyNAxisOffsets[0], jconfig.keyNAxisDirection[0], jconfig.MAXAXIS, &jconfig.keyNAxisCount[0]);
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey2, jconfig.keyNAxisOffsets[1], jconfig.keyNAxisDirection[1], jconfig.MAXAXIS, &jconfig.keyNAxisCount[1]);
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey3, jconfig.keyNAxisOffsets[2], jconfig.keyNAxisDirection[2], jconfig.MAXAXIS, &jconfig.keyNAxisCount[2]);
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey4, jconfig.keyNAxisOffsets[3], jconfig.keyNAxisDirection[3], jconfig.MAXAXIS, &jconfig.keyNAxisCount[3]);
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey5, jconfig.keyNAxisOffsets[4], jconfig.keyNAxisDirection[4], jconfig.MAXAXIS, &jconfig.keyNAxisCount[4]);
			ReadJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey6, jconfig.keyNAxisOffsets[5], jconfig.keyNAxisDirection[5], jconfig.MAXAXIS, &jconfig.keyNAxisCount[5]);

			// Read map of game pov to C64 keys.
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey1, jconfig.keyNPovOffsets[0], jconfig.keyNPovDirection[0], jconfig.MAXPOV, &jconfig.keyNPovCount[0]);
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey2, jconfig.keyNPovOffsets[1], jconfig.keyNPovDirection[1], jconfig.MAXPOV, &jconfig.keyNPovCount[1]);
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey3, jconfig.keyNPovOffsets[2], jconfig.keyNPovDirection[2], jconfig.MAXPOV, &jconfig.keyNPovCount[2]);
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey4, jconfig.keyNPovOffsets[3], jconfig.keyNPovDirection[3], jconfig.MAXPOV, &jconfig.keyNPovCount[3]);
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey5, jconfig.keyNPovOffsets[4], jconfig.keyNPovDirection[4], jconfig.MAXPOV, &jconfig.keyNPovCount[4]);
			ReadJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey6, jconfig.keyNPovOffsets[5], jconfig.keyNPovDirection[5], jconfig.MAXPOV, &jconfig.keyNPovCount[5]);

			unsigned int i;
			DWORD numberOfKeysAssigned = 0;
			DWORD numberOfKeysValid = 0;
			DWORD tempLen;
			tempLen = 0;
			lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], NULL, NULL, NULL, &numberOfKeysAssigned);
			if (lRetCode != ERROR_SUCCESS)
			{
				numberOfKeysAssigned = 0;
			}

			tempLen = 0;
			lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], NULL, NULL, NULL, &numberOfKeysValid);
			if (lRetCode != ERROR_SUCCESS)
			{
				numberOfKeysValid = 0;
			}

			unsigned int numberOfKeys = 0;
			if (numberOfKeysAssigned == numberOfKeysValid)
			{
				numberOfKeys = numberOfKeysValid;
			}

			if (numberOfKeys > joyconfig::MaxUserKeyAssignCount)
			{
				numberOfKeys = 0;
			}

			if (numberOfKeys > 0)
			{
				// Read key assignment values.
				C64Keys::C64Key c64KeyAssign[joyconfig::MaxUserKeyAssignCount];
				tempLen = (DWORD)numberOfKeys;
				lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], NULL, NULL, (LPBYTE)&c64KeyAssign[0], &tempLen);
				if (lRetCode == ERROR_SUCCESS)
				{
					for (i = 0; i < _countof(jconfig.keyNoAssign); i++)
					{					
						if (i < numberOfKeys && i < tempLen)
						{
							jconfig.keyNoAssign[i] = c64KeyAssign[i];
						}
					}
				}

				// Read key assignment validities.
				bit8 c64KeyAssignValid[joyconfig::MaxUserKeyAssignCount];
				tempLen = (DWORD)numberOfKeys;
				lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], NULL, NULL, (LPBYTE)&c64KeyAssignValid[0], &tempLen);
				if (lRetCode == ERROR_SUCCESS)
				{
					for (i = 0; i < _countof(jconfig.isValidKeyNoAssign); i++)
					{
						if (i < numberOfKeys && i < tempLen)
						{
							if (c64KeyAssignValid[i] != 0)
							{
								jconfig.isValidKeyNoAssign[i] = true;
								jconfig.enableKeyAssign = true;
							}
						}
					}
				}
			}
		}

		RegCloseKey(hKey1);
	}

	return S_OK;
}

HRESULT CConfig::WriteJoystickAxisList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pAxisOffsets, const GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int axisCount)
{
DWORD dwValue;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;
DWORD dwitem[joyconfig::MAXAXIS];

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	if (axisCount > joyconfig::MAXAXIS)
	{
		axisCount = joyconfig::MAXAXIS;
	}

	for (i=0; i < axisCount; i++)
	{
		dwitem[i] = (DWORD)pAxisDirection[i];
	}

	dwValue = (DWORD)axisCount;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.count], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], 0, REG_BINARY, (LPBYTE) &pAxisOffsets[0], sizeof(DWORD) * axisCount);
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.mask], 0, REG_BINARY, (LPBYTE) &dwitem[0], sizeof(DWORD) * axisCount);
	return S_OK;
}

HRESULT CConfig::ReadJoystickAxisList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pAxisOffsets, GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int maxAxisBufferCount, unsigned int *pAxisCount)
{
LONG lRetCode;
DWORD dwOffset;
DWORD offsetList[joyconfig::MAXAXIS];
DWORD storedAxisCount;
DWORD tempLen;
unsigned int axisCount = 0;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	// Axis
	axisCount = 0;
	lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][regnames.count], (LPDWORD)&storedAxisCount);
	if (lRetCode == ERROR_SUCCESS)
	{
		if (storedAxisCount > joyconfig::MAXAXIS)
		{
			storedAxisCount = joyconfig::MAXAXIS;
		}

		tempLen = sizeof(offsetList);
		lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], NULL, NULL, (LPBYTE)&offsetList[0], &tempLen);
		if (lRetCode == ERROR_SUCCESS)
		{
			tempLen = tempLen / sizeof(DWORD);
			if (storedAxisCount > tempLen)
			{
				storedAxisCount = tempLen;
			}

			DWORD numAxes = 0;
			for (i = 0; i < storedAxisCount; i++)
			{
				dwOffset = offsetList[i];
				if (dwOffset <= sizeof(DIJOYSTATE2) - sizeof(LONG))
				{
					if (pAxisOffsets != NULL && numAxes < maxAxisBufferCount)
					{
						pAxisOffsets[numAxes] = dwOffset;
					}

					numAxes++;
				}
			}

			axisCount = numAxes;
		}

		// Axis direction
		joyconfig::defaultClearAxisDirection(pAxisDirection, GameControllerItem::DirectionAny, maxAxisBufferCount);
		tempLen = sizeof(offsetList);
		lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.mask], NULL, NULL, (LPBYTE)&offsetList[0], &tempLen);
		if (lRetCode == ERROR_SUCCESS)
		{
			tempLen = tempLen / sizeof(DWORD);
			if (storedAxisCount > tempLen)
			{
				storedAxisCount = tempLen;
			}

			DWORD numAxes = 0;
			for (i = 0; i < storedAxisCount; i++)
			{
				GameControllerItem::ControllerAxisDirection dir = (GameControllerItem::ControllerAxisDirection)offsetList[i];
				switch (dir)
				{
				case GameControllerItem::DirectionAny:
				case GameControllerItem::DirectionMin:
				case GameControllerItem::DirectionMax:
					break;
				default:
					dir = GameControllerItem::DirectionAny;
				}

				if (pAxisDirection != NULL && numAxes < maxAxisBufferCount)
				{
					pAxisDirection[numAxes] = (GameControllerItem::ControllerAxisDirection)dir;
				}

				numAxes++;
			}
		}
	}

	if (pAxisCount != NULL)
	{
		*pAxisCount = axisCount;
	}

	return S_OK;
}

HRESULT CConfig::WriteJoystickButtonList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pButtonOffsets, const unsigned int &buttonCount)
{
DWORD dwValue;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	dwValue = (DWORD)buttonCount;
	if (dwValue > joyconfig::MAXBUTTONS)
	{
		dwValue = joyconfig::MAXBUTTONS;
	}

	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.count], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], 0, REG_BINARY, (LPBYTE) &pButtonOffsets[0], sizeof(DWORD) * dwValue);

	return S_OK;
}

HRESULT CConfig::ReadJoystickPovList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pPovOffsets, GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int maxPovBufferCount, unsigned int* pPovCount)
{
LONG lRetCode;
DWORD dwOffset;
DWORD offsetList[joyconfig::MAXPOV];
DWORD storedPovCount;
DWORD tempLen;
unsigned int povCount = 0;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	povCount = 0;
	lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][regnames.count], (LPDWORD)&storedPovCount);
	if (lRetCode == ERROR_SUCCESS)
	{
		if (storedPovCount > joyconfig::MAXPOV)
		{
			storedPovCount = joyconfig::MAXPOV;
		}

		tempLen = sizeof(offsetList);
		lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], NULL, NULL, (LPBYTE)&offsetList[0], &tempLen);
		if (lRetCode == ERROR_SUCCESS)
		{
			tempLen = tempLen / sizeof(DWORD);
			if (storedPovCount > tempLen)
			{
				storedPovCount = tempLen;
			}

			DWORD numPov = 0;
			for (i = 0; i < storedPovCount; i++)
			{
				dwOffset = offsetList[i];
				if (dwOffset >= DIJOFS_POV(0) && dwOffset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
				{
					if (pPovOffsets != NULL && numPov < maxPovBufferCount)
					{
						pPovOffsets[numPov] = dwOffset;
					}

					numPov++;
				}
			}

			povCount = numPov;
		}

		// Pov direction
		joyconfig::defaultClearAxisDirection(pPovDirection, GameControllerItem::DirectionAny, maxPovBufferCount);
		tempLen = sizeof(offsetList);
		lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.mask], NULL, NULL, (LPBYTE)&offsetList[0], &tempLen);
		if (lRetCode == ERROR_SUCCESS)
		{
			tempLen = tempLen / sizeof(DWORD);
			if (storedPovCount > tempLen)
			{
				storedPovCount = tempLen;
			}

			DWORD numPov = 0;
			for (i = 0; i < storedPovCount; i++)
			{
				GameControllerItem::ControllerAxisDirection dir = (GameControllerItem::ControllerAxisDirection)offsetList[i];
				switch (dir)
				{
				case GameControllerItem::DirectionAny:
				case GameControllerItem::DirectionUp:
				case GameControllerItem::DirectionDown:
				case GameControllerItem::DirectionLeft:
				case GameControllerItem::DirectionRight:
					break;
				default:
					dir = GameControllerItem::DirectionAny;
				}

				if (pPovDirection != NULL && numPov < maxPovBufferCount)
				{
					pPovDirection[numPov] = (GameControllerItem::ControllerAxisDirection)dir;
				}

				numPov++;
			}
		}
	}

	if (pPovCount != NULL)
	{
		*pPovCount = povCount;
	}

	return S_OK;
}

HRESULT CConfig::WriteJoystickPovList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pPovOffsets, const GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int povCount)
{
DWORD dwValue;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;
DWORD dwitem[joyconfig::MAXPOV];

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	if (povCount > joyconfig::MAXPOV)
	{
		povCount = joyconfig::MAXPOV;
	}

	for (i=0; i < povCount; i++)
	{
		dwitem[i] = (DWORD)pPovDirection[i];
	}

	dwValue = (DWORD)povCount;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.count], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], 0, REG_BINARY, (LPBYTE) &pPovOffsets[0], sizeof(DWORD) * povCount);
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.mask], 0, REG_BINARY, (LPBYTE) &dwitem[0], sizeof(DWORD) * povCount);
	return S_OK;
}

HRESULT CConfig::ReadJoystickButtonList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pButtonOffsets, unsigned int &buttonCount)
{
LONG lRetCode;
DWORD dw;
DWORD dwOffset;
DWORD buttonIndexList[joyconfig::MAXBUTTONS];
DWORD storedButtonCount;
DWORD tempLen;
DWORD numButtons = 0;
unsigned int i;
unsigned int j;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name[joyIndex]))
	{
		return E_FAIL;
	}

	buttonCount = 0;
	lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][regnames.count], (LPDWORD)&storedButtonCount);
	if (lRetCode == ERROR_SUCCESS)
	{
		// Array of 32 bit indexes. Maximum of 128 buttons.
		if (storedButtonCount > joyconfig::MAXBUTTONS)
		{
			storedButtonCount = joyconfig::MAXBUTTONS;
		}

		tempLen = sizeof(buttonIndexList);
		lRetCode = RegQueryValueEx(hKey1, JoyKeyName::Name[joyIndex][regnames.list], NULL, NULL, (LPBYTE)&buttonIndexList[0], &tempLen);
		if (lRetCode == ERROR_SUCCESS)
		{
			tempLen = tempLen / sizeof(DWORD);
			if (storedButtonCount > tempLen)
			{
				storedButtonCount = tempLen;
			}

			for (i = 0; i < storedButtonCount; i++)
			{
				dwOffset = buttonIndexList[i];
				if (dwOffset >= DIJOFS_BUTTON0 && dwOffset < DIJOFS_BUTTON(joyconfig::MAXBUTTONS))
				{
					pButtonOffsets[numButtons++] = dwOffset;
				}
			}

			buttonCount = numButtons;
		}
	}
	else if (lRetCode == ERROR_FILE_NOT_FOUND)
	{
		// 32 bit mask representation. Maximum of 32 buttons.
		lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][regnames.mask], &dw);
		if (lRetCode == ERROR_SUCCESS)
		{
			for (i = 0, j = 1; i < joyconfig::MAXBUTTONS32; i++, j<<=1)
			{
				if (dw & j)
				{
					dwOffset = DIJOFS_BUTTON0 + i;
					if (dwOffset >= DIJOFS_BUTTON0 && dwOffset <= DIJOFS_BUTTON31)
					{
						pButtonOffsets[numButtons++] = dwOffset;
					}
				}
			}

			buttonCount = numButtons;
		}

		// Single button. One of a 128 buttons.
		if (lRetCode == ERROR_FILE_NOT_FOUND || (lRetCode == ERROR_SUCCESS && numButtons == 0))
		{
			lRetCode = RegReadDWordOrStr(hKey1, JoyKeyName::Name[joyIndex][regnames.single], &dw);
			if (lRetCode == ERROR_SUCCESS)
			{
				dwOffset = dw;
				if (dwOffset >= DIJOFS_BUTTON0 && dwOffset < DIJOFS_BUTTON(joyconfig::MAXBUTTONS))
				{
					pButtonOffsets[numButtons++] = dwOffset;
				}
			}

			buttonCount = numButtons;
		}
	}
	else
	{
		return lRetCode;
	}
	
	return S_OK;
}


#define writeregkeyboarditem(n)	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_KeyMap[n]));\
	RegSetValueEx(hKey1, TEXT(#n), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR))

HRESULT CConfig::SaveWindowSetting(HWND hWnd)
{
TCHAR szValue[20];
HKEY  hKey1; 
DWORD  dwDisposition; 
LONG   lRetCode; 
WINDOWPLACEMENT wp;

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	lRetCode = GetWindowPlacement(hWnd, &wp);
	if (lRetCode == 0)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}
	POINT pt_winpos;
	pt_winpos.x = wp.rcNormalPosition.left;
	pt_winpos.y = wp.rcNormalPosition.top;

	lRetCode = RegCreateKeyEx ( HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	} 
	
	wsprintf(szValue, TEXT("%d"), (int) (pt_winpos.x));
	RegSetValueEx(hKey1, TEXT("MainWinPosX"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%d"), (int) (pt_winpos.y));
	RegSetValueEx(hKey1, TEXT("MainWinPosY"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	DWORD dwWindowedCustomSize = this->m_bWindowedCustomSize ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("WindowedCustomSize"), 0, REG_DWORD, (LPBYTE) &dwWindowedCustomSize, sizeof(dwWindowedCustomSize));

	int w = max(0, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	int h = max(0, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);		

	//Warning
	//VS2005 does not have an templated overload for fixed size arrays which then does not expect the count pararmter to be passed in.
	//In other words, would be passing in too many arguments. 
	//The count (_TRUNCATE) would be seen as an additional argument for formatting would cause a buffer overrun in szValue.
	//The cast of buffer to (TCHAR *) and the passing of count as _TRUNCATE ensures that VS2005 will compile correctly
	_sntprintf_s((TCHAR *)(&szValue[0]), _countof(szValue), _TRUNCATE, TEXT("%d"), w);
	RegSetValueEx(hKey1, TEXT("MainWinWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	
	_sntprintf_s((TCHAR *)(&szValue[0]), _countof(szValue), _TRUNCATE, TEXT("%d"), h);
	RegSetValueEx(hKey1, TEXT("MainWinHeight"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%d"), (int) (m_bDoubleSizedWindow ? 1: 0));
	RegSetValueEx(hKey1, TEXT("DoubleSizedWindow"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	
	RegCloseKey(hKey1);

	return S_OK;
}

HRESULT CConfig::SaveMDIWindowSetting(HWND hWnd)
{
TCHAR szValue[20];
HKEY  hKey1; 
DWORD  dwDisposition; 
LONG   lRetCode; 
WINDOWPLACEMENT wp;

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	lRetCode = GetWindowPlacement(hWnd, &wp);
	if (lRetCode == 0)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}
	POINT pt_mdidebuggerwinpos;
	SIZE sz_mdidebuggerwinsize;
	pt_mdidebuggerwinpos.x =wp.rcNormalPosition.left;
	pt_mdidebuggerwinpos.y =wp.rcNormalPosition.top;
	sz_mdidebuggerwinsize.cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	sz_mdidebuggerwinsize.cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

	lRetCode = RegCreateKeyEx ( HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	} 

	wsprintf(szValue, TEXT("%ld"), (LONG) (pt_mdidebuggerwinpos.x));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosX"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (LONG) (pt_mdidebuggerwinpos.y));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosY"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (LONG) (sz_mdidebuggerwinsize.cx));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (LONG) (sz_mdidebuggerwinsize.cy));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerHeight"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	RegCloseKey(hKey1);

	return S_OK;
}

HRESULT CConfig::LoadWindowSetting(POINT& pos, bool& bWindowedCustomSize, int& winWidth, int& winHeight)
{
TCHAR szValue[20];
HKEY  hKey1; 
LONG   lRetCode; 
ULONG tempLenValue,lenValue;

	DWORD _WindowedCustomSize = 0;
	POINT _pos = {0, 0};
	int w = 0;
	int h = 0;
	bool ok = false;
	bool customok = false;
	do
	{
		lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
			TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
			0, KEY_READ,
			&hKey1);	
		if (lRetCode != ERROR_SUCCESS)
			break;

		const int max_width = GetSystemMetrics(SM_CXMAXTRACK);
		const int max_height = GetSystemMetrics(SM_CYMAXTRACK);
		const int min_width = GetSystemMetrics(SM_CXMINTRACK);
		const int min_height = GetSystemMetrics(SM_CYMINTRACK);

		lenValue = sizeof(szValue);
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MainWinPosX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		errno = 0;
		_pos.x = _ttol(szValue);
		if (errno != 0)
		{
			break;
		}
		
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MainWinPosY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		errno = 0;
		_pos.y = _ttol(szValue);
		if (errno != 0)
		{
			break;
		}
		ok = true;

		tempLenValue = sizeof(_WindowedCustomSize);
		lRetCode = RegQueryValueEx(hKey1, TEXT("WindowedCustomSize"), NULL, NULL, (PBYTE) &_WindowedCustomSize, &tempLenValue);
		if (lRetCode != ERROR_SUCCESS)
			break;

		if (_WindowedCustomSize)
		{
			tempLenValue = lenValue;
			lRetCode = RegReadStr(hKey1, TEXT("MainWinWidth"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
			if (lRetCode != ERROR_SUCCESS)
			{
				break;
			}

			errno = 0;
			w = max(min(_ttol(szValue), max_width), min_width);
			if (errno != 0)
			{
				break;
			}

			tempLenValue = lenValue;
			lRetCode = RegReadStr(hKey1, TEXT("MainWinHeight"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
			if (lRetCode != ERROR_SUCCESS)
			{
				break;
			}

			errno = 0;
			h = max(min(_ttol(szValue), max_height), min_height);
			if (errno != 0)
			{
				break;
			}

			customok = true;

		}		
	} while(false);
	if (ok)
	{
		if (!customok)
		{
			_WindowedCustomSize = false;
			w = 0;
			h = 0;
		}
		pos = _pos;
		winWidth = w;
		winHeight = h;
		bWindowedCustomSize = _WindowedCustomSize != 0;
		m_bWindowedCustomSize = bWindowedCustomSize;
	}
	return ok ? S_OK : E_FAIL;
}

HRESULT CConfig::LoadMDIWindowSetting(POINT& pos, SIZE& size)
{
TCHAR szValue[20];
HKEY  hKey1; 
LONG   lRetCode; 
ULONG tempLenValue,lenValue;

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, KEY_READ,
		&hKey1);	

	int top = 0;
	int left = 0;

	int max_x = GetSystemMetrics(SM_CXMAXTRACK);
	int max_y = GetSystemMetrics(SM_CYMAXTRACK);

	int min_x = GetSystemMetrics(SM_CXMIN);
	int min_y = GetSystemMetrics(SM_CYMIN);

	pos.x = left;
	pos.y = top;
	size.cx = 0;
	size.cy = 0;
	if (lRetCode == ERROR_SUCCESS)
	{
		lenValue = sizeof(szValue);

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MDIWinDebuggerPosX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			pos.x = max(min(_ttol(szValue), max_x), left);
		}
		
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MDIWinDebuggerPosY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			pos.y = max(min(_ttol(szValue), max_y), top);
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MDIWinDebuggerWidth"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{			
			size.cx = max(min(_ttol(szValue), max_x), min_x);
		}
		
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("MDIWinDebuggerHeight"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			size.cy = max(min(_ttol(szValue), max_y), min_y);
		}
		
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT CConfig::SaveCurrentSetting()
{
TCHAR szValue[50];
HKEY  hKey1; 
DWORD  dwDisposition; 
LONG   lRetCode; 
int i;

	lRetCode = RegCreateKeyEx (HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Keyboard"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 

	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	} 

	writeregkeyboarditem(C64Keys::C64K_0);
	writeregkeyboarditem(C64Keys::C64K_1);
	writeregkeyboarditem(C64Keys::C64K_2);
	writeregkeyboarditem(C64Keys::C64K_3);
	writeregkeyboarditem(C64Keys::C64K_4);
	writeregkeyboarditem(C64Keys::C64K_5);
	writeregkeyboarditem(C64Keys::C64K_6);
	writeregkeyboarditem(C64Keys::C64K_7);
	writeregkeyboarditem(C64Keys::C64K_8);
	writeregkeyboarditem(C64Keys::C64K_9);
	writeregkeyboarditem(C64Keys::C64K_A);
	writeregkeyboarditem(C64Keys::C64K_B);
	writeregkeyboarditem(C64Keys::C64K_C);
	writeregkeyboarditem(C64Keys::C64K_D);
	writeregkeyboarditem(C64Keys::C64K_E);
	writeregkeyboarditem(C64Keys::C64K_F);
	writeregkeyboarditem(C64Keys::C64K_G);
	writeregkeyboarditem(C64Keys::C64K_H);
	writeregkeyboarditem(C64Keys::C64K_I);
	writeregkeyboarditem(C64Keys::C64K_J);
	writeregkeyboarditem(C64Keys::C64K_K);
	writeregkeyboarditem(C64Keys::C64K_L);
	writeregkeyboarditem(C64Keys::C64K_M);
	writeregkeyboarditem(C64Keys::C64K_N);
	writeregkeyboarditem(C64Keys::C64K_O);
	writeregkeyboarditem(C64Keys::C64K_P);
	writeregkeyboarditem(C64Keys::C64K_Q);
	writeregkeyboarditem(C64Keys::C64K_R);
	writeregkeyboarditem(C64Keys::C64K_S);
	writeregkeyboarditem(C64Keys::C64K_T);
	writeregkeyboarditem(C64Keys::C64K_U);
	writeregkeyboarditem(C64Keys::C64K_V);
	writeregkeyboarditem(C64Keys::C64K_W);
	writeregkeyboarditem(C64Keys::C64K_X);
	writeregkeyboarditem(C64Keys::C64K_Y);
	writeregkeyboarditem(C64Keys::C64K_Z);
	writeregkeyboarditem(C64Keys::C64K_PLUS);
	writeregkeyboarditem(C64Keys::C64K_MINUS);
	writeregkeyboarditem(C64Keys::C64K_ASTERISK);
	writeregkeyboarditem(C64Keys::C64K_SLASH);
	writeregkeyboarditem(C64Keys::C64K_COMMA);
	writeregkeyboarditem(C64Keys::C64K_DOT);
	writeregkeyboarditem(C64Keys::C64K_ARROWLEFT);
	writeregkeyboarditem(C64Keys::C64K_COLON);
	writeregkeyboarditem(C64Keys::C64K_SEMICOLON);
	writeregkeyboarditem(C64Keys::C64K_CONTROL);
	writeregkeyboarditem(C64Keys::C64K_STOP);
	writeregkeyboarditem(C64Keys::C64K_COMMODORE);
	writeregkeyboarditem(C64Keys::C64K_LEFTSHIFT);
	writeregkeyboarditem(C64Keys::C64K_RIGHTSHIFT);
	writeregkeyboarditem(C64Keys::C64K_RESTORE);
	writeregkeyboarditem(C64Keys::C64K_HOME);
	writeregkeyboarditem(C64Keys::C64K_DEL);
	writeregkeyboarditem(C64Keys::C64K_RETURN);
	writeregkeyboarditem(C64Keys::C64K_ARROWUP);
	writeregkeyboarditem(C64Keys::C64K_POUND);
	writeregkeyboarditem(C64Keys::C64K_EQUAL);
	writeregkeyboarditem(C64Keys::C64K_CURSORDOWN);
	writeregkeyboarditem(C64Keys::C64K_CURSORRIGHT);
	writeregkeyboarditem(C64Keys::C64K_CURSORUP);
	writeregkeyboarditem(C64Keys::C64K_CURSORLEFT);
	writeregkeyboarditem(C64Keys::C64K_SPACE);
	writeregkeyboarditem(C64Keys::C64K_AT);
	writeregkeyboarditem(C64Keys::C64K_F1);
	writeregkeyboarditem(C64Keys::C64K_F2);
	writeregkeyboarditem(C64Keys::C64K_F3);
	writeregkeyboarditem(C64Keys::C64K_F4);
	writeregkeyboarditem(C64Keys::C64K_F5);
	writeregkeyboarditem(C64Keys::C64K_F6);
	writeregkeyboarditem(C64Keys::C64K_F7);
	writeregkeyboarditem(C64Keys::C64K_F8);

	writeregkeyboarditem(C64Keys::C64K_JOY1FIRE);
	writeregkeyboarditem(C64Keys::C64K_JOY1UP);
	writeregkeyboarditem(C64Keys::C64K_JOY1DOWN);
	writeregkeyboarditem(C64Keys::C64K_JOY1LEFT);
	writeregkeyboarditem(C64Keys::C64K_JOY1RIGHT);
	writeregkeyboarditem(C64Keys::C64K_JOY2FIRE);
	writeregkeyboarditem(C64Keys::C64K_JOY2UP);
	writeregkeyboarditem(C64Keys::C64K_JOY2DOWN);
	writeregkeyboarditem(C64Keys::C64K_JOY2LEFT);
	writeregkeyboarditem(C64Keys::C64K_JOY2RIGHT);	

	writeregkeyboarditem(C64Keys::C64K_JOY1FIRE2);
	writeregkeyboarditem(C64Keys::C64K_JOY2FIRE2);
	RegCloseKey(hKey1);
	lRetCode = RegCreateKeyEx ( HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bD1541_Emulation_Enable ? 1: 0));
	RegSetValueEx(hKey1, TEXT("D1541_Emulation"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bSID_Emulation_Enable ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SID_Emulation"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bShowSpeed ? 1: 0));
	RegSetValueEx(hKey1, TEXT("ShowSpeed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bLimitSpeed ? 1: 0));
	RegSetValueEx(hKey1, TEXT("LimitSpeed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bSkipFrames ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SkipAltFrames"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_bSIDResampleMode);
	RegSetValueEx(hKey1, TEXT("SIDSampleMode"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_syncModeFullscreen);
	RegSetValueEx(hKey1, TEXT("SyncMode1"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_syncModeWindowed);
	RegSetValueEx(hKey1, TEXT("SyncMode2"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bDoubleSizedWindow ? 1: 0));
	RegSetValueEx(hKey1, TEXT("DoubleSizedWindow"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bUseBlitStretch ? 1: 0));
	RegSetValueEx(hKey1, TEXT("UseBlitStretch"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bUseKeymap ? 1: 0));
	RegSetValueEx(hKey1, TEXT("UseKeymap"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bSwapJoysticks ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SwapJoysticks"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bCPUFriendly ? 1: 0));
	RegSetValueEx(hKey1, TEXT("CPUFriendly"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bAudioClockSync ? 1: 0));
	RegSetValueEx(hKey1, TEXT("AudioClockSync"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	
	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bSidDigiBoost ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SIDDigiBoost"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	G::SaveClsidToRegValue(hKey1, TEXT("FullscreenAdapterId"), &m_fullscreenAdapterId);

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenAdapterNumber);
	RegSetValueEx(hKey1, TEXT("FullscreenAdapterNumber"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenWidth);
	RegSetValueEx(hKey1, TEXT("FullscreenWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenHeight);
	RegSetValueEx(hKey1, TEXT("FullscreenHeight"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenRefresh);
	RegSetValueEx(hKey1, TEXT("FullscreenRefresh"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenFormat);
	RegSetValueEx(hKey1, TEXT("FullscreenFormat"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fullscreenStretch);
	RegSetValueEx(hKey1, TEXT("FullscreenStretch"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_blitFilter);
	RegSetValueEx(hKey1, TEXT("BlitFilter"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_borderSize);
	RegSetValueEx(hKey1, TEXT("BorderSize"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bShowFloppyLed ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("ShowFloppyLed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) m_fps);
	RegSetValueEx(hKey1, TEXT("FPS"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_TrackZeroSensorStyle));
	RegSetValueEx(hKey1, TEXT("TrackZeroSensor"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_CIAMode));
	RegSetValueEx(hKey1, TEXT("CIAMode"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bTimerBbug ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("CIATimerBbug"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	DWORD dwValue = (DWORD)this->m_numberOfExtraSIDs;
	if (dwValue >= 8)
	{
		dwValue = 0;
	}

	RegSetValueEx(hKey1, TEXT("NumberOfExtraSidChips"), 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	dwValue = m_bSIDStereo ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SIDStereo"), 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	LPCTSTR sidAddressName[] = { 
		TEXT("Sid2Address"), 
		TEXT("Sid3Address"), 
		TEXT("Sid4Address"), 
		TEXT("Sid5Address"), 
		TEXT("Sid6Address"), 
		TEXT("Sid7Address"), 
		TEXT("Sid8Address") 
	};

	bit16* sidAddressValue[] = { 
		&this->m_Sid2Address,
		&this->m_Sid3Address,
		&this->m_Sid4Address,
		&this->m_Sid5Address,
		&this->m_Sid6Address,
		&this->m_Sid7Address,
		&this->m_Sid8Address
	};

	for (int i=0; i < _countof(sidAddressName); i++)
	{
		DWORD dwValue = *sidAddressValue[i];
		RegSetValueEx(hKey1, sidAddressName[i], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	}
	
	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bD1541_Thread_Enable ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("DiskThreadEnable"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bAllowOpposingJoystick ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("AllowOpposingJoystick"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (ULONG) (m_bDisableDwmFullscreen ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("DisableDwmFullscreen"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	lstrcpy(szValue, TEXT("1"));
	RegSetValueEx(hKey1, TEXT("PrefsSaved"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	RegCloseKey(hKey1);

	lRetCode = RegCreateKeyEx ( HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\VICIIPalette"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	} 

	std::basic_string<TCHAR> colorregkeyname;
	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		colorregkeyname.clear();
		colorregkeyname.append(TEXT("color_"));
		if (i < 0xa)
		{
			colorregkeyname.push_back(TEXT('0') + i);
		}
		else
		{
			colorregkeyname.push_back(TEXT('a') + i - 0xa);
		}
		DWORD rgbcolor = this->m_colour_palette[i];
		RegSetValueEx(hKey1, colorregkeyname.c_str(), NULL, REG_DWORD, (PBYTE) &rgbcolor, sizeof(DWORD));
	}

	RegCloseKey(hKey1);

	//Save joystick 1 setting.
	SaveCurrentJoystickSetting(1, this->m_joy1config);

	//Save joystick 2 setting.
	SaveCurrentJoystickSetting(2, this->m_joy2config);
	return S_OK;
}

HRESULT CConfig::SaveCurrentJoystickSetting(int joystickNumber, const struct joyconfig& jconfig)
{
HKEY  hKey1; 
DWORD  dwDisposition; 
LONG   lRetCode; 
bool bGuidOK;
DWORD dwValue;
DWORD dwByteLength;
unsigned int i;

	int joyIndex = joystickNumber - 1;
	lRetCode = RegCreateKeyEx ( HKEY_CURRENT_USER, 
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Joystick"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hKey1, 
		&dwDisposition); 
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	} 

	//Save the joystick enabled option.
	dwValue = (jconfig.IsEnabled ? 1: 0);
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynEnabled], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the POV option.
	dwValue = (jconfig.isPovEnabled ? 0xffffffff: 0);
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynPOV], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	bGuidOK = false;
	if (jconfig.IsValidId)
	{
		if (SUCCEEDED(G::SaveClsidToRegValue(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynGUID], &jconfig.joystickID)))
		{
			bGuidOK = true;
		}
	}

	//Save the device ID.
	dwValue = ((jconfig.IsValidId && bGuidOK) ? 1: 0);
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynValid], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the X axis validity.
	dwValue = (jconfig.isValidXAxis && jconfig.horizontalAxisAxisCount > 0) ? 1 : 0;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisX], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the X axis.
	dwValue = jconfig.dwOfs_X;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisX], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the Y axis validity.
	dwValue =(jconfig.isValidYAxis && jconfig.verticalAxisAxisCount > 0) ? 1 : 0;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisY], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the Y axis.
	dwValue = jconfig.dwOfs_Y;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisY], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	
	//Save the X axis reverse.
	dwValue = jconfig.isXReverse ? 1 : 0;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevX], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));

	//Save the Y axis reverse.
	dwValue = jconfig.isYReverse ? 1 : 0;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevY], 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD));
	
	// Save map of game buttons to C64 joystick.
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Fire1, &jconfig.fire1ButtonOffsets[0], jconfig.fire1ButtonCount);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Fire2, &jconfig.fire2ButtonOffsets[0], jconfig.fire2ButtonCount);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Up, &jconfig.upButtonOffsets[0], jconfig.upButtonCount);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Down, &jconfig.downButtonOffsets[0], jconfig.downButtonCount);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Left, &jconfig.leftButtonOffsets[0], jconfig.leftButtonCount);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::Right, &jconfig.rightButtonOffsets[0], jconfig.rightButtonCount);

	// Save map of game buttons to C64 keys.
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey1, &jconfig.keyNButtonOffsets[0][0], jconfig.keyNButtonCount[0]);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey2, &jconfig.keyNButtonOffsets[1][0], jconfig.keyNButtonCount[1]);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey3, &jconfig.keyNButtonOffsets[2][0], jconfig.keyNButtonCount[2]);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey4, &jconfig.keyNButtonOffsets[3][0], jconfig.keyNButtonCount[3]);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey5, &jconfig.keyNButtonOffsets[4][0], jconfig.keyNButtonCount[4]);
	WriteJoystickButtonList(hKey1, joystickNumber, JoyKeyName::ButtonKey6, &jconfig.keyNButtonOffsets[5][0], jconfig.keyNButtonCount[5]);

	// Save map of game axes to C64 keys.
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey1, &jconfig.keyNAxisOffsets[0][0], &jconfig.keyNAxisDirection[0][0], jconfig.keyNAxisCount[0]);
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey2, &jconfig.keyNAxisOffsets[1][0], &jconfig.keyNAxisDirection[1][0], jconfig.keyNAxisCount[1]);
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey3, &jconfig.keyNAxisOffsets[2][0], &jconfig.keyNAxisDirection[2][0], jconfig.keyNAxisCount[2]);
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey4, &jconfig.keyNAxisOffsets[3][0], &jconfig.keyNAxisDirection[3][0], jconfig.keyNAxisCount[3]);
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey5, &jconfig.keyNAxisOffsets[4][0], &jconfig.keyNAxisDirection[4][0], jconfig.keyNAxisCount[4]);
	WriteJoystickAxisList(hKey1, joystickNumber, JoyKeyName::AxisKey6, &jconfig.keyNAxisOffsets[5][0], &jconfig.keyNAxisDirection[5][0], jconfig.keyNAxisCount[5]);

	// Save map of game pov to C64 keys.
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey1, &jconfig.keyNPovOffsets[0][0], &jconfig.keyNPovDirection[0][0], jconfig.keyNPovCount[0]);
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey2, &jconfig.keyNPovOffsets[1][0], &jconfig.keyNPovDirection[1][0], jconfig.keyNPovCount[1]);
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey3, &jconfig.keyNPovOffsets[2][0], &jconfig.keyNPovDirection[2][0], jconfig.keyNPovCount[2]);
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey4, &jconfig.keyNPovOffsets[3][0], &jconfig.keyNPovDirection[3][0], jconfig.keyNPovCount[3]);
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey5, &jconfig.keyNPovOffsets[4][0], &jconfig.keyNPovDirection[4][0], jconfig.keyNPovCount[4]);
	WriteJoystickPovList(hKey1, joystickNumber, JoyKeyName::PovKey6, &jconfig.keyNPovOffsets[5][0], &jconfig.keyNPovDirection[5][0], jconfig.keyNPovCount[5]);

	//Save the key assignment array length.
	unsigned int numberOfKeys = joyconfig::MaxUserKeyAssignCount;

	//Save the key assignment array.	
	dwByteLength = numberOfKeys;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], 0, REG_BINARY, (LPBYTE) &jconfig.keyNoAssign[0], dwByteLength);	

	//Save the key assignment validity array.
	bit8 c64KeyAssignValid[joyconfig::MaxUserKeyAssignCount];
	for (i = 0; i < _countof(c64KeyAssignValid); i++)
	{
		if (i < _countof(jconfig.isValidKeyNoAssign))
		{
			c64KeyAssignValid[i] = jconfig.isValidKeyNoAssign[i] ? 1 : 0;
		}
		else
		{
			c64KeyAssignValid[i] = 0;
		}
	}

	dwByteLength = numberOfKeys;
	RegSetValueEx(hKey1, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], 0, REG_BINARY, (LPBYTE) &c64KeyAssignValid[0], dwByteLength);	

	// Close the reg key.
	RegCloseKey(hKey1);
	return S_OK;
}

void CConfig::SetPalettePepto()
{
int i;
	for(i = 0; i < VicIIPalette::NumColours; i++)
	{
		m_colour_palette[i] = VicIIPalette::Pepto[i];
	}
}

void CConfig::LoadDefaultSetting()
{
	SetPalettePepto();
	ZeroMemory(&m_KeyMap[0], sizeof(m_KeyMap));
	m_joy1config.LoadDefault();
	m_joy2config.LoadDefault();
	m_KeyMap[C64Keys::C64K_PLUS]=	DIK_MINUS;
	m_KeyMap[C64Keys::C64K_MINUS]=	DIK_EQUALS;
	m_KeyMap[C64Keys::C64K_ASTERISK]= DIK_RBRACKET;
	m_KeyMap[C64Keys::C64K_SLASH]=	DIK_SLASH;
	m_KeyMap[C64Keys::C64K_COMMA]=	DIK_COMMA;
	m_KeyMap[C64Keys::C64K_DOT]= DIK_PERIOD;
	m_KeyMap[C64Keys::C64K_ARROWLEFT]=	DIK_GRAVE;
	m_KeyMap[C64Keys::C64K_COLON]=	DIK_SEMICOLON;
	m_KeyMap[C64Keys::C64K_SEMICOLON]=	DIK_APOSTROPHE;
	m_KeyMap[C64Keys::C64K_AT]= DIK_LBRACKET;
	m_KeyMap[C64Keys::C64K_EQUAL]=	DIK_BACKSLASH;
	m_KeyMap[C64Keys::C64K_RESTORE]= DIK_PRIOR;
	m_KeyMap[C64Keys::C64K_HOME]= DIK_HOME;
	m_KeyMap[C64Keys::C64K_ARROWUP]= DIK_DELETE;
	m_KeyMap[C64Keys::C64K_POUND]=	DIK_INSERT;
	m_KeyMap[C64Keys::C64K_CURSORDOWN]= DIK_DOWN;
	m_KeyMap[C64Keys::C64K_CURSORRIGHT]= DIK_RIGHT;
	m_KeyMap[C64Keys::C64K_CURSORUP]= DIK_UP;
	m_KeyMap[C64Keys::C64K_CURSORLEFT]= DIK_LEFT;
	m_KeyMap[C64Keys::C64K_CONTROL]= GetKeyScanCode(VK_TAB);
	m_KeyMap[C64Keys::C64K_LEFTSHIFT]= DIK_LSHIFT;
	m_KeyMap[C64Keys::C64K_RIGHTSHIFT]= DIK_RSHIFT;
	m_KeyMap[C64Keys::C64K_STOP]= GetKeyScanCode(VK_ESCAPE);
	m_KeyMap[C64Keys::C64K_COMMODORE]= DIK_LCONTROL;
	m_KeyMap[C64Keys::C64K_DEL]= GetKeyScanCode(VK_BACK);
	m_KeyMap[C64Keys::C64K_RETURN]= GetKeyScanCode(VK_RETURN);
	m_KeyMap[C64Keys::C64K_SPACE]=	GetKeyScanCode(' ');
	m_KeyMap[C64Keys::C64K_JOY1FIRE]= DIK_NUMPAD0;
	m_KeyMap[C64Keys::C64K_JOY1UP]= DIK_DIVIDE;
	m_KeyMap[C64Keys::C64K_JOY1DOWN]= DIK_NUMPAD5;
	m_KeyMap[C64Keys::C64K_JOY1LEFT]= DIK_NUMPAD7;
	m_KeyMap[C64Keys::C64K_JOY1RIGHT]= DIK_NUMPAD9;
	m_KeyMap[C64Keys::C64K_JOY2FIRE]= DIK_RCONTROL;
	m_KeyMap[C64Keys::C64K_JOY2UP]= DIK_NUMPAD8;
	m_KeyMap[C64Keys::C64K_JOY2DOWN]= DIK_NUMPAD2;
	m_KeyMap[C64Keys::C64K_JOY2LEFT]= DIK_NUMPAD4;
	m_KeyMap[C64Keys::C64K_JOY2RIGHT]= DIK_NUMPAD6;
	m_KeyMap[C64Keys::C64K_0]= GetKeyScanCode('0');
	m_KeyMap[C64Keys::C64K_1]=	GetKeyScanCode('1');
	m_KeyMap[C64Keys::C64K_2]=	GetKeyScanCode('2');
	m_KeyMap[C64Keys::C64K_3]=	GetKeyScanCode('3');
	m_KeyMap[C64Keys::C64K_4]=	GetKeyScanCode('4');
	m_KeyMap[C64Keys::C64K_5]=	GetKeyScanCode('5');
	m_KeyMap[C64Keys::C64K_6]=	GetKeyScanCode('6');
	m_KeyMap[C64Keys::C64K_7]=	GetKeyScanCode('7');
	m_KeyMap[C64Keys::C64K_8]=	GetKeyScanCode('8');
	m_KeyMap[C64Keys::C64K_9]=	GetKeyScanCode('9');
	m_KeyMap[C64Keys::C64K_A]=	GetKeyScanCode('A');
	m_KeyMap[C64Keys::C64K_B]=	GetKeyScanCode('B');
	m_KeyMap[C64Keys::C64K_C]=	GetKeyScanCode('C');
	m_KeyMap[C64Keys::C64K_D]=	GetKeyScanCode('D');
	m_KeyMap[C64Keys::C64K_E]=	GetKeyScanCode('E');
	m_KeyMap[C64Keys::C64K_F]=	GetKeyScanCode('F');
	m_KeyMap[C64Keys::C64K_G]=	GetKeyScanCode('G');
	m_KeyMap[C64Keys::C64K_H]=	GetKeyScanCode('H');
	m_KeyMap[C64Keys::C64K_I]=	GetKeyScanCode('I');
	m_KeyMap[C64Keys::C64K_J]=	GetKeyScanCode('J');
	m_KeyMap[C64Keys::C64K_K]=	GetKeyScanCode('K');
	m_KeyMap[C64Keys::C64K_L]=	GetKeyScanCode('L');
	m_KeyMap[C64Keys::C64K_M]=	GetKeyScanCode('M');
	m_KeyMap[C64Keys::C64K_N]=	GetKeyScanCode('N');
	m_KeyMap[C64Keys::C64K_O]=	GetKeyScanCode('O');
	m_KeyMap[C64Keys::C64K_P]=	GetKeyScanCode('P');
	m_KeyMap[C64Keys::C64K_Q]=	GetKeyScanCode('Q');
	m_KeyMap[C64Keys::C64K_R]=	GetKeyScanCode('R');
	m_KeyMap[C64Keys::C64K_S]=	GetKeyScanCode('S');
	m_KeyMap[C64Keys::C64K_T]=	GetKeyScanCode('T');
	m_KeyMap[C64Keys::C64K_U]=	GetKeyScanCode('U');
	m_KeyMap[C64Keys::C64K_V]=	GetKeyScanCode('V');
	m_KeyMap[C64Keys::C64K_W]=	GetKeyScanCode('W');
	m_KeyMap[C64Keys::C64K_X]=	GetKeyScanCode('X');
	m_KeyMap[C64Keys::C64K_Y]=	GetKeyScanCode('Y');
	m_KeyMap[C64Keys::C64K_Z]=	GetKeyScanCode('Z');
	m_KeyMap[C64Keys::C64K_F1]= GetKeyScanCode(VK_F1);
	m_KeyMap[C64Keys::C64K_F2]= GetKeyScanCode(VK_F2);
	m_KeyMap[C64Keys::C64K_F3]= GetKeyScanCode(VK_F3);
	m_KeyMap[C64Keys::C64K_F4]= GetKeyScanCode(VK_F4);
	m_KeyMap[C64Keys::C64K_F5]= GetKeyScanCode(VK_F5);
	m_KeyMap[C64Keys::C64K_F6]= GetKeyScanCode(VK_F6);
	m_KeyMap[C64Keys::C64K_F7]= GetKeyScanCode(VK_F7);
	m_KeyMap[C64Keys::C64K_F8]= GetKeyScanCode(VK_F8);
	m_bSID_Emulation_Enable = true;
	m_bD1541_Emulation_Enable = true;
	m_bSkipFrames = false;
	m_bShowSpeed = true;
	m_bLimitSpeed = true;
	m_bSIDResampleMode = true;
	m_syncModeFullscreen = HCFG::FSSM_VBL;
	m_syncModeWindowed = HCFG::FSSM_LINE;
	m_bDoubleSizedWindow = true;
	m_bUseBlitStretch = true;
	m_bUseKeymap = false;
	m_bSwapJoysticks = false;
	m_bCPUFriendly = true;
	m_bAudioClockSync = true;
	m_bSidDigiBoost = false;

	if (G::IsMultiCore())
	{
		m_bD1541_Thread_Enable = true;
	}
	else
	{
		m_bD1541_Thread_Enable = false;
	}

	m_bAllowOpposingJoystick = false;
	m_bDisableDwmFullscreen = false;
	m_fullscreenAdapterNumber = 0;
	m_fullscreenWidth = 0;
	m_fullscreenHeight = 0;
	m_fullscreenRefresh = 0;
	m_fullscreenFormat = 0;
	m_fullscreenStretch = HCFG::EMUWINSTR_AUTO;
	m_blitFilter = HCFG::EMUWINFILTER_AUTO;
	m_borderSize = HCFG::EMUBORDER_TV;
	m_bShowFloppyLed = true;
	m_fps = HCFG::EMUFPS_50_12;
	m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
	m_CIAMode = HCFG::CM_CIA6526A;
	m_bTimerBbug = false;
	SetCiaNewOldMode(true);
	m_numberOfExtraSIDs = 0;
	m_Sid2Address = 0;
	m_Sid3Address = 0;
	m_Sid4Address = 0;
	m_Sid5Address = 0;
	m_Sid6Address = 0;
	m_Sid7Address = 0;
	m_Sid8Address = 0;
	m_bSIDStereo = true;
}

int CConfig::GetKeyScanCode(UINT ch)
{
	return MapVirtualKey(ch, 0);
}

LONG CConfig::RegReadDWordOrStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD dwValue)
{
LONG lRetCode;
DWORD type;
DWORD tempLenValue;
DWORD dw;
TCHAR szValue[20];
LPDWORD lpReserved = 0;
	const DWORD lenValue = sizeof(szValue);
	lRetCode = RegQueryValueEx(hKey, lpValueName, lpReserved, &type, NULL, NULL);
	if (lRetCode == ERROR_SUCCESS)
	{
		if (type == REG_DWORD)
		{
			tempLenValue = sizeof(dw);
			lRetCode = RegQueryValueEx(hKey, lpValueName, lpReserved, NULL, (LPBYTE)&dw, &tempLenValue);
			if (lRetCode == ERROR_SUCCESS)
			{
				if (dwValue != NULL)
				{
					*dwValue = dw;
				}
			}
		}
		else if (type == REG_SZ)
		{
			tempLenValue = lenValue;
			lRetCode = RegReadStr(hKey, lpValueName, lpReserved, NULL, (PBYTE) &szValue[0], &tempLenValue);
			if (lRetCode == ERROR_SUCCESS)
			{
				errno = 0;
				dw = _tcstoul(szValue, NULL, 10);				
				if (errno == 0)
				{
					if (dwValue != NULL)
					{
						*dwValue = dw;
					}
				}
				else
				{
					lRetCode = ERROR_FILE_NOT_FOUND;
				}
			}
		}
		else
		{
			lRetCode = ERROR_FILE_NOT_FOUND;
		}
	}

	return lRetCode;
}

LONG CConfig::RegReadStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	DWORD maxlen = 0;
	if (lpcbData != NULL)
	{
		maxlen = *lpcbData;
	}

	LONG r = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (r == ERROR_SUCCESS)
	{
		DWORD bytesCopied = 0;
		if (lpcbData == NULL)
		{
			return r;
		}

		bytesCopied = *lpcbData;
		DWORD fixup = 0;
		if (sizeof(TCHAR) > 1)
		{
			//fixup if a TCHAR is cut in half.
			fixup = bytesCopied % sizeof(TCHAR);
		}

		if (fixup == 0 && bytesCopied >=sizeof(TCHAR))
		{
			if (((TCHAR *)lpData)[(bytesCopied / sizeof(TCHAR)) - 1] == TEXT('\0'))
			{
				//If the string is already NULL terminated then return.
				return r;
			}
		}

		if (bytesCopied + fixup + sizeof(TCHAR) >= maxlen)
		{
			//No room for the NULL terminator.
			return ERROR_MORE_DATA;
		}

		//Add the TCHAR null terminator plus any fix up zeros if the last TCHAR was cut in half.
		for (unsigned int i = 0; i < sizeof(TCHAR) + fixup; i++)
		{
			lpData[bytesCopied + i] = 0;
		}
	}
	return r;
}

joyconfig::joyconfig()
{	
	LoadDefault();
	joyNotAcquiredClock = 0;
}

void joyconfig::defaultClearAxisDirection(GameControllerItem::ControllerAxisDirection offsets[], GameControllerItem::ControllerAxisDirection axisDirection, unsigned int count)
{
	if (offsets != NULL)
	{
		for(unsigned int i = 0; i < count; i++)
		{
			offsets[i] = axisDirection;
		}
	}
}

void joyconfig::LoadDefault()
{
	unsigned int i;
	ZeroMemory(&joystickID, sizeof(joystickID));
	ZeroMemory(&fire1ButtonOffsets, sizeof(fire1ButtonOffsets));
	ZeroMemory(&fire2ButtonOffsets, sizeof(fire2ButtonOffsets));
	ZeroMemory(&upButtonOffsets, sizeof(upButtonOffsets));
	ZeroMemory(&downButtonOffsets, sizeof(downButtonOffsets));
	ZeroMemory(&leftButtonOffsets, sizeof(leftButtonOffsets));
	ZeroMemory(&rightButtonOffsets, sizeof(rightButtonOffsets));
	ZeroMemory(&keyNoAssign, sizeof(keyNoAssign));
	ZeroMemory(&isValidKeyNoAssign, sizeof(isValidKeyNoAssign));		
	ZeroMemory(&axes, sizeof(axes));
	IsEnabled = false;
	isPovEnabled = true;
	IsValidId = false;
	isXReverse = false;
	isYReverse = false;
	dwOfs_X = DIJOFS_X;
	horizontalAxisAxisCount = 1;
	horizontalAxisButtonCount = 0;
	dwOfs_Y = DIJOFS_Y;
	verticalAxisAxisCount = 1;
	verticalAxisButtonCount = 0;
	isValidXAxis = true;
	isValidYAxis = true;
	fire1ButtonOffsets[0] = DIJOFS_BUTTON0;
	fire1ButtonCount = 1;
	fire1AxisCount = 0;
	fire1PovCount = 0;
	fire2ButtonCount = 0;
	fire2AxisCount = 0;
	fire2PovCount = 0;
	upButtonCount = 0;
	upAxisCount = 0;
	upPovCount = 0;
	downButtonCount = 0;
	downAxisCount = 0;
	downPovCount = 0;
	leftButtonCount = 0;
	leftAxisCount = 0;
	leftPovCount = 0;
	rightButtonCount = 0;
	rightAxisCount = 0;
	rightPovCount = 0;
	enableKeyAssign = false;
	for (i=0; i < MAXKEYMAPS; i++)
	{
		defaultClearAxisDirection(keyNAxisDirection[i], GameControllerItem::DirectionAny, MAXAXIS);
		ZeroMemory(&keyNButtonOffsets[i], sizeof(keyNButtonOffsets[0]));
		ZeroMemory(&keyNAxisOffsets[i], sizeof(keyNAxisOffsets[0]));
		ZeroMemory(&pov[i], sizeof(pov[0]));
		keyNButtonCount[i] = 0;
		keyNAxisCount[i] = 0;
		keyNPovCount[i] = 0;
	}

	inputDeviceFormat = &c_dfDIJoystick;
	sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
};

