#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>
#include <math.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hexconv.h"
#include "dchelper.h"
#include "edln.h"
#include "resource.h"

const TCHAR EdLn::m_szMeasureChar[] = TEXT("W");

EdLnControlEventArgs::EdLnControlEventArgs(EdLn* pEdLnControl)
{
	this->pEdLnControl = pEdLnControl;
}

EdLnTextChangedEventArgs::EdLnTextChangedEventArgs(EdLn* pEdLnControl)
	: EdLnControlEventArgs(pEdLnControl)
{
	
}

EdLnCancelControlEventArgs::EdLnCancelControlEventArgs(EdLn* pEdLnControl)
	: EdLnControlEventArgs(pEdLnControl)
{
}

EdLnTabControlEventArgs::EdLnTabControlEventArgs(EdLn* pEdLnControl, bool isNext)
	: EdLnControlEventArgs(pEdLnControl)
{
	this->IsNext = isNext;
	this->IsCancelled = false;
}

EdLnSaveControlEventArgs::EdLnSaveControlEventArgs(EdLn* pEdLnControl)
	: EdLnControlEventArgs(pEdLnControl)
{
	this->IsCancelled = false;
}

EdLn::EdLn()
{
	InitVars();
}

void EdLn::InitVars()
{
	m_iValue = 0;
	m_iTextBufferLength = 0;
	this->szEditTextBuffer = NULL;
	m_pTextExtents = NULL;
	m_iTextExtents = 0;
	m_iNumChars = 0;
	m_pszCaption = 0;
	this->IsChanged = false;
	this->IsSelected = false;
	m_hRegionHitAll = NULL;
	m_hRegionHitEdit = NULL;
	m_iInsertionPoint = 0;
	m_iShowCaretCount = 0;
	m_bIsVisible = true;
	m_posX = 0;
	m_posY = 0;
	m_MinSizeW = 0;
	m_MinSizeH = 0;
}


EdLn::~EdLn()
{
	Cleanup();
}

void EdLn::Cleanup()
{
	FreeTextBuffer();
	FreeCaption();
	if (m_hRegionHitAll)
	{
		DeleteObject(m_hRegionHitAll);
		m_hRegionHitAll = NULL;
	}

	if (m_hRegionHitEdit)
	{
		DeleteObject(m_hRegionHitEdit);
		m_hRegionHitEdit = NULL;
	}
}

