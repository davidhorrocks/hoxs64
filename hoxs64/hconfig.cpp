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

const LPCTSTR CConfig::Section_General = TEXT("General");
const LPCTSTR CConfig::Section_Joystick = TEXT("Joystick");
const LPCTSTR CConfig::Section_Keyboard = TEXT("Keyboard");
const LPCTSTR CConfig::Section_VICIIPalette = TEXT("VICIIPalette");
const LPCTSTR CConfig::Key_ImGuiAutoloadPath = TEXT("ImGuiAutoloadPath");
const LPCTSTR CConfig::Key_ReuImageFilename = TEXT("ReuImageFilename");
const LPCTSTR CConfig::Key_ReuUseImageFile = TEXT("ReuUseImageFile");
const LPCTSTR CConfig::Key_ReuExtraAddressBits = TEXT("ReuExtraAddressBits");
const LPCTSTR CConfig::Key_ReuInsertCart = TEXT("ReuInsertCart");

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
	m_bSaveWindowPositionOnExit = true;
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
	m_reu_extraAddressBits = 0;
	m_reu_use_image_file = false;
	m_reu_insertCartridge = false;
	LoadDefaultSetting();
}

void CConfig::SetCiaNewOldMode(bool isNew) noexcept
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

void CConfig::SetConfigLocation(bool isFile, std::wstring location)
{
	m_wsAppConfigFilenameFullPath = location;
	m_bIsUsingConfigFile = isFile;
}

std::shared_ptr<IConfigDataSource> CConfig::GetConfigSource()
{	
	std::shared_ptr<IConfigDataSource> p;
	if (m_bIsUsingConfigFile)
	{
		p = std::shared_ptr<IConfigDataSource>(new ConfigFileIni());
		p->SetLocation(m_wsAppConfigFilenameFullPath);
	}
	else
	{
		return GetConfigRegistrySource();
	}

	return p;
}

std::shared_ptr<IConfigDataSource> CConfig::GetConfigRegistrySource()
{
	std::shared_ptr<IConfigDataSource> p;
	p = std::shared_ptr<IConfigDataSource>(new ConfigRegistry());
	return p;
}

