#ifndef __CDX9_H__
#define __CDX9_H__

#include "oldos_multimon.h"
#include "dxwindow.h"

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
	
	CDX9();
	~CDX9();
	
	HRESULT Init(CAppStatus *appStatus);
	HRESULT InitD3D(HWND hWndDevice, HWND hWndFocus, bool bWindowedMode, bool bDoubleSizedWindow, bool bWindowedCustomSize, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, HCFG::FULLSCREENSYNCMODE syncMode, DWORD adapterNumber, GUID fullscreenAdapterId, const D3DDISPLAYMODE &displayMode);
	HRESULT CreateDxDevice(UINT adapterNumber, HWND hFocusWindow, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface);
	HRESULT GetPresentationParams(HWND hWndDevice, HWND hWndFocus, bool bWindowedMode, HCFG::FULLSCREENSYNCMODE syncMode, DWORD adapterNumber, const D3DDISPLAYMODE &displayMode, D3DPRESENT_PARAMETERS& d3dpp);
	HRESULT Reset();
	void SetDefaultPalette();
	void SetDefaultPalette(const DWORD pallet[], int numentries);
	void CheckFilterCap(bool bIsMagnifying, D3DTEXTUREFILTERTYPE filter);
	void CleanupD3D();
	void CleanupD3D_Devices();
	void FreeSurfaces();
	HRESULT LoadTextures(D3DFORMAT format);
	void FreeTextures();
	HRESULT LoadFonts();
	void FreeFonts();
	HRESULT LoadSprites();
	void FreeSprites();
	void OnLostDevice();
	HRESULT OnResetDevice();
	HRESULT OnInitaliseDevice(IDirect3DDevice9 *pd3dDevice);
	void FreeSmallSurface();
	HRESULT CreateSmallSurface(int Width, int Height, D3DFORMAT Format);
	void ClearTargets(D3DCOLOR dwSolidColourFill);
	void ClearSurfaces(D3DCOLOR colour);
	HRESULT UpdateBackBuffer(D3DTEXTUREFILTERTYPE filter);
	IDirect3DSurface9 *GetSmallSurface();
	void DrawDriveSprites();
	void DrawUi();
	HRESULT Present(DWORD dwFlags);
	HRESULT EnumDevices(DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
	HRESULT InitJoy(HWND hWnd, int joyindex, struct joyconfig& joycfg);
	HRESULT InitJoys(HWND hWnd, struct joyconfig&, struct joyconfig&);
	void ReleaseJoy();
	HRESULT EnumObjectsJoy(int, LPDIENUMDEVICEOBJECTSCALLBACK, LPVOID, DWORD);
	HRESULT CreateDeviceJoy(int, REFGUID refguid);
	HRESULT GetObjectInfo(int joyindex, LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);
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
	void CalcStretchToFitClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, bool bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar);
	void CalcClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, const DWORD scale, bool bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar);
	int GetDisplayRect(LPRECT pDisplayRect);
	HRESULT RestoreSoundBuffers();
	void SoundHalt(short value);
	void SoundResume();
	HRESULT OpenDirectInput(HINSTANCE hInstance, HWND hWnd);
	HRESULT OpenDirectSound(HWND hWnd, HCFG::EMUFPS fps);
	void CloseDirectInput();
	void CloseDirectSound();
	void ClearSoundBuffer(LPDIRECTSOUNDBUFFER pSoundBuffer, short value);
	void ClearSoundBuffer(short value);
	DWORD ConvertColour2(D3DFORMAT format, COLORREF rgb);

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
	static int GetToolBarHeight(BOOL bShowFloppyLed);

	WAVEFORMATEX m_wfx;
	DWORD BufferLockSize;
	DWORD SoundBufferSize;
	DWORD SoundBytesPerFrame;
	DWORD SoundBytesPerSecond;
	
	CAppStatus *m_appStatus;
	IDirect3D9            *m_pD3D; // Used to create the D3DDevice
	IDirect3DDevice9      *m_pd3dDevice; // Our rendering device
	IDirect3DSwapChain9   *m_pd3dSwapChain;
	//The rectangle (in device pixels) of the C64 display with in the scaled dx backbuffer.
	//This is used for a blit from the dx small surface to the dx backbuffer.
	RECT m_rcTargetRect;
	bool m_bTargetRectOk;

	//The first C64 raster line to be displayed [0-311] at the top most edge of the display window.
	unsigned int m_displayFirstVicRaster;

	//The last C64 raster line to be displayed [0-311] at the bottom most edge of the display window.
	unsigned int m_displayLastVicRaster;

	//The width of the display in C64 pixels.
	unsigned int m_displayWidth;

	//The height of the display in C64 pixels.
	unsigned int m_displayHeight;

	//The first C64 X pixel position (zero based) that is at the left most edge of the display window.
	unsigned int m_displayStart;

	//Status bar dimensions
	LPDIRECTINPUTDEVICE7	pKeyboard;
	LPDIRECTINPUTDEVICE7 joy[NUMJOYS];
	LPDIRECTSOUND lpDirectSound;
	LPDIRECTSOUNDBUFFER pPrimarySoundBuffer;
	LPDIRECTSOUNDBUFFER pSecondarySoundBuffer;
	bool joyok[NUMJOYS];
	bool m_bWindowedCustomSize; 
	static const D3DFORMAT Formats[];

private:
	HRESULT SetRenderStyle(bool bWindowedMode, bool bDoubleSizedWindow, bool bWindowedCustomSize, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, D3DDISPLAYMODE currentDisplayMode);
	void SetClearingRects(D3DRECT [], int);
	long CheckForMode(long width,long height, long depth, bool is_rgb);
	LPD3DXFONT m_dxfont;
	LPD3DXSPRITE m_sprMessageText;
	DxLabel m_lblPaused;
	IDirect3DSurface9       *m_pSmallSurface[1];
	PALETTEENTRY m_paletteEntry[256];
	SIZE m_sizeSmallSurface;
	int	m_iIndexSmallSurface;

	double m_assumed_dpi_y;
	double m_assumed_dpi_x;

	LPD3DXSPRITE m_psprLedDrive;

	LPDIRECT3DTEXTURE9 m_ptxLedGreenOn;
	LPDIRECT3DTEXTURE9 m_ptxLedGreenOff;
	LPDIRECT3DTEXTURE9 m_ptxLedRedOn;
	LPDIRECT3DTEXTURE9 m_ptxLedRedOff;
	LPDIRECT3DTEXTURE9 m_ptxLedBlueOn;
	LPDIRECT3DTEXTURE9 m_ptxLedBlueOff;

	D3DXVECTOR3 m_vecPositionLedMotor;
	D3DXVECTOR3 m_vecPositionLedDrive;
	D3DXVECTOR3 m_vecPositionLedWrite;

	//Atmost four clearing rectangles to be blanked out.
	D3DRECT m_drcEraseRects[4];
	//The number of clearing rectangles [0 - 4] to be blanked out
	DWORD m_iEraseCount;
	D3DRECT m_drcStatusBar;
	bool m_bStatusBarOk;
	LPDIRECTINPUT7		pDI;
	int m_soundResumeDelay;
	HINSTANCE DIHinst;
	DIRECTDRAWCREATEEX pDirectDrawCreateEx;
	DIRECTINPUTCREATEEX pDirectInputCreateEx;
	D3DDISPLAYMODE *m_pDiplaymodes;
	long m_diplaymodes_count;
	D3DPRESENT_PARAMETERS m_d3dpp;
	HWND m_hWndDevice;
	HWND m_hWndFocus;
	int m_iAdapterNumber;
	D3DDISPLAYMODE m_displayModeActual;
	bool m_bWindowedMode;
	bool m_bDoubleSizedWindow;
	HCFG::EMUBORDERSIZE m_borderSize;
	bool m_bShowFloppyLed;
	bool m_bUseBlitStretch;
	D3DTEXTUREFILTERTYPE m_filter;
	HCFG::EMUWINDOWSTRETCH m_stretch;
	static const int SOUNDRESUMEDELAY = 25;
	static const int SOUNDVOLUMEDELAY = 25;
	static const int m_iToolbarHeight = 10;
};

#endif