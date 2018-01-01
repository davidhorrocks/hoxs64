#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "besttextwidth.h"

BestTextWidthDC::BestTextWidthDC(HDC hdc)
	: hdc(hdc)
{
	tm_ok = false;
	oldfont = NULL;
	InitSizes();
	Reset();
	if (hdc)
	{
		if (GetTextMetrics(hdc, &tm))
		{
			tm_ok = true;
		}
	}
}

BestTextWidthDC::~BestTextWidthDC()
{
	Clean();
}

void BestTextWidthDC::InitSizes()
{
	comboBoxPaddingX = 2 * GetSystemMetrics(SM_CYFIXEDFRAME);
	comboBoxPaddingX += GetSystemMetrics(SM_CXVSCROLL);
}

void BestTextWidthDC::SetFont(HFONT font)
{
	if (hdc)
	{
		HFONT prev = (HFONT)SelectObject(hdc, font);
		if (oldfont == NULL)
		{
			oldfont = prev;
		}
	}
}

void BestTextWidthDC::Reset()
{
	maxWidth = 0;
}

int BestTextWidthDC::GetWidth(LPCTSTR s)
{
	if (hdc != NULL && s != NULL)
	{
		SIZE sztext;
		if (GetTextExtentPoint32(hdc, s, lstrlen(s), &sztext))
		{
			if (sztext.cx > maxWidth)
			{
				maxWidth = sztext.cx;
			}			

			return sztext.cx;
		}
	}

	return 0;
}

void BestTextWidthDC::Clean()	
{
	if (hdc)
	{
		if (oldfont)
		{
			SelectObject(hdc, oldfont);
			oldfont = NULL;
		}

		hdc = NULL;
	}
}

int BestTextWidthDC::GetSuggestedDlgComboBoxWidth(HWND hDialog)
{
	if (this->hdc != NULL)
	{
		if (this->maxWidth > 0)
		{
			RECT rcMaxText;
			int h;
			if (this->tm_ok)
			{
				h = tm.tmHeight;
			}
			else
			{
				h = 1;
			}

			SetRect(&rcMaxText, 0, 0, this->maxWidth + 1, h);
			if (MapDialogRect(hDialog, &rcMaxText))
			{
				return rcMaxText.right - rcMaxText.left + this->comboBoxPaddingX;
			}
		}
	}

	return 0;
}