HRESULT CConfig::LoadCurrentSetting()
{
	HRESULT lRetCode; 
	DWORD dwValue;
	int i;	
	
	LoadDefaultSetting();
	std::shared_ptr<IConfigDataSource> configSource = GetConfigSource();
	lRetCode = configSource->ParseFile();
	if (FAILED(lRetCode))
	{
		return lRetCode;
	}

	if (configSource == nullptr)
	{
		return S_OK;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("PrefsSaved"), dwValue);
	if (FAILED(lRetCode) || dwValue == 0)
	{
		return S_OK;
	}

	// Read Keyboard config.
	//Patch soft keys to work after version v1.0.7.2
	m_KeyMap[C64Keys::C64K_CURSORUP] = DIK_UP;
	m_KeyMap[C64Keys::C64K_CURSORLEFT] = DIK_LEFT;
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_0"), C64Keys::C64K_0);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_1"), C64Keys::C64K_1);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_2"), C64Keys::C64K_2);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_3"), C64Keys::C64K_3);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_4"), C64Keys::C64K_4);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_5"), C64Keys::C64K_5);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_6"), C64Keys::C64K_6);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_7"), C64Keys::C64K_7);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_8"), C64Keys::C64K_8);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_9"), C64Keys::C64K_9);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_A"), C64Keys::C64K_A);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_B"), C64Keys::C64K_B);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_C"), C64Keys::C64K_C);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_D"), C64Keys::C64K_D);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_E"), C64Keys::C64K_E);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F"), C64Keys::C64K_F);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_G"), C64Keys::C64K_G);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_H"), C64Keys::C64K_H);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_I"), C64Keys::C64K_I);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_J"), C64Keys::C64K_J);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_K"), C64Keys::C64K_K);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_L"), C64Keys::C64K_L);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_M"), C64Keys::C64K_M);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_N"), C64Keys::C64K_N);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_O"), C64Keys::C64K_O);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_P"), C64Keys::C64K_P);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_Q"), C64Keys::C64K_Q);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_R"), C64Keys::C64K_R);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_S"), C64Keys::C64K_S);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_T"), C64Keys::C64K_T);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_U"), C64Keys::C64K_U);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_V"), C64Keys::C64K_V);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_W"), C64Keys::C64K_W);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_X"), C64Keys::C64K_X);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_Y"), C64Keys::C64K_Y);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_Z"), C64Keys::C64K_Z);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_PLUS"), C64Keys::C64K_PLUS);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_MINUS"), C64Keys::C64K_MINUS);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_ASTERISK"), C64Keys::C64K_ASTERISK);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_SLASH"), C64Keys::C64K_SLASH);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_COMMA"), C64Keys::C64K_COMMA);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_DOT"), C64Keys::C64K_DOT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_ARROWLEFT"), C64Keys::C64K_ARROWLEFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_COLON"), C64Keys::C64K_COLON);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_SEMICOLON"), C64Keys::C64K_SEMICOLON);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_CONTROL"), C64Keys::C64K_CONTROL);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_STOP"), C64Keys::C64K_STOP);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_COMMODORE"), C64Keys::C64K_COMMODORE);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_LEFTSHIFT"), C64Keys::C64K_LEFTSHIFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_RIGHTSHIFT"), C64Keys::C64K_RIGHTSHIFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_RESTORE"), C64Keys::C64K_RESTORE);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_HOME"), C64Keys::C64K_HOME);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_DEL"), C64Keys::C64K_DEL);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_RETURN"), C64Keys::C64K_RETURN);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_ARROWUP"), C64Keys::C64K_ARROWUP);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_POUND"), C64Keys::C64K_POUND);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_EQUAL"), C64Keys::C64K_EQUAL);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORDOWN"), C64Keys::C64K_CURSORDOWN);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORRIGHT"), C64Keys::C64K_CURSORRIGHT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORUP"), C64Keys::C64K_CURSORUP);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORLEFT"), C64Keys::C64K_CURSORLEFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_SPACE"), C64Keys::C64K_SPACE);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_AT"), C64Keys::C64K_AT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F1"), C64Keys::C64K_F1);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F2"), C64Keys::C64K_F2);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F3"), C64Keys::C64K_F3);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F4"), C64Keys::C64K_F4);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F5"), C64Keys::C64K_F5);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F6"), C64Keys::C64K_F6);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F7"), C64Keys::C64K_F7);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_F8"), C64Keys::C64K_F8);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1FIRE"), C64Keys::C64K_JOY1FIRE);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1UP"), C64Keys::C64K_JOY1UP);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1DOWN"), C64Keys::C64K_JOY1DOWN);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1LEFT"), C64Keys::C64K_JOY1LEFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1RIGHT"), C64Keys::C64K_JOY1RIGHT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1FIRE2"), C64Keys::C64K_JOY1FIRE2);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2FIRE"), C64Keys::C64K_JOY2FIRE);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2UP"), C64Keys::C64K_JOY2UP);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2DOWN"), C64Keys::C64K_JOY2DOWN);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2LEFT"), C64Keys::C64K_JOY2LEFT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2RIGHT"), C64Keys::C64K_JOY2RIGHT);
	ReadRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2FIRE2"), C64Keys::C64K_JOY2FIRE2);

	// Read General config.
	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("D1541_Emulation"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bD1541_Emulation_Enable = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SID_Emulation"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSID_Emulation_Enable = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("LimitSpeed"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bLimitSpeed = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("ShowSpeed"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bShowSpeed = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SkipAltFrames"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSkipFrames = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SIDSampleMode"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSIDResampleMode = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("WindowedLockAspectRatio"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		this->m_bWindowedLockAspectRatio = dwValue != 0;
	}
	else
	{
		this->m_bWindowedLockAspectRatio = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SIDStereo"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSIDStereo = dwValue != 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SyncMode1"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_syncModeFullscreen = (HCFG::FULLSCREENSYNCMODE)dwValue;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SyncMode2"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_syncModeWindowed = (HCFG::FULLSCREENSYNCMODE)dwValue;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SwapJoysticks"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSwapJoysticks = dwValue != 0;
	}
	else
	{
		m_bSwapJoysticks = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("CPUFriendly"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bCPUFriendly = dwValue != 0;
	}
	else
	{
		m_bCPUFriendly = true;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("AudioClockSync"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bAudioClockSync = dwValue != 0;
	}
	else
	{
		m_bAudioClockSync = true;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("SIDDigiBoost"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSidDigiBoost = dwValue != 0;
	}
	else
	{
		m_bSidDigiBoost = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenAdapterIsDefault"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenAdapterIsDefault = dwValue != 0;
	}
	else
	{
		m_fullscreenAdapterIsDefault = true;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenAdapterNumber"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenAdapterNumber = dwValue;
	}
	else
	{
		m_fullscreenAdapterNumber = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenOutputNumber"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenOutputNumber = dwValue;
	}
	else
	{
		m_fullscreenOutputNumber = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenWidth"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenWidth = dwValue;
	}
	else
	{
		m_fullscreenWidth = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenHeight"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenHeight = dwValue;
	}
	else
	{
		m_fullscreenHeight = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenRefreshNumerator"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenRefreshNumerator = dwValue;
	}
	else
	{
		m_fullscreenRefreshNumerator = 0;
	}

	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenRefreshDenominator"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenRefreshDenominator = dwValue;
	}
	else
	{
		m_fullscreenRefreshDenominator = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenFormat"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenFormat = dwValue;
	}
	else
	{
		m_fullscreenFormat = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenDxGiModeScaling"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenDxGiModeScaling = (DXGI_MODE_SCALING)dwValue;
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
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenDxGiModeScanlineOrdering"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenDxGiModeScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)dwValue;
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
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FullscreenStretch"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH)dwValue;
	}
	else
	{
		m_fullscreenStretch = (HCFG::EMUWINDOWSTRETCH)0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("BlitFilter"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_blitFilter = (HCFG::EMUWINDOWFILTER)dwValue;
	}
	else
	{
		m_blitFilter = (HCFG::EMUWINDOWFILTER)0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("BorderSize"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_borderSize = (HCFG::EMUBORDERSIZE)dwValue;
	}
	else
	{
		m_borderSize = HCFG::EMUBORDER_TV;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("ShowFloppyLed"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bShowFloppyLed = dwValue != 0;
	}
	else
	{
		m_bShowFloppyLed = true;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("PrgAlwaysQuickload"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bPrgAlwaysQuickload = dwValue != 0;
	}
	else
	{
		m_bPrgAlwaysQuickload = true;
	}	

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("FPS"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_fps = (HCFG::EMUFPS)dwValue;
	}
	else
	{
		m_fps = HCFG::EMUFPS_50;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("TrackZeroSensor"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_TrackZeroSensorStyle = (HCFG::ETRACKZEROSENSORSTYLE)dwValue;
	}
	else
	{
		m_TrackZeroSensorStyle = HCFG::TZSSPositiveHigh;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("CIAMode"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_CIAMode = (HCFG::CIAMODE)dwValue;
	}
	else
	{
		m_CIAMode = HCFG::CM_CIA6526A;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("CIATimerBbug"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		this->m_bTimerBbug = dwValue != 0;
	}
	else
	{
		this->m_bTimerBbug = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("NumberOfExtraSidChips"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		this->m_numberOfExtraSIDs = dwValue;
		if (this->m_numberOfExtraSIDs < 0 || this->m_numberOfExtraSIDs > 7)
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
		lRetCode = configSource->ReadDWord(Section_General, sidAddressName[i], dwValue);
		bit16 sidAddress = 0;
		if (SUCCEEDED(lRetCode))
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
	lRetCode = configSource->ReadDWord(Section_General, TEXT("DiskThreadEnable"), dwValue);
	if (SUCCEEDED(lRetCode))
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
	lRetCode = configSource->ReadDWord(Section_General, TEXT("AllowOpposingJoystick"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bAllowOpposingJoystick = dwValue != 0;
	}
	else
	{
		m_bAllowOpposingJoystick = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("DisableDwmFullscreen"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bDisableDwmFullscreen = dwValue != 0;
	}
	else
	{
		m_bDisableDwmFullscreen = false;
	}

	lRetCode = configSource->ReadDWord(Section_General, TEXT("EnableImGuiWindowed"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bEnableImGuiWindowed = dwValue != 0;
	}
	else
	{
		m_bEnableImGuiWindowed = true;
	}

	lRetCode = configSource->ReadDWord(Section_General, TEXT("SaveWindowPositionOnExit"), dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_bSaveWindowPositionOnExit = dwValue != 0;
	}
	else
	{
		m_bSaveWindowPositionOnExit = true;
	}

	// Read VICIIPalette config
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

		lRetCode = configSource->ReadDWord(Section_VICIIPalette, colorregkeyname.c_str(), dwValue);
		if (SUCCEEDED(lRetCode))
		{
			this->m_colour_palette[i] = dwValue;
		}
		else
		{
			this->m_colour_palette[i] = VicIIPalette::Pepto[i];
		}
	}

	// Load joystick 1 setting
	LoadCurrentJoystickSetting(configSource.get(), 1, this->m_joy1config);

	// Load joystick 2 setting
	LoadCurrentJoystickSetting(configSource.get(), 2, this->m_joy2config);

	// Load REU settings
	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, CConfig::Key_ReuInsertCart, dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_reu_insertCartridge = dwValue != 0;
	}
	else
	{
		m_reu_insertCartridge = false;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, CConfig::Key_ReuExtraAddressBits, dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_reu_extraAddressBits = dwValue;
	}
	else
	{
		m_reu_extraAddressBits = 0;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, CConfig::Key_ReuUseImageFile, dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_reu_use_image_file = dwValue != 0;
	}
	else
	{
		m_reu_use_image_file = true;
	}

	LoadReuImageFilenameSetting(configSource.get());
	return S_OK;
}

HRESULT CConfig::LoadCurrentJoystickSetting(IConfigDataSource* configSource, int joystickNumber, struct joyconfig& jconfig)
{
	LONG  lRetCode; 
	DWORD dw;
	DWORD byteCount;

	if (joystickNumber < 1 || joystickNumber > 2)
	{
		return E_FAIL;
	}

	unsigned int joyIndex = joystickNumber - 1;
	if (joyIndex >= _countof(JoyKeyName::Name))
	{
		return E_FAIL;
	}

	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynValid], dw);
	if (SUCCEEDED(lRetCode))
	{
		jconfig.IsValidId = dw != 0;
	}
	else
	{
		jconfig.IsValidId = false;
	}

	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynEnabled], dw);
	if (SUCCEEDED(lRetCode))
	{
		jconfig.IsEnabled = dw != 0;
	}
	else
	{
		jconfig.IsEnabled = false;
	}

	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynPOV], dw);
	if (SUCCEEDED(lRetCode))
	{
		jconfig.isPovEnabled = dw != 0;
	}
	else
	{
		jconfig.isPovEnabled = true;
	}

	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevX], dw);
	if (SUCCEEDED(lRetCode))
	{
		jconfig.isXReverse = dw != 0;
	}

	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevY], dw);
	if (SUCCEEDED(lRetCode))
	{
		jconfig.isYReverse = dw != 0;
	}

	if (jconfig.IsValidId)
	{
		if (SUCCEEDED(configSource->ReadGUID(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynGUID], jconfig.joystickID)))
		{
			jconfig.IsValidId = true;
		}
		else
		{
			jconfig.IsValidId = false;
		}

		//Joystick X axis
		jconfig.horizontalAxisAxisCount = 0;
		jconfig.dwOfs_X = DIJOFS_X;
		lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisX], dw);
		if (SUCCEEDED(lRetCode))
		{
			jconfig.isValidXAxis = dw != 0;
		}
		else
		{
			jconfig.isValidXAxis = true;
		}

		if (jconfig.isValidXAxis)
		{
			lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisX], dw);
			if (SUCCEEDED(lRetCode))
			{
				if (dw <= (sizeof(DIJOYSTATE2) - sizeof(LONG)))
				{
					jconfig.dwOfs_X = dw;
					jconfig.horizontalAxisAxisCount = 1;
				}
			}
		}
		else
		{
			jconfig.horizontalAxisAxisCount = 0;
		}

		//Joystick Y axis
		jconfig.verticalAxisAxisCount = 0;
		jconfig.dwOfs_Y = DIJOFS_Y;
		lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisY], dw);
		if (SUCCEEDED(lRetCode))
		{
			jconfig.isValidYAxis = dw != 0;
		}
		else
		{
			jconfig.isValidYAxis = true;
		}

		if (jconfig.isValidYAxis)
		{
			lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisY], dw);
			if (SUCCEEDED(lRetCode))
			{
				if (dw <= (sizeof(DIJOYSTATE2) - sizeof(LONG)))
				{
					jconfig.dwOfs_Y = dw;
					jconfig.verticalAxisAxisCount = 1;
				}
			}
		}
		else
		{
			jconfig.verticalAxisAxisCount = 0;
		}

		// Read map of game buttons to C64 joystick.
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Fire1, jconfig.fire1ButtonOffsets, jconfig.fire1ButtonCount);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Fire2, jconfig.fire2ButtonOffsets, jconfig.fire2ButtonCount);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Up, jconfig.upButtonOffsets, jconfig.upButtonCount);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Down, jconfig.downButtonOffsets, jconfig.downButtonCount);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Left, jconfig.leftButtonOffsets, jconfig.leftButtonCount);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::Right, jconfig.rightButtonOffsets, jconfig.rightButtonCount);

		// Read map of game buttons to C64 keys.
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey1, jconfig.keyNButtonOffsets[0], jconfig.keyNButtonCount[0]);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey2, jconfig.keyNButtonOffsets[1], jconfig.keyNButtonCount[1]);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey3, jconfig.keyNButtonOffsets[2], jconfig.keyNButtonCount[2]);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey4, jconfig.keyNButtonOffsets[3], jconfig.keyNButtonCount[3]);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey5, jconfig.keyNButtonOffsets[4], jconfig.keyNButtonCount[4]);
		ReadJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey6, jconfig.keyNButtonOffsets[5], jconfig.keyNButtonCount[5]);

		// Read map of game axes to C64 keys.
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey1, jconfig.keyNAxisOffsets[0], jconfig.keyNAxisDirection[0], jconfig.MAXAXIS, &jconfig.keyNAxisCount[0]);
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey2, jconfig.keyNAxisOffsets[1], jconfig.keyNAxisDirection[1], jconfig.MAXAXIS, &jconfig.keyNAxisCount[1]);
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey3, jconfig.keyNAxisOffsets[2], jconfig.keyNAxisDirection[2], jconfig.MAXAXIS, &jconfig.keyNAxisCount[2]);
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey4, jconfig.keyNAxisOffsets[3], jconfig.keyNAxisDirection[3], jconfig.MAXAXIS, &jconfig.keyNAxisCount[3]);
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey5, jconfig.keyNAxisOffsets[4], jconfig.keyNAxisDirection[4], jconfig.MAXAXIS, &jconfig.keyNAxisCount[4]);
		ReadJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey6, jconfig.keyNAxisOffsets[5], jconfig.keyNAxisDirection[5], jconfig.MAXAXIS, &jconfig.keyNAxisCount[5]);

		// Read map of game pov to C64 keys.
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey1, jconfig.keyNPovOffsets[0], jconfig.keyNPovDirection[0], jconfig.MAXPOV, &jconfig.keyNPovCount[0]);
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey2, jconfig.keyNPovOffsets[1], jconfig.keyNPovDirection[1], jconfig.MAXPOV, &jconfig.keyNPovCount[1]);
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey3, jconfig.keyNPovOffsets[2], jconfig.keyNPovDirection[2], jconfig.MAXPOV, &jconfig.keyNPovCount[2]);
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey4, jconfig.keyNPovOffsets[3], jconfig.keyNPovDirection[3], jconfig.MAXPOV, &jconfig.keyNPovCount[3]);
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey5, jconfig.keyNPovOffsets[4], jconfig.keyNPovDirection[4], jconfig.MAXPOV, &jconfig.keyNPovCount[4]);
		ReadJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey6, jconfig.keyNPovOffsets[5], jconfig.keyNPovDirection[5], jconfig.MAXPOV, &jconfig.keyNPovCount[5]);

		unsigned int i;
		DWORD numberOfKeysAssigned = 0;
		DWORD numberOfKeysValid = 0;
		lRetCode = configSource->ReadByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], nullptr, numberOfKeysAssigned);
		if (FAILED(lRetCode))
		{
			numberOfKeysAssigned = 0;
		}

		lRetCode = configSource->ReadByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], nullptr, numberOfKeysValid);
		if (FAILED(lRetCode))
		{
			numberOfKeysValid = 0;
		}

		unsigned int numberOfKeys = numberOfKeysAssigned;
		if (numberOfKeys > numberOfKeysValid)
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
			for (i = 0; i < _countof(jconfig.keyNoAssign); i++)
			{
				jconfig.keyNoAssign[i] = C64Keys::C64K_NONE;
			}

			byteCount = (DWORD)numberOfKeys;
			lRetCode = configSource->ReadByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], (LPBYTE)&c64KeyAssign[0], byteCount);
			if (SUCCEEDED(lRetCode))
			{
				for (i = 0; i < _countof(jconfig.keyNoAssign); i++)
				{
					if (i < numberOfKeys && i < byteCount)
					{
						jconfig.keyNoAssign[i] = c64KeyAssign[i];
					}
				}
			}

			// Read key assignment validities.
			bit8 c64KeyAssignValid[joyconfig::MaxUserKeyAssignCount];
			for (i = 0; i < _countof(jconfig.isValidKeyNoAssign); i++)
			{
				jconfig.isValidKeyNoAssign[i] = false;
			}

			byteCount = (DWORD)numberOfKeys;
			lRetCode = configSource->ReadByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], (LPBYTE)&c64KeyAssignValid[0], byteCount);
			if (SUCCEEDED(lRetCode))
			{
				for (i = 0; i < _countof(jconfig.isValidKeyNoAssign); i++)
				{
					if (i < numberOfKeys && i < byteCount)
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

	return S_OK;
}

HRESULT CConfig::WriteJoystickAxisList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pAxisOffsets, const GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int axisCount)
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
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], dwValue);
	configSource->WriteDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], (LPDWORD)&pAxisOffsets[0], axisCount);
	configSource->WriteDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.mask], (LPDWORD)&dwitem[0], axisCount);
	return S_OK;
}