HRESULT EdLn::Init(HWND hWnd, int iControlID, int iTabIndex, HFONT hFont, LPCTSTR pszCaption)
{
HRESULT hr = E_FAIL;
	Cleanup();
	InitVars();
	this->m_hFont = hFont;
	this->m_hWnd = hWnd;
	this->m_iControlID = iControlID;
	this->m_iTabIndex = iTabIndex;
	hr = AllocTextBuffer(LengthOfCharBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = SetCaption(pszCaption);
	if (FAILED(hr))
	{
		return hr;
	}

	this->SetEditString(NULL, 0);
	this->SpacePadBuffer(this->szEditTextBuffer, LengthOfCharBuffer);
	return hr;
}

void EdLn::SetStyle(EditStyle style, bool isVisible, bool isEditable, int numChars, bool isHex)
{
	this->m_style = style;
	this->m_bIsEditable = isEditable;
	this->m_bIsVisible = isVisible;
	this->m_bIsHex = isHex;
	int m = numChars;

	if (numChars <= 0)
	{
		switch (m_style)
		{
		case Address:
			m = isHex ? 4 : 5;
			break;
		case Byte:
			m = isHex ? 2 : 3;
			break;
		case CpuFlags:
			m = 8;
			break;
		case Number:
			m = numChars;
			break;
		case DiskTrack:
			m = 4;
			break;
		default:
			m = 6;
			break;
		}
	}

	if (m <= 0)
	{
		m = 6;
	}

	if (m > this->MaxChars)
	{
		m = this->MaxChars;
	}

	this->m_iNumChars = m;
}

HRESULT EdLn::AllocTextBuffer(int i)
{
HRESULT hr = E_FAIL;
	FreeTextBuffer();
	int n = i + 1;
	do
	{
		this->szEditTextBuffer = (TCHAR *)malloc(n * sizeof(TCHAR));
		if (this->szEditTextBuffer==NULL)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		m_pTextExtents = (int *)malloc(n * sizeof(INT));
		if (m_pTextExtents==NULL)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = S_OK;
	} while (false);

	if (FAILED(hr))
	{
		FreeTextBuffer();
		return hr;
	}

	ZeroMemory(m_pTextExtents, n);
	m_iTextBufferLength = n;
	m_iNumChars = i;
	m_iTextExtents = n;	
	return S_OK;
}

void EdLn::FreeTextBuffer()
{
	if (this->szEditTextBuffer != NULL)
	{
		free(this->szEditTextBuffer);
		this->szEditTextBuffer = NULL;
	}

	if (m_pTextExtents != NULL)
	{
		free(m_pTextExtents);
		m_pTextExtents = NULL;
	}

	m_iTextBufferLength = 0;
	m_iNumChars = 0;
	m_iTextExtents = 0;
}

HRESULT EdLn::SetCaption(LPCTSTR pszCaption)
{
	if (pszCaption)
	{
		int i = lstrlen(pszCaption);
		void *p = malloc((i + 1) * sizeof(TCHAR));
		if (p == NULL)
		{
			return E_OUTOFMEMORY;
		}

		FreeCaption();
		m_pszCaption = (TCHAR *)p;
		lstrcpy(m_pszCaption, pszCaption);
	}
	else
	{
		FreeCaption();
	}

	return S_OK;
}

void EdLn::FreeCaption()
{
	if (m_pszCaption != NULL)
	{
		free(m_pszCaption);
		m_pszCaption = NULL;
	}
}

void EdLn::UpdateCaretPosition(HDC hdc)
{
HRESULT hr;
	int iCellIndex = this->GetInsertionPoint();
	POINT pt;
	hr = this->GetCharPoint(hdc, iCellIndex, NULL, &pt);
	if (SUCCEEDED(hr))
	{
		::SetCaretPos(pt.x, pt.y);
	}
}

HRESULT EdLn::CreateDefaultHitRegion(HDC hdc)
{
HRESULT hr = S_OK;

	RECT rcAll;
	RECT rcCaption;
	RECT rcEdit;
	hr = GetRects(hdc, &rcCaption, &rcEdit, &rcAll);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_hRegionHitAll==NULL)
	{
		m_hRegionHitAll = CreateRectRgnIndirect(&rcAll);
		if (m_hRegionHitAll == NULL)
		{
			hr = E_FAIL;
		}
	}
	else
	{
		SetRectRgn(m_hRegionHitAll, rcAll.left, rcAll.top, rcAll.right, rcAll.bottom);
	}

	if (m_hRegionHitEdit==NULL)
	{
		m_hRegionHitEdit = CreateRectRgnIndirect(&rcEdit);
		if (m_hRegionHitEdit == NULL)
		{
			hr = E_FAIL;
		}
	}
	else
	{
		SetRectRgn(m_hRegionHitEdit, rcEdit.left, rcEdit.top, rcEdit.right, rcEdit.bottom);
	}

	return hr;
}

bool EdLn::IsHitAll(int x, int y)
{
	if (m_hRegionHitAll==NULL)
	{
		return false;
	}

	return 0 != ::PtInRegion(m_hRegionHitAll, x, y);
}

bool EdLn::IsHitEdit(int x, int y)
{
	if (m_hRegionHitEdit==NULL)
	{
		return false;
	}

	return 0 != ::PtInRegion(m_hRegionHitEdit, x, y);
}

bool EdLn::GetIsEditable()
{
	return m_bIsEditable;
}

bool EdLn::GetIsVisible()
{
	return m_bIsVisible;
}

int EdLn::GetControlID()
{
	return m_iControlID;
}

