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

	m_joy1config.bEnabled = false;
	m_joy2config.bEnabled = false;

	m_joy1config.bValid = false;
	m_joy2config.bValid = false;

	m_joy1config.bXReverse = false;
	m_joy2config.bYReverse = false;

	m_bSwapJoysticks = false;
	m_bCPUFriendly = true;
	m_bAudioClockSync = true;
	m_bSidDigiBoost = true;

	m_bMaxSpeed = false;
	m_bSkipFrames = false;
	m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
	m_CIAMode = HCFG::CM_CIA6526A;
	m_bTimerBbug = false;
	SetPalettePepto();
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
DWORD type,dw;
const int maxbutton = 31;
int i;
	
	const ULONG lenValue = sizeof(szValue);
	ZeroMemory(&m_KeyMap[0], sizeof(m_KeyMap));
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
			LoadDefaultSetting();
			return S_OK;
		}
	}
	else
	{
		LoadDefaultSetting();
		return S_OK;
	}

	//Patch soft keys to work after version v1.0.7.2
	m_KeyMap[C64K_CURSORUP]= DIK_UP;
	m_KeyMap[C64K_CURSORLEFT]= DIK_LEFT;

	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Keyboard"),
		0, KEY_READ,
		&hKey1);	
	
	if (lRetCode == ERROR_SUCCESS)
	{
		readregkeyboarditem(C64K_0);
		readregkeyboarditem(C64K_1);
		readregkeyboarditem(C64K_2);
		readregkeyboarditem(C64K_3);
		readregkeyboarditem(C64K_4);
		readregkeyboarditem(C64K_5);
		readregkeyboarditem(C64K_6);
		readregkeyboarditem(C64K_7);
		readregkeyboarditem(C64K_8);
		readregkeyboarditem(C64K_9);
		readregkeyboarditem(C64K_A);
		readregkeyboarditem(C64K_B);
		readregkeyboarditem(C64K_C);
		readregkeyboarditem(C64K_D);
		readregkeyboarditem(C64K_E);
		readregkeyboarditem(C64K_F);
		readregkeyboarditem(C64K_G);
		readregkeyboarditem(C64K_H);
		readregkeyboarditem(C64K_I);
		readregkeyboarditem(C64K_J);
		readregkeyboarditem(C64K_K);
		readregkeyboarditem(C64K_L);
		readregkeyboarditem(C64K_M);
		readregkeyboarditem(C64K_N);
		readregkeyboarditem(C64K_O);
		readregkeyboarditem(C64K_P);
		readregkeyboarditem(C64K_Q);
		readregkeyboarditem(C64K_R);
		readregkeyboarditem(C64K_S);
		readregkeyboarditem(C64K_T);
		readregkeyboarditem(C64K_U);
		readregkeyboarditem(C64K_V);
		readregkeyboarditem(C64K_W);
		readregkeyboarditem(C64K_X);
		readregkeyboarditem(C64K_Y);
		readregkeyboarditem(C64K_Z);
		readregkeyboarditem(C64K_PLUS);
		readregkeyboarditem(C64K_MINUS);
		readregkeyboarditem(C64K_ASTERISK);
		readregkeyboarditem(C64K_SLASH);
		readregkeyboarditem(C64K_COMMA);
		readregkeyboarditem(C64K_DOT);
		readregkeyboarditem(C64K_ARROWLEFT);
		readregkeyboarditem(C64K_COLON);
		readregkeyboarditem(C64K_SEMICOLON);
		readregkeyboarditem(C64K_CONTROL);
		readregkeyboarditem(C64K_STOP);
		readregkeyboarditem(C64K_COMMODORE);
		readregkeyboarditem(C64K_LEFTSHIFT);
		readregkeyboarditem(C64K_RIGHTSHIFT);
		readregkeyboarditem(C64K_RESTORE);		
		readregkeyboarditem(C64K_HOME);
		readregkeyboarditem(C64K_DEL);
		readregkeyboarditem(C64K_RETURN);
		readregkeyboarditem(C64K_ARROWUP);
		readregkeyboarditem(C64K_POUND);
		readregkeyboarditem(C64K_EQUAL);
		readregkeyboarditem(C64K_CURSORDOWN);
		readregkeyboarditem(C64K_CURSORRIGHT);
		readregkeyboarditem(C64K_CURSORUP);
		readregkeyboarditem(C64K_CURSORLEFT);
		readregkeyboarditem(C64K_SPACE);
		readregkeyboarditem(C64K_AT);
		readregkeyboarditem(C64K_F1);
		readregkeyboarditem(C64K_F2);
		readregkeyboarditem(C64K_F3);
		readregkeyboarditem(C64K_F4);
		readregkeyboarditem(C64K_F5);
		readregkeyboarditem(C64K_F6);
		readregkeyboarditem(C64K_F7);
		readregkeyboarditem(C64K_F8);

		readregkeyboarditem(C64K_JOY1FIRE);
		readregkeyboarditem(C64K_JOY1UP);
		readregkeyboarditem(C64K_JOY1DOWN);
		readregkeyboarditem(C64K_JOY1LEFT);
		readregkeyboarditem(C64K_JOY1RIGHT);
		readregkeyboarditem(C64K_JOY2FIRE);
		readregkeyboarditem(C64K_JOY2UP);
		readregkeyboarditem(C64K_JOY2DOWN);
		readregkeyboarditem(C64K_JOY2LEFT);
		readregkeyboarditem(C64K_JOY2RIGHT);

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
		lRetCode = RegReadStr(hKey1, TEXT("SyncMode1"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_syncMode = (HCFG::FULLSCREENSYNCMODE) _ttol(szValue);
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
			m_bSidDigiBoost = true;
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
	}
	
	m_joy1config.bValid = false;
	m_joy2config.bValid = false;
	lRetCode = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Hoxs64\\1.0\\Joystick"),
		0, KEY_READ,
		&hKey1);
	if (lRetCode == ERROR_SUCCESS)
	{
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy1Valid"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy1config.bValid = _ttol(szValue) != 0;
		}
		else
		{
			m_joy1config.bValid = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy2Valid"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy2config.bValid = _ttol(szValue) != 0;
		}
		else
		{
			m_joy2config.bValid = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy1Enabled"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy1config.bEnabled = _ttol(szValue) != 0;
		}
		else
		{
			m_joy1config.bEnabled = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy2Enabled"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy2config.bEnabled = _ttol(szValue) != 0;
		}
		else
		{
			m_joy2config.bEnabled = false;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy1POV"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy1config.bPovEnabled = _ttol(szValue) != 0;
		}
		else
		{
			m_joy1config.bPovEnabled = true;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy2POV"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy2config.bPovEnabled = _ttol(szValue) != 0;
		}
		else
		{
			m_joy2config.bPovEnabled = true;
		}

		m_joy1config.bXReverse = false;
		m_joy1config.bYReverse = false;
		m_joy2config.bXReverse = false;
		m_joy2config.bYReverse = false;
		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy1RevX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy1config.bXReverse = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy1RevY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy1config.bYReverse = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy2RevX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy2config.bXReverse = _ttol(szValue) != 0;
		}

		tempLenValue = lenValue;
		lRetCode = RegReadStr(hKey1, TEXT("Joy2RevY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
		if (lRetCode == ERROR_SUCCESS)
		{
			m_joy2config.bYReverse = _ttol(szValue) != 0;
		}

		m_joy1config.dwOfs_X = DIJOFS_X;
		m_joy1config.dwOfs_Y = DIJOFS_Y;
		m_joy1config.dwOfs_firebutton = DIJOFS_BUTTON0;
		m_joy2config.dwOfs_X = DIJOFS_X;
		m_joy2config.dwOfs_Y = DIJOFS_Y;
		m_joy2config.dwOfs_firebutton = DIJOFS_BUTTON0;

		if (m_joy1config.bValid)
		{
			if (SUCCEEDED(G::GetClsidFromRegValue(hKey1, TEXT("Joy1GUID"), &m_joy1config.joystickID)))
			{
				m_joy1config.bValid = true;

				//Joystick 1 X Axis
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy1AxisX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)) && dw>= DIJOFS_BUTTON0)
					{
						m_joy1config.dwOfs_X = dw;
					}
				}
					
				//Joystick 1 Y Axis
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy1AxisY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)))
					{
						m_joy1config.dwOfs_Y = dw;
					}
				}					

				//Joystick 1 Fire
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy1Fire"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)))
					{
						m_joy1config.dwOfs_firebutton = dw;
					}
				}
			}
			else
			{
				m_joy1config.bValid = false;
			}
		}

		if (m_joy2config.bValid)
		{
			if (SUCCEEDED(G::GetClsidFromRegValue(hKey1, TEXT("Joy2GUID"), &m_joy2config.joystickID)))
			{

				m_joy2config.bValid = true;

				//Joystick 2 X Axis
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy2AxisX"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)))
					{
						m_joy2config.dwOfs_X = dw;
					}
				}

				//Joystick 2 Y Axis
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy2AxisY"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)))
					{
						m_joy2config.dwOfs_Y = dw;
					}
				}

				//Joystick 2 Fire
				tempLenValue = lenValue;
				lRetCode = RegReadStr(hKey1, TEXT("Joy2Fire"), NULL, NULL, (PBYTE) &szValue[0], &tempLenValue);
				if (lRetCode == ERROR_SUCCESS)
				{
					dw = _ttol(szValue);
					if (dw < (sizeof(DIJOYSTATE) - sizeof(DWORD)) && dw>= DIJOFS_BUTTON0)
						m_joy2config.dwOfs_firebutton = dw;
				}
			}
		}
		else
		{
			m_joy2config.bValid = false;
		}
		
		RegCloseKey(hKey1);
	}
	return S_OK;
}