HRESULT CConfig::ReadJoystickAxisList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pAxisOffsets, GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int maxAxisBufferCount, unsigned int *pAxisCount)
{
LONG lRetCode;
DWORD dwOffset;
DWORD offsetList[joyconfig::MAXAXIS];
DWORD storedAxisCount;
DWORD dwCount;
unsigned int axisCount = 0;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name))
	{
		return E_FAIL;
	}

	// Axis
	axisCount = 0;
	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], storedAxisCount);
	if (SUCCEEDED(lRetCode))
	{
		if (storedAxisCount > joyconfig::MAXAXIS)
		{
			storedAxisCount = joyconfig::MAXAXIS;
		}

		dwCount = _countof(offsetList);
		lRetCode = configSource->ReadDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], &offsetList[0], dwCount);
		if (SUCCEEDED(lRetCode))
		{
			if (storedAxisCount > dwCount)
			{
				storedAxisCount = dwCount;
			}

			DWORD numAxes = 0;
			for (i = 0; i < storedAxisCount; i++)
			{
				dwOffset = offsetList[i];
				if (dwOffset <= sizeof(DIJOYSTATE2) - sizeof(LONG))
				{
					if (numAxes < maxAxisBufferCount)
					{
						if (pAxisOffsets != NULL)
						{
							pAxisOffsets[numAxes] = dwOffset;
						}
					}

					numAxes++;
				}
			}

			axisCount = numAxes;
		}

		// Axis direction
		joyconfig::defaultClearAxisDirection(pAxisDirection, GameControllerItem::DirectionAny, maxAxisBufferCount);
		dwCount = _countof(offsetList);
		lRetCode = configSource->ReadDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.mask], &offsetList[0], dwCount);
		if (SUCCEEDED(lRetCode))
		{
			if (storedAxisCount > dwCount)
			{
				storedAxisCount = dwCount;
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

				if (numAxes < maxAxisBufferCount)
				{
					if (pAxisDirection != NULL)
					{
						pAxisDirection[numAxes] = (GameControllerItem::ControllerAxisDirection)dir;
					}
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

HRESULT CConfig::WriteJoystickButtonList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pButtonOffsets, const unsigned int &buttonCount)
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

	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], dwValue);
	configSource->WriteDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], (LPDWORD)&pButtonOffsets[0], dwValue);
	return S_OK;
}

