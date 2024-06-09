#pragma once
#include "errormsg.h"

#undef USESYSMEMSURFACE
#define DISPLAYMODELISTSIZE (100)

#define JOYTEMP (0)
#define JOY1 (1)
#define JOY2 (2)

#define NUMJOYS (3)

typedef HRESULT(WINAPI * DIRECTINPUTCREATE)( HINSTANCE, DWORD, LPDIRECTINPUT*,IUnknown* );
typedef HRESULT(WINAPI * DIRECTDRAWCREATEEX)( GUID*, VOID**, REFIID, IUnknown* );
typedef HRESULT(WINAPI * DIRECTINPUTCREATEEX)( HINSTANCE, DWORD, REFIID, VOID**,IUnknown* );

class CDX9 : public ErrorMsg
{
public:

	enum JoystickIndex
	{
		Temporary=0,
		Joy1=1,
		Joy2=2
	};
	
	CDX9() noexcept;
	~CDX9();
	CDX9(const CDX9&) = delete;
	CDX9& operator=(const CDX9&) = delete;
	CDX9(CDX9&&) = default;
	CDX9& operator=(CDX9&&) = default;
	
	HRESULT Init(CAppStatus *appStatus);
	HRESULT EnumDevices(DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
	HRESULT InitJoy(HWND hWnd, int joyindex, struct joyconfig& joycfg);
	HRESULT InitJoys(HWND hWnd, struct joyconfig&, struct joyconfig&);
	void ReleaseJoy() noexcept;
	HRESULT CreateDeviceJoy(int, REFGUID refguid);
	HRESULT AcquireJoy(int);
	HRESULT UnacquireJoy(int);
	HRESULT PollJoy(int);
	LPDIRECTINPUTDEVICE7 GetJoy(int joyindex);
	HRESULT RestoreSoundBuffers();
	void SoundHalt(bit32 value);
	void SoundResume();
	HRESULT OpenDirectInput(HINSTANCE hInstance, HWND hWnd);
	HRESULT OpenDirectSound(HWND hWnd, HCFG::EMUFPS fps);
	void CloseDirectInput() noexcept;
	void CloseDirectSound() noexcept;
	void ClearSoundBuffer(LPDIRECTSOUNDBUFFER pSoundBuffer, bit32 value);
	void ClearSoundBuffer(bit32 value);

	WAVEFORMATEX m_wfx = {};
	DWORD SoundBufferByteSize = 0;
	DWORD SoundBytesPerSecond = 0;
	LPDIRECTINPUT7 pDI = nullptr;
	LPDIRECTINPUTDEVICE7	pKeyboard = nullptr;
	LPDIRECTINPUTDEVICE7 joy[NUMJOYS];
	LPDIRECTSOUND lpDirectSound = nullptr;
	LPDIRECTSOUNDBUFFER pPrimarySoundBuffer = nullptr;
	LPDIRECTSOUNDBUFFER pSecondarySoundBuffer = nullptr;
	bool joyok[NUMJOYS] = {};
	CAppStatus* m_appStatus = nullptr;
private:
	int m_soundResumeDelay = 0;
	HINSTANCE DIHinst = 0;
	DIRECTINPUTCREATEEX pDirectInputCreateEx;
	HWND m_hWndDevice = 0;
	HWND m_hWndFocus = 0;
	static const int SOUNDRESUMEDELAY = 25;
	static const int SOUNDVOLUMEDELAY = 25;
	static const int m_iToolbarHeight = 10;
};
