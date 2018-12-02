#ifndef __DIAGJOYSTICK_H__
#define __DIAGJOYSTICK_H__

#include "CDPI.h"
#include "gamedeviceitem.h"
#include "buttonitem.h"

extern BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

class CDiagJoystick : public CVirDialog , public ErrorMsg
{
	struct JoyControlNum
	{
		int cbo_joydevice;
		int cbo_joyfire1;
		int cbo_joyfire2;
		int cbo_joyv;
		int cbo_joyh;
		int cbo_joyvrev;
		int cbo_joyhrev;
		int cbo_joyup;
		int cbo_joydown;
		int cbo_joyleft;
		int cbo_joyright;
		int cbo_joyenable;
		int cbo_joyenablepov;
	};

	struct C64JoyItem
	{
		C64JoyItem(int ctrlid, C64JoystickButton::C64JoystickButtonNumber buttonnumber, DWORD * const pItemOffsets, unsigned int &itemCount);
		const C64JoystickButton::C64JoystickButtonNumber buttonnumber;
		DWORD * const pItemOffsets;
		unsigned int &itemCount;
		const int ctrlid;
	};

	static JoyControlNum joy1ControlNum;
	static JoyControlNum joy2ControlNum;

	class JoyUi
	{
	public:
		JoyUi(CDX9 *pDX, const struct joyconfig& outerjconfig, int ID, const JoyControlNum& controlNum);
		CDiagJoystick *dialog;
		CDX9 *pDX;
		struct joyconfig jconfig;
		std::vector<ButtonItem> buttonOptions;
		std::vector<ButtonItem> axisOptions;
		int ID;
		CDPI m_dpi;
		const JoyControlNum& controlNum;
		unsigned int numButtons;
		unsigned int numAxis;
		BestTextWidthDC tw;
		C64JoyItem c64buttonFire1;
		C64JoyItem c64buttonFire2;
		C64JoyItem c64buttonUp;
		C64JoyItem c64buttonDown;
		C64JoyItem c64buttonLeft;
		C64JoyItem c64buttonRight;
		C64JoyItem c64AxisHorizontal;
		C64JoyItem c64AxisVertical;
		BOOL EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE);
		BOOL EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE);
		void FillDeviceSelection();
		void DeviceChanged(bool bSetConfig);
		void ButtonSelectionChanged(C64JoyItem& c64joybutton);
		void AxisSelectionChanged(C64JoyItem& c64joybutton);
		void DeviceItemSelectionChanged(C64JoyItem& c64joybutton, std::vector<ButtonItem> buttonAxisOptions);
		void FillJoyAxis(bool bSetConfig);
		void FillJoyAxisDropdown(int ctrlid);
		void FillJoyButton(bool bSetConfig);
		void FillJoyButtonDropdown(int ctrlid, std::vector<ButtonItem> buttonAxisOptions);
		void SelectJoyButtonDropdownItem(C64JoyItem& c64joyaxis, bool bSetConfig);
		void SelectJoyAxisDropdownItem(C64JoyItem& c64joyaxis, bool bSetConfig);
		void SelectJoyButtonAxisDropdownItem(C64JoyItem& c64joybutton, std::vector<ButtonItem> buttonAxisOptions, bool bSetConfig);
		void loadconfig(const joyconfig& cfg);
		void saveconfig(joyconfig* cfg);
	private:
		std::basic_string<TCHAR> tmpDeviceName;
	};

public:
	CDiagJoystick(CDX9 *dx, const CConfig *);
	virtual ~CDiagJoystick();
	void Init();
	JoyUi joy1;
	JoyUi joy2;
	CConfig newCfg;
	const CConfig *currentCfg;
	virtual void loadconfig(const CConfig &);
	virtual void saveconfig(CConfig *);
private:
	int m_bActive;
	std::vector<shared_ptr<GameDeviceItem>> devices;
	HFONT defaultFont;
	
	CDPI m_dpi;
	void OpenDevices();
	HRESULT FillDevices();
	bool InitDialog(HWND hWndDlg);
	void CleanDialog();
	BOOL EnumDevices(LPCDIDEVICEINSTANCE lpddi);
	void ShowButtonConfig(JoyUi &joyui, C64JoyItem& c64joybutton);	
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	template<class T>
	static void AddToVec(std::vector<T> &vec, const T *source, unsigned int count);

	friend BOOL CALLBACK ::EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend JoyUi;

	CDX9 *pDX;
};

#endif