HRESULT CConfig::ReadJoystickPovList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pPovOffsets, GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int maxPovBufferCount, unsigned int* pPovCount)
{
LONG lRetCode;
DWORD dwOffset;
DWORD offsetList[joyconfig::MAXPOV];
DWORD storedPovCount;
DWORD dwCount;
unsigned int povCount = 0;
unsigned int i;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name))
	{
		return E_FAIL;
	}

	povCount = 0;
	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], storedPovCount);
	if (SUCCEEDED(lRetCode))
	{
		if (storedPovCount > joyconfig::MAXPOV)
		{
			storedPovCount = joyconfig::MAXPOV;
		}

		dwCount = _countof(offsetList);
		lRetCode = configSource->ReadDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], &offsetList[0], dwCount);
		if (SUCCEEDED(lRetCode))
		{
			if (storedPovCount > dwCount)
			{
				storedPovCount = dwCount;
			}

			DWORD numPov = 0;
			for (i = 0; i < storedPovCount; i++)
			{
				dwOffset = offsetList[i];
				if (dwOffset >= DIJOFS_POV(0) && dwOffset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
				{
					if (numPov < maxPovBufferCount)
					{
						if (pPovOffsets != NULL)
						{
							pPovOffsets[numPov] = dwOffset;
						}
					}

					numPov++;
				}
			}

			povCount = numPov;
		}

		// Pov direction
		joyconfig::defaultClearAxisDirection(pPovDirection, GameControllerItem::DirectionAny, maxPovBufferCount);
		dwCount = _countof(offsetList);
		lRetCode = configSource->ReadDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.mask], &offsetList[0], dwCount);
		if (SUCCEEDED(lRetCode))
		{
			if (storedPovCount > dwCount)
			{
				storedPovCount = dwCount;
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

				if (numPov < maxPovBufferCount)
				{
					if (pPovDirection != NULL)
					{
						pPovDirection[numPov] = (GameControllerItem::ControllerAxisDirection)dir;
					}
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

HRESULT CConfig::WriteJoystickPovList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pPovOffsets, const GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int povCount)
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
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], dwValue);
	configSource->WriteDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], (LPDWORD)&pPovOffsets[0], povCount);
	configSource->WriteDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.mask], (LPDWORD)&dwitem[0], povCount);
	return S_OK;
}

