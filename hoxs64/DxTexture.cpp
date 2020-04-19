#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "dx_version.h"
#include "boost2005.h"
#include <tchar.h>
#include <string.h>
#include "DxWindow.h"

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
