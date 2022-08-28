#define INITGUID
#include <windows.h>
#include "dx_version.h"
#include <stdio.h>
#include "util.h"
#include "utils.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"

#define ASSUMED_DPI_DEFAULT (96)

CDX9::CDX9()
{
	int i;
	pDirectInputCreateEx = 0;
	DIHinst = 0;
	pKeyboard = 0;
	pDI = 0;

	for (i = 0; i < _countof(joy); i++)
	{
		joy[i] = nullptr;
	}

	for (i = 0; i < _countof(joyok); i++)
	{
		joyok[i] = false;
	}

	m_wfx = {};
	SoundBufferByteSize = 0;
	SoundBytesPerSecond = 0;
	pDI = nullptr;
	pKeyboard = nullptr;
	lpDirectSound = nullptr;
	pPrimarySoundBuffer = nullptr;
	pSecondarySoundBuffer = nullptr;
	m_appStatus = nullptr;
	m_soundResumeDelay = 0;
	DIHinst = 0;
	pDirectInputCreateEx = nullptr;
	m_hWndDevice = 0;
	m_hWndFocus = 0;
}

CDX9::~CDX9()
{
}

HRESULT CDX9::Init(CAppStatus *appStatus)
{
	m_appStatus = appStatus;
	return S_OK;
}

void CDX9::CloseDirectInput() noexcept
{
	if (DIHinst)
	{
		if (pDI) 
		{ 
			ReleaseJoy();

			if (pKeyboard) 
			{ 
				pKeyboard->Unacquire(); 
				pKeyboard->Release();
				pKeyboard = NULL; 
			} 

			pDI->Release();
			pDI = NULL; 
		} 

		FreeLibrary(DIHinst);
		DIHinst=0;
	}
}

void CDX9::CloseDirectSound() noexcept
{
	if (lpDirectSound)
	{
		if (pSecondarySoundBuffer)
		{
			pSecondarySoundBuffer->Stop();
			pSecondarySoundBuffer->Release();
			pSecondarySoundBuffer=NULL;
		}

		if (pPrimarySoundBuffer)
		{
			pPrimarySoundBuffer->Stop();
			pPrimarySoundBuffer->Release();
			pPrimarySoundBuffer=NULL;
		}

		lpDirectSound->Release();
		lpDirectSound=NULL;
	}
}

HRESULT CDX9::OpenDirectInput(HINSTANCE hInstance, HWND hWnd)
{
HRESULT hr;

	CloseDirectInput();
	DIHinst = LoadLibrary(TEXT("DINPUT.DLL"));
	if (DIHinst==0)
	{
		return SetError(E_FAIL, TEXT("Could not load DINPUT.DLL"));
	}

	pDirectInputCreateEx = (DIRECTINPUTCREATEEX)GetProcAddress( DIHinst, "DirectInputCreateEx");
	if (pDirectInputCreateEx==0)
	{
		CloseDirectInput();
		return SetError(E_FAIL, TEXT("Could find DirectInputCreateEx in DINPUT.DLL"));
	}

	hr = pDirectInputCreateEx(hInstance, 0x700 , IID_IDirectInput7, (void**)&pDI, NULL);
	if FAILED(hr)
	{
		return SetError(hr, TEXT("Could not open direct input."));
	}

	hr = pDI->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7, (void**)&pKeyboard, NULL); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return SetError(hr,  TEXT("Could not create direct input keyboard."));
	} 

	hr = pKeyboard->SetDataFormat(&c_dfDIKeyboard); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return hr; 
	} 

	hr = pKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return hr; 
	} 

	return S_OK;
}

