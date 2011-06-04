#ifndef __CDX9_H__
#define __CDX9_H__

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
	
	CDX9();
	~CDX9();
	
	HRESULT Init(CConfig *cfg, CAppStatus *appStatus, bit32 vicColorTable[]);
	HRESULT InitD3D(HWND hWndDevice, HWND hWndFocus, BOOL bWindowedMode, BOOL bDoubleSizedWindow, HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, HCFG::FULLSCREENSYNCMODE syncMode, DWORD adapterNumber, GUID fullscreenAdapterId, const D3DDISPLAYMODE &displayMode);
	HRESULT Reset();
	void CheckFilterCap(bool bIsMagnifying, D3DTEXTUREFILTERTYPE filter);

	void CleanupD3D_Surfaces();
	void CleanupD3D_Devices();
	void CleanupD3D();
	void FreeSmallSurface();
	void FreeSysMemSurface();
	HRESULT CreateSmallSurface(int Width, int Height, D3DFORMAT Format);
	void ClearTargets(D3DCOLOR dwSolidColourFill);
	void ClearSurfaces(D3DCOLOR colour);
	void UpdateBackbuffer(D3DTEXTUREFILTERTYPE filter);
	IDirect3DSurface9 *GetSmallSurface();
	IDirect3DSurface9 *GetSysMemSurface();

	HRESULT LoadTextures(D3DFORMAT format);
	void FreeTextures();
	/*Textures*/
	
	LPD3DXSPRITE m_psprLedMotor;
	LPD3DXSPRITE m_psprLedDrive;
	LPD3DXSPRITE m_psprLedWrite;

	LPDIRECT3DTEXTURE9 m_ptxLedGreenOn;
	LPDIRECT3DTEXTURE9 m_ptxLedGreenOff;
	LPDIRECT3DTEXTURE9 m_ptxLedRedOn;
	LPDIRECT3DTEXTURE9 m_ptxLedRedOff;
	LPDIRECT3DTEXTURE9 m_ptxLedBlueOn;
	LPDIRECT3DTEXTURE9 m_ptxLedBlueOff;

	D3DXVECTOR3 m_vecPositionLedMotor;
	D3DXVECTOR3 m_vecPositionLedDrive;
	D3DXVECTOR3 m_vecPositionLedWrite;


	HRESULT OpenDirectInput(HINSTANCE hInstance, HWND hWnd);
	HRESULT OpenDirectSound(HWND hWnd, HCFG::EMUFPS fps);
	void CloseDirectInput();
	void CloseDirectSound();


	CAppStatus *m_appStatus;
	CConfig *m_cfg;

	PALETTEENTRY m_paletteEntry[256];

	/*Direct3D 9*/
	IDirect3D9            *m_pD3D; // Used to create the D3DDevice
	IDirect3DDevice9      *m_pd3dDevice; // Our rendering device
	IDirect3DSurface9       *m_pSmallSurface[1];
	IDirect3DSurface9       *m_pSysMemSurface;
	SIZE m_sizeSmallSurface;
	int	m_iIndexSmallSurface;
	RECT m_rcTargetRect;//Use the destination rectangle for a blit from m_pSmallSurface to a target.

	unsigned int m_displayFirstVicRaster;
	unsigned int m_displayLastVicRaster;
	unsigned int m_displayXPos;
	unsigned int m_displayYPos;
	unsigned int m_displayWidth;
	unsigned int m_displayHeight;
	unsigned int m_displayStart;

	D3DRECT m_drcEraseRects[4];
	DWORD m_iEraseCount;

	D3DRECT m_drcStatusBar;

private:
	HRESULT SetRenderStyle(BOOL bWindowedMode, BOOL bDoubleSizedWindow, HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, D3DDISPLAYMODE currentDisplayMode);
	void SetClearingRects(D3DRECT [], int);

