#pragma once

class BestTextWidthDC
{
public:
	BestTextWidthDC();
	BestTextWidthDC(HDC hdc);
	~BestTextWidthDC();
	void SetDC(HDC hdc);
	void SetFont(HFONT font);
	void Reset(); 
	void Clean();
	int GetWidth(LPCTSTR s);
	int GetWidthW(LPCWSTR s);
	int GetSuggestedDlgComboBoxWidth(HWND hDialog);
	int maxWidth;
private:
	HDC hdc;
	HFONT oldfont;
	TEXTMETRIC tm;
	bool tm_ok;
	int comboBoxPaddingX;
	void Init();
	void InitSizes();
	void RestoreFont();
	void InitTextMetrics();
};