HRESULT CConfig::ReadJoystickButtonList(IConfigDataSource* configSource, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pButtonOffsets, unsigned int &buttonCount)
{
LONG lRetCode;
DWORD dw;
DWORD dwOffset;
DWORD buttonIndexList[joyconfig::MAXBUTTONS];
DWORD storedButtonCount;
DWORD dwCount;
DWORD numButtons = 0;
unsigned int i;
unsigned int j;
unsigned int joyIndex = joystickNumber - 1;

	if (joyIndex >= _countof(JoyKeyName::Name))
	{
		return E_FAIL;
	}

	buttonCount = 0;
	lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.count], storedButtonCount);
	if (SUCCEEDED(lRetCode))
	{
		// Array of 32 bit indexes. Maximum of 128 buttons.
		if (storedButtonCount > joyconfig::MAXBUTTONS)
		{
			storedButtonCount = joyconfig::MAXBUTTONS;
		}

		dwCount = _countof(buttonIndexList);
		lRetCode = configSource->ReadDWordList(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.list], &buttonIndexList[0], dwCount);
		if (SUCCEEDED(lRetCode))
		{
			if (storedButtonCount > dwCount)
			{
				storedButtonCount = dwCount;
			}

			for (i = 0; i < storedButtonCount; i++)
			{
				dwOffset = buttonIndexList[i];
				if (dwOffset >= DIJOFS_BUTTON0 && dwOffset < DIJOFS_BUTTON(joyconfig::MAXBUTTONS))
				{
					if (pButtonOffsets != nullptr)
					{
						pButtonOffsets[numButtons++] = dwOffset;
					}
				}
			}

			buttonCount = numButtons;
		}
	}
	else if ((HRESULT)lRetCode == ConfigFileIni::ErrorNotFound)
	{
		// 32 bit mask representation. Maximum of 32 buttons.
		lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.mask], dw);
		if (SUCCEEDED(lRetCode))
		{
			for (i = 0, j = 1; i < joyconfig::MAXBUTTONS32; i++, j<<=1)
			{
				if (dw & j)
				{
					dwOffset = DIJOFS_BUTTON0 + i;
					if (dwOffset >= DIJOFS_BUTTON0 && dwOffset <= DIJOFS_BUTTON31)
					{
						if (pButtonOffsets != nullptr)
						{
							pButtonOffsets[numButtons++] = dwOffset;
						}
					}
				}
			}

			buttonCount = numButtons;
		}

		// Single button. One of a 128 buttons.
		if ((HRESULT)lRetCode == ConfigFileIni::ErrorNotFound || (SUCCEEDED(lRetCode) && numButtons == 0))
		{
			lRetCode = configSource->ReadDWord(Section_Joystick, JoyKeyName::Name[joyIndex][regnames.single], dw);
			if (SUCCEEDED(lRetCode))
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


HRESULT CConfig::WriteRegKeyboardItem(IConfigDataSource* configSource, LPCTSTR keyname, unsigned char keyvalue)
{
	DWORD dwValue = m_KeyMap[keyvalue];
	if (dwValue != 0)
	{
		return configSource->WriteDWord(CConfig::Section_Keyboard, keyname, dwValue);
	}
	else
	{
		return configSource->WriteDWord(CConfig::Section_Keyboard, keyname, 0);
	}
}

HRESULT CConfig::ReadRegKeyboardItem(IConfigDataSource* configSource, LPCTSTR keyname, unsigned char keyvalue)
{
	DWORD dwValue = 0;
	HRESULT lRetCode = configSource->ReadDWord(Section_Keyboard, keyname, dwValue);
	if (SUCCEEDED(lRetCode))
	{
		m_KeyMap[keyvalue] = dwValue & 0xff;
		return S_OK;
	}
	else
	{
		m_KeyMap[keyvalue] = 0;
		return lRetCode;
	}

}

HRESULT CConfig::LoadImGuiSetting()
{
	try
	{
		this->m_imgui_autoload_folder.clear();
		std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
		DWORD cchbuffer = 0;
		if (SUCCEEDED(configSource->ReadString(CConfig::Section_General, CConfig::Key_ImGuiAutoloadPath, nullptr, cchbuffer)))
		{
			if (cchbuffer > 0 && cchbuffer <= G::MaxApplicationPathLength)
			{
				DWORD extra = cchbuffer + 1;// Add one in case we receive a string that is not zero terminated.
				auto buffer = std::shared_ptr<wchar_t[]>(new wchar_t[extra]);
				if (SUCCEEDED(configSource->ReadString(CConfig::Section_General, CConfig::Key_ImGuiAutoloadPath, buffer.get(), cchbuffer)))
				{
					buffer[cchbuffer] = L'\0';// Ensures we are zero terminated.
					this->m_imgui_autoload_folder.assign(buffer.get());					
					return S_OK;
				}
			}
		}
	}
	catch (...)
	{
	}

	return E_FAIL;
}

HRESULT CConfig::SaveImGuiSetting()
{
	try
	{
		std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
		const wchar_t* p;
		if (m_imgui_autoload_folder.length() <= G::MaxApplicationPathLength)
		{
			p = this->m_imgui_autoload_folder.c_str();
		}
		else
		{
			p = G::EmptyString;
		}

		DWORD cchbuffer = (DWORD)((wcslen(p) + 1) * sizeof(wchar_t));
		if (SUCCEEDED(configSource->WriteString(CConfig::Section_General, CConfig::Key_ImGuiAutoloadPath, p, cchbuffer)))
		{
			return S_OK;
		}
	}
	catch (...)
	{
	}

	return E_FAIL;
}

HRESULT CConfig::LoadReuImageFilenameSetting(IConfigDataSource* configSource)
{
	try
	{
		std::wstring& str = m_reu_image_filename;
		LPCTSTR pstrSection = CConfig::Section_General;
		LPCTSTR pstrKey = CConfig::Key_ReuImageFilename;
		str.clear();
		DWORD cchbuffer = 0;
		if (SUCCEEDED(configSource->ReadString(pstrSection, pstrKey, nullptr, cchbuffer)))
		{
			if (cchbuffer > 0 && cchbuffer <= G::MaxApplicationPathLength)
			{
				DWORD extra = cchbuffer + 1;// Add one in case we receive a string that is not zero terminated.
				auto buffer = std::shared_ptr<wchar_t[]>(new wchar_t[extra]);
				if (SUCCEEDED(configSource->ReadString(pstrSection, pstrKey, buffer.get(), cchbuffer)))
				{
					buffer[cchbuffer] = L'\0';// Ensures we are zero terminated.
					str.assign(buffer.get());
					G::Trim(str);
					return S_OK;
				}
			}
		}
	}
	catch (...)
	{
	}

	return E_FAIL;
}

HRESULT CConfig::SaveReuImageFilenameSetting(IConfigDataSource* configSource)
{
	try
	{
		std::wstring& str = m_reu_image_filename;
		LPCTSTR pstrSection = CConfig::Section_General;
		LPCTSTR pstrKey = CConfig::Key_ReuImageFilename;
		const wchar_t* p;
		if (str.length() <= G::MaxApplicationPathLength)
		{
			p = str.c_str();
		}
		else
		{
			p = G::EmptyString;
		}

		DWORD cchbuffer = (DWORD)((wcslen(p) + 1) * sizeof(wchar_t));
		if (SUCCEEDED(configSource->WriteString(pstrSection, pstrKey, p, cchbuffer)))
		{
			return S_OK;
		}
	}
	catch (...)
	{
	}

	return E_FAIL;
}

HRESULT CConfig::SaveWindowSetting(HWND hWnd)
{
	LONG   lRetCode; 
	WINDOWPLACEMENT wp;
	DWORD dwValue;

	if (!m_bSaveWindowPositionOnExit)
	{
		return S_FALSE;
	}

	std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
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

	dwValue = (DWORD)pt_winpos.x;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MainWinPosX"), dwValue);

	dwValue = (DWORD)pt_winpos.y;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MainWinPosY"), dwValue);

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
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MainWinWidth"), dwValue);
	
	dwValue = (DWORD)h;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MainWinHeight"), dwValue);
	return S_OK;
}

HRESULT CConfig::SaveMDIWindowSetting(HWND hWnd)
{
	LONG   lRetCode; 
	WINDOWPLACEMENT wp;
	DWORD dwValue;

	std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
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
	
	dwValue = (DWORD)pt_mdidebuggerwinpos.x;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MDIWinDebuggerPosX"), dwValue);

	dwValue = (DWORD)pt_mdidebuggerwinpos.y;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MDIWinDebuggerPosY"), dwValue);

	dwValue = (DWORD)sz_mdidebuggerwinsize.cx;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MDIWinDebuggerWidth"), dwValue);

	dwValue = (DWORD)sz_mdidebuggerwinsize.cy;
	lRetCode = configSource->WriteDWord(CConfig::Section_General, TEXT("MDIWinDebuggerHeight"), dwValue);
	return S_OK;
}

HRESULT CConfig::LoadWindowSetting(POINT& pos, int& winWidth, int& winHeight)
{
	DWORD dwValue;
	LONG   lRetCode; 
	const int max_width = GetSystemMetrics(SM_CXMAXTRACK);
	const int max_height = GetSystemMetrics(SM_CYMAXTRACK);
	const int min_width = GetSystemMetrics(SM_CXMINTRACK);
	const int min_height = GetSystemMetrics(SM_CYMINTRACK);
	POINT posbuffer = {0, 0};
	int w = min_width;
	int h = min_width;
	bool ok = false;
	int v;

	std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
	if (configSource == nullptr)
	{
		return E_FAIL;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MainWinPosX"), dwValue);
	if (FAILED(lRetCode))
	{
		return lRetCode;
	}

	posbuffer.x = dwValue;
	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MainWinPosY"), dwValue);
	if (FAILED(lRetCode))
	{
		return lRetCode;
	}

	posbuffer.y = dwValue;
	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MainWinWidth"), dwValue);
	if (FAILED(lRetCode))
	{
		return lRetCode;
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
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MainWinHeight"), dwValue);
	if (FAILED(lRetCode))
	{
		return lRetCode;
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

	pos = posbuffer;
	winWidth = w;
	winHeight = h;
	return S_OK;
}

HRESULT CConfig::LoadMDIWindowSetting(POINT& pos, SIZE& size)
{
LONG  lRetCode; 
DWORD dwValue;
POINT posbuffer = pos;

	std::shared_ptr<IConfigDataSource> configSource = GetConfigRegistrySource();
	if (configSource == nullptr)
	{
		return E_FAIL;
	}

	int top = 0;
	int left = 0;
	int max_x = GetSystemMetrics(SM_CXMAXTRACK);
	int max_y = GetSystemMetrics(SM_CYMAXTRACK);
	int min_x = GetSystemMetrics(SM_CXMIN);
	int min_y = GetSystemMetrics(SM_CYMIN);
	int v;
	posbuffer.x = left;
	posbuffer.y = top;
	size.cx = 0;
	size.cy = 0;

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MDIWinDebuggerPosX"), dwValue);
	if (SUCCEEDED(lRetCode))
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

		posbuffer.x = v;
	}
	else
	{
		return lRetCode;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MDIWinDebuggerPosY"), dwValue);
	if (SUCCEEDED(lRetCode))
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

		posbuffer.y = v;
	}
	else
	{
		return lRetCode;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MDIWinDebuggerWidth"), dwValue);
	if (SUCCEEDED(lRetCode))
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
	else
	{
		return lRetCode;
	}

	dwValue = 0;
	lRetCode = configSource->ReadDWord(Section_General, TEXT("MDIWinDebuggerHeight"), dwValue);
	if (SUCCEEDED(lRetCode))
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
	else
	{
		return lRetCode;
	}

	pos = posbuffer;
	return S_OK;
}

HRESULT CConfig::SaveCurrentSettings()
{
	DWORD  dwValue;
	int i;

	std::shared_ptr<IConfigDataSource> configSource = GetConfigSource();	
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_0"), C64Keys::C64K_0);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_1"), C64Keys::C64K_1);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_2"), C64Keys::C64K_2);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_3"), C64Keys::C64K_3);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_4"), C64Keys::C64K_4);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_5"), C64Keys::C64K_5);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_6"), C64Keys::C64K_6);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_7"), C64Keys::C64K_7);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_8"), C64Keys::C64K_8);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_9"), C64Keys::C64K_9);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_A"), C64Keys::C64K_A);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_B"), C64Keys::C64K_B);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_C"), C64Keys::C64K_C);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_D"), C64Keys::C64K_D);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_E"), C64Keys::C64K_E);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F"), C64Keys::C64K_F);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_G"), C64Keys::C64K_G);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_H"), C64Keys::C64K_H);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_I"), C64Keys::C64K_I);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_J"), C64Keys::C64K_J);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_K"), C64Keys::C64K_K);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_L"), C64Keys::C64K_L);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_M"), C64Keys::C64K_M);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_N"), C64Keys::C64K_N);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_O"), C64Keys::C64K_O);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_P"), C64Keys::C64K_P);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_Q"), C64Keys::C64K_Q);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_R"), C64Keys::C64K_R);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_S"), C64Keys::C64K_S);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_T"), C64Keys::C64K_T);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_U"), C64Keys::C64K_U);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_V"), C64Keys::C64K_V);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_W"), C64Keys::C64K_W);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_X"), C64Keys::C64K_X);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_Y"), C64Keys::C64K_Y);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_Z"), C64Keys::C64K_Z);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_PLUS"), C64Keys::C64K_PLUS);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_MINUS"), C64Keys::C64K_MINUS);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_ASTERISK"), C64Keys::C64K_ASTERISK);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_SLASH"), C64Keys::C64K_SLASH);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_COMMA"), C64Keys::C64K_COMMA);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_DOT"), C64Keys::C64K_DOT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_ARROWLEFT"), C64Keys::C64K_ARROWLEFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_COLON"), C64Keys::C64K_COLON);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_SEMICOLON"), C64Keys::C64K_SEMICOLON);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_CONTROL"), C64Keys::C64K_CONTROL);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_STOP"), C64Keys::C64K_STOP);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_COMMODORE"), C64Keys::C64K_COMMODORE);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_LEFTSHIFT"), C64Keys::C64K_LEFTSHIFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_RIGHTSHIFT"), C64Keys::C64K_RIGHTSHIFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_RESTORE"), C64Keys::C64K_RESTORE);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_HOME"), C64Keys::C64K_HOME);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_DEL"), C64Keys::C64K_DEL);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_RETURN"), C64Keys::C64K_RETURN);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_ARROWUP"), C64Keys::C64K_ARROWUP);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_POUND"), C64Keys::C64K_POUND);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_EQUAL"), C64Keys::C64K_EQUAL);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORDOWN"), C64Keys::C64K_CURSORDOWN);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORRIGHT"), C64Keys::C64K_CURSORRIGHT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORUP"), C64Keys::C64K_CURSORUP);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_CURSORLEFT"), C64Keys::C64K_CURSORLEFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_SPACE"), C64Keys::C64K_SPACE);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_AT"), C64Keys::C64K_AT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F1"), C64Keys::C64K_F1);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F2"), C64Keys::C64K_F2);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F3"), C64Keys::C64K_F3);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F4"), C64Keys::C64K_F4);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F5"), C64Keys::C64K_F5);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F6"), C64Keys::C64K_F6);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F7"), C64Keys::C64K_F7);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_F8"), C64Keys::C64K_F8);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1FIRE"), C64Keys::C64K_JOY1FIRE);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1UP"), C64Keys::C64K_JOY1UP);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1DOWN"), C64Keys::C64K_JOY1DOWN);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1LEFT"), C64Keys::C64K_JOY1LEFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1RIGHT"), C64Keys::C64K_JOY1RIGHT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY1FIRE2"), C64Keys::C64K_JOY1FIRE2);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2FIRE"), C64Keys::C64K_JOY2FIRE);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2UP"), C64Keys::C64K_JOY2UP);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2DOWN"), C64Keys::C64K_JOY2DOWN);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2LEFT"), C64Keys::C64K_JOY2LEFT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2RIGHT"), C64Keys::C64K_JOY2RIGHT);
	WriteRegKeyboardItem(configSource.get(), TEXT("C64K_JOY2FIRE2"), C64Keys::C64K_JOY2FIRE2);

	dwValue = m_bD1541_Emulation_Enable ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("D1541_Emulation"), dwValue);

	dwValue = m_bSID_Emulation_Enable ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SID_Emulation"), dwValue);

	dwValue = m_bShowSpeed ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("ShowSpeed"), dwValue);

	dwValue = m_bLimitSpeed ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("LimitSpeed"), dwValue);

	dwValue = m_bSkipFrames ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SkipAltFrames"), dwValue);

	dwValue = m_bSIDResampleMode ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SIDSampleMode"), dwValue);

	dwValue = m_syncModeFullscreen;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SyncMode1"), dwValue);

	dwValue = m_syncModeWindowed;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SyncMode2"), dwValue);

	dwValue = m_bSwapJoysticks ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SwapJoysticks"), dwValue);

	dwValue = m_bCPUFriendly ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("CPUFriendly"), dwValue);

	dwValue = m_bAudioClockSync ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("AudioClockSync"), dwValue);

	dwValue = m_bSidDigiBoost ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SIDDigiBoost"), dwValue);

	dwValue = this->m_fullscreenAdapterIsDefault ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenAdapterIsDefault"), dwValue);

	dwValue = m_fullscreenAdapterNumber;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenAdapterNumber"), dwValue);

	dwValue = m_fullscreenOutputNumber;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenOutputNumber"), dwValue);

	dwValue = m_fullscreenWidth;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenWidth"), dwValue);

	dwValue = m_fullscreenHeight;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenHeight"), dwValue);

	dwValue = (DWORD)m_fullscreenRefreshNumerator;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenRefreshNumerator"), dwValue);

	dwValue = (DWORD)m_fullscreenRefreshDenominator;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenRefreshDenominator"), dwValue);

	dwValue = m_fullscreenFormat;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenFormat"), dwValue);

	dwValue = (DWORD)m_fullscreenDxGiModeScaling;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenDxGiModeScaling"), dwValue);

	dwValue = (DWORD)m_fullscreenDxGiModeScanlineOrdering;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenDxGiModeScanlineOrdering"), dwValue);

	dwValue = m_fullscreenStretch;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FullscreenStretch"), dwValue);

	dwValue = m_blitFilter;
	configSource->WriteDWord(CConfig::Section_General, TEXT("BlitFilter"), dwValue);

	dwValue = m_borderSize;
	configSource->WriteDWord(CConfig::Section_General, TEXT("BorderSize"), dwValue);

	dwValue = m_bShowFloppyLed ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("ShowFloppyLed"), dwValue);

	dwValue = m_bPrgAlwaysQuickload ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("PrgAlwaysQuickload"), dwValue);	

	dwValue = m_fps;
	configSource->WriteDWord(CConfig::Section_General, TEXT("FPS"), dwValue);

	dwValue = m_TrackZeroSensorStyle;
	configSource->WriteDWord(CConfig::Section_General, TEXT("TrackZeroSensor"), dwValue);

	dwValue = m_CIAMode;
	configSource->WriteDWord(CConfig::Section_General, TEXT("CIAMode"), dwValue);

	dwValue = m_bTimerBbug ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("CIATimerBbug"), dwValue);
	
	dwValue = (DWORD)this->m_numberOfExtraSIDs;
	if (dwValue >= 8)
	{
		dwValue = 0;
	}

	configSource->WriteDWord(CConfig::Section_General, TEXT("NumberOfExtraSidChips"), dwValue);

	dwValue = this->m_bWindowedLockAspectRatio ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("WindowedLockAspectRatio"), dwValue);

	dwValue = m_bSIDStereo ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SIDStereo"), dwValue);

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
		configSource->WriteDWord(CConfig::Section_General, sidAddressName[i], dwValue);
	}

	dwValue = m_bD1541_Thread_Enable ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("DiskThreadEnable"), dwValue);

	dwValue = m_bAllowOpposingJoystick ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("AllowOpposingJoystick"), dwValue);

	dwValue = m_bDisableDwmFullscreen ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("DisableDwmFullscreen"), dwValue);

	dwValue = m_bEnableImGuiWindowed ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("EnableImGuiWindowed"), dwValue);

	dwValue = m_bSaveWindowPositionOnExit ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, TEXT("SaveWindowPositionOnExit"), dwValue);

	dwValue = 1;
	configSource->WriteDWord(CConfig::Section_General, TEXT("PrefsSaved"), dwValue);

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
		configSource->WriteDWord(CConfig::Section_VICIIPalette, colorregkeyname.c_str(), rgbcolor);
	}

	//Save joystick 1 setting.
	SaveCurrentJoystickSetting(configSource.get(), 1, this->m_joy1config);

	//Save joystick 2 setting.
	SaveCurrentJoystickSetting(configSource.get(), 2, this->m_joy2config);

	// Save REU settings.
	dwValue = m_reu_insertCartridge ? 1 : 0;
	configSource->WriteDWord(CConfig::Section_General, CConfig::Key_ReuInsertCart, dwValue);

	dwValue = m_reu_extraAddressBits;
	configSource->WriteDWord(CConfig::Section_General, CConfig::Key_ReuExtraAddressBits, dwValue);

	configSource->WriteDWord(Section_General, CConfig::Key_ReuUseImageFile, m_reu_use_image_file ? 1 : 0);
	SaveReuImageFilenameSetting(configSource.get());

	configSource->WriteToFile();
	configSource->Close();
	return S_OK;
}