bool EdLn::CanCharEdit(TCHAR c)
{
	switch(this->m_style)
	{
	case Address:
	case Byte:
	case Number:
		if (this->m_bIsHex)
		{
			if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c == ' '))
			{
				return true;
			}
		}
		else
		{
			if ((c >= '0' && c <= '9') || (c == ' '))
			{
				return true;
			}
		}

		break;
	case CpuFlags:
		if ((c == '0' || c == '1'))
		{
			return true;
		}

		break;
	}

	return false;
}

void EdLn::CharEdit(TCHAR c)
{
bool bChanged = false;
	switch(this->m_style)
	{
	case Address:
	case Byte:
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c == ' '))
		{
			if (c >= 'a' && c <= 'f')
			{
				c = _totupper(c);
			}

			bChanged = OverwriteCharAndUpdateInsertionPoint(c);
			Refresh();
		}

		break;
	case CpuFlags:
		if ((c == '0' || c == '1'))
		{
			if (m_iInsertionPoint == 2)
			{
				c = '1';
			}

			bChanged = OverwriteCharAndUpdateInsertionPoint(c);
			if (m_iInsertionPoint == 2)
			{
				m_iInsertionPoint++;
			}

			Refresh();
		}

		break;
	}

	if (bChanged)
	{
		this->IsChanged = true;
		EdLnTextChangedEventArgs eTextChanged(this);
		EsOnTextChanged.Raise(this, eTextChanged);
	}
}

bool EdLn::OverwriteCharAndUpdateInsertionPoint(TCHAR c)
{
bool bChanged = false;
	if (m_iNumChars <= 0)
	{
		m_iInsertionPoint = 0;
		return false;
	}

	if (m_iInsertionPoint < 0)
	{
		m_iInsertionPoint = 0;
	}

	if (m_iInsertionPoint >= m_iNumChars)
	{
		m_iInsertionPoint = m_iNumChars - 1;
	}

	if (c != this->szEditTextBuffer[m_iInsertionPoint])
	{
		this->szEditTextBuffer[m_iInsertionPoint] = c;
		bChanged = true;
	}

	m_iInsertionPoint = (m_iInsertionPoint + 1) % m_iNumChars;
	return bChanged;
}

void EdLn::KeyDown(int keycode)
{
SHORT nVirtKey;
#define SHIFTED (0x8000)
	if (keycode == VK_TAB)
	{
        nVirtKey = GetKeyState(VK_SHIFT); 
		EdLnTabControlEventArgs eTabControl(this, (nVirtKey & SHIFTED) == 0);
		EsOnTabControl.Raise(this, eTabControl);
	}
	else if (keycode == VK_LEFT)
	{
		if (m_iInsertionPoint > 0)
		{
			m_iInsertionPoint--;
		}
		else
		{
			m_iInsertionPoint = m_iNumChars - 1;
		}
	}
	else if (keycode == VK_RIGHT)
	{
		if (m_iInsertionPoint < m_iNumChars - 1)
		{
			m_iInsertionPoint++;
		}
		else
		{
			m_iInsertionPoint = 0;
		}
	}
	else if (keycode == VK_HOME)
	{
		m_iInsertionPoint = 0;
	}
	else if (keycode == VK_END)
	{
		if (m_iNumChars > 0)
		{
			m_iInsertionPoint =  m_iNumChars - 1;
		}
	}
	else if (keycode == VK_ESCAPE)
	{
		EdLnCancelControlEventArgs evt(this);
		EsOnCancelControl.Raise(this, evt); 
	}
	else if (keycode == VK_RETURN)
	{
		EdLnSaveControlEventArgs evt(this);
		EsOnSaveControl.Raise(this, evt);
	}

	this->Refresh();
}

void EdLn::Refresh()
{
	HDC hdc = GetDC(m_hWnd);
	if (hdc)
	{
		DcHelper dch(hdc);
		dch.UseMapMode(MM_TEXT);
		dch.UseFont(m_hFont);
		if (this->IsSelected)
		{
			::HideCaret(m_hWnd);
			UpdateCaretPosition(hdc);
		}

		Draw(hdc);
		if (this->IsSelected)
		{
			::ShowCaret(m_hWnd);
		}

		ReleaseDC(m_hWnd, hdc);
	}
}

