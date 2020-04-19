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
#include "StringConverter.h"
#include "ErrorLogger.h"
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

CConfig::CConfig() noexcept
{
	m_fullscreenAdapterIsDefault = true;
	m_fullscreenAdapterNumber = 0;
	m_fullscreenOutputNumber = 0;
	ZeroMemory(&m_joy1config.joystickID, sizeof(m_joy1config.joystickID));
	ZeroMemory(&m_joy2config.joystickID, sizeof(m_joy2config.joystickID));
	m_bD1541_Thread_Enable = false;
	m_bAllowOpposingJoystick = false;
	m_bDisableDwmFullscreen = false;
	m_bEnableImGuiWindowed = true;
	m_bWindowedLockAspectRatio = false;
	m_bSwapJoysticks = false;
	m_bCPUFriendly = true;
	m_bAudioClockSync = true;
	m_bSidDigiBoost = false;
	m_bMaxSpeed = false;
	m_bSkipFrames = false;
	m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
	m_CIAMode = HCFG::CM_CIA6526A;
	m_bTimerBbug = false;
	m_bWindowedLockAspectRatio = false;
	m_bSIDStereo = true;
	SetPalettePepto();
	m_numberOfExtraSIDs = 0;
	m_Sid2Address = 0;
	m_Sid3Address = 0;
	m_Sid4Address = 0;
	m_Sid5Address = 0;
	m_Sid6Address = 0;
	m_Sid7Address = 0;
	m_Sid8Address = 0;
	LoadDefaultSetting();
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

HRESULT CConfig::LoadCurrentSetting()
{
	HKEY  hKey1; 
	LONG   lRetCode; 
	ULONG tempLenValue;
	DWORD type;
	DWORD dwValue;
	int i;
	
	LoadDefaultSetting();
	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("PrefsSaved"), &dwValue);
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
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_0"), C64Keys::C64K_0);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_1"), C64Keys::C64K_1);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_2"), C64Keys::C64K_2);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_3"), C64Keys::C64K_3);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_4"), C64Keys::C64K_4);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_5"), C64Keys::C64K_5);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_6"), C64Keys::C64K_6);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_7"), C64Keys::C64K_7);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_8"), C64Keys::C64K_8);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_9"), C64Keys::C64K_9);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_A"), C64Keys::C64K_A);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_B"), C64Keys::C64K_B);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_C"), C64Keys::C64K_C);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_D"), C64Keys::C64K_D);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_E"), C64Keys::C64K_E);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F"), C64Keys::C64K_F);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_G"), C64Keys::C64K_G);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_H"), C64Keys::C64K_H);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_I"), C64Keys::C64K_I);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_J"), C64Keys::C64K_J);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_K"), C64Keys::C64K_K);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_L"), C64Keys::C64K_L);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_M"), C64Keys::C64K_M);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_N"), C64Keys::C64K_N);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_O"), C64Keys::C64K_O);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_P"), C64Keys::C64K_P);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Q"), C64Keys::C64K_Q);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_R"), C64Keys::C64K_R);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_S"), C64Keys::C64K_S);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_T"), C64Keys::C64K_T);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_U"), C64Keys::C64K_U);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_V"), C64Keys::C64K_V);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_W"), C64Keys::C64K_W);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_X"), C64Keys::C64K_X);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Y"), C64Keys::C64K_Y);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Z"), C64Keys::C64K_Z);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_PLUS"), C64Keys::C64K_PLUS);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_MINUS"), C64Keys::C64K_MINUS);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ASTERISK"), C64Keys::C64K_ASTERISK);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SLASH"), C64Keys::C64K_SLASH);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COMMA"), C64Keys::C64K_COMMA);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_DOT"), C64Keys::C64K_DOT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ARROWLEFT"), C64Keys::C64K_ARROWLEFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COLON"), C64Keys::C64K_COLON);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SEMICOLON"), C64Keys::C64K_SEMICOLON);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CONTROL"), C64Keys::C64K_CONTROL);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_STOP"), C64Keys::C64K_STOP);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COMMODORE"), C64Keys::C64K_COMMODORE);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_LEFTSHIFT"), C64Keys::C64K_LEFTSHIFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RIGHTSHIFT"), C64Keys::C64K_RIGHTSHIFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RESTORE"), C64Keys::C64K_RESTORE);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_HOME"), C64Keys::C64K_HOME);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_DEL"), C64Keys::C64K_DEL);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RETURN"), C64Keys::C64K_RETURN);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ARROWUP"), C64Keys::C64K_ARROWUP);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_POUND"), C64Keys::C64K_POUND);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_EQUAL"), C64Keys::C64K_EQUAL);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORDOWN"), C64Keys::C64K_CURSORDOWN);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORRIGHT"), C64Keys::C64K_CURSORRIGHT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORUP"), C64Keys::C64K_CURSORUP);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORLEFT"), C64Keys::C64K_CURSORLEFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SPACE"), C64Keys::C64K_SPACE);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_AT"), C64Keys::C64K_AT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F1"), C64Keys::C64K_F1);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F2"), C64Keys::C64K_F2);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F3"), C64Keys::C64K_F3);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F4"), C64Keys::C64K_F4);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F5"), C64Keys::C64K_F5);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F6"), C64Keys::C64K_F6);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F7"), C64Keys::C64K_F7);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F8"), C64Keys::C64K_F8);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1FIRE"), C64Keys::C64K_JOY1FIRE);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1UP"), C64Keys::C64K_JOY1UP);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1DOWN"), C64Keys::C64K_JOY1DOWN);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1LEFT"), C64Keys::C64K_JOY1LEFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1RIGHT"), C64Keys::C64K_JOY1RIGHT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1FIRE2"), C64Keys::C64K_JOY1FIRE2);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2FIRE"), C64Keys::C64K_JOY2FIRE);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2UP"), C64Keys::C64K_JOY2UP);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2DOWN"), C64Keys::C64K_JOY2DOWN);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2LEFT"), C64Keys::C64K_JOY2LEFT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2RIGHT"), C64Keys::C64K_JOY2RIGHT);
		ReadRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2FIRE2"), C64Keys::C64K_JOY2FIRE2);
		RegCloseKey(hKey1);
	}

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("D1541_Emulation"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bD1541_Emulation_Enable = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SID_Emulation"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSID_Emulation_Enable = dwValue != 0;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("LimitSpeed"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bLimitSpeed = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("ShowSpeed"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bShowSpeed = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SkipAltFrames"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSkipFrames = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SIDSampleMode"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSIDResampleMode = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("WindowedLockAspectRatio"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			this->m_bWindowedLockAspectRatio = dwValue != 0;
		}
		else
		{
			this->m_bWindowedLockAspectRatio = false;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SIDStereo"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSIDStereo = dwValue != 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SyncMode1"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_syncModeFullscreen = (HCFG::FULLSCREENSYNCMODE) dwValue;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SyncMode2"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_syncModeWindowed = (HCFG::FULLSCREENSYNCMODE) dwValue;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SwapJoysticks"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSwapJoysticks = dwValue != 0;
		}
		else
		{
			m_bSwapJoysticks = false;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("CPUFriendly"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bCPUFriendly = dwValue != 0;
		}
		else
		{
			m_bCPUFriendly = true;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("AudioClockSync"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bAudioClockSync = dwValue != 0;
		}
		else
		{
			m_bAudioClockSync = true;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("SIDDigiBoost"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bSidDigiBoost = dwValue != 0;
		}
		else
		{
			m_bSidDigiBoost = false;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenAdapterIsDefault"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenAdapterIsDefault = dwValue != 0;
		}
		else
		{
			m_fullscreenAdapterIsDefault = true;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenAdapterNumber"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenAdapterNumber = dwValue;
		}
		else
		{
			m_fullscreenAdapterNumber = 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenOutputNumber"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenOutputNumber = dwValue;
		}
		else
		{
			m_fullscreenOutputNumber = 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenWidth"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenWidth = dwValue;
		}
		else
		{
			m_fullscreenWidth = 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenHeight"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenHeight = dwValue;
		}
		else
		{
			m_fullscreenHeight = 0;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenRefreshNumerator"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenRefreshNumerator = dwValue;
		}
		else
		{
			m_fullscreenRefreshNumerator = 0;
		}

		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenRefreshDenominator"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenRefreshDenominator = dwValue;
		}
		else
		{
			m_fullscreenRefreshDenominator = 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenFormat"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenFormat = dwValue;
		}
		else
		{
			m_fullscreenFormat = 0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenDxGiModeScaling"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenDxGiModeScaling = (DXGI_MODE_SCALING) dwValue;
		}
		else
		{
			m_fullscreenDxGiModeScaling = (DXGI_MODE_SCALING)0;
		}

		if ((unsigned int)m_fullscreenDxGiModeScaling > (unsigned int)DXGI_MODE_SCALING::DXGI_MODE_SCALING_STRETCHED)
		{
			m_fullscreenDxGiModeScaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenDxGiModeScanlineOrdering"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenDxGiModeScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER) dwValue;
		}
		else
		{
			m_fullscreenDxGiModeScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)0;
		}

		if ((unsigned int)m_fullscreenDxGiModeScanlineOrdering > (unsigned int)DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST)
		{
			m_fullscreenDxGiModeScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FullscreenStretch"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH) dwValue;
		}
		else
		{
			m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH)0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("BlitFilter"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_blitFilter = (HCFG::EMUWINDOWFILTER)dwValue;
		}
		else
		{
			m_blitFilter = (HCFG::EMUWINDOWFILTER)0;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("BorderSize"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_borderSize = (HCFG::EMUBORDERSIZE)dwValue;
		}
		else
		{
			m_borderSize = HCFG::EMUBORDER_TV;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("ShowFloppyLed"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bShowFloppyLed = dwValue != 0;
		}
		else
		{
			m_bShowFloppyLed = true;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("FPS"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_fps = (HCFG::EMUFPS)dwValue;
		}
		else
		{
			m_fps = HCFG::EMUFPS_50;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("TrackZeroSensor"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_TrackZeroSensorStyle = (HCFG::ETRACKZEROSENSORSTYLE)dwValue;
		}
		else
		{
			m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("CIAMode"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_CIAMode = (HCFG::CIAMODE)dwValue;
		}
		else
		{
			m_CIAMode = HCFG::CM_CIA6526A;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("CIATimerBbug"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			this->m_bTimerBbug = dwValue != 0;
		}
		else
		{
			this->m_bTimerBbug = false;
		}

		dwValue = 0;
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

		for (i = 0; i < _countof(sidAddressName); i++)
		{
			dwValue = 0;
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

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("DiskThreadEnable"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bD1541_Thread_Enable = dwValue != 0;
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

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("AllowOpposingJoystick"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bAllowOpposingJoystick = dwValue != 0;
		}
		else
		{
			m_bAllowOpposingJoystick = false;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("DisableDwmFullscreen"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bDisableDwmFullscreen = dwValue != 0;
		}
		else
		{
			m_bDisableDwmFullscreen = false;
		}

		lRetCode = RegReadDWordOrStr(hKey1, TEXT("EnableImGuiWindowed"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_bEnableImGuiWindowed = dwValue != 0;
		}
		else
		{
			m_bEnableImGuiWindowed = true;
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

			tempLenValue = sizeof(dwValue);
			type = REG_DWORD;
			lRetCode = RegQueryValueEx(hKey1, colorregkeyname.c_str(), NULL, &type, (PBYTE) &dwValue, &tempLenValue);
			if (lRetCode == ERROR_SUCCESS && tempLenValue == sizeof(DWORD))
			{
				this->m_colour_palette[i] = dwValue;
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
	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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

	if (joyIndex >= _countof(JoyKeyName::Name))
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


HRESULT CConfig::WriteRegKeyboardItem(HKEY hkey, LPCTSTR keyname, unsigned char keyvalue) noexcept
{
	DWORD dwValue = m_KeyMap[keyvalue];
	LSTATUS r = RegSetValueEx(hkey, keyname, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
	if (r == ERROR_SUCCESS)
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT CConfig::ReadRegKeyboardItem(HKEY hkey, LPCTSTR keyname, unsigned char keyvalue) noexcept
{
	DWORD dwValue = 0;
	LSTATUS r = RegReadDWordOrStr(hkey, keyname, &dwValue);
	if (r == ERROR_SUCCESS)
	{
		m_KeyMap[keyvalue] = dwValue & 0xff;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT CConfig::SaveWindowSetting(HWND hWnd)
{
HKEY  hKey1; 
DWORD  dwDisposition; 
LONG   lRetCode; 
WINDOWPLACEMENT wp;
DWORD dwValue;

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
	
	dwValue = (DWORD)pt_winpos.x;
	RegSetValueEx(hKey1, TEXT("MainWinPosX"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = (DWORD)pt_winpos.y;
	RegSetValueEx(hKey1, TEXT("MainWinPosY"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	int w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	if (w < 0)
	{
		w = 0;
	}

	int h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	if (h < 0)
	{
		h = 0;
	}

	dwValue = (DWORD)w;
	RegSetValueEx(hKey1, TEXT("MainWinWidth"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
	
	dwValue = (DWORD)h;
	RegSetValueEx(hKey1, TEXT("MainWinHeight"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
	
	RegCloseKey(hKey1);
	return S_OK;
}

HRESULT CConfig::SaveMDIWindowSetting(HWND hWnd)
{
	HKEY  hKey1; 
	DWORD  dwDisposition; 
	LONG   lRetCode; 
	WINDOWPLACEMENT wp;
	DWORD dwValue;

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

	dwValue = (DWORD)pt_mdidebuggerwinpos.x;
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosX"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = (DWORD)pt_mdidebuggerwinpos.y;
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosY"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = (DWORD)sz_mdidebuggerwinsize.cx;
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerWidth"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = (DWORD)sz_mdidebuggerwinsize.cy;
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerHeight"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	RegCloseKey(hKey1);
	return S_OK;
}

HRESULT CConfig::LoadWindowSetting(POINT& pos, int& winWidth, int& winHeight)
{
	DWORD dwValue;
	HKEY  hKey1; 
	LONG   lRetCode; 
	const int max_width = GetSystemMetrics(SM_CXMAXTRACK);
	const int max_height = GetSystemMetrics(SM_CYMAXTRACK);
	const int min_width = GetSystemMetrics(SM_CXMINTRACK);
	const int min_height = GetSystemMetrics(SM_CYMINTRACK);
	POINT _pos = {0, 0};
	int w = min_width;
	int h = min_width;
	bool ok = false;
	int v;
	do
	{
		lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
			TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
			0, KEY_READ,
			&hKey1);	
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MainWinPosX"), &dwValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		_pos.x = dwValue;		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MainWinPosY"), &dwValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		_pos.y = dwValue;
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MainWinWidth"), &dwValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		v = dwValue;
		w = v;
		if (w < min_width)
		{
			w = min_width;
		}
		else if (w > max_width)
		{
			w = max_width;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MainWinHeight"), &dwValue);
		if (lRetCode != ERROR_SUCCESS)
		{
			break;
		}

		v = dwValue;
		h = v;
		if (h < min_height)
		{
			h = min_height;
		}
		else if (h > max_height)
		{
			h = max_height;
		}

		ok = true;
	} while(false);
	if (ok)
	{
		pos = _pos;
		winWidth = w;
		winHeight = h;
	}

	return ok ? S_OK : E_FAIL;
}

HRESULT CConfig::LoadMDIWindowSetting(POINT& pos, SIZE& size)
{
HKEY  hKey1; 
LONG   lRetCode; 
DWORD dwValue;

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
	int v;
	pos.x = left;
	pos.y = top;
	size.cx = 0;
	size.cy = 0;
	if (lRetCode == ERROR_SUCCESS)
	{
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MDIWinDebuggerPosX"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			v = dwValue;
			if (v > max_x)
			{
				v = max_x;
			}
			else if (v < left)
			{
				v = left;
			}

			pos.x = v;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MDIWinDebuggerPosY"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			v = dwValue;
			if (v > max_y)
			{
				v = max_y;
			}
			else if (v < top)
			{
				v = top;
			}

			pos.y = v;
		}

		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MDIWinDebuggerWidth"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{			
			v = dwValue;
			if (v > max_x)
			{
				v = max_x;
			}
			else if (v < min_x)
			{
				v = min_x;
			}

			size.cx = v;
		}
		
		dwValue = 0;
		lRetCode = RegReadDWordOrStr(hKey1, TEXT("MDIWinDebuggerHeight"), &dwValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			v = dwValue;
			if (v > max_y)
			{
				v = max_y;
			}
			else if (v < min_y)
			{
				v = min_y;
			}

			size.cy = v;
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
	HKEY  hKey1;
	DWORD  dwDisposition;
	DWORD  dwValue;
	LONG   lRetCode;
	int i;

	lRetCode = RegCreateKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Keyboard"),
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
		NULL, &hKey1,
		&dwDisposition);

	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}

	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_0"), C64Keys::C64K_0);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_1"), C64Keys::C64K_1);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_2"), C64Keys::C64K_2);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_3"), C64Keys::C64K_3);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_4"), C64Keys::C64K_4);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_5"), C64Keys::C64K_5);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_6"), C64Keys::C64K_6);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_7"), C64Keys::C64K_7);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_8"), C64Keys::C64K_8);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_9"), C64Keys::C64K_9);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_A"), C64Keys::C64K_A);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_B"), C64Keys::C64K_B);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_C"), C64Keys::C64K_C);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_D"), C64Keys::C64K_D);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_E"), C64Keys::C64K_E);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F"), C64Keys::C64K_F);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_G"), C64Keys::C64K_G);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_H"), C64Keys::C64K_H);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_I"), C64Keys::C64K_I);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_J"), C64Keys::C64K_J);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_K"), C64Keys::C64K_K);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_L"), C64Keys::C64K_L);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_M"), C64Keys::C64K_M);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_N"), C64Keys::C64K_N);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_O"), C64Keys::C64K_O);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_P"), C64Keys::C64K_P);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Q"), C64Keys::C64K_Q);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_R"), C64Keys::C64K_R);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_S"), C64Keys::C64K_S);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_T"), C64Keys::C64K_T);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_U"), C64Keys::C64K_U);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_V"), C64Keys::C64K_V);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_W"), C64Keys::C64K_W);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_X"), C64Keys::C64K_X);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Y"), C64Keys::C64K_Y);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_Z"), C64Keys::C64K_Z);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_PLUS"), C64Keys::C64K_PLUS);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_MINUS"), C64Keys::C64K_MINUS);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ASTERISK"), C64Keys::C64K_ASTERISK);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SLASH"), C64Keys::C64K_SLASH);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COMMA"), C64Keys::C64K_COMMA);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_DOT"), C64Keys::C64K_DOT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ARROWLEFT"), C64Keys::C64K_ARROWLEFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COLON"), C64Keys::C64K_COLON);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SEMICOLON"), C64Keys::C64K_SEMICOLON);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CONTROL"), C64Keys::C64K_CONTROL);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_STOP"), C64Keys::C64K_STOP);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_COMMODORE"), C64Keys::C64K_COMMODORE);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_LEFTSHIFT"), C64Keys::C64K_LEFTSHIFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RIGHTSHIFT"), C64Keys::C64K_RIGHTSHIFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RESTORE"), C64Keys::C64K_RESTORE);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_HOME"), C64Keys::C64K_HOME);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_DEL"), C64Keys::C64K_DEL);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_RETURN"), C64Keys::C64K_RETURN);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_ARROWUP"), C64Keys::C64K_ARROWUP);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_POUND"), C64Keys::C64K_POUND);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_EQUAL"), C64Keys::C64K_EQUAL);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORDOWN"), C64Keys::C64K_CURSORDOWN);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORRIGHT"), C64Keys::C64K_CURSORRIGHT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORUP"), C64Keys::C64K_CURSORUP);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_CURSORLEFT"), C64Keys::C64K_CURSORLEFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_SPACE"), C64Keys::C64K_SPACE);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_AT"), C64Keys::C64K_AT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F1"), C64Keys::C64K_F1);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F2"), C64Keys::C64K_F2);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F3"), C64Keys::C64K_F3);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F4"), C64Keys::C64K_F4);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F5"), C64Keys::C64K_F5);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F6"), C64Keys::C64K_F6);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F7"), C64Keys::C64K_F7);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_F8"), C64Keys::C64K_F8);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1FIRE"), C64Keys::C64K_JOY1FIRE);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1UP"), C64Keys::C64K_JOY1UP);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1DOWN"), C64Keys::C64K_JOY1DOWN);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1LEFT"), C64Keys::C64K_JOY1LEFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1RIGHT"), C64Keys::C64K_JOY1RIGHT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY1FIRE2"), C64Keys::C64K_JOY1FIRE2);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2FIRE"), C64Keys::C64K_JOY2FIRE);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2UP"), C64Keys::C64K_JOY2UP);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2DOWN"), C64Keys::C64K_JOY2DOWN);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2LEFT"), C64Keys::C64K_JOY2LEFT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2RIGHT"), C64Keys::C64K_JOY2RIGHT);
	WriteRegKeyboardItem(hKey1, TEXT("C64Keys::C64K_JOY2FIRE2"), C64Keys::C64K_JOY2FIRE2);
	RegCloseKey(hKey1);
	lRetCode = RegCreateKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\General"),
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
		NULL, &hKey1,
		&dwDisposition);
	if (lRetCode != ERROR_SUCCESS)
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}

	dwValue = m_bD1541_Emulation_Enable ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("D1541_Emulation"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSID_Emulation_Enable ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SID_Emulation"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bShowSpeed ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("ShowSpeed"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bLimitSpeed ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("LimitSpeed"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSkipFrames ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SkipAltFrames"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSIDResampleMode ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SIDSampleMode"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_syncModeFullscreen;
	RegSetValueEx(hKey1, TEXT("SyncMode1"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_syncModeWindowed;
	RegSetValueEx(hKey1, TEXT("SyncMode2"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSwapJoysticks ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SwapJoysticks"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bCPUFriendly ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("CPUFriendly"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bAudioClockSync ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("AudioClockSync"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSidDigiBoost ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SIDDigiBoost"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	//G::SaveClsidToRegValue(hKey1, TEXT("FullscreenAdapterId"), &m_fullscreenAdapterId);

	dwValue = this->m_fullscreenAdapterIsDefault ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("FullscreenAdapterIsDefault"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_fullscreenAdapterNumber;
	RegSetValueEx(hKey1, TEXT("FullscreenAdapterNumber"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_fullscreenOutputNumber;
	RegSetValueEx(hKey1, TEXT("FullscreenOutputNumber"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_fullscreenWidth;
	RegSetValueEx(hKey1, TEXT("FullscreenWidth"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_fullscreenHeight;
	RegSetValueEx(hKey1, TEXT("FullscreenHeight"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = (DWORD)m_fullscreenRefreshNumerator;
	RegSetValueEx(hKey1, TEXT("FullscreenRefreshNumerator"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = (DWORD)m_fullscreenRefreshDenominator;
	RegSetValueEx(hKey1, TEXT("FullscreenRefreshDenominator"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_fullscreenFormat;
	RegSetValueEx(hKey1, TEXT("FullscreenFormat"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = (DWORD)m_fullscreenDxGiModeScaling;
	RegSetValueEx(hKey1, TEXT("FullscreenDxGiModeScaling"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = (DWORD)m_fullscreenDxGiModeScanlineOrdering;
	RegSetValueEx(hKey1, TEXT("FullscreenDxGiModeScanlineOrdering"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_fullscreenStretch;
	RegSetValueEx(hKey1, TEXT("FullscreenStretch"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_blitFilter;
	RegSetValueEx(hKey1, TEXT("BlitFilter"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_borderSize;
	RegSetValueEx(hKey1, TEXT("BorderSize"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_bShowFloppyLed ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("ShowFloppyLed"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_fps;
	RegSetValueEx(hKey1, TEXT("FPS"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_TrackZeroSensorStyle;
	RegSetValueEx(hKey1, TEXT("TrackZeroSensor"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_CIAMode;
	RegSetValueEx(hKey1, TEXT("CIAMode"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_bTimerBbug ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("CIATimerBbug"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = (DWORD)this->m_numberOfExtraSIDs;
	if (dwValue >= 8)
	{
		dwValue = 0;
	}

	RegSetValueEx(hKey1, TEXT("NumberOfExtraSidChips"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = this->m_bWindowedLockAspectRatio ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("WindowedLockAspectRatio"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

	dwValue = m_bSIDStereo ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("SIDStereo"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

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

	for (i = 0; i < _countof(sidAddressName); i++)
	{
		dwValue = *sidAddressValue[i];
		RegSetValueEx(hKey1, sidAddressName[i], 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
	}

	dwValue = m_bD1541_Thread_Enable ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("DiskThreadEnable"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_bAllowOpposingJoystick ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("AllowOpposingJoystick"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_bDisableDwmFullscreen ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("DisableDwmFullscreen"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = m_bEnableImGuiWindowed ? 1 : 0;
	RegSetValueEx(hKey1, TEXT("EnableImGuiWindowed"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	dwValue = 1;
	RegSetValueEx(hKey1, TEXT("PrefsSaved"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

	RegCloseKey(hKey1);

	lRetCode = RegCreateKeyEx(HKEY_CURRENT_USER,
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
		RegSetValueEx(hKey1, colorregkeyname.c_str(), NULL, REG_DWORD, (PBYTE)&rgbcolor, sizeof(DWORD));
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

void CConfig::SetPalettePepto() noexcept
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
	m_bEnableImGuiWindowed = true;
	m_fullscreenAdapterIsDefault = true;
	m_fullscreenAdapterNumber = 0;
	m_fullscreenOutputNumber = 0;
	m_fullscreenWidth = 0;
	m_fullscreenHeight = 0;
	m_fullscreenRefreshNumerator = 0;
	m_fullscreenRefreshDenominator = 0;
	m_fullscreenFormat = 0;
	m_fullscreenDxGiModeScaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	m_fullscreenDxGiModeScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
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
	m_bWindowedLockAspectRatio = false;
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
			tempLenValue = sizeof(szValue);
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

joyconfig::joyconfig() noexcept
{	
	LoadDefault();
	joyNotAcquiredClock = 0;
}

void joyconfig::defaultClearAxisDirection(GameControllerItem::ControllerAxisDirection offsets[], GameControllerItem::ControllerAxisDirection axisDirection, unsigned int count) noexcept
{
	if (offsets != NULL)
	{
		for(unsigned int i = 0; i < count; i++)
		{
			offsets[i] = axisDirection;
		}
	}
}

void joyconfig::LoadDefault() noexcept
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
	ZeroMemory(&povAvailable, sizeof(povAvailable));
	ZeroMemory(&povIndex, sizeof(povIndex));
	ZeroMemory(&keyNButtonCount, sizeof(keyNButtonCount));
	ZeroMemory(&keyNButtonOffsets, sizeof(keyNButtonOffsets));
	ZeroMemory(&keyNAxisCount, sizeof(keyNAxisCount));
	ZeroMemory(&keyNAxisOffsets, sizeof(keyNAxisOffsets));
	ZeroMemory(&keyNAxisDirection, sizeof(keyNAxisDirection));
	ZeroMemory(&keyNPovCount, sizeof(keyNPovCount));
	ZeroMemory(&keyNPovOffsets, sizeof(keyNPovOffsets));
	ZeroMemory(&keyNPovDirection, sizeof(keyNPovDirection));
	ZeroMemory(&axes, sizeof(axes));
	ZeroMemory(&pov, sizeof(pov));

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
	for (i = 0; i < MAXKEYMAPS; i++)
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

