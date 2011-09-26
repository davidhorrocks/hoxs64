#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string>

#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
#include "register.h"
#include "errormsg.h"

#include "assert.h"
#include "hexconv.h"
#include "cevent.h"
//include "monitor.h"
#include "edln.h"
#include "resource.h"


const TCHAR EdLn::m_szMeasureAddress[] = TEXT("0000");
const TCHAR EdLn::m_szMeasureByte[] = TEXT("00");
const TCHAR EdLn::m_szMeasureCpuFlags[] = TEXT("NV-BDIZC");
const TCHAR EdLn::m_szMeasureDigit[] = TEXT("0");

EdLn::EdLn()
{
	m_iValue = 0;
	m_TextBufferLength = 0;
	m_TextBuffer = NULL;
	m_iNumChars = 0;
	m_pszCaption = 0;
}

EdLn::~EdLn()
{
	FreeTextBuffer();
	FreeCaption();
}

HRESULT EdLn::Init(HFONT hFont, LPCTSTR pszCaption, EditStyle style, bool isEditable, int numChars)
{
HRESULT hr = E_FAIL;
	this->m_style = style;
	this->m_bIsEditable = isEditable;
	this->m_iNumChars = numChars;
	this->m_hFont = hFont;
	this->m_MinSizeDone = false;

	int m = 1;
	switch (m_style)
	{
	case HexAddress:
		m = 4;
		break;
	case HexByte:
		m = 2;
		break;
	case CpuFlags:
		m = 8;
		break;
	case Hex:
	case Dec:
		m = numChars;
		break;
	default:
		m = numChars;
		break;
	}

	if (m==0)
		return E_FAIL;

	hr = AllocTextBuffer(m);
	if (FAILED(hr))
		return hr;
	hr = SetCaption(pszCaption);
	if (FAILED(hr))
		return hr;
	return hr;
}

HRESULT EdLn::AllocTextBuffer(int i)
{
	void *p = malloc((i + 1) * sizeof(TCHAR));
	if (p==NULL)
		return E_OUTOFMEMORY;
	FreeTextBuffer();
	m_TextBuffer = (TCHAR *)p;
	m_TextBufferLength = i + 1;
	m_iNumChars = i;
	return S_OK;
}

void EdLn::FreeTextBuffer()
{
	if (m_TextBuffer != NULL)
	{
		free(m_TextBuffer);
		m_TextBuffer = NULL;
		m_TextBufferLength = 0;
		m_iNumChars = 0;
	}
}