HRESULT EdLn::GetCharIndex(HDC hdc, int x, int y, int *pOutCellIndex, POINT *pPos)
{
RECT rcEdit;
HRESULT hr;

	if (m_pTextExtents == NULL || m_iNumChars <=0)
	{
		return E_FAIL;
	}

	DcHelper dch(hdc);
	dch.UseMapMode(MM_TEXT);
	dch.UseFont(m_hFont);

	if (!IsHitAll(x, y))
	{
		return E_FAIL;
	}

	hr = GetRects(hdc, NULL, &rcEdit, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pPos)
	{
		pPos->x = rcEdit.left;
		pPos->y = rcEdit.top;
	}
	if (pOutCellIndex)
	{
		*pOutCellIndex = 0;
	}

	if (x <= rcEdit.left)
	{
		if (pPos)
		{
			pPos->x = rcEdit.left;
			pPos->y = rcEdit.top;
		}

		if (pOutCellIndex)
		{
			*pOutCellIndex = 0;
		}

		return S_OK;
	}

	BOOL br;
	int k;
	TCHAR *pszBuffer;
	pszBuffer = this->szEditTextBuffer;
	k = lstrlen(pszBuffer);
	if (k > m_iTextExtents)
	{
		k = m_iTextExtents;
	}

	INT iNumFit = 0;
	SIZE sizeText;
	int iWidth = abs(rcEdit.right - rcEdit.left);
	br = GetTextExtentExPoint(hdc, pszBuffer, k, iWidth, &iNumFit, m_pTextExtents, &sizeText);
	if (!br)
	{
		return E_FAIL;
	}

	if (iNumFit <= 0)
	{
		if (pPos)
		{
			pPos->x = rcEdit.left;
			pPos->y = rcEdit.top;
		}

		if (pOutCellIndex)
		{
			*pOutCellIndex = 0;
		}

		return S_OK;
	}

	int cx;
	int nextcx;
	int w = abs(rcEdit.right - rcEdit.left);
	cx = rcEdit.left;
	for (int i = 0; i < iNumFit; i++, cx = nextcx)
	{
		nextcx = rcEdit.left + m_pTextExtents[i];
		if (x < nextcx)
		{
			if (pPos)
			{
				pPos->x = cx;
				pPos->y = rcEdit.top;
			}

			if (pOutCellIndex)
			{
				*pOutCellIndex = i;
			}

			return S_OK;
		}
	}

	cx = rcEdit.left + m_pTextExtents[iNumFit - 1];
	if (x >= cx)
	{
		if (pPos)
		{
			pPos->x = cx;
			pPos->y = rcEdit.top;
		}

		if (pOutCellIndex)
		{
			*pOutCellIndex = iNumFit - 1;
		}

		return S_OK;
	}

	return S_OK;
}

HRESULT EdLn::GetCharPoint(HDC hdc, int cellIndex, int *pOutCellIndex, POINT *pPos)
{
RECT rcEdit;
HRESULT hr;

	if (m_pTextExtents == NULL || m_iNumChars <=0)
	{
		return E_FAIL;
	}

	DcHelper dch(hdc);
	dch.UseMapMode(MM_TEXT);
	dch.UseFont(m_hFont);
	hr = GetRects(hdc, NULL, &rcEdit, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pPos)
	{
		pPos->x = rcEdit.left;
		pPos->y = rcEdit.top;
	}

	if (pOutCellIndex)
	{
		*pOutCellIndex = 0;
	}

	if (cellIndex <= 0)
	{
		return S_OK;
	}

	BOOL br;
	int k;
	TCHAR *pszBuffer;
	pszBuffer = this->szEditTextBuffer;
	k = lstrlen(pszBuffer);
	if (k > m_iTextExtents)
	{
		k = m_iTextExtents;
	}

	INT numFit = 0;
	SIZE sizeText;
	int iWidth = abs(rcEdit.right - rcEdit.left);
	br = GetTextExtentExPoint(hdc, pszBuffer, k, iWidth, &numFit, m_pTextExtents, &sizeText);
	if (!br)
	{
		return E_FAIL;
	}

	int i;
	POINT p;
	if (numFit <= 0 || cellIndex <= 0)
	{
		i = 0;
		p.x = rcEdit.left;
		p.y = rcEdit.top;
	}
	else if (cellIndex < numFit)
	{
		i = cellIndex;
		p.x = rcEdit.left + m_pTextExtents[i - 1];
		p.y = rcEdit.top;
	}
	else
	{
		i = numFit - 1;
		p.x = rcEdit.left + m_pTextExtents[i];
		p.y = rcEdit.top;
	}

	if (pOutCellIndex)
	{
		*pOutCellIndex = i;
	}

	if (pPos)
	{
		*pPos = p;
	}

	return S_OK;
}

