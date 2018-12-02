#pragma once

#include "gamecontrolleritem.h"

class CDiagButtonSelection : public CVirDialog, public ErrorMsg
{
public:
	CDiagButtonSelection();
	CDiagButtonSelection(LPDIRECTINPUT7 pDI, GUID deviceId, int c64JoystickNumber, C64JoystickButton::C64JoystickButtonNumber c64button, std::vector<DWORD> &buttonOffsets);
	~CDiagButtonSelection();

	std::vector<DWORD> resultButtonOffsets;
private:
	struct ButtonItemData
	{
		ButtonItemData(DWORD buttonOffset);
		~ButtonItemData();
		DWORD buttonOffset;
	};

	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT init();
	void initvars();
	void PollJoystick();
	void ClearAllButtons();
	void ClearSelectedButton();
	void AmendList(unsigned int buttonNumber);
	void UpdateResult();
	BOOL EnumJoyButton(LPCDIDEVICEOBJECTINSTANCE);
	bool GetButtonNameFromOffset(std::basic_string<TCHAR> &name, DWORD offset);

	GUID deviceId;
	std::basic_string<TCHAR> deviceName;
	std::basic_string<TCHAR> mappedName;
	std::basic_string<TCHAR> deviceButtonName;
	std::vector<GameControllerItem> buttons;
	LPDIRECTINPUT7 pDI;
	LPDIRECTINPUTDEVICE7 pJoy;
	LPCDIDATAFORMAT inputDeviceFormat;
	DWORD sizeOfInputDeviceFormat;
	C64JoystickButton::C64JoystickButtonNumber c64button;
	int c64JoystickNumber;
	std::vector<shared_ptr<ButtonItemData>> currentButtonOffsets;	
	HWND hwndDeviceName;
	HWND hwndMappedName;
	HWND hwndListBox;

	friend BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
};

extern BOOL CALLBACK EnumDlgJoyButtonSelectionCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);