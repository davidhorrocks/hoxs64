#pragma once
#include "cvirwindow.h"
#include "errormsg.h"
#include "CDPI.h"
#include "gamecontrolleritem.h"

extern BOOL CALLBACK EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyPovCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
class CDiagJoystick : public CVirDialog , public ErrorMsg
{
	struct JoyControlNum
	{
		int cbo_joydevice;
		int cbo_joyfire1;
		int cbo_joyfire2;
		int cbo_joyv;
		int cbo_joyh;
		int chk_joyvrev;
		int chk_joyhrev;
		int cbo_joyup;
		int cbo_joydown;
		int cbo_joyleft;
		int cbo_joyright;
		int chk_joyenable;
		int chk_joyenablepov;
		int cbo_joykey1button;
		int cbo_joykey2button;
		int cbo_joykey3button;
		int cbo_joykey4button;
		int cbo_joykey5button;
		int cbo_joykey6button;
		int cbo_joykey1;
		int cbo_joykey2;
		int cbo_joykey3;
		int cbo_joykey4;
		int cbo_joykey5;
		int cbo_joykey6;
	};

	struct C64JoyItem
	{
		C64JoyItem(int ctrlid, C64JoystickButton::C64JoystickButtonNumber buttonnumber
			, DWORD * const pButtonOffsets, unsigned int &buttonCount
			, DWORD * const pAxisOffsets, GameControllerItem::ControllerAxisDirection * const pAxisDirection, unsigned int &axisCount
			, DWORD * const pPovOffsets, GameControllerItem::ControllerAxisDirection * const pPovDirection, unsigned int &povCount
			);

		// Targeted UI control type
		const C64JoystickButton::C64JoystickButtonNumber buttonnumber;

		// Number of buttons
		unsigned int &buttonCount;

		// Pointer to array of button offsets.
		DWORD * const pButtonOffsets;

		// Number axes
		unsigned int &axisCount;

		// Pointer to array of axis offsets.
		DWORD * const pAxisOffsets;

		// Pointer to array of axis directions.
		GameControllerItem::ControllerAxisDirection * const pAxisDirection;

		// Number of pov items
		unsigned int &povCount;

		// Pointer to array of pov offsets.
		DWORD * const pPovOffsets;

		// Pointer to array of pov directions.
		GameControllerItem::ControllerAxisDirection * const pPovDirection;

		// Dropdown list control ID
		const int ctrlid;

		// pointer a list of game device items
		vector<GameControllerItem> *pListItems;
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
		vector<GameControllerItem> listButton;
		vector<GameControllerItem> listAxis;
		vector<GameControllerItem> listPov;
		vector<GameControllerItem> buttonOptions;
		vector<GameControllerItem> axisOptions;
		vector<GameControllerItem> buttonAndAxisOptions;
		int ID;
		CDPI m_dpi;
		const JoyControlNum& controlNum;
		unsigned int numButtons;
		unsigned int numAxis;
		unsigned int numPov;
		BestTextWidthDC tw;
		C64JoyItem c64buttonFire1;
		C64JoyItem c64buttonFire2;
		C64JoyItem c64buttonUp;
		C64JoyItem c64buttonDown;
		C64JoyItem c64buttonLeft;
		C64JoyItem c64buttonRight;
		C64JoyItem c64AxisHorizontal;
		C64JoyItem c64AxisVertical;
		C64JoyItem c64buttonKey1;
		C64JoyItem c64buttonKey2;
		C64JoyItem c64buttonKey3;
		C64JoyItem c64buttonKey4;
		C64JoyItem c64buttonKey5;
		C64JoyItem c64buttonKey6;
		BOOL EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE);
		BOOL EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE);
		BOOL EnumJoyPov(LPCDIDEVICEOBJECTINSTANCE);
		void FillDeviceSelection();
		void DeviceChanged(bool bSetConfig);
		void ButtonSelectionChanged(C64JoyItem& c64joyitem);
		void AxisSelectionChanged(C64JoyItem& c64joyitem);
		void ButtonAndAxisSelectionChanged(C64JoyItem& c64joyitem);
		void DropdownAutoSelectionChanged(C64JoyItem& c64joyitem);
		void DeviceItemSelectionChanged(C64JoyItem& c64joybutton, vector<GameControllerItem> &items, GameControllerItem::ObjectTypeFilter filter);
		HRESULT GetJoyAxisList(shared_ptr<GameDeviceItem> spDevice);
		HRESULT GetJoyButtonList(shared_ptr<GameDeviceItem> spDevice);
		HRESULT GetJoyPovList(shared_ptr<GameDeviceItem> spDevice);
		void FillJoyAxis(C64JoyItem& horizontal, C64JoyItem& vertical, bool bSetConfig);
		void FillJoyAxisDropdown(C64JoyItem& c64joybutton, vector<GameControllerItem> &items);
		void FillJoyButton(bool bSetConfig);
		void FillJoyButtonDropdown(C64JoyItem& c64joybutton, vector<GameControllerItem> &items);
		//void FillJoyButtonAndAxis(bool bSetConfig);//
		//void FillJoyButtonAndAxisDropdown(int ctrlid, vector<GameControllerItem> &items);//
		void FillJoyButtonKeys(const joyconfig& cfg);
		void FillJoyButtonKeyDropdown(int ctrlid, bool isValid, bit8 keyValue);
		void SelectJoyButtonKeyDropdown(int ctrlid, bool isValid, bit8 keyValue);
		void SelectJoyButtonAxisDropdownItem(C64JoyItem& c64joybutton, bool bSetConfig);
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
	vector<shared_ptr<GameDeviceItem>> devices;
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
	static void AddToVec(vector<T> &vec, const T *source, unsigned int count);

	friend BOOL CALLBACK ::EnumDlgJoyCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyButtonCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK ::EnumDlgJoyPovCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend JoyUi;

	CDX9 *pDX;
};