HRESULT EdLn::GetRects(HDC hdc, RECT *prcCaption, RECT *prcEdit, RECT *prcAll)
{
BOOL br;
HRESULT hr = E_FAIL;
	RECT rcCaption;
	RECT rcEdit;
	RECT rcAll;
	SIZE szCharLineBox;
	SetRectEmpty(&rcCaption);
	SetRectEmpty(&rcEdit);
	SetRectEmpty(&rcAll);
	if (hdc != NULL && m_hFont != NULL)
	{
		int prevMapMode = SetMapMode(hdc, MM_TEXT);
		if (prevMapMode)
		{
			HFONT hFontPrev = (HFONT)SelectObject(hdc, m_hFont);
			if (hFontPrev)
			{
				TEXTMETRIC tm;
				int numEditChars = m_iNumChars;
				const TCHAR *psData = this->m_szMeasureChar;
				br = GetTextMetrics(hdc, &tm);
				if (br)
				{
					if (G::GetCurrentFontLineBox(hdc, &szCharLineBox))
					{
						SIZE sizeAll;
						sizeAll.cx = sizeAll.cy = 0;
						SIZE sizeCaption = sizeAll;
						SIZE sizeEdit = sizeAll;
						int tx1;
						int tx2;
						int slen1;
						int slen2;
						if (m_pszCaption != NULL)
						{
							slen1 = lstrlen(m_pszCaption);
						}
						else
						{
							slen1 = 0;
						}

						slen2 = lstrlen(psData);
						tx1 = slen1 * tm.tmAveCharWidth;
						tx2 = numEditChars * tm.tmAveCharWidth;
						BOOL brCaption = FALSE;
						BOOL brEdit = FALSE;
						if (slen1 > 0)
						{
							brCaption = GetTextExtentExPoint(hdc, m_pszCaption, slen1, 0, NULL, NULL, &sizeCaption);
							if (brCaption)
							{
								SetRect(&rcCaption, 0, 0, sizeCaption.cx, sizeCaption.cy);
								tx1 = (sizeCaption.cx);
							}
						}

						if (numEditChars > 0)
						{
							brEdit = GetTextExtentExPoint(hdc, psData, slen2, 0, NULL, NULL, &sizeEdit);
							if (brEdit)
							{
								sizeEdit.cx = sizeEdit.cx * numEditChars;
								SetRect(&rcEdit, 0, 0, sizeEdit.cx, sizeEdit.cy);
								if (tx2 > sizeEdit.cx)
								{
									sizeEdit.cx = tx2;
								}

								if (brCaption)
								{
									OffsetRect(&rcEdit, 0, szCharLineBox.cy);
								}

								tx2 = (sizeEdit.cx);
							}
						}

						OffsetRect(&rcCaption, m_posX, m_posY);
						OffsetRect(&rcEdit, m_posX, m_posY);
						UnionRect(&rcAll, &rcCaption, &rcEdit);
						if (prcCaption)
						{
							CopyRect(prcCaption, &rcCaption);
						}

						if (prcEdit)
						{
							CopyRect(prcEdit, &rcEdit);
						}

						if (prcAll)
						{
							CopyRect(prcAll, &rcAll);
						}

						hr = S_OK;
					}
				}

				SelectObject(hdc, hFontPrev);
			}

			SetMapMode(hdc, prevMapMode);
		}
	}

	return hr;
}

