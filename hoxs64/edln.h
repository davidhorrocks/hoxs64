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
	EdLn();
	virtual ~EdLn();	

	HRESULT Init(HWND hParentWin, HFONT hFont, EditStyle style, bool isEditable, int numChars);

	HRESULT GetMinWindowSize(int &w, int &h);
	HRESULT SetPos(int x, int y);

	void SetValue(int v);
	HRESULT GetValue(int& v);

	size_t GetString(TCHAR buffer[], int bufferSize);
	void SetString(const TCHAR *data, int count);

	void Draw(HDC hdc);
protected:

private:
	static const int PADDING_LEFT  = 0;
	static const int PADDING_RIGHT  = 0;
	static const int PADDING_TOP  = 0;
	static const int PADDING_BOTTOM  = 0;
	static const int MARGIN_TOP  = 0;
	
	bool m_bIsEditable;
	EditStyle m_style;
	HWND m_hParentWin;
	HFONT m_hFont;

	int m_posX;
	int m_posY;
	int m_MinSizeW;
	int m_MinSizeH;
	bool m_MinSizeDone;

	static const TCHAR m_szMeasureAddress[];
	static const TCHAR m_szMeasureByte[];
	static const TCHAR m_szMeasureCpuFlags[];
	static const TCHAR m_szMeasureDigit[];

	int m_iValue;

	TCHAR *m_TextBuffer;
	int m_TextBufferLength;
	int m_iNumChars;

	HRESULT AllocTextBuffer(int i);
	void FreeTextBuffer();

	void SetHexAddress(int v);
	void SetHexByte(int v);
	void SetFlags(int v);
	void SetHex(int v);
	void SetDec(int v);

	HRESULT GetFlags(int& v);
	HRESULT GetHex(int& v);
	HRESULT GetDec(int& v);

};

#endif