HRESULT CConfig::SaveCurrentJoystickSetting(IConfigDataSource* configSource, int joystickNumber, const struct joyconfig& jconfig)
{
bool bGuidOK;
DWORD dwValue;
DWORD dwByteLength;
unsigned int i;

	int joyIndex = joystickNumber - 1;
	//Save the joystick enabled option.
	dwValue = (jconfig.IsEnabled ? 1: 0);
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynEnabled], dwValue);

	//Save the POV option.
	dwValue = (jconfig.isPovEnabled ? 0xffffffff: 0);
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynPOV], dwValue);

	bGuidOK = false;
	if (jconfig.IsValidId)
	{
		if (SUCCEEDED(configSource->WriteGUID(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynGUID], jconfig.joystickID)))
		{
			bGuidOK = true;
		}
	}

	//Save the device ID.
	dwValue = ((jconfig.IsValidId && bGuidOK) ? 1: 0);
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynValid], dwValue);

	//Save the X axis validity.
	dwValue = (jconfig.isValidXAxis && jconfig.horizontalAxisAxisCount > 0) ? 1 : 0;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisX], dwValue);

	//Save the X axis.
	dwValue = jconfig.dwOfs_X;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisX], dwValue);

	//Save the Y axis validity.
	dwValue =(jconfig.isValidYAxis && jconfig.verticalAxisAxisCount > 0) ? 1 : 0;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynIsValidAxisY], dwValue);

	//Save the Y axis.
	dwValue = jconfig.dwOfs_Y;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynAxisY], dwValue);
	
	//Save the X axis reverse.
	dwValue = jconfig.isXReverse ? 1 : 0;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevX], dwValue);

	//Save the Y axis reverse.
	dwValue = jconfig.isYReverse ? 1 : 0;
	configSource->WriteDWord(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynRevY], dwValue);
	
	// Save map of game buttons to C64 joystick.
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Fire1, &jconfig.fire1ButtonOffsets[0], jconfig.fire1ButtonCount);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Fire2, &jconfig.fire2ButtonOffsets[0], jconfig.fire2ButtonCount);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Up, &jconfig.upButtonOffsets[0], jconfig.upButtonCount);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Down, &jconfig.downButtonOffsets[0], jconfig.downButtonCount);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Left, &jconfig.leftButtonOffsets[0], jconfig.leftButtonCount);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::Right, &jconfig.rightButtonOffsets[0], jconfig.rightButtonCount);

	// Save map of game buttons to C64 keys.
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey1, &jconfig.keyNButtonOffsets[0][0], jconfig.keyNButtonCount[0]);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey2, &jconfig.keyNButtonOffsets[1][0], jconfig.keyNButtonCount[1]);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey3, &jconfig.keyNButtonOffsets[2][0], jconfig.keyNButtonCount[2]);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey4, &jconfig.keyNButtonOffsets[3][0], jconfig.keyNButtonCount[3]);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey5, &jconfig.keyNButtonOffsets[4][0], jconfig.keyNButtonCount[4]);
	WriteJoystickButtonList(configSource, joystickNumber, JoyKeyName::ButtonKey6, &jconfig.keyNButtonOffsets[5][0], jconfig.keyNButtonCount[5]);

	// Save map of game axes to C64 keys.
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey1, &jconfig.keyNAxisOffsets[0][0], &jconfig.keyNAxisDirection[0][0], jconfig.keyNAxisCount[0]);
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey2, &jconfig.keyNAxisOffsets[1][0], &jconfig.keyNAxisDirection[1][0], jconfig.keyNAxisCount[1]);
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey3, &jconfig.keyNAxisOffsets[2][0], &jconfig.keyNAxisDirection[2][0], jconfig.keyNAxisCount[2]);
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey4, &jconfig.keyNAxisOffsets[3][0], &jconfig.keyNAxisDirection[3][0], jconfig.keyNAxisCount[3]);
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey5, &jconfig.keyNAxisOffsets[4][0], &jconfig.keyNAxisDirection[4][0], jconfig.keyNAxisCount[4]);
	WriteJoystickAxisList(configSource, joystickNumber, JoyKeyName::AxisKey6, &jconfig.keyNAxisOffsets[5][0], &jconfig.keyNAxisDirection[5][0], jconfig.keyNAxisCount[5]);

	// Save map of game pov to C64 keys.
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey1, &jconfig.keyNPovOffsets[0][0], &jconfig.keyNPovDirection[0][0], jconfig.keyNPovCount[0]);
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey2, &jconfig.keyNPovOffsets[1][0], &jconfig.keyNPovDirection[1][0], jconfig.keyNPovCount[1]);
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey3, &jconfig.keyNPovOffsets[2][0], &jconfig.keyNPovDirection[2][0], jconfig.keyNPovCount[2]);
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey4, &jconfig.keyNPovOffsets[3][0], &jconfig.keyNPovDirection[3][0], jconfig.keyNPovCount[3]);
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey5, &jconfig.keyNPovOffsets[4][0], &jconfig.keyNPovDirection[4][0], jconfig.keyNPovCount[4]);
	WriteJoystickPovList(configSource, joystickNumber, JoyKeyName::PovKey6, &jconfig.keyNPovOffsets[5][0], &jconfig.keyNPovDirection[5][0], jconfig.keyNPovCount[5]);

	//Save the key assignment array length.
	unsigned int numberOfKeys = joyconfig::MaxUserKeyAssignCount;

	//Save the key assignment array.	
	dwByteLength = numberOfKeys;
	configSource->WriteByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssign], (LPBYTE)&jconfig.keyNoAssign[0], dwByteLength);

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
	configSource->WriteByteList(Section_Joystick, JoyKeyName::Name[joyIndex][JoyKeyName::JoynButtonKeyNoAssignIsValid], (LPBYTE)&c64KeyAssignValid[0], dwByteLength);
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