HRESULT EdLn::SetPos(int x, int y)
{
	m_posX = x;
	m_posY = y;
	return S_OK;
}

int EdLn::GetXPos()
{
	return m_posX;
}

int EdLn::GetYPos()
{
	return m_posY;
}

void EdLn::Draw(HDC hdc)
{
	if (!m_bIsVisible)
	{
		return;
	}

	TCHAR *pszBuffer;
	pszBuffer = this->szEditTextBuffer;
	if (pszBuffer==NULL)
	{
		return;
	}

	RECT rcCaption;
	RECT rcEdit;
	RECT rcAll;
	if (SUCCEEDED(this->GetRects(hdc, &rcCaption, &rcEdit, &rcAll)))
	{
		int width = (int)(rcAll.right - rcAll.left);
		SIZE szFontChar;
		if (G::GetCurrentFontLineBox(hdc, &szFontChar))
		{
			SIZE szCaption;
			SIZE szEdit;
			RECT rcClip;
			int x = m_posX;
			int y = m_posY;
			if (this->m_pszCaption != NULL)
			{
				int lenCaption = lstrlen(m_pszCaption);
				GetTextExtentPoint32(hdc, m_pszCaption, lenCaption, &szCaption);
				SetRect(&rcClip, x, y, x + width, szFontChar.cy);
				SetTextAlign(hdc, TA_TOP | TA_RIGHT | TA_NOUPDATECP);
				ExtTextOut(hdc, x + width, y, ETO_NUMERICSLATIN | ETO_OPAQUE, &rcClip, m_pszCaption, lenCaption, NULL);
				y += szFontChar.cy;		
			}

			int lenEdit = (int)GetEditString(NULL, 0);
			if (lenEdit > m_iNumChars)
			{
				lenEdit = m_iNumChars;
			}

			if (lenEdit > 0)
			{
				GetTextExtentPoint32(hdc, m_pszCaption, lenEdit, &szEdit);
				SetRect(&rcClip, x, y, x + width, szFontChar.cy);
				SetTextAlign(hdc, TA_TOP | TA_RIGHT | TA_NOUPDATECP);
				ExtTextOut(hdc, x + width, y, ETO_NUMERICSLATIN | ETO_OPAQUE, &rcClip, pszBuffer, lenEdit, NULL);
			}
		}
	}
}

void EdLn::SpacePadBuffer(TCHAR *pszBuffer, int bufferSize)
{
	if (bufferSize >= this->m_iTextBufferLength)
	{
		bufferSize = this->m_iTextBufferLength;
	}

	int k = (int)_tcsnlen(pszBuffer, bufferSize);
	int i;
	for (i = k; i < bufferSize; i++)
	{
		pszBuffer[i] = TEXT(' ');
	}

	if (k >= this->m_iTextBufferLength && k > 0)
	{
		k--;
	}

	pszBuffer[k] = 0;
}

HRESULT EdLn::GetEditValue(int& v)
{
HRESULT hr=E_FAIL;
	
	switch (m_style)
	{
	case Address:
	case Byte:
	case Number:
		if (this->m_bIsHex)
		{
			hr = GetHex(v);
		}
		else
		{
			hr = GetDec(v);
		}

		break;
	case CpuFlags:
		hr = GetFlags(v);
		break;
	case DiskTrack:
		hr = GetHalfTrackIndex(v);
		break;
	}

	return hr;
}

