#pragma once
#include "cvirwindow.h"
#include "errormsg.h"
#include "bits.h"

extern LRESULT CALLBACK VicColorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct ColourControlState
{
	int ControlId;
	HWND HWnd;
	int VicColour;
	bit32 RGBColour;
	bool IsSelected;
	bool IsFocused;
	bool IsPalette;
};

struct NotifyColour : NMHDR
{
	enum NotificationTag
	{
		SelectionChanged = NM_FIRST + 1
	};
	ColourControlState *paletteitem;
};

class CDiagColour : public CVirDialog, public ErrorMsg
{
public:
	CDiagColour();
	~CDiagColour();
	CConfig newCfg;
	HRESULT Init(const CConfig *currentCfg);
private:
	static const unsigned int NumColours = 16;
	HPEN hPen_highlight;
	HPEN hPen_shadow;
	HPEN hPen_darkshadow;

	ColourControlState* selectedpaletteitem;
	CConfig currentCfg;	
	ColourControlState controlpalette[NumColours];
	ColourControlState largepaletteitem;
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnEventVicColorControl(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend extern LRESULT CALLBACK VicColorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT InitVicColourControls(HWND hwndDlg);
	void PaintControl(const ColourControlState* pcontrol);
	ColourControlState *SelectPaletteItem(ColourControlState* pcontrol, bool redraw);
	void UpdateSliders(ColourControlState* pcontrol);
	void DrawSelectBox(HDC hdc, const RECT& rc);
	ColourControlState* FindPaletteControlById(int id);
	ColourControlState* FindPaletteControlByWindow(HWND hWnd);
	void InitSlider(HWND hwndDlg, int trackBarId, int editId);
	void TBNotifications(WPARAM wParam, LPARAM lParam, int trackBarId, int editId);
	void EditNotifications(WPARAM wParam, LPARAM lParam, int editId, int trackBarId);
	void UpdateSelectedPaletteItem();
	void SaveNewColours();
	void LoadPepto();
	void LoadPreviousColors();
};