HRESULT EdLn::SetCaption(LPCTSTR pszCaption)
{
	if (pszCaption)
	{
		int i = lstrlen(pszCaption);
		void *p = malloc((i + 1) * sizeof(TCHAR));
		if (p == NULL)
			return E_OUTOFMEMORY;
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

HRESULT EdLn::GetMinWindowSize(HDC hdc, int &w, int &h)
{
BOOL br;
HRESULT hr = E_FAIL;
	w = PADDING_LEFT + PADDING_RIGHT;
	h = MARGIN_TOP + PADDING_TOP + PADDING_BOTTOM;
	if (hdc != NULL && m_hFont != NULL)
	{
		int prevMapMode = SetMapMode(hdc, MM_TEXT);
		if (prevMapMode)
		{
			HFONT hFontPrev = (HFONT)SelectObject(hdc, m_hFont);
			if (hFontPrev)
			{
				TEXTMETRIC tm;
				br = GetTextMetrics(hdc, &tm);
				if (br)
				{
					const TCHAR *psData;
					int m = 1;
					switch (m_style)
					{
					case HexAddress:
						psData = m_szMeasureAddress;
						m = 4;
						break;
					case HexByte:
						psData = m_szMeasureByte;
						m = 2;
						break;
					case CpuFlags:
						psData = m_szMeasureCpuFlags;
						m = 8;
						break;
					case Hex:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					case Dec:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					default:
						psData = m_szMeasureDigit;
						m = m_iNumChars;
						break;
					}

					SIZE sizeText;
					BOOL br;
					int tx1;
					int tx2;
					int slen1;
					int slen2;
					if (m_pszCaption!=NULL)
						slen1 = lstrlen(m_pszCaption);
					else
						slen1 = 0;
					slen2 = lstrlen(psData);
					tx1 = slen1 * tm.tmAveCharWidth;
					tx2 = m * tm.tmAveCharWidth;
					if (slen1>0)
					{
						br = GetTextExtentExPoint(hdc, m_pszCaption, slen1, 0, NULL, NULL, &sizeText);
						if (br)
						{
							tx1 = (sizeText.cx);
						}
					}
					br = GetTextExtentExPoint(hdc, psData, slen2, 0, NULL, NULL, &sizeText);
					if (br)
					{
						tx2 = (sizeText.cx);
					}
						
					if (tx2 > tx1)
						w += tx2;
					else
						w += tx1;

					hr = S_OK;

					if (m_pszCaption!=NULL)
						h += tm.tmHeight * 2;
					else
						h += tm.tmHeight * 1;
					m_MinSizeW = w;
					m_MinSizeH = h;
					m_MinSizeDone = true;
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
	if (m_TextBuffer==NULL)
		return;
	int k = (int)GetString(NULL, 0);

	int iHeight = 10;
	TEXTMETRIC tm;
	BOOL br = GetTextMetrics(hdc, &tm);
	if (br)
	{
		iHeight = tm.tmHeight;
	}
	if (this->m_pszCaption != NULL)
	{
		int lenCaption = lstrlen(m_pszCaption);

		G::DrawDefText(hdc, m_posX, m_posY, m_pszCaption, lenCaption, NULL, NULL);
		G::DrawDefText(hdc, m_posX, m_posY + iHeight, m_TextBuffer, k, NULL, NULL);
	}
	else
	{
		G::DrawDefText(hdc, m_posX, m_posY, m_TextBuffer, k, NULL, NULL);
	}
}


HRESULT EdLn::GetValue(int& v)
{
HRESULT hr=E_FAIL;
	if (m_TextBuffer == NULL)
		return E_FAIL;

	switch (m_style)
	{
	case HexAddress:
		hr = GetHex(v);
		break;
	case HexByte:
		hr = GetHex(v);
		break;
	case CpuFlags:
		hr = GetFlags(v);
		break;
	case Hex:
		hr = GetHex(v);
		break;
	case Dec:
		hr = GetDec(v);
		break;
	}
	return hr;
}


void EdLn::SetValue(int v)
{
	switch (m_style)
	{
	case HexAddress:
		SetHexAddress(v);
		break;
	case HexByte:
		SetHexByte(v);
		break;
	case CpuFlags:
		SetFlags(v);
		break;
	case Hex:
		SetHex(v);
		break;
	case Dec:
		SetDec(v);
		break;
	}
}

HRESULT EdLn::GetFlags(int& v)
{
const int NUMDIGITS = 8;
	if (m_TextBuffer == NULL)
		return E_FAIL;
	int i;
	int k;
	v = 0;
	for (i=0, k=0x80 ; i < m_iNumChars ; i++, k>>=1)
	{
		TCHAR c = m_TextBuffer[i];
		if (c == 0)
			break;
		if (c != ' ' && c != '0')
		{
			v = v | k;
		}
	}
	return S_OK;
}

void EdLn::SetFlags(int v)
{
TCHAR szBitByte[9];
const int NUMDIGITS = 8;

	if (m_TextBuffer == NULL || m_iNumChars < NUMDIGITS)
		return ;
	int i = 0;
	m_TextBuffer[0] = 0;
	ZeroMemory(szBitByte, _countof(szBitByte) * sizeof(TCHAR));
	for (i=0; i < _countof(szBitByte) && i < NUMDIGITS; i++)
	{
		int k = (((unsigned int)v & (1UL << (7-i))) != 0);
		szBitByte[i] = TEXT('0') + k;
	}
	_tcsncpy_s(m_TextBuffer, m_TextBufferLength, szBitByte, _TRUNCATE);
}

HRESULT EdLn::GetHex(int& v)
{
HRESULT hr = E_FAIL;

	if (m_TextBuffer == NULL || m_iNumChars <= 0)
		return E_FAIL;

	int r = _stscanf_s(m_TextBuffer, TEXT(" %x"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}
	return S_OK;
}

void EdLn::SetHex(int v)
{
TCHAR szTemp[9];
const int NUMDIGITS = 8;
	if (m_TextBuffer == NULL)
		return ;
	HexConv::long_to_hex((bit32)v, szTemp, m_iNumChars);
	_tcsncpy_s(m_TextBuffer, m_TextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexAddress(int v)
{
TCHAR szTemp[5];
const int NUMDIGITS = 4;
	if (m_TextBuffer == NULL || m_iNumChars < NUMDIGITS)
		return ;
	HexConv::long_to_hex(v & 0xffff, szTemp, NUMDIGITS);
	_tcsncpy_s(m_TextBuffer, m_TextBufferLength, szTemp, _TRUNCATE);
}

void EdLn::SetHexByte(int v)
{
TCHAR szTemp[3];
const int NUMDIGITS = 2;
	if (m_TextBuffer == NULL)
		return ;
	HexConv::long_to_hex(v & 0xff, szTemp, NUMDIGITS);
	_tcsncpy_s(m_TextBuffer, m_TextBufferLength, szTemp, _TRUNCATE);
}


HRESULT EdLn::GetDec(int& v)
{
HRESULT hr = E_FAIL;

	if (m_TextBuffer == NULL || m_iNumChars <= 0)
		return E_FAIL;

	int r = _stscanf_s(m_TextBuffer, TEXT(" %d"), &v);
	if (r < 1)
	{
		v = 0;
		return E_FAIL;
	}
	return S_OK;
}

void EdLn::SetDec(int v)
{
	_sntprintf_s(m_TextBuffer, m_TextBufferLength, _TRUNCATE, TEXT("%d"), v);
}

size_t EdLn::GetString(TCHAR buffer[], int bufferSize)
{
	if (m_TextBuffer == NULL)
		return E_FAIL;
	size_t k;
	if (buffer == NULL || bufferSize == 0)
	{
		k = _tcsnlen(m_TextBuffer, m_TextBufferLength);
		return k;
	}
	else
	{
		k = _tcsnlen(m_TextBuffer, m_TextBufferLength);
		if (k == 0)
		{
			buffer[0];
		}
		else if (k < m_TextBufferLength)
		{
			//NULL terminated m_TextBuffer
			_tcsncpy_s(buffer, bufferSize, m_TextBuffer, _TRUNCATE);
		}
		else 
		{
			//m_TextBuffer is not NULL terminated.
			if (k < bufferSize)
			{
				_tcsncpy_s(buffer, bufferSize, m_TextBuffer, k);
			}
			else
			{
				_tcsncpy_s(buffer, bufferSize, m_TextBuffer, bufferSize - 1);
			}
		}
		return k;
	}
}

void EdLn::SetString(const TCHAR *data, int count)
{
	if (m_TextBuffer == NULL || m_TextBufferLength <= 0)
		return;
	if (data == NULL || count == 0)
	{
		m_TextBuffer[0] = 0;
		return;
	}

	size_t k = _tcsnlen(data, count);
	if (k == 0)
	{
		m_TextBuffer[0] = 0;
	}
	else if (k < count)
	{
		//NULL terminated data
		_tcsncpy_s(m_TextBuffer, m_TextBufferLength, data, _TRUNCATE);
	}
	else 
	{
		//data is not NULL terminated.
		if (k < m_TextBufferLength)
		{
			_tcsncpy_s(m_TextBuffer, m_TextBufferLength, data, k);
		}
		else
		{
			_tcsncpy_s(m_TextBuffer, m_TextBufferLength, data, m_TextBufferLength - 1);
		}
	}
}