void EdLn::SetValue(TCHAR *pszBuffer, int v)
{
	switch (m_style)
	{
	case Address:
		if (this->m_bIsHex)
		{
			SetHexAddress(pszBuffer, v);
		}
		else
		{
			SetDec(pszBuffer, v);
		}

		break;
	case Byte:
		if (this->m_bIsHex)
		{
			SetHexByte(pszBuffer, v);
		}
		else
		{
			SetDec(pszBuffer, v);
		}

		break;
	case Number:
		if (this->m_bIsHex)
		{
			SetHex(pszBuffer, v);
		}
		else
		{
			SetDec(pszBuffer, v);
		}

		break;
	case CpuFlags:
		SetFlags(pszBuffer, v);
		break;
	case DiskTrack:
		SetHalfTrackIndex(pszBuffer, v);
		break;
	}
}

void EdLn::SetEditValue(int v)
{
	TCHAR *pBuffer = this->szEditTextBuffer;
	this->SetValue(pBuffer, v);
	this->SpacePadBuffer(pBuffer, this->m_iNumChars);
}

HRESULT EdLn::GetFlags(int& v)
{
const int NUMDIGITS = 8;
	
	TCHAR *pBuffer = this->szEditTextBuffer;
	if (pBuffer == NULL)
	{
		return E_FAIL;
	}

	int i;
	int k;
	v = 0;
	for (i=0, k=0x80 ; i < m_iNumChars ; i++, k>>=1)
	{
		TCHAR c = pBuffer[i];
		if (c == 0)
		{
			break;
		}

		if (c != ' ' && c != '0')
		{
			v = v | k;
		}
	}

	return S_OK;
}

void EdLn::SetFlags(TCHAR *pszBuffer, int v)
{
const int NUMDIGITS = 8;
TCHAR szBitByte[NUMDIGITS + 1];

	if (pszBuffer == NULL || m_iNumChars < NUMDIGITS)
	{
		return ;
	}

	int i = 0;
	ZeroMemory(szBitByte, _countof(szBitByte) * sizeof(TCHAR));
	for (i=0; i < NUMDIGITS; i++)
	{
		int k = (((unsigned int)v & (1UL << (7-i))) != 0);
		szBitByte[i] = TEXT('0') + k;
	}

	szBitByte[NUMDIGITS] = 0;
	_tcsncpy_s(pszBuffer, m_iTextBufferLength, szBitByte, _TRUNCATE);
}

