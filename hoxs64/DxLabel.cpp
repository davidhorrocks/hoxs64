#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "dx_version.h"
#include "boost2005.h"
#include <tchar.h>
#include <string.h>
#include "DxWindow.h"

DxLabel::DxLabel()
{
}

DxLabel::~DxLabel()
{
}

void DxLabel::Draw()
{
HRESULT hr;
	if (m_sprMessageText != NULL)
	{
		if (SUCCEEDED(hr = m_sprMessageText->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE)))
		{
			RECT rc;
			if (SUCCEEDED(GetTextRect(&rc)))
			{
				SetRect(&rc, xpos, ypos, 0, 0);
				m_dxfont->DrawText(m_sprMessageText, m_psztext, -1, &rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
			}
			m_sprMessageText->End();
		}
	}
}