void CConfig::LoadDefaultSetting() noexcept
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
	m_bSaveWindowPositionOnExit = true;
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
	m_bPrgAlwaysQuickload = true;
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
	m_reu_extraAddressBits = 0;
	m_reu_use_image_file = false;
	m_reu_insertCartridge = false;
	m_reu_image_filename.clear();
}

int CConfig::GetKeyScanCode(UINT ch) noexcept
{
	return MapVirtualKey(ch, 0);
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
		for (unsigned int i = 0; i < count; i++)
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
	//ZeroMemory(&pov, sizeof(pov));

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
		//ZeroMemory(&pov[i], sizeof(pov[0]));
		keyNButtonCount[i] = 0;
		keyNAxisCount[i] = 0;
		keyNPovCount[i] = 0;
	}

	inputDeviceFormat = &c_dfDIJoystick;
	sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
};


void joyconfig::SafeGuardMaxOffsets()
{
	unsigned int i;
	unsigned int j;
	if (sizeOfInputDeviceFormat == 0 || sizeOfInputDeviceFormat > sizeof(DIJOYSTATE2))
	{
		sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
	}

	SafeGuardAxis(dwOfs_X);
	SafeGuardAxis(dwOfs_Y);
	for (i = 0; i < _countof(fire1ButtonOffsets); i++)
	{
		SafeGuardButton(fire1ButtonOffsets[i]);
	}

	for (i = 0; i < _countof(fire2ButtonOffsets); i++)
	{
		SafeGuardButton(fire2ButtonOffsets[i]);
	}

	for (i = 0; i < MAXKEYMAPS; i++)
	{
		for (j = 0; j < _countof(keyNButtonOffsets); j++)
		{
			SafeGuardButton(keyNButtonOffsets[i][j]);
		}

		for (j = 0; j < _countof(keyNAxisOffsets); j++)
		{
			SafeGuardAxis(keyNAxisOffsets[i][j]);
		}

		for (j = 0; j < _countof(keyNPovOffsets); j++)
		{
			SafeGuardPov(keyNPovOffsets[i][j]);
		}
	}
}


void joyconfig::SafeGuardAxis(DWORD& dwOffset)
{
	if (dwOffset > sizeOfInputDeviceFormat - sizeof(LONG))
	{
		dwOffset = 0;
	}
}

void joyconfig::SafeGuardPov(DWORD& dwOffset)
{
	if (dwOffset >= DIJOFS_POV(0) && dwOffset <= DIJOFS_POV(joyconfig::MAXDIRECTINPUTPOVNUMBER))
	{
		dwOffset = DIJOFS_POV(0);
	}
}

void joyconfig::SafeGuardButton(DWORD& dwOffset)
{
	if (dwOffset > sizeOfInputDeviceFormat - sizeof(BYTE))
	{
		if (DIJOFS_BUTTON0 < sizeOfInputDeviceFormat)
		{
			dwOffset = DIJOFS_BUTTON0;
		}
		else
		{
			dwOffset = 0;
		}
	}
}
