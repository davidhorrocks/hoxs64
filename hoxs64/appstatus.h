#pragma once
#include "hconfig.h"

class CAppStatus : public CConfig
{
public:
	CAppStatus() noexcept;
	~CAppStatus() = default;
	CAppStatus(const CAppStatus&) = default;
	CAppStatus(CAppStatus&&) = default;
	CAppStatus& operator=(const CAppStatus&) = default;
	CAppStatus& operator=(CAppStatus&&) = default;

	bool m_bBusy = false;
	bool m_bSoundOK = false;
	bool m_bSoundMute = false;
	bool m_bFilterOK = false;
	bool m_bActive = false;				// App window is either active or not minimised
	bool m_bReady = false;				// DirectX is ready for use.
	bool m_bWindowed = true;			// App is in windowed mode
	bool m_bRunning = false;
	bool m_bDebug = false;
	bool m_bBreak = false;
	bool m_bPaused = true;
	bool m_bClosing = false;
	bool m_bIsDebugCart = false;
	bool m_preferQuickload = false;
	signed int m_fskip = 0;

	HCFG::AUDIOSPEED m_audioSpeedStatus = HCFG::AUDIOSPEED::AUDIO_OK;
	ULARGE_INTEGER m_systemfrequency = {};
	ULARGE_INTEGER m_framefrequency = {};
	ULARGE_INTEGER m_framefrequencyDoubler = {};

	bool m_bAutoload = false;
	bool m_bInitDone = false;
	bool m_bUpdateWindowTitle = false;
	bool m_bDiskLedMotor = false;
	bool m_bDiskLedDrive = false;
	bool m_bDiskLedWrite = false;
	bool m_bSerialTooBusyForSeparateThread = true;

	void SaveSpeedSettings() noexcept;
	void RestoreSpeedSettings() noexcept;
private:
	bool m_bSaveSkipFrames = false;
	bool m_bSaveLimitSpeed = true;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncModeFullscreen = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncModeWindowed = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
};
