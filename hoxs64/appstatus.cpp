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
	m_bSoundOK=FALSE;
	m_bSoundMute=FALSE;
	m_bFilterOK=false;
	m_bActive=FALSE;				// App is running/active
	m_bReady=FALSE;				// App is ready for updates
	m_bWindowed=TRUE;			// App is in windowed mode
	m_bRunning=FALSE;
	m_bDebug=FALSE;
	m_bBreak=FALSE;
	m_bPaused=FALSE;
	m_bFixWindowSize=FALSE;
	m_bClosing=false;
	m_bInitDone=false;

	m_fskip = 0;
	m_displayFormat = 0;
	m_ScreenDepth = 0;
	m_bUseCPUDoubler = false;
	m_blitFilter = 0;

	m_audioSpeedStatus = HCFG::AUDIO_OK;

	m_bAutoload = 0;
	m_bDiskLedMotor = false;
	m_bDiskLedDrive = false;
	m_bSerialTooBusyForSeparateThread = false;
	m_lastAudioVBLCatchUpCounter = 0;	

	m_bGotOldDwm = false;
	m_oldDwm = FALSE;
}