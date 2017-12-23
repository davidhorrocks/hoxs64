#ifndef __DIAGJOYSTICK_H__
#define __DIAGJOYSTICK_H__

struct ButtonItem
{
	typedef enum tagButtonItemOption
	{
		SingleButton,
		SingleAxis,
		AllButtons,
		AllAxis
	} ButtonItemOption;

	ButtonItem()
	{
	}

	ButtonItem(ButtonItemOption option)
		: option(option)
	{
		ZeroMemory(&objectInfo, sizeof(objectInfo));
	}

	ButtonItem(ButtonItemOption option, const DIDEVICEOBJECTINSTANCE& objectInfo)
		: option(option)
		, objectInfo(objectInfo)
	{
	}

	ButtonItemOption option;
	DIDEVICEOBJECTINSTANCE objectInfo;	
};

typedef class CArrayElement<DIDEVICEINSTANCE> CDevElement;
typedef class CArray<DIDEVICEINSTANCE> CDevArray;

typedef class CArrayElement<DIDEVICEOBJECTINSTANCE> CDevAxisElement;
typedef class CArray<ButtonItem> CDevAxisArray;

extern BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

class CDiagJoystick : public CVirDialog , public ErrorMsg
{
	class JoyUi
	{
	public:
		JoyUi(CDX9 *pDX, const struct joyconfig& jconfig, int ID, int cbo_joydevice, int cbo_joyfire, int cbo_joyv, int cbo_joyh, int cbo_joyenable);
		CDiagJoystick *dialog;
		CDX9 *pDX;
		struct joyconfig jconfig;
		CDevAxisArray DevAxisArray;
		CDevAxisArray DevButtonArray;
		int ID;
		int cbo_joydevice;
		int cbo_joyfire;
		int cbo_joyv;
		int cbo_joyh;
		int cbo_joyenable;		
		bool bJoyAxisSetConfig;
		bool bGotDefaultX;
		bool bGotDefaultY;
		bool bGotDefaultFire;
		unsigned int defaultX;
		unsigned int defaultY;
		unsigned int defaultFire;

		BOOL EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE);
		BOOL EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE);
		void FillJoyAxis(bool bSetConfig);
		void FillJoyButton(bool bSetConfig);
	};

public:
	CDiagJoystick(CDX9 *dx, const CConfig *);
	virtual ~CDiagJoystick();
	void Init();
	JoyUi joy1;
	JoyUi joy2;
	CConfig newCfg;
	const CConfig *currentCfg;
	virtual void loadconfig(const CConfig *);
	virtual void saveconfig(CConfig *);
private:
	CDevArray DevArray;

	HRESULT FillDevices();
	int m_bActive;
	BOOL EnumDevices(LPCDIDEVICEINSTANCE lpddi);
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend BOOL CALLBACK ::EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend JoyUi;

	CDX9 *pDX;
};


#endif