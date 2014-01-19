#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "hconfig.h"
#include "appstatus.h"


CAppStatus::CAppStatus()
{
	m_bSoundOK=false;
	m_bSoundMute=false;
	m_bFilterOK=false;
	m_bActive=false;				// App is running/active
	m_bReady=false;				// App is ready for updates
	m_bWindowed=true;			// App is in windowed mode
	m_bRunning=false;
	m_bDebug=false;
	m_bBreak=false;
	m_bPaused=false;
	//m_bFixWindowSize=true;
	m_bClosing=false;
	m_bInitDone=false;

	m_fskip = 0;
	m_displayFormat = 0;
	m_ScreenDepth = 0;
	m_bUseCPUDoubler = false;
	m_blitFilterDX = 0;

	m_audioSpeedStatus = HCFG::AUDIO_OK;

	m_bAutoload = 0;
	m_bDiskLedMotor = false;
	m_bDiskLedDrive = false;
	m_bSerialTooBusyForSeparateThread = false;

	SaveSpeedSettings();
}


void CAppStatus::SaveSpeedSettings()
{
	m_bSaveSkipFrames = m_bSkipFrames;
	m_bSaveLimitSpeed = m_bLimitSpeed;
	m_bSaveUseBlitStretch = m_bUseBlitStretch;
	m_SaveSyncMode = m_syncMode;
}

void CAppStatus::RestoreSpeedSettings()
{
	m_bSkipFrames = m_bSaveSkipFrames;
	m_bLimitSpeed = m_bSaveLimitSpeed;
	m_bUseBlitStretch = m_bSaveUseBlitStretch;
	m_syncMode = m_SaveSyncMode;
}