HRESULT EdLn::GetDec(int& v)
{
HRESULT hr = E_FAIL;
TCHAR *pBuffer = this->szEditTextBuffer;
	if (pBuffer == NULL || m_iNumChars <= 0)
	{
		return E_FAIL;
	}

	int r = _stscanf_s(pBuffer, TEXT(" %d"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}

	return S_OK;
}

void EdLn::SetDec(TCHAR *pszBuffer, int v)
{
TCHAR formatStr[30];

	if (pszBuffer == NULL || m_iNumChars <= 0)
	{
		return;
	}

	_sntprintf_s(formatStr, _countof(formatStr), _TRUNCATE, TEXT("%%%-dd"), m_iNumChars);
	_sntprintf_s(pszBuffer, m_iTextBufferLength, _TRUNCATE, formatStr, v);
}

HRESULT EdLn::GetHex(int& v)
{
HRESULT hr = E_FAIL;

TCHAR *pszBuffer = this->szEditTextBuffer;

	if (pszBuffer == NULL || m_iNumChars <= 0)
	{
		return E_FAIL;
	}

	int r = _stscanf_s(pszBuffer, TEXT(" %4x"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}

	return S_OK;
}

void EdLn::SetHex(TCHAR *pszBuffer, int v)
{
const int NUMDIGITS = 8;
TCHAR szTemp[NUMDIGITS + 1];
	if (pszBuffer == NULL)
	{
		return ;
	}

	HexConv::long_to_hex((bit32)v, szTemp, m_iNumChars);
	_tcsncpy_s(pszBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexAddress(TCHAR *pszBuffer, int v)
{
const int NUMDIGITS = 4;
TCHAR szTemp[NUMDIGITS + 1];
	if (pszBuffer == NULL || m_iNumChars < NUMDIGITS)
	{
		return ;
	}

	HexConv::long_to_hex(v & 0xffff, szTemp, NUMDIGITS);
	_tcsncpy_s(pszBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexByte(TCHAR *pszBuffer, int v)
{
const int NUMDIGITS = 2;
TCHAR szTemp[NUMDIGITS + 1];
	if (pszBuffer == NULL)
	{
		return ;
	}

	HexConv::long_to_hex(v & 0xff, szTemp, NUMDIGITS);
	_tcsncpy_s(pszBuffer, m_iTextBufferLength, szTemp, _TRUNCATE);
}

HRESULT EdLn::GetHalfTrackIndex(int& v)
{
HRESULT hr = E_FAIL;
double t;
TCHAR *pBuffer = this->szEditTextBuffer;
	v = 0;
	if (pBuffer == NULL || m_iNumChars <= 0)
	{
		return E_FAIL;
	}

	t = 0.0;
	int r = _stscanf_s(pBuffer, TEXT(" %f"), &t);
	if (r < 1)
	{
		return E_FAIL;
	}

	if (t<1.0 || t >=100.0)
	{
		return E_FAIL;
	}

	v = (int)floor ((t - 1.0) * 2.0);
	return S_OK;
}

void EdLn::SetHalfTrackIndex(TCHAR *pszBuffer, int v)
{
	if (v < 0 || v >= 100)
	{
		pszBuffer[0] = 0;
	}
	else
	{
		double t = ((double)v / 2.0) + 1.0;
		_sntprintf_s(pszBuffer, m_iTextBufferLength, _TRUNCATE, TEXT("%.1f"), t);
	}
}

size_t EdLn::GetEditString(TCHAR buffer[], int bufferSize)
{
	TCHAR *pszSourceBuffer = this->szEditTextBuffer;
	int lengthSource = m_iTextBufferLength;
	if (pszSourceBuffer == NULL)
	{
		return E_FAIL;
	}

	size_t k;
	if (buffer == NULL || bufferSize == 0)
	{
		k = _tcsnlen(pszSourceBuffer, lengthSource);
		return k;
	}
	else
	{
		k = _tcsnlen(pszSourceBuffer, lengthSource);
		if (k == 0)
		{
			buffer[0];
		}
		else if (k < (size_t)lengthSource)
		{
			// NULL terminated pszSourceBuffer.
			_tcsncpy_s(buffer, bufferSize, pszSourceBuffer, _TRUNCATE);
		}
		else 
		{
			// pszSourceBuffer is not NULL terminated.
			if (k < (size_t)bufferSize)
			{
				_tcsncpy_s(buffer, bufferSize, pszSourceBuffer, k);
			}
			else
			{
				_tcsncpy_s(buffer, bufferSize, pszSourceBuffer, bufferSize - 1);
			}
		}

		return k;
	}
}

void EdLn::SetEditString(const TCHAR *data, int count)
{
	this->SetString(this->szEditTextBuffer, data, count);
}

void EdLn::SetString(TCHAR *pszBuffer, const TCHAR *data, int count)
{
	if (pszBuffer == NULL || m_iTextBufferLength <= 0)
	{
		return;
	}

	int lengthDestBuffer = m_iTextBufferLength;
	if (data == NULL || count == 0)
	{
		pszBuffer[0] = 0;
		return;
	}

	size_t k = _tcsnlen(data, count);
	if (k == 0)
	{
		pszBuffer[0] = 0;
	}
	else if (k < (size_t)count)
	{
		//NULL terminated data
		_tcsncpy_s(pszBuffer, lengthDestBuffer, data, _TRUNCATE);
	}
	else 
	{
		//data is not NULL terminated.
		if (k < (size_t)lengthDestBuffer)
		{
			_tcsncpy_s(pszBuffer, lengthDestBuffer, data, k);
		}
		else
		{
			_tcsncpy_s(pszBuffer, lengthDestBuffer, data, lengthDestBuffer - 1);
		}
	}
}

void EdLn::SetInsertionPoint(int v)
{
	m_iInsertionPoint = v;
}

int EdLn::GetInsertionPoint()
{
	return m_iInsertionPoint;
}