public:
	/*Direct Draw*/
	HRESULT RestoreAllSurfaces();


	/*Direct Input*/
	//void m_SetDIPointer(void * pDI);
	HRESULT EnumDevices(DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
	//void m_ReleaseAll();
	HRESULT InitJoys(HWND , struct joyconfig &,struct joyconfig &);
	void ReleaseJoy();
	HRESULT EnumObjectsJoy(int, LPDIENUMDEVICEOBJECTSCALLBACK, LPVOID, DWORD);
	HRESULT CreateDeviceJoy(int, REFGUID refguid);
	HRESULT GetPropJoy(int joyindex, REFGUID rguid, LPDIPROPHEADER pph);
	HRESULT SetPropJoy(int joyindex, REFGUID rguid, LPCDIPROPHEADER pph);
	HRESULT SetDataFormatJoy(int, LPCDIDATAFORMAT);
	HRESULT AcquireJoy(int);
	HRESULT UnacquireJoy(int);
	HRESULT PollJoy(int);
	HRESULT SetCooperativeLevelJoy(int, HWND, DWORD);
	HRESULT GetDeviceState(int joyindex, LPVOID pData);
	LPDIRECTINPUTDEVICE7 GetJoy(int joyindex);


	HRESULT ListDisplayMode(DWORD adapterNumber);
	HRESULT ChooseDisplayMode(DWORD adapterNumber, D3DDISPLAYMODE *pMode);

	bool CanDisplayManualMode(DWORD adapterNumber, const D3DDISPLAYMODE &displayMode, D3DDISPLAYMODE &chooseDisplayMode);
	bool CanDisplayCurrentMode(DWORD adapterNumber, D3DDISPLAYMODE &chooseDisplayMode, DWORD &chooseAdapterNumber);
	bool CanDisplayOtherMode(DWORD adapterNumber, D3DDISPLAYMODE &chooseDisplayMode);
	bool IsAcceptableMode(const D3DDISPLAYMODE &displayMode);
	bool CanMode1X(const D3DDISPLAYMODE &displayMode, const C64WindowDimensions &dims, BOOL bShowFloppyLed);
	bool CanMode2X(const D3DDISPLAYMODE &displayMode, const C64WindowDimensions &dims, BOOL bShowFloppyLed);
	int GetAdapterOrdinalFromGuid(const GUID &id);
	void CalcStretchToFitClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, BOOL bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar);
	void CalcClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, const DWORD scale, BOOL bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar);

	/*Direct Sound*/
	HRESULT RestoreSoundBuffers();
	void SoundHalt(short value);
	void SoundResume();
	WAVEFORMATEX m_wfx;
	DWORD BufferLockSize;
	DWORD SoundBufferSize;
	DWORD SoundBytesPerFrame;
	DWORD SoundBytesPerSecond;

protected:
	int m_soundResumeDelay;
	static const int SOUNDRESUMEDELAY = 25;
	int m_soundVolumeDelay;
	static const int SOUNDVOLUMEDELAY = 25;
	static const int SOUNDVOLUMEDELAYZERO = 26;

	HINSTANCE DIHinst;

	long CheckForMode(long width,long height, long depth, bool is_rgb);

	DIRECTDRAWCREATEEX pDirectDrawCreateEx;
	DIRECTINPUTCREATEEX pDirectInputCreateEx;

	D3DDISPLAYMODE *m_pDiplaymodes;
	long m_diplaymodes_count;
	D3DPRESENT_PARAMETERS m_d3dpp;

public:
	LPDIRECTINPUT7		pDI;
	LPDIRECTINPUTDEVICE7	pKeyboard;
	LPDIRECTINPUTDEVICE7 joy[NUMJOYS];

	LPDIRECTSOUND lpDirectSound;
	LPDIRECTSOUNDBUFFER pPrimarySoundBuffer;
	LPDIRECTSOUNDBUFFER pSecondarySoundBuffer;
	void ClearSoundBuffer(LPDIRECTSOUNDBUFFER pSoundBuffer, short value);
	void ClearSoundBuffer(short value);

	BOOL joyok[NUMJOYS];


	static DWORD DDColorMatch(IDirect3DSurface9 *pdds, COLORREF rgb);
	static DWORD GetBitsPerPixel(D3DFORMAT fmt);
	static DWORD ReduceBits(BYTE v, BYTE bits);
	static DWORD ConvertColour(D3DFORMAT format, COLORREF rgb);
	static BOOL DXUTGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo);
	static HRESULT GetAdapterFromWindow(IDirect3D9 *pD3D, HWND hWndDevice, UINT& adapterNumber);

	static DWORD CheckDXVersion9();
	static HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion);
	static HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
	static HRESULT GetDirectXVersionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
	static HRESULT GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion );
	static ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d );
	static int CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 );
	static UINT GetDisplayResolutionText(const D3DDISPLAYMODE &displayMode, LPTSTR buffer, UINT charBufferLen);
	static UINT GetDisplayFormatText(const D3DDISPLAYMODE &displayMode, LPTSTR buffer, UINT charBufferLen);
	static D3DTEXTUREFILTERTYPE GetDxFilterFromEmuFilter(HCFG::EMUWINDOWFILTER emuFilter);
	
private:
	BOOL m_bWindowedMode;;
	BOOL m_bDoubleSizedWindow;
	HCFG::EMUBORDERSIZE m_borderSize;
	BOOL m_bShowFloppyLed;
	BOOL m_bUseBlitStretch;
	D3DTEXTUREFILTERTYPE m_filter;
	HCFG::EMUWINDOWSTRETCH m_stretch;
public:
	static const D3DFORMAT Formats[];

	static int GetToolBarHeight(BOOL bShowFloppyLed);
private:
	static const int m_iToolbarHeight = 10;
};

#endif