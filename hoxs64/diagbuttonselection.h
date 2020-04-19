#pragma once
#include "cvirwindow.h"
#include "errormsg.h"
#include "gamecontrolleritem.h"

class CDiagButtonSelection : public CVirDialog, public ErrorMsg
{
	struct AxisState : GameControllerItem
	{
	public:
		AxisState();
		AxisState(GameControllerItem::ControllerItemOption option);
		AxisState(GameControllerItem::ControllerItemOption option, GameControllerItem::ControllerAxisDirection direction);
		AxisState(GameControllerItem::ControllerItemOption option, GameControllerItem::ControllerAxisDirection direction, const DIDEVICEOBJECTINSTANCE& objectInfo);
		void Init();

		bool IsCentred(LONG value);
		bool IsTriggeredMin(LONG value);
		bool IsTriggeredMax(LONG value);
		void GetName(std::basic_string<TCHAR> &name);

		bool hasMinMax;
		LONG maxValue;
		LONG minValue;
		LONG minActiveValue;
		LONG maxActiveValue;
		unsigned int notMinCount;
		unsigned int notMaxCount;
		unsigned int centredCount;
		unsigned int minCount;
		unsigned int maxCount;
	};

public:
	CDiagButtonSelection(LPDIRECTINPUT7 pDI, GUID deviceId, int c64JoystickNumber, C64JoystickButton::C64JoystickButtonNumber c64button, vector<ButtonItemData> &controllerItemOffsets);
	~CDiagButtonSelection();

	vector<ButtonItemData> resultButtons;
private:
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT init();
	void initvars();
	void PollJoystick();
	void ClearAllButtons();
	void ClearSelectedButton();
	void AmendList(AxisState item);
	void UpdateResult();
	BOOL EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE);
	BOOL EnumJoyAxis(LPCDIDEVICEOBJECTINSTANCE);
	BOOL EnumJoyPov(LPCDIDEVICEOBJECTINSTANCE);
	void InitAllAxes();
	void InitAxis(AxisState *axis);
	bool IsValidControllerItemNameOffset(DWORD offset);
	char *AllocAnsiStringBuffer(size_t size);

	GUID deviceId;
	std::basic_string<TCHAR> deviceName;
	std::basic_string<TCHAR> mappedName;
	std::basic_string<TCHAR> deviceButtonName;
	vector<AxisState> listAllControllerItem;
	LPDIRECTINPUT7 pDI;
	LPDIRECTINPUTDEVICE7 pJoy;
	LPCDIDATAFORMAT inputDeviceFormat;
	DWORD sizeOfInputDeviceFormat;
	C64JoystickButton::C64JoystickButtonNumber c64button;
	int c64JoystickNumber;
	vector<shared_ptr<ButtonItemData>> listCurrentControllerItem;	
	HWND hwndDeviceName;
	HWND hwndMappedName;
	HWND hwndListBox;
	char *pAnsiStringBuffer;
	size_t lenAnsiStringBuffer;	
	AxisState ArrayAxisState[joyconfig::MAXAXIS];
	bool allowButtons;
	bool allowAxes;
	bool allowPov;

	friend BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK EnumDlgJoyAxisSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	friend BOOL CALLBACK EnumDlgJoyPovSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
};

extern BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyAxisSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
extern BOOL CALLBACK EnumDlgJoyPovSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
