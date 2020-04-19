#pragma once
#include <windows.h>
#include <commctrl.h>
#include "cevent.h"

class EdLn;

class EdLnControlEventArgs : public EventArgs
{
public:
	EdLnControlEventArgs(EdLn* pEdLnControl);		

	EdLn* pEdLnControl;
};

class EdLnTextChangedEventArgs : public EdLnControlEventArgs
{
public:
	EdLnTextChangedEventArgs(EdLn* pEdLnControl);
};

class EdLnTabControlEventArgs : public EdLnControlEventArgs
{
public:
	EdLnTabControlEventArgs(EdLn* pEdLnControl, bool isNext);

	bool IsNext;
	bool IsCancelled;
};

class EdLnCancelControlEventArgs : public EdLnControlEventArgs
{
public:
	EdLnCancelControlEventArgs(EdLn* pEdLnControl);
};

class EdLnSaveControlEventArgs : public EdLnControlEventArgs
{
public:
	EdLnSaveControlEventArgs(EdLn* pEdLnControl);

	bool IsCancelled;
};

class EdLn
{
public:
	enum EditStyle
	{
		Address,
		Byte,
		CpuFlags,
		DiskTrack,
		Number
	};

	EdLn();
	virtual ~EdLn();	

	EventSource<EdLnTextChangedEventArgs> EsOnTextChanged;
	EventSource<EdLnTabControlEventArgs> EsOnTabControl;
	EventSource<EdLnCancelControlEventArgs> EsOnCancelControl;
	EventSource<EdLnSaveControlEventArgs> EsOnSaveControl;
	bool IsChanged;
	bool IsSelected;
	int m_iTabIndex;	

	HRESULT Init(HWND hWnd, int iControlID, int iTabIndex, HFONT hFont, LPCTSTR pszCaption);
	void Cleanup();
	HRESULT CreateDefaultHitRegion(HDC hdc);	
	void Refresh();
	HRESULT GetRects(HDC hdc, RECT *prcCaption, RECT *prcEdit, RECT *prcAll);
	HRESULT SetPos(int x, int y);
	int GetXPos();
	int GetYPos();
	void SetEditValue(int v);
	HRESULT GetEditValue(int& v);
	void SetInsertionPoint(int v);
	int GetInsertionPoint();
	bool GetIsEditable();
	bool GetIsVisible();
	int GetControlID();
	size_t GetEditString(TCHAR buffer[], int bufferSize);
	void SetEditString(const TCHAR *data, int count);
	void CharEdit(TCHAR c);
	bool CanCharEdit(TCHAR c);
	bool OverwriteCharAndUpdateInsertionPoint(TCHAR c);
	void KeyDown(int keycode);
	void UpdateCaretPosition(HDC hdc);
	void Draw(HDC hdc);
	bool IsHitAll(int x, int y);
	bool IsHitEdit(int x, int y);
	HRESULT GetCharIndex(HDC hdc, int x, int y, int *pOutCellIndex, POINT *pPos);
	HRESULT GetCharPoint(HDC hdc, int cellIndex, int *pOutCellIndex, POINT *pPos);
	void SetStyle(EditStyle style, bool isVisible, bool isEditable, int numChars, bool isHex);
protected:

private:	
	static const TCHAR m_szMeasureChar[];
	static const int MaxChars = 256;
	static const int LengthOfCharBuffer = MaxChars + 1;

	bool m_bIsEditable;
	bool m_bIsVisible;
	bool m_bIsHex;
	EditStyle m_style;
	HFONT m_hFont;
	int m_iControlID;
	HRGN m_hRegionHitAll;
	HRGN m_hRegionHitEdit;
	int m_posX;
	int m_posY;
	int m_MinSizeW;
	int m_MinSizeH;
	int m_iInsertionPoint;
	int m_iShowCaretCount;
	int m_iValue;
	TCHAR *szEditTextBuffer;
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
	void SetHexAddress(TCHAR *pszBuffer, int v);
	void SetHexByte(TCHAR *pszBuffer, int v);
	void SetFlags(TCHAR *pszBuffer, int v);
	void SetHex(TCHAR *pszBuffer, int v);
	void SetDec(TCHAR *pszBuffer, int v);
	void SetHalfTrackIndex(TCHAR *pszBuffer, int v);
	HRESULT GetHalfTrackIndex(int& v);
	HRESULT GetFlags(int& v);
	HRESULT GetHex(int& v);
	HRESULT GetDec(int& v);
	void SetValue(TCHAR *pszBuffer, int v);
	void SetString(TCHAR *pszBuffer, const TCHAR *data, int count);
	void InitVars();
	void SpacePadBuffer(TCHAR *pszBuffer, int bufferSize);
};
