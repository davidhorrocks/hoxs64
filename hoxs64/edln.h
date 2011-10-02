#ifndef __EDLN_H__
#define __EDLN_H__

class EdLn
{
public:
	enum EditStyle
	{
		HexAddress,
		HexByte,
		CpuFlags,
		Hex,
		Dec,
	};
	bool IsFocused;
	EdLn();
	virtual ~EdLn();	

	HRESULT Init(HWND hWnd, HFONT hFont, LPCTSTR pszCaption, EditStyle style, bool isEditable, int numChars);
	void Cleanup();

	HRESULT CreateDefaultHitRegion(HDC hdc);

	//HRESULT GetMinWindowSize(HDC hdc, int &w, int &h);
	HRESULT GetRects(HDC hdc, RECT *prcCaption, RECT *prcEdit, RECT *prcAll);
	HRESULT SetPos(int x, int y);

	int GetXPos();
	int GetYPos();

	void SetValue(int v);
	HRESULT GetValue(int& v);
	void SetInsertionPoint(int v);
	int GetInsertionPoint();
	bool GetIsEditable();
	size_t GetString(TCHAR buffer[], int bufferSize);
	void SetString(const TCHAR *data, int count);
	void CharEdit(TCHAR c);
	void UpdateCaret(HDC hdc);
	void Draw(HDC hdc);
	bool IsHitAll(int x, int y);
	bool IsHitEdit(int x, int y);
	HRESULT GetCharIndex(HDC hdc, int x, int y, int *pOutCellIndex, POINT *pPos);
	HRESULT GetCharPoint(HDC hdc, int cellIndex, int *pOutCellIndex, POINT *pPos);
protected:

private:
	//static const int PADDING_LEFT  = 0;
	//static const int PADDING_RIGHT  = 0;
	//static const int PADDING_TOP  = 0;
	//static const int PADDING_BOTTOM  = 0;
	//static const int MARGIN_TOP  = 0;
	
	bool m_bIsEditable;
	EditStyle m_style;
	HFONT m_hFont;

	HRGN m_hRegionHitAll;
	HRGN m_hRegionHitEdit;
	int m_posX;
	int m_posY;
	int m_MinSizeW;
	int m_MinSizeH;
	bool m_MinSizeDone;
	int m_iInsertionPoint;
	int m_iShowCaretCount;

	static const TCHAR m_szMeasureAddress[];
	static const TCHAR m_szMeasureByte[];
	static const TCHAR m_szMeasureCpuFlags[];
	static const TCHAR m_szMeasureDigit[];

	int m_iValue;

	TCHAR *m_TextBuffer;
	INT * m_pTextExtents;
	int m_iTextExtents;
	TCHAR *m_pszCaption;
	int m_iTextBufferLength;
	int m_iNumChars;
	HWND m_hWnd;

	HRESULT AllocTextBuffer(int i);
	void FreeTextBuffer();
	HRESULT SetCaption(LPCTSTR pszCaption);
	void FreeCaption();
	void SetHexAddress(int v);
	void SetHexByte(int v);
	void SetFlags(int v);
	void SetHex(int v);
	void SetDec(int v);
	HRESULT GetFlags(int& v);
	HRESULT GetHex(int& v);
	HRESULT GetDec(int& v);
	void InitVars();
};

#endif