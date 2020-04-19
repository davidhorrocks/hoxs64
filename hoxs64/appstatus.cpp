#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "hconfig.h"
#include "appstatus.h"


CAppStatus::CAppStatus() noexcept
{
	m_bBusy = false;
	m_bSoundOK = false;
	m_bSoundMute = false;
	m_bFilterOK = false;
	m_bActive = false;
	m_bReady = false;
	m_bWindowed = true;
	m_bRunning = false;
	m_bDebug = false;
	m_bBreak = false;
	m_bPaused = false;
	m_bClosing = false;
	m_bIsDebugCart = false;
	signed int m_fskip = 0;
	m_audioSpeedStatus = HCFG::AUDIOSPEED::AUDIO_OK;
	m_systemfrequency = {};
	m_framefrequency = {};
	m_framefrequencyDoubler = {};
	m_bAutoload = false;
	m_bInitDone = false;
	m_bUpdateWindowTitle = false;
	m_bDiskLedMotor = false;
	m_bDiskLedDrive = false;
	m_bDiskLedWrite = false;
	m_bSerialTooBusyForSeparateThread = false;
	m_bSaveSkipFrames = false;
	m_bSaveLimitSpeed = false;
	m_SaveSyncModeFullscreen = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	m_SaveSyncModeWindowed = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	SaveSpeedSettings();
}

void CAppStatus::SaveSpeedSettings() noexcept
{
	m_bSaveSkipFrames = m_bSkipFrames;
	m_bSaveLimitSpeed = m_bLimitSpeed;
	m_SaveSyncModeFullscreen = m_syncModeFullscreen;
	m_syncModeWindowed = m_SaveSyncModeWindowed;
}

void CAppStatus::RestoreSpeedSettings() noexcept
{
	m_bSkipFrames = m_bSaveSkipFrames;
	m_bLimitSpeed = m_bSaveLimitSpeed;
	m_syncModeFullscreen = m_SaveSyncModeFullscreen;
	m_syncModeWindowed = m_SaveSyncModeWindowed;
}