HRESULT CDX9::OpenDirectSound(HWND hWnd, HCFG::EMUFPS fps)
{
HRESULT hr;
DSBUFFERDESC dsbdesc;

	ClearError();
	CloseDirectSound();
	hr= DirectSoundCreate(NULL, &lpDirectSound, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

    hr = lpDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);		
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, &pPrimarySoundBuffer, NULL);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

    ZeroMemory(&m_wfx, sizeof(WAVEFORMATEX)); 
    m_wfx.wFormatTag = WAVE_FORMAT_PCM; 
    m_wfx.nChannels = SID_SOUND_BYTES_PER_SAMPLE; 
    m_wfx.nSamplesPerSec = SID_SOUND_SAMPLES_PER_SEC; 
    m_wfx.wBitsPerSample = SID_SOUND_BYTES_PER_SAMPLE * 8; // 16 bits per sample.
    m_wfx.nBlockAlign = m_wfx.wBitsPerSample / 8 * m_wfx.nChannels;
    m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;
 
	hr = pPrimarySoundBuffer->SetFormat(&m_wfx);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	pPrimarySoundBuffer->Release();
	pPrimarySoundBuffer = NULL;
	
	SoundBufferByteSize = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;// / 50  * 6;

	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;// | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = SoundBufferByteSize; 
	dsbdesc.lpwfxFormat = &m_wfx;
	SoundBytesPerSecond = (m_wfx.nSamplesPerSec * m_wfx.nBlockAlign);	
	hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, &pSecondarySoundBuffer, NULL);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	hr	= RestoreSoundBuffers();
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	hr = pSecondarySoundBuffer->SetCurrentPosition(0);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	ClearSoundBuffer(0);
	return S_OK;
}

void CDX9::ClearSoundBuffer(bit32 value)
{
	ClearSoundBuffer(pSecondarySoundBuffer, value);
}

void CDX9::ClearSoundBuffer(LPDIRECTSOUNDBUFFER pSoundBuffer, bit32 value)
{
LPVOID p1= NULL;
LPVOID p2= NULL;
DWORD i1 =0;
DWORD i2 =0;

	if (!pSoundBuffer)
	{
		return;
	}

	if (SUCCEEDED(pSoundBuffer->Lock(0, 0, &p1, &i1, &p2, &i2, DSBLOCK_ENTIREBUFFER)))
	{
		for (DWORD i = 0 ; i < (i1 / (DWORD)sizeof(bit32)) ; i++)
		{
			((bit32 *)p1)[i] = value;
		}

		for (DWORD i = 0 ; i < (i2 / (DWORD)sizeof(bit32)) ; i++)
		{
			((bit32 *)p2)[i] = value;
		}

		pSoundBuffer->Unlock(p1, i1, p2, i2);
	}
}

HRESULT CDX9::RestoreSoundBuffers()
{
HRESULT hr;
DWORD dwStatus;
int i;
	ClearError();
    if( NULL == pSecondarySoundBuffer)
	{
        return E_POINTER;
	}

    if( FAILED( hr = pSecondarySoundBuffer->GetStatus( &dwStatus ) ) )
	{
        return hr;
	}

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
		i=0;
        do 
        {
            hr = pSecondarySoundBuffer->Restore();
			if (hr == DSERR_BUFFERLOST)
			{
				Sleep(10);
			}

			if (++i > 1000)
			{
				return hr;
			}

			hr = pSecondarySoundBuffer->Restore();
        }
        while(FAILED(hr));
    }

	ClearSoundBuffer(0);
    return S_OK;
}

void CDX9::SoundHalt(bit32 value)
{
	m_soundResumeDelay = 0;
	if (pSecondarySoundBuffer)
	{
		ClearSoundBuffer(value);
		pSecondarySoundBuffer->Stop();
	}
}

void CDX9::SoundResume()
{
DWORD soundStatus;
	if (pSecondarySoundBuffer)
	{
		if (m_soundResumeDelay > 0)
		{
			--m_soundResumeDelay;
		}
		else
		{
			m_soundResumeDelay = SOUNDRESUMEDELAY;
			if (S_OK == pSecondarySoundBuffer->GetStatus(&soundStatus))
			{
				if (soundStatus & DSBSTATUS_BUFFERLOST)
				{
					if (S_OK == pSecondarySoundBuffer->Restore())
					{
						pSecondarySoundBuffer->Play(0,0,DSBPLAY_LOOPING);
					}
					m_soundResumeDelay = 0;
				}
				else if ((soundStatus & DSBSTATUS_PLAYING) == 0)
				{
					pSecondarySoundBuffer->Play(0,0,DSBPLAY_LOOPING);
					m_soundResumeDelay = 0;
				}
			}
		}
	}
}

HRESULT CDX9::CreateDeviceJoy(int joyindex, REFGUID refguid)
{
HRESULT hr;

	joyok[joyindex] = false;
	if (joy[joyindex])
	{
		joy[joyindex]->Unacquire();
		joy[joyindex]->Release();
	}

	joy[joyindex] = NULL;
	hr = pDI->CreateDeviceEx(refguid, IID_IDirectInputDevice7, (LPVOID *)&joy[joyindex], NULL);
	return hr;
}

LPDIRECTINPUTDEVICE7 CDX9::GetJoy(int joyindex)
{
	return joy[joyindex];
}