#define writeregkeyboarditem(n)	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_KeyMap[n]));\
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

	_sntprintf_s(szValue, _TRUNCATE, TEXT("%d"), w);
	RegSetValueEx(hKey1, TEXT("MainWinWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	_sntprintf_s(szValue, _TRUNCATE, TEXT("%d"), h);
	RegSetValueEx(hKey1, TEXT("MainWinHeight"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bDoubleSizedWindow ? 1: 0));
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

	wsprintf(szValue, TEXT("%ld"), (DWORD) (pt_mdidebuggerwinpos.x));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosX"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (DWORD) (pt_mdidebuggerwinpos.y));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerPosY"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (DWORD) (sz_mdidebuggerwinsize.cx));
	RegSetValueEx(hKey1, TEXT("MDIWinDebuggerWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%ld"), (DWORD) (sz_mdidebuggerwinsize.cy));
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
bool bGuidOK;
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

	writeregkeyboarditem(C64K_0);
	writeregkeyboarditem(C64K_1);
	writeregkeyboarditem(C64K_2);
	writeregkeyboarditem(C64K_3);
	writeregkeyboarditem(C64K_4);
	writeregkeyboarditem(C64K_5);
	writeregkeyboarditem(C64K_6);
	writeregkeyboarditem(C64K_7);
	writeregkeyboarditem(C64K_8);
	writeregkeyboarditem(C64K_9);
	writeregkeyboarditem(C64K_A);
	writeregkeyboarditem(C64K_B);
	writeregkeyboarditem(C64K_C);
	writeregkeyboarditem(C64K_D);
	writeregkeyboarditem(C64K_E);
	writeregkeyboarditem(C64K_F);
	writeregkeyboarditem(C64K_G);
	writeregkeyboarditem(C64K_H);
	writeregkeyboarditem(C64K_I);
	writeregkeyboarditem(C64K_J);
	writeregkeyboarditem(C64K_K);
	writeregkeyboarditem(C64K_L);
	writeregkeyboarditem(C64K_M);
	writeregkeyboarditem(C64K_N);
	writeregkeyboarditem(C64K_O);
	writeregkeyboarditem(C64K_P);
	writeregkeyboarditem(C64K_Q);
	writeregkeyboarditem(C64K_R);
	writeregkeyboarditem(C64K_S);
	writeregkeyboarditem(C64K_T);
	writeregkeyboarditem(C64K_U);
	writeregkeyboarditem(C64K_V);
	writeregkeyboarditem(C64K_W);
	writeregkeyboarditem(C64K_X);
	writeregkeyboarditem(C64K_Y);
	writeregkeyboarditem(C64K_Z);
	writeregkeyboarditem(C64K_PLUS);
	writeregkeyboarditem(C64K_MINUS);
	writeregkeyboarditem(C64K_ASTERISK);
	writeregkeyboarditem(C64K_SLASH);
	writeregkeyboarditem(C64K_COMMA);
	writeregkeyboarditem(C64K_DOT);
	writeregkeyboarditem(C64K_ARROWLEFT);
	writeregkeyboarditem(C64K_COLON);
	writeregkeyboarditem(C64K_SEMICOLON);
	writeregkeyboarditem(C64K_CONTROL);
	writeregkeyboarditem(C64K_STOP);
	writeregkeyboarditem(C64K_COMMODORE);
	writeregkeyboarditem(C64K_LEFTSHIFT);
	writeregkeyboarditem(C64K_RIGHTSHIFT);
	writeregkeyboarditem(C64K_RESTORE);
	writeregkeyboarditem(C64K_HOME);
	writeregkeyboarditem(C64K_DEL);
	writeregkeyboarditem(C64K_RETURN);
	writeregkeyboarditem(C64K_ARROWUP);
	writeregkeyboarditem(C64K_POUND);
	writeregkeyboarditem(C64K_EQUAL);
	writeregkeyboarditem(C64K_CURSORDOWN);
	writeregkeyboarditem(C64K_CURSORRIGHT);
	writeregkeyboarditem(C64K_CURSORUP);
	writeregkeyboarditem(C64K_CURSORLEFT);
	writeregkeyboarditem(C64K_SPACE);
	writeregkeyboarditem(C64K_AT);
	writeregkeyboarditem(C64K_F1);
	writeregkeyboarditem(C64K_F2);
	writeregkeyboarditem(C64K_F3);
	writeregkeyboarditem(C64K_F4);
	writeregkeyboarditem(C64K_F5);
	writeregkeyboarditem(C64K_F6);
	writeregkeyboarditem(C64K_F7);
	writeregkeyboarditem(C64K_F8);

	writeregkeyboarditem(C64K_JOY1FIRE);
	writeregkeyboarditem(C64K_JOY1UP);
	writeregkeyboarditem(C64K_JOY1DOWN);
	writeregkeyboarditem(C64K_JOY1LEFT);
	writeregkeyboarditem(C64K_JOY1RIGHT);
	writeregkeyboarditem(C64K_JOY2FIRE);
	writeregkeyboarditem(C64K_JOY2UP);
	writeregkeyboarditem(C64K_JOY2DOWN);
	writeregkeyboarditem(C64K_JOY2LEFT);
	writeregkeyboarditem(C64K_JOY2RIGHT);	
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

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bD1541_Emulation_Enable ? 1: 0));
	RegSetValueEx(hKey1, TEXT("D1541_Emulation"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bSID_Emulation_Enable ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SID_Emulation"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bShowSpeed ? 1: 0));
	RegSetValueEx(hKey1, TEXT("ShowSpeed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bLimitSpeed ? 1: 0));
	RegSetValueEx(hKey1, TEXT("LimitSpeed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bSkipFrames ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SkipAltFrames"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_bSIDResampleMode);
	RegSetValueEx(hKey1, TEXT("SIDSampleMode"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_syncMode);
	RegSetValueEx(hKey1, TEXT("SyncMode1"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bDoubleSizedWindow ? 1: 0));
	RegSetValueEx(hKey1, TEXT("DoubleSizedWindow"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bUseBlitStretch ? 1: 0));
	RegSetValueEx(hKey1, TEXT("UseBlitStretch"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bUseKeymap ? 1: 0));
	RegSetValueEx(hKey1, TEXT("UseKeymap"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bSwapJoysticks ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SwapJoysticks"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bCPUFriendly ? 1: 0));
	RegSetValueEx(hKey1, TEXT("CPUFriendly"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bAudioClockSync ? 1: 0));
	RegSetValueEx(hKey1, TEXT("AudioClockSync"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	
	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bSidDigiBoost ? 1: 0));
	RegSetValueEx(hKey1, TEXT("SIDDigiBoost"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	G::SaveClsidToRegValue(hKey1, TEXT("FullscreenAdapterId"), &m_fullscreenAdapterId);

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenAdapterNumber);
	RegSetValueEx(hKey1, TEXT("FullscreenAdapterNumber"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenWidth);
	RegSetValueEx(hKey1, TEXT("FullscreenWidth"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenHeight);
	RegSetValueEx(hKey1, TEXT("FullscreenHeight"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenRefresh);
	RegSetValueEx(hKey1, TEXT("FullscreenRefresh"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenFormat);
	RegSetValueEx(hKey1, TEXT("FullscreenFormat"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fullscreenStretch);
	RegSetValueEx(hKey1, TEXT("FullscreenStretch"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_blitFilter);
	RegSetValueEx(hKey1, TEXT("BlitFilter"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_borderSize);
	RegSetValueEx(hKey1, TEXT("BorderSize"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bShowFloppyLed ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("ShowFloppyLed"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_fps);
	RegSetValueEx(hKey1, TEXT("FPS"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_TrackZeroSensorStyle));
	RegSetValueEx(hKey1, TEXT("TrackZeroSensor"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_CIAMode));
	RegSetValueEx(hKey1, TEXT("CIAMode"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bTimerBbug ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("CIATimerBbug"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	
	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bD1541_Thread_Enable ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("DiskThreadEnable"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bAllowOpposingJoystick ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("AllowOpposingJoystick"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_bDisableDwmFullscreen ? 1 : 0));
	RegSetValueEx(hKey1, TEXT("DisableDwmFullscreen"), 0, REG_SZ, (LPBYTE) szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

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

	lstrcpy(szValue, TEXT("1"));
	RegSetValueEx(hKey1, TEXT("PrefsSaved"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	RegCloseKey(hKey1);

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

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_joy1config.bEnabled ? 1: 0));
	RegSetValueEx(hKey1, TEXT("Joy1Enabled"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_joy2config.bEnabled ? 1: 0));
	RegSetValueEx(hKey1, TEXT("Joy2Enabled"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_joy1config.bPovEnabled ? 0xffffffff: 0));
	RegSetValueEx(hKey1, TEXT("Joy1POV"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) (m_joy2config.bPovEnabled ? 0xffffffff: 0));
	RegSetValueEx(hKey1, TEXT("Joy2POV"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	bGuidOK = false;
	if (m_joy1config.bValid)
	{
		if (SUCCEEDED(G::SaveClsidToRegValue(hKey1, TEXT("Joy1GUID"), &m_joy1config.joystickID)))
		{
			bGuidOK = true;
		}
	}
	wsprintf(szValue, TEXT("%lu"), (DWORD) ((m_joy1config.bValid && bGuidOK) ? 1: 0));
	RegSetValueEx(hKey1, TEXT("Joy1Valid"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));


	bGuidOK = false;
	if (m_joy2config.bValid)
	{
		if (SUCCEEDED(G::SaveClsidToRegValue(hKey1, TEXT("Joy2GUID"), &m_joy2config.joystickID)))
		{
			bGuidOK = true;
		}
	}
	wsprintf(szValue, TEXT("%lu"), (DWORD) ((m_joy2config.bValid && bGuidOK) ? 1: 0));
	RegSetValueEx(hKey1, TEXT("Joy2Valid"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy1config.dwOfs_X);
	RegSetValueEx(hKey1, TEXT("Joy1AxisX"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy1config.dwOfs_Y);
	RegSetValueEx(hKey1, TEXT("Joy1AxisY"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy2config.dwOfs_X);
	RegSetValueEx(hKey1, TEXT("Joy2AxisX"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy2config.dwOfs_Y);
	RegSetValueEx(hKey1, TEXT("Joy2AxisY"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy1config.bXReverse);
	RegSetValueEx(hKey1, TEXT("Joy1RevX"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy1config.bYReverse);
	RegSetValueEx(hKey1, TEXT("Joy1RevY"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy2config.bXReverse);
	RegSetValueEx(hKey1, TEXT("Joy2RevX"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy2config.bYReverse);
	RegSetValueEx(hKey1, TEXT("Joy2RevY"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

	
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy1config.dwOfs_firebutton);
	RegSetValueEx(hKey1, TEXT("Joy1Fire"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));
	wsprintf(szValue, TEXT("%lu"), (DWORD) m_joy2config.dwOfs_firebutton);
	RegSetValueEx(hKey1, TEXT("Joy2Fire"), 0, REG_SZ, (LPBYTE) &szValue[0], (lstrlen(&szValue[0]) + 1) * sizeof(TCHAR));

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
	m_KeyMap[C64K_PLUS]=	DIK_MINUS;
	m_KeyMap[C64K_MINUS]=	DIK_EQUALS;
	m_KeyMap[C64K_ASTERISK]= DIK_RBRACKET;
	m_KeyMap[C64K_SLASH]=	DIK_SLASH;
	m_KeyMap[C64K_COMMA]=	DIK_COMMA;
	m_KeyMap[C64K_DOT]= DIK_PERIOD;
	m_KeyMap[C64K_ARROWLEFT]=	DIK_GRAVE;
	m_KeyMap[C64K_COLON]=	DIK_SEMICOLON;
	m_KeyMap[C64K_SEMICOLON]=	DIK_APOSTROPHE;
	m_KeyMap[C64K_AT]= DIK_LBRACKET;
	m_KeyMap[C64K_EQUAL]=	DIK_BACKSLASH;
	m_KeyMap[C64K_RESTORE]= DIK_PRIOR;
	m_KeyMap[C64K_HOME]= DIK_HOME;
	m_KeyMap[C64K_ARROWUP]= DIK_DELETE;
	m_KeyMap[C64K_POUND]=	DIK_INSERT;
	m_KeyMap[C64K_CURSORDOWN]= DIK_DOWN;
	m_KeyMap[C64K_CURSORRIGHT]= DIK_RIGHT;
	m_KeyMap[C64K_CURSORUP]= DIK_UP;
	m_KeyMap[C64K_CURSORLEFT]= DIK_LEFT;
	m_KeyMap[C64K_CONTROL]= GetKeyScanCode(VK_TAB);
	m_KeyMap[C64K_LEFTSHIFT]= DIK_LSHIFT;
	m_KeyMap[C64K_RIGHTSHIFT]= DIK_RSHIFT;
	m_KeyMap[C64K_STOP]= GetKeyScanCode(VK_ESCAPE);
	m_KeyMap[C64K_COMMODORE]= DIK_LCONTROL;
	m_KeyMap[C64K_DEL]= GetKeyScanCode(VK_BACK);
	m_KeyMap[C64K_RETURN]= GetKeyScanCode(VK_RETURN);
	m_KeyMap[C64K_SPACE]=	GetKeyScanCode(' ');
	m_KeyMap[C64K_JOY1FIRE]= DIK_NUMPAD0;
	m_KeyMap[C64K_JOY1UP]= DIK_DIVIDE;
	m_KeyMap[C64K_JOY1DOWN]= DIK_NUMPAD5;
	m_KeyMap[C64K_JOY1LEFT]= DIK_NUMPAD7;
	m_KeyMap[C64K_JOY1RIGHT]= DIK_NUMPAD9;
	m_KeyMap[C64K_JOY2FIRE]= DIK_RCONTROL;
	m_KeyMap[C64K_JOY2UP]= DIK_NUMPAD8;
	m_KeyMap[C64K_JOY2DOWN]= DIK_NUMPAD2;
	m_KeyMap[C64K_JOY2LEFT]= DIK_NUMPAD4;
	m_KeyMap[C64K_JOY2RIGHT]= DIK_NUMPAD6;
	m_KeyMap[C64K_0]= GetKeyScanCode('0');
	m_KeyMap[C64K_1]=	GetKeyScanCode('1');
	m_KeyMap[C64K_2]=	GetKeyScanCode('2');
	m_KeyMap[C64K_3]=	GetKeyScanCode('3');
	m_KeyMap[C64K_4]=	GetKeyScanCode('4');
	m_KeyMap[C64K_5]=	GetKeyScanCode('5');
	m_KeyMap[C64K_6]=	GetKeyScanCode('6');
	m_KeyMap[C64K_7]=	GetKeyScanCode('7');
	m_KeyMap[C64K_8]=	GetKeyScanCode('8');
	m_KeyMap[C64K_9]=	GetKeyScanCode('9');
	m_KeyMap[C64K_A]=	GetKeyScanCode('A');
	m_KeyMap[C64K_B]=	GetKeyScanCode('B');
	m_KeyMap[C64K_C]=	GetKeyScanCode('C');
	m_KeyMap[C64K_D]=	GetKeyScanCode('D');
	m_KeyMap[C64K_E]=	GetKeyScanCode('E');
	m_KeyMap[C64K_F]=	GetKeyScanCode('F');
	m_KeyMap[C64K_G]=	GetKeyScanCode('G');
	m_KeyMap[C64K_H]=	GetKeyScanCode('H');
	m_KeyMap[C64K_I]=	GetKeyScanCode('I');
	m_KeyMap[C64K_J]=	GetKeyScanCode('J');
	m_KeyMap[C64K_K]=	GetKeyScanCode('K');
	m_KeyMap[C64K_L]=	GetKeyScanCode('L');
	m_KeyMap[C64K_M]=	GetKeyScanCode('M');
	m_KeyMap[C64K_N]=	GetKeyScanCode('N');
	m_KeyMap[C64K_O]=	GetKeyScanCode('O');
	m_KeyMap[C64K_P]=	GetKeyScanCode('P');
	m_KeyMap[C64K_Q]=	GetKeyScanCode('Q');
	m_KeyMap[C64K_R]=	GetKeyScanCode('R');
	m_KeyMap[C64K_S]=	GetKeyScanCode('S');
	m_KeyMap[C64K_T]=	GetKeyScanCode('T');
	m_KeyMap[C64K_U]=	GetKeyScanCode('U');
	m_KeyMap[C64K_V]=	GetKeyScanCode('V');
	m_KeyMap[C64K_W]=	GetKeyScanCode('W');
	m_KeyMap[C64K_X]=	GetKeyScanCode('X');
	m_KeyMap[C64K_Y]=	GetKeyScanCode('Y');
	m_KeyMap[C64K_Z]=	GetKeyScanCode('Z');
	m_KeyMap[C64K_F1]= GetKeyScanCode(VK_F1);
	m_KeyMap[C64K_F2]= GetKeyScanCode(VK_F2);
	m_KeyMap[C64K_F3]= GetKeyScanCode(VK_F3);
	m_KeyMap[C64K_F4]= GetKeyScanCode(VK_F4);
	m_KeyMap[C64K_F5]= GetKeyScanCode(VK_F5);
	m_KeyMap[C64K_F6]= GetKeyScanCode(VK_F6);
	m_KeyMap[C64K_F7]= GetKeyScanCode(VK_F7);
	m_KeyMap[C64K_F8]= GetKeyScanCode(VK_F8);
	m_bSID_Emulation_Enable = true;
	m_bD1541_Emulation_Enable = true;
	m_bSkipFrames = false;
	m_bShowSpeed = true;
	m_bLimitSpeed = true;
	m_bSIDResampleMode = true;
	m_syncMode = HCFG::FSSM_VBL;
	m_bDoubleSizedWindow = true;
	m_bUseBlitStretch = true;
	m_bUseKeymap = false;
	m_joy1config.bValid = false;
	m_joy1config.bEnabled = false;
	m_joy1config.bPovEnabled = true;
	m_joy2config.bValid = false;
	m_joy2config.bEnabled = false;
	m_joy2config.bPovEnabled = true;
	m_bSwapJoysticks = false;
	m_bCPUFriendly = true;
	m_bAudioClockSync = true;
	m_bSidDigiBoost = true;

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
}

int CConfig::GetKeyScanCode(UINT ch)
{
	return MapVirtualKey(ch, 0);
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
			if (((TCHAR *)lpData)[(bytesCopied / 2) - 1] == TEXT('\0'))
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