#ifndef __DIAGJOYSTICK_H__
#define __DIAGJOYSTICK_H__


typedef class CArrayElement<DIDEVICEINSTANCE> CDevElement;
typedef class CArray<DIDEVICEINSTANCE> CDevArray;

typedef class CArrayElement<DIDEVICEOBJECTINSTANCE> CDevAxisElement;
typedef class CArray<DIDEVICEOBJECTINSTANCE> CDevAxisArray;

extern BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoy1AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoy2AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

extern BOOL CALLBACK EnumDlgJoy1ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoy2ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

class CDiagJoystick : public CVirDialog , public ErrorMsg
{
public:
	CDiagJoystick();
	virtual ~CDiagJoystick();
	HRESULT Init(CDX9 *dx, const CConfig *);
	virtual void loadconfig(const CConfig *);
	virtual void saveconfig(CConfig *);
	CConfig newCfg;
	const CConfig *currentCfg;

private:
	CDevArray DevArray;
	CDevAxisArray DevAxis1Array;
	CDevAxisArray DevAxis2Array;
	CDevAxisArray DevButton1Array;
	CDevAxisArray DevButton2Array;

	HRESULT FillDevices();
	BOOL EnumDevices(LPCDIDEVICEINSTANCE lpddi);
	BOOL EnumJoy1Axis(LPCDIDEVICEOBJECTINSTANCE);
	BOOL EnumJoy2Axis(LPCDIDEVICEOBJECTINSTANCE);

	BOOL EnumJoy1Button(LPCDIDEVICEOBJECTINSTANCE);
	BOOL EnumJoy2Button(LPCDIDEVICEOBJECTINSTANCE);

	int m_bActive;
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend BOOL CALLBACK ::EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

	friend BOOL CALLBACK ::EnumDlgJoy1AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoy2AxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoy1ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoy2ButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

	void FillJoy1Axis(BOOL bSetConfig);
	void FillJoy2Axis(BOOL bSetConfig);

	void FillJoy1Button(BOOL bSetConfig);
	void FillJoy2Button(BOOL bSetConfig);

	BOOL bJoy1AxisSetConfig,bJoy2AxisSetConfig;

	BOOL bGotDefault1X;
	BOOL bGotDefault1Y;
	BOOL bGotDefault1Fire;

	__int3264 default1X;
	__int3264 default1Y;
	__int3264 default1Fire;

	BOOL bGotDefault2X;
	BOOL bGotDefault2Y;
	BOOL bGotDefault2Fire;

	__int3264 default2X;
	__int3264 default2Y;
	__int3264 default2Fire;

	CDX9 *pDX;
};


#endif