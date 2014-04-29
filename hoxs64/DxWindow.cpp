#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include "boost2005.h"
#include <tchar.h>
#include <string.h>
#include "DxWindow.h"


DxObject::DxObject()
{
}

DxObject::~DxObject()
{
	FreeDeviceObjects();
}

HRESULT DxObject::SetDevice(IDirect3DDevice9 *pd3dDevice)
{
HRESULT hr = S_OK;
	if (pd3dDevice == NULL || pd3dDevice != m_pd3dDevice)
	{		
		m_pd3dDevice = pd3dDevice;
		if (m_pd3dDevice != NULL)
		{
			m_pd3dDevice->AddRef();
			hr = LoadDeviceObjects();
			if (FAILED(hr))
				return hr;
		}			
	}
	return hr;
}

void DxObject::FreeDeviceObjects()
{
	if (m_pd3dDevice)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}
}

HRESULT DxObject::LoadDeviceObjects()
{
	return S_OK;
}

DxWindow::DxWindow()
{
	m_pd3dDevice = NULL;
	m_sprMessageText = NULL;
	m_psztext = NULL;
	m_dxfont = NULL;
	xpos = 0;
	ypos = 0;
	width = 0;
	height = 0;
	m_bIsLost = false;
}

void DxWindow::OnLostDevice()
{
	if (!m_bIsLost)
	{
		m_bIsLost = true;
		if (m_dxfont != NULL)
			m_dxfont->OnLostDevice();
		if (m_sprMessageText != NULL)
			m_sprMessageText->OnLostDevice();
	}
}

void DxWindow::OnResetDevice()
{
	m_bIsLost = false;
	if (m_dxfont != NULL)
		m_dxfont->OnResetDevice();
	if (m_sprMessageText != NULL)
		m_sprMessageText->OnResetDevice();
}

HRESULT DxWindow::LoadDeviceObjects()
{
HRESULT hr;
	hr = DxObject::LoadDeviceObjects();
	if (FAILED(hr))
		return hr;
	LPD3DXSPRITE sprMessageText = NULL;
	do
	{
		if (m_pd3dDevice == NULL)
		{
			hr = E_POINTER;
			break;
		}
		hr = D3DXCreateSprite(m_pd3dDevice, &sprMessageText);
		if (FAILED(hr))
			break;
		hr = S_OK;
	} while (false);
	if (SUCCEEDED(hr))
	{
		if (m_sprMessageText!=NULL)
			m_sprMessageText->Release();
		m_sprMessageText = sprMessageText;
	}
	else
	{
		if (sprMessageText!=NULL)
		{
			sprMessageText->Release();
		}
	}
	return hr;
}

DxWindow::~DxWindow()
{
	FreeDeviceObjects();
}

void DxWindow::FreeDeviceObjects()
{
	if (m_sprMessageText != NULL)
	{
		m_sprMessageText->Release();
		m_sprMessageText = NULL;
	}
	if (m_dxfont != NULL)
	{
		m_dxfont->Release();
		m_dxfont = NULL;
	}
}

HRESULT DxWindow::SetText(LPCTSTR psztext)
{
	LPTSTR p = _tcsdup(psztext);
	if (!p)
		return E_OUTOFMEMORY;
	if (m_psztext)
		free(m_psztext);
	m_psztext = p;
	return S_OK;
}

HRESULT DxWindow::GetText(LPTSTR *ppsztext)
{
	if (!ppsztext)
		return E_POINTER;
	*ppsztext = m_psztext;
	return S_OK;
}

HRESULT DxWindow::GetTextRect(RECT *prc)
{
HRESULT hr;
	if (!prc || !m_psztext || !m_sprMessageText || !m_dxfont)
		return E_POINTER;
	hr = m_dxfont->DrawText(m_sprMessageText, m_psztext, -1, prc, DT_CALCRECT, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
	return hr;
}


HRESULT DxWindow::SetFont(LPCTSTR fontname, INT fontsize)
{
LPD3DXFONT dxfont;
	HRESULT hr = D3DXCreateFont(this->m_pd3dDevice, fontsize, 0, FW_NORMAL, 1, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("ARIEL"), &dxfont);		
	if (SUCCEEDED(hr))
	{
		if (m_dxfont != NULL)
		{
			m_dxfont->Release();
			m_dxfont = NULL;
		}
		m_dxfont = dxfont;
	}
	return hr;
}

HRESULT DxWindow::SetFont(LPD3DXFONT dxfont)
{
	if (m_dxfont != NULL)
	{
		m_dxfont->Release();
		m_dxfont = NULL;
	}
	m_dxfont = dxfont;
	if (m_dxfont)
	{
		m_dxfont->AddRef();
	}
	return S_OK;
}

LPD3DXFONT DxWindow::GetFont()
{
	if (m_dxfont)
	{
		m_dxfont->AddRef();
	}
	return m_dxfont;
}

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


DxTexture::DxTexture()
{
	m_resourceid = 0;
	m_ptx = NULL;
}

DxTexture::~DxTexture()
{
}

void DxTexture::FreeDeviceObjects()
{
	if (m_ptx!=NULL)
	{
		m_ptx->Release();
		m_ptx = NULL;
	}
}

HRESULT DxTexture::LoadDeviceObjects()
{
	return S_OK;
}

void DxTexture::OnLostDevice()
{
}
void DxTexture::OnResetDevice()
{
}
void DxTexture::SetResourceId()
{
}

