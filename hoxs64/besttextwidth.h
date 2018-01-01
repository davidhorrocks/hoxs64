#pragma once

class BestTextWidthDC
{
public:
	BestTextWidthDC(HDC hdc);
	~BestTextWidthDC();
	void SetFont(HFONT font);
	void Reset(); 
	int GetWidth(LPCTSTR s);
	void InitSizes();
	void Clean();
	int GetSuggestedDlgComboBoxWidth(HWND);
	int maxWidth;
	int comboBoxPaddingX;
	HDC hdc;
	HFONT oldfont;
	TEXTMETRIC tm;
	bool tm_ok;
};