void CDX9::ReleaseJoy() noexcept
{
	int i;
	for (i = 0; i < _countof(joy); i++)
	{
		if (joy[i])
		{
			joy[i]->Unacquire();
			joy[i]->Release();
			joy[i] = nullptr;
			joyok[i] = false;
		}
	}
}


HRESULT CDX9::EnumDevices(DWORD dwDevType, 	LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return pDI->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT CDX9::AcquireJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Acquire();
}

HRESULT CDX9::UnacquireJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Unacquire();
}

HRESULT CDX9::PollJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Poll();
}

HRESULT CDX9::InitJoy(HWND hWnd, int joyindex, struct joyconfig &joycfg)
{
	HRESULT hr = S_OK;
	DIPROPRANGE diprg; 
	DIDEVICEOBJECTINSTANCE didoi;
	DIDEVCAPS dicaps;
	unsigned int i;
	unsigned int j;

	ClearError();
	if (hWnd == 0 || this->DIHinst == nullptr || this->pDI == nullptr)
	{
		return E_FAIL;
	}

	joycfg.joyObjectKindX = HCFG::JoyKindNone;
	joycfg.joyObjectKindY = HCFG::JoyKindNone;
	joycfg.xMax= ButtonItemData::DefaultMax;
	joycfg.xMin= ButtonItemData::DefaultMin;
	joycfg.yMax= ButtonItemData::DefaultMax;
	joycfg.yMin= ButtonItemData::DefaultMin;;
	for(i = 0; i < _countof(joycfg.povAvailable); i++)
	{
		joycfg.povAvailable[0] = 0;
		joycfg.povIndex[0] = 0;
	}

	bool ok = false;
	if (joycfg.IsValidId && joycfg.IsEnabled)
	{
		UnacquireJoy(joyindex);
		hr = CreateDeviceJoy(joyindex, joycfg.joystickID);
		if (SUCCEEDED(hr))
		{
			LPDIRECTINPUTDEVICE7 pJoy = (LPDIRECTINPUTDEVICE7) this->GetJoy(joyindex);
			ZeroMemory(&dicaps, sizeof(dicaps));
			dicaps.dwSize = sizeof(dicaps);
			hr = pJoy->GetCapabilities(&dicaps);
			if (SUCCEEDED(hr))
			{
				if (G::IsLargeGameDevice(dicaps))
				{					
					joycfg.sizeOfInputDeviceFormat = sizeof(DIJOYSTATE2);
					joycfg.inputDeviceFormat = &c_dfDIJoystick2;
				}
				else
				{
					joycfg.sizeOfInputDeviceFormat = sizeof(DIJOYSTATE);
					joycfg.inputDeviceFormat = &c_dfDIJoystick;
				}

				joycfg.SafeGuardMaxOffsets();
				hr = pJoy->SetDataFormat(joycfg.inputDeviceFormat);
				if (SUCCEEDED(hr))
				{
					hr = pJoy->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
					if (SUCCEEDED(hr))
					{
						ok = true;
						if (joycfg.isValidXAxis)
						{
							//X Axis
							ZeroMemory(&didoi, sizeof(didoi));
							didoi.dwSize = sizeof(didoi);
							hr = pJoy->GetObjectInfo(&didoi, joycfg.dwOfs_X, DIPH_BYOFFSET);
							if (SUCCEEDED(hr))
							{
								if ((didoi.dwType & DIDFT_AXIS) != 0)
								{
									joycfg.joyObjectKindX = HCFG::JoyKindAxis;
									ZeroMemory(&diprg, sizeof(diprg));
									diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
									diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
									diprg.diph.dwHow        = DIPH_BYOFFSET; 
									diprg.diph.dwObj        = joycfg.dwOfs_X; // Specify the enumerated axis
									diprg.lMin              = joycfg.xMin; 
									diprg.lMax              = joycfg.xMax; 
									hr = pJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
									if (FAILED(hr))
									{
										ZeroMemory(&diprg, sizeof(diprg));
										diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
										diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
										diprg.diph.dwHow        = DIPH_BYOFFSET; 
										diprg.diph.dwObj        = joycfg.dwOfs_X; // Specify the enumerated axis
										hr = pJoy->GetProperty(DIPROP_RANGE, &diprg.diph);
										if (FAILED(hr))
										{
											SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
										}

										if (SUCCEEDED(hr))
										{
											joycfg.xMin = diprg.lMin; 
											joycfg.xMax = diprg.lMax; 
										}
									}

									joycfg.xLeft = (LONG)((double)joycfg.xMin + ButtonItemData::SensitivityGame * (double)(joycfg.xMax - joycfg.xMin) / 2.0);
									joycfg.xRight = (LONG)((double)joycfg.xMax - ButtonItemData::SensitivityGame * (double)(joycfg.xMax - joycfg.xMin) / 2.0);
								}
								else if ((didoi.dwType & DIDFT_POV) != 0)
								{
									joycfg.joyObjectKindX = HCFG::JoyKindPov;
								}
							}
						}

						//Y Axis
						if (joycfg.isValidYAxis)
						{
							ZeroMemory(&didoi, sizeof(didoi));
							didoi.dwSize = sizeof(didoi);
							hr = pJoy->GetObjectInfo(&didoi, joycfg.dwOfs_Y, DIPH_BYOFFSET);
							if (SUCCEEDED(hr))
							{
								if ((didoi.dwType & DIDFT_AXIS) != 0)
								{
									joycfg.joyObjectKindY = HCFG::JoyKindAxis;
									diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
									diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
									diprg.diph.dwHow        = DIPH_BYOFFSET; 
									diprg.diph.dwObj        = joycfg.dwOfs_Y; // Specify the enumerated axis
									diprg.lMin              = joycfg.yMin; 
									diprg.lMax              = joycfg.yMax; 
									hr = pJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
									if (FAILED(hr))
									{
										ZeroMemory(&diprg, sizeof(diprg));
										diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
										diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
										diprg.diph.dwHow        = DIPH_BYOFFSET; 
										diprg.diph.dwObj        = joycfg.dwOfs_Y; // Specify the enumerated axis
										hr = pJoy->GetProperty(DIPROP_RANGE, &diprg.diph);
										if (SUCCEEDED(hr))
										{
											joycfg.yMin = diprg.lMin; 
											joycfg.yMax = diprg.lMax; 
										}
									}

									joycfg.yUp = (LONG)((double)joycfg.yMin + (double)ButtonItemData::SensitivityGame * (double)(joycfg.yMax - joycfg.yMin) / 2.0);
									joycfg.yDown = (LONG)((double)joycfg.yMax - (double)ButtonItemData::SensitivityGame * (double)(joycfg.yMax - joycfg.yMin) / 2.0);
								}
								else if ((didoi.dwType & DIDFT_POV) != 0)
								{
									joycfg.joyObjectKindY = HCFG::JoyKindPov;
								}
							}
						}

						//POV
						if (joycfg.isPovEnabled)
						{
							ZeroMemory(&didoi, sizeof(didoi));
							didoi.dwSize = sizeof(didoi);
							for(i = 0, j = 0; i < _countof(joycfg.povAvailable); i++)
							{
								hr = pJoy->GetObjectInfo(&didoi, DIJOFS_POV(i), DIPH_BYOFFSET); 
								if (hr == DIERR_OBJECTNOTFOUND)
								{
									continue;
								}

								if (FAILED(hr))
								{
									break;
								}

								joycfg.povAvailable[j] = DIJOFS_POV(i);
								joycfg.povIndex[j] = i;
								j++;
							}
						}

						//Other axes
						for (j = 0; j < joycfg.MAXKEYMAPS; j++)
						{
							for (i = 0; i < joycfg.keyNAxisCount[j]; i++)
							{
								ButtonItemData axis(GameControllerItem::Axis, joycfg.keyNAxisOffsets[j][i], joycfg.keyNAxisDirection[j][i]);
								joycfg.axes[j][i] = axis;
								joycfg.axes[j][i].InitAxis(pJoy);
							}
						}
					}
				}
			}
		}
	}

	if (ok)
	{
		joyok[joyindex] = true;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT CDX9::InitJoys(HWND hWnd, struct joyconfig &joy1config,struct joyconfig &joy2config)
{
	ClearError();
	ReleaseJoy();
	if (hWnd == 0 || this->DIHinst == nullptr || this->pDI == nullptr)
	{
		return E_FAIL;
	}

	InitJoy(hWnd, JOY1, joy1config);
	InitJoy(hWnd, JOY2, joy2config);	
	if ((joy1config.IsValidId && joy1config.IsEnabled && !joyok[JOY1]) || (joy2config.IsValidId && joy2config.IsEnabled && !joyok[JOY2]))
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}
}
