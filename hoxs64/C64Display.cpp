#include "vicpixelbuffer.h"
#include "C64Display.h"

HRESULT C64Display::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d, ID3D11SamplerState* pointSamplerState, ID3D11SamplerState* linearSamplerState, HWND hWnd, unsigned int width, unsigned int height, bool bWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter, IC64* c64, CAppStatus* appStatus)
{
	HRESULT hr;
	this->device = device;
	this->deviceContext = deviceContext;
	this->hWnd = hWnd;
	this->c64 = c64;
	this->appStatus = appStatus;
	this->isInitOK = false;
	this->cb_vs_vertexshader_2d = &cb_vs_vertexshader_2d;
	this->pointSamplerState = pointSamplerState;
	this->linearSamplerState = linearSamplerState;
	std::vector<Vertex2D> vertexData =
	{
		Vertex2D(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f), //TopLeft
		Vertex2D(0.5f, -0.5f, 0.0f, 1.0f, 0.0f), //TopRight
		Vertex2D(-0.5, 0.5, 0.0f, 0.0f, 1.0f), //Bottom Left
		Vertex2D(0.5, 0.5, 0.0f, 1.0f, 1.0f), //Bottom Right
	};

	std::vector<DWORD> indexData =
	{
		0, 1, 2,
		2, 1, 3
	};

	constexpr float widthLed = 8.0f;
	constexpr float heightLed = 8.0f;
	this->screenPosition.SetPosition(0.0f, 0.0f, 0.0f);
	this->screenPosition.SetRotation(0.0f, 0.0f, 0.0f);
	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	hr = vertices.Initialize(device, vertexData.data(), (UINT)vertexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for C64Display.");

	hr = indices.Initialize(device, indexData.data(), (UINT)indexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for C64Display.");

	this->spriteToolbarBackground.Initialize(device, deviceContext, widthLed, heightLed, Color(0, 0, 0), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedMotorOn.Initialize(device, deviceContext, widthLed, heightLed, Color(0, 255, 0), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedMotorOff.Initialize(device, deviceContext, widthLed, heightLed, Color(0, 64, 64), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedDriveOn.Initialize(device, deviceContext, widthLed, heightLed, Color(0, 128, 255), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedDriveOff.Initialize(device, deviceContext, widthLed, heightLed, Color(19, 21, 83), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedWriteOn.Initialize(device, deviceContext, widthLed, heightLed, Color(255, 0, 0), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteLedWriteOff.Initialize(device, deviceContext, widthLed, heightLed, Color(62, 14, 13), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	this->spriteVicCursorBar.Initialize(device, deviceContext, 8, 1, Color(200, 200, 200), aiTextureType::aiTextureType_DIFFUSE, cb_vs_vertexshader_2d);
	hr = this->SetRenderStyle(width, height, bWindowedMode, borderSize, bShowFloppyLed, stretch, bUsePointFilter);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "SetRenderStyle failed.");
		return hr;
	}

	isInitOK = true;
	return hr;
}

void C64Display::Cleanup()
{
	this->ReleaseSamplerState();
	this->screenTexture.Cleanup();
	this->spriteToolbarBackground.Cleanup();
	this->spriteLedMotorOn.Cleanup();
	this->spriteLedMotorOff.Cleanup();
	this->spriteLedDriveOn.Cleanup();
	this->spriteLedDriveOff.Cleanup();
	this->spriteLedWriteOn.Cleanup();
	this->spriteLedWriteOff.Cleanup();
	this->spriteVicCursorBar.Cleanup();
	this->vertices.Cleanup();
	this->indices.Cleanup();
	if (this->deviceContext != nullptr)
	{
		this->deviceContext->ClearState();
		this->deviceContext.Reset();
	}

	if (this->device != nullptr)
	{
		this->device.Reset();
	}
}

void C64Display::SetSamplerState(ID3D11SamplerState* point, ID3D11SamplerState* linear)
{
	pointSamplerState = point;
	linearSamplerState = linear;
}

void C64Display::ReleaseSamplerState()
{
	pointSamplerState.Reset();
	linearSamplerState.Reset();
}

void C64Display::Draw(const XMMATRIX& orthoMatrix)
{
	if (this->bScreenOk)
	{
		if (bUsePointFilter)
		{
			if (this->pointSamplerState != nullptr)
			{
				deviceContext->PSSetSamplers(0, 1, pointSamplerState.GetAddressOf());
			}
		}
		else
		{
			if (this->linearSamplerState != nullptr)
			{
				deviceContext->PSSetSamplers(0, 1, linearSamplerState.GetAddressOf());
			}
		}

		XMMATRIX wvpMatrix = this->screenPosition.GetWorldMatrix() * orthoMatrix;
		deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader_2d->GetAddressOf());
		cb_vs_vertexshader_2d->data.wvpMatrix = wvpMatrix;
		cb_vs_vertexshader_2d->ApplyChanges();
		deviceContext->PSSetShaderResources(0, 1, screenTexture.GetTextureResourceViewAddress());
		const UINT offsets = 0;
		deviceContext->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), vertices.StridePtr(), &offsets);
		deviceContext->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexed(indices.IndexCount(), 0, 0);
	}

	if (this->bStatusBarOk)
	{
		this->spriteToolbarBackground.Draw(orthoMatrix);
		if (this->appStatus->m_bDiskLedMotor)
		{
			this->spriteLedMotorOn.Draw(orthoMatrix);
		}
		else
		{
			this->spriteLedMotorOff.Draw(orthoMatrix);
		}

		if (this->appStatus->m_bDiskLedDrive)
		{
			this->spriteLedDriveOn.Draw(orthoMatrix);
		}
		else
		{
			this->spriteLedDriveOff.Draw(orthoMatrix);
		}

		if (this->appStatus->m_bDiskLedWrite)
		{
			this->spriteLedWriteOn.Draw(orthoMatrix);
		}
		else
		{
			this->spriteLedWriteOff.Draw(orthoMatrix);
		}
	}

	if (this->enableVicCursorSprites)
	{
		for (vector<VicCursorPosition>::iterator it = listVicCursorPosition.begin(); it != listVicCursorPosition.end(); it++)
		{
			VicCursorPosition& cursorpos = *it;
			DrawCursorAtVicPosition(orthoMatrix, cursorpos.VicCycle, cursorpos.VicLine);
		}
	}
}

void C64Display::UpdateMatrix()
{
	worldMatrixTotalArea = XMMatrixScaling(scale.x, scale.y, 1.0f) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x + scale.x / 2.0f, pos.y + scale.y / 2.0f, pos.z);
}

float C64Display::GetWidth()
{
	return scale.x;
}

float C64Display::GetHeight()
{
	return scale.y;
}

void C64Display::SetPixelFilter(bool usePointFilter)
{
	this->bUsePointFilter = usePointFilter;
}

bool C64Display::CanMode1X(UINT Width, UINT Height, const C64WindowDimensions& dims, BOOL bShowFloppyLed)
{
	if (Width < (UINT)dims.Width * 1 || Height < (UINT)(dims.Height * 1 + GetToolBarHeight(bShowFloppyLed, false, Height, nullptr, nullptr)))
		return false;
	else
		return true;
}

bool C64Display::CanMode2X(UINT Width, UINT Height, const C64WindowDimensions& dims, BOOL bShowFloppyLed)
{
	if (Width < (UINT)dims.Width * 2 || Height < (UINT)(dims.Height * 2 + GetToolBarHeight(bShowFloppyLed, false, Height, nullptr, nullptr)))
		return false;
	else
		return true;
}

int C64Display::GetToolBarHeight(bool bShowFloppyLed)
{
	return GetToolBarHeight(bShowFloppyLed, true, 0, nullptr, nullptr);
}

int C64Display::GetToolBarHeight(bool bShowFloppyLed, bool bWindowedMode, int displayHeight, int* pLedHeight, int *pLedSpacing)
{
	if (pLedHeight != nullptr)
	{
		*pLedHeight = 0;
	}

	if (pLedSpacing != nullptr)
	{
		*pLedSpacing = 0;
	}

	if (bShowFloppyLed)
	{
		if (bWindowedMode)
		{
			if (pLedHeight != nullptr)
			{
				*pLedHeight = defaultLedHeight;
			}

			if (pLedSpacing != nullptr)
			{
				*pLedSpacing = defaultLedSpacing;
			}

			return defaultToolbarHeight;
		}
		else
		{
			int heightToolbar = defaultToolbarHeight;
			int heightLed = defaultLedHeight;
			int spacingLed = defaultLedSpacing;
			if (displayHeight >= screenDefaultHeight)
			{
				double scaleFactor = (double)displayHeight / screenDefaultHeight;
				int scaledHeightToolbar = (int)ceil(scaleFactor * (double)defaultToolbarHeight);
				heightToolbar = scaledHeightToolbar;
				spacingLed = (int)ceil(scaleFactor * (double)defaultLedSpacing);
				heightLed = heightToolbar - 2 * defaultToolbarPadding;
			}

			if (pLedHeight != nullptr)
			{
				*pLedHeight = heightLed;
			}

			if (pLedSpacing != nullptr)
			{
				*pLedSpacing = spacingLed;
			}

			return heightToolbar;
		}
	}
	else
	{
		return 0;
	}
}

void C64Display::SetDisplaySize(bool isWindowedMode, unsigned int width, unsigned int height)
{
	this->isWindowedMode = isWindowedMode;
	int heightLed = 0;
	int spacingLed = 0;
	bStatusBarOk = false;
	drcEraseRects.clear();
	int heightToolbar = GetToolBarHeight(bShowFloppyLed, isWindowedMode, height, &heightLed, &spacingLed);
	if (isWindowedMode)
	{	
		SetRect(&drcScreen, 0, 0, width, height);

		//Place toolbar at the bottom
		drcStatusBar.left = 0;
		drcStatusBar.right = width;
		drcStatusBar.top = height - heightToolbar;
		drcStatusBar.bottom = height;
		if (heightToolbar > 0)
		{
			if (height >= (unsigned int)heightToolbar)
			{
				// Reduce the screen size to fit the toolbar
				drcScreen.bottom -= heightToolbar;
			}

			drcEraseRects.push_back(drcStatusBar);
		}

		this->SetScale(static_cast<float>(width), static_cast<float>(height));
		this->screenPosition.SetScale(static_cast<float>(drcScreen.right - drcScreen.left), static_cast<float>(drcScreen.bottom - drcScreen.top));
		this->screenPosition.SetPosition(0.0f, 0.0f, 0.0f);
	}
	else
	{
		C64WindowDimensions dims;
		if (stretch == HCFG::EMUWINSTR_1X)
		{
			dims.SetBorder(borderSize);
			CalcClearingRects(width, height, dims, 1, bShowFloppyLed, drcScreen, drcEraseRects, drcStatusBar);
		}
		else if (stretch == HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP)
		{
			dims.SetBorder2(width, height - heightToolbar);
			CalcStretchToFitClearingRects(width, height, dims, bShowFloppyLed, drcScreen, drcEraseRects, drcStatusBar);
		}
		else
		{
			dims.SetBorder(borderSize);
			CalcStretchToFitClearingRects(width, height, dims, bShowFloppyLed, drcScreen, drcEraseRects, drcStatusBar);
		}

		this->SetScale(static_cast<float>(width), static_cast<float>(height));
		this->screenPosition.SetScale(static_cast<float>(drcScreen.right - drcScreen.left), static_cast<float>(drcScreen.bottom - drcScreen.top));
		this->screenPosition.SetPosition(static_cast<float>(drcScreen.left), static_cast<float>(drcScreen.top), 0.0f);
	}

	if (heightToolbar > 0)
	{
		bStatusBarOk = true;
		// Toolbar background
		spriteToolbarBackground.SetPosition(DirectX::XMFLOAT3((float)drcStatusBar.left, (float)drcStatusBar.top, 1.0f));
		spriteToolbarBackground.SetScale((float)(drcStatusBar.right - drcStatusBar.left), (float)(drcStatusBar.bottom - drcStatusBar.top), 1.0f);

		int ledHeight = heightToolbar - 2 * defaultToolbarPadding;
		if (ledHeight < 0)
		{
			ledHeight = 1;
		}

		int ledWidth = ledHeight;

		// Toolbar items
		float ledYpos = (float)(drcStatusBar.top + defaultToolbarPadding);
		float ledXPos = (float)drcStatusBar.left;

		// Motor LED
		spriteLedMotorOn.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedMotorOn.SetScale((float)ledWidth, (float)ledHeight, 1.0f);
		spriteLedMotorOff.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedMotorOff.SetScale((float)ledWidth, (float)ledHeight, 1.0f);

		// Drive programmable LED
		ledXPos += (float)(spacingLed + ledWidth);
		spriteLedDriveOn.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedDriveOn.SetScale((float)ledWidth, (float)ledHeight, 1.0f);
		spriteLedDriveOff.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedDriveOff.SetScale((float)ledWidth, (float)ledHeight, 1.0f);

		// Drive write activity LED
		ledXPos += (float)(spacingLed + ledWidth);
		spriteLedWriteOn.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedWriteOn.SetScale((float)ledWidth, (float)ledHeight, 1.0f);
		spriteLedWriteOff.SetPosition(DirectX::XMFLOAT3(ledXPos, ledYpos, 1.0f));
		spriteLedWriteOff.SetScale((float)ledWidth, (float)ledHeight, 1.0f);
	}
}

HRESULT C64Display::SetRenderStyle(unsigned int width, unsigned int height, bool isWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter)
{
	HRESULT hr = E_FAIL;	
	this->width = width;
	this->height = height;
	this->isWindowedMode = isWindowedMode;
	this->borderSize = borderSize;
	this->bShowFloppyLed = bShowFloppyLed;
	this->stretch = stretch;
	this->bUsePointFilter = bUsePointFilter;
	drcEraseRects.clear();
	bScreenOk = false;
	bStatusBarOk = false;
	SetRect(&drcScreen, 0, 0, 0, 0);
	SetRect(&drcStatusBar, 0, 0, 0, 0);

	int heightLed = 0;
	int spacingLed = 0;
	int heightToolbar = GetToolBarHeight(bShowFloppyLed, isWindowedMode, height, &heightLed, &spacingLed);
	hr = E_FAIL;
	if (isWindowedMode)
	{
		C64WindowDimensions dims;
		dims.SetBorder(borderSize);
		displayWidth = dims.Width;
		displayHeight = dims.Height;
		displayFirstVicRaster = dims.FirstRasterLine;
		displayLastVicRaster = dims.LastRasterLine;
		displayStart = dims.Start;
		if (hWnd)
		{
			RECT rcWindow;
			if (GetClientRect(hWnd, &rcWindow))
			{
				if (IsValidRect(rcWindow))
				{
					SetDisplaySize(isWindowedMode, rcWindow.right, rcWindow.bottom);
					hr = this->screenTexture.ResizeOrKeep(device.Get(), dims.Width, dims.Height);
					if (SUCCEEDED(hr))
					{
						bScreenOk = true;
					}
				}
			}
		}
		
	}
	else //Fullscreen
	{
		if (width == 0 || height == 0)
		{
			width = this->width;
			height = this->height;
		}

		if (width < 320 || height < 200)
		{
			return E_FAIL;
		}

		C64WindowDimensions dims;
		if (stretch == HCFG::EMUWINSTR_1X)
		{
			dims.SetBorder(borderSize);
		}
		else if (stretch == HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP)
		{
			dims.SetBorder2(width, height - heightToolbar);
		}
		else
		{
			// Aspect stretch
			//HCFG::EMUWINSTR_ASPECTSTRETCH
			dims.SetBorder(borderSize);
		}

		displayFirstVicRaster = dims.FirstRasterLine;
		displayLastVicRaster = dims.LastRasterLine;
		displayWidth = dims.Width;
		displayHeight = dims.Height;
		displayStart = dims.Start;
		hr = this->screenTexture.ResizeOrKeep(device.Get(), dims.Width, dims.Height);
		if (SUCCEEDED(hr))
		{
			bScreenOk = true;
		}

		SetDisplaySize(isWindowedMode, width, height);
	}

	return hr;
}

bool C64Display::IsValidRect(const D3D11_RECT& rc)
{
	if (rc.left < rc.right && rc.top < rc.bottom)
	{
		return true;
	}
	else
	{ 
		return false;
	}
}

void C64Display::CalcStretchToFitClearingRects(int modeWidth, int modeHeight, const C64WindowDimensions& dims, bool bShowFloppyLed, RECT& rcTargetRect, std::vector<D3D11_RECT>& drcEraseRects, D3D11_RECT& drcStatusBar)
{
	double c64ratio, screenratio;
	c64ratio = (double)dims.Width / (double)dims.Height;
	int toolBarHeight = GetToolBarHeight(bShowFloppyLed, false, modeHeight, nullptr, nullptr);
	int modeHeightRemaining;
	if (modeHeight > toolBarHeight)
	{
		modeHeightRemaining = modeHeight - toolBarHeight;
		screenratio = (double)modeWidth / (double)modeHeightRemaining;
	}
	else
	{
		screenratio = (double)modeWidth;
		modeHeightRemaining = 0;
	}

	if (c64ratio <= screenratio)
	{
		rcTargetRect.top = 0;
		rcTargetRect.bottom = modeHeightRemaining;
		rcTargetRect.left = (modeWidth - ((DWORD)(LONG)(c64ratio * ((double)(modeHeightRemaining))))) / 2L;
		rcTargetRect.right = modeWidth - rcTargetRect.left;
	}
	else
	{
		rcTargetRect.top = (modeHeightRemaining - ((DWORD)(LONG)((1.0 / c64ratio) * ((double)(modeWidth))))) / 2L;
		rcTargetRect.bottom = modeHeightRemaining - rcTargetRect.top;
		rcTargetRect.left = 0;
		rcTargetRect.right = modeWidth;
	}

	D3D11_RECT rcTop;
	D3D11_RECT rcBottom;
	D3D11_RECT rcLeft;
	D3D11_RECT rcRight;
	//Top
	rcTop.left = 0;
	rcTop.top = 0;
	rcTop.right = modeWidth;
	rcTop.bottom = rcTargetRect.top;

	//Bottom
	rcBottom.left = 0;
	rcBottom.top = rcTargetRect.bottom;
	rcBottom.right = modeWidth;
	rcBottom.bottom = modeHeight;

	//Left
	rcLeft.left = 0;
	rcLeft.top = rcTargetRect.top;
	rcLeft.right = rcTargetRect.left;
	rcLeft.bottom = rcTargetRect.bottom;

	//Right
	rcRight.left = rcTargetRect.right;
	rcRight.top = rcTargetRect.top;
	rcRight.right = modeWidth;
	rcRight.bottom = rcTargetRect.bottom;

	if (rcTargetRect.bottom + toolBarHeight > (int)modeHeight)
	{
		drcStatusBar.left = rcTargetRect.left;
		if (modeHeight > toolBarHeight)
		{
			drcStatusBar.top = modeHeight - toolBarHeight;
		}
		else
		{
			drcStatusBar.top = 0;
		}

		drcStatusBar.right = rcTargetRect.right;
		drcStatusBar.bottom = modeHeight;
	}
	else
	{
		drcStatusBar.left = rcTargetRect.left;
		drcStatusBar.top = rcTargetRect.bottom;
		drcStatusBar.right = rcTargetRect.right;
		drcStatusBar.bottom = rcTargetRect.bottom + toolBarHeight;
	}

	drcEraseRects.clear();
	if (IsValidRect(rcTop))
	{
		drcEraseRects.push_back(rcTop);
	}

	if (IsValidRect(rcBottom))
	{
		drcEraseRects.push_back(rcBottom);
	}

	if (IsValidRect(rcLeft))
	{
		drcEraseRects.push_back(rcLeft);
	}

	if (IsValidRect(rcRight))
	{
		drcEraseRects.push_back(rcRight);
	}
}

void C64Display::CalcClearingRects(int modeWidth, int modeHeight, const C64WindowDimensions& dims, const DWORD scale, bool bShowFloppyLed, RECT& rcTargetRect, std::vector<D3D11_RECT>& drcEraseRects, D3D11_RECT& drcStatusBar)
{
	int toolBarHeight = GetToolBarHeight(bShowFloppyLed, false, modeHeight, nullptr, nullptr);
	int h = dims.Height * scale;
	int w = dims.Width * scale;
	rcTargetRect.top = (h + toolBarHeight) < modeHeight ? (modeHeight - (h + toolBarHeight)) / 2 : 0;
	rcTargetRect.bottom = (rcTargetRect.top + h + toolBarHeight) < modeHeight ? rcTargetRect.top + h : modeHeight - toolBarHeight;
	rcTargetRect.left = w < modeWidth ? (modeWidth - w) / 2 : 0;
	rcTargetRect.right = (rcTargetRect.left + w) < modeWidth ? rcTargetRect.left + w : modeWidth;

	D3D11_RECT rcTop;
	D3D11_RECT rcBottom;
	D3D11_RECT rcLeft;
	D3D11_RECT rcRight;

	//Top
	rcTop.left = 0;
	rcTop.top = 0;
	rcTop.right = modeWidth;
	rcTop.bottom = rcTargetRect.top;

	//Bottom
	rcBottom.left = 0;
	rcBottom.top = rcTargetRect.bottom;
	rcBottom.right = modeWidth;
	rcBottom.bottom = modeHeight;

	//Left
	rcLeft.left = 0;
	rcLeft.top = rcTargetRect.top;
	rcLeft.right = rcTargetRect.left;
	rcLeft.bottom = rcTargetRect.bottom;

	//Right
	rcRight.left = rcTargetRect.right;
	rcRight.top = rcTargetRect.top;
	rcRight.right = modeWidth;
	rcRight.bottom = rcTargetRect.bottom;

	if (rcTargetRect.bottom + toolBarHeight > (int)modeHeight)
	{
		drcStatusBar.left = rcTargetRect.left;
		drcStatusBar.top = modeHeight - toolBarHeight;
		drcStatusBar.right = rcTargetRect.right;
		drcStatusBar.bottom = modeHeight;
	}
	else
	{
		drcStatusBar.left = rcTargetRect.left;
		drcStatusBar.top = rcTargetRect.bottom;
		drcStatusBar.right = rcTargetRect.right;
		drcStatusBar.bottom = rcTargetRect.bottom + toolBarHeight;
	}

	drcEraseRects.clear();
	if (IsValidRect(rcTop))
	{
		drcEraseRects.push_back(rcTop);
	}

	if (IsValidRect(rcBottom))
	{
		drcEraseRects.push_back(rcBottom);
	}

	if (IsValidRect(rcLeft))
	{
		drcEraseRects.push_back(rcLeft);
	}

	if (IsValidRect(rcRight))
	{
		drcEraseRects.push_back(rcRight);
	}
}

void C64Display::SetClearingRects(D3D11_RECT rects[], unsigned int count)
{
	drcEraseRects.clear();
	try
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (IsValidRect(rects[i]))
			{
				drcEraseRects.push_back(rects[i]);
			}
		}
	}
	catch (...)
	{

	}
}

void C64Display::GetDisplayRectFromVicRect(const RECT& rcVicCycle, float& left, float& top, float &width, float& height)
{
	int s = this->displayStart + DISPLAY_START;
	int st = this->displayFirstVicRaster;
	int sb = this->displayLastVicRaster;
	int wc = this->displayWidth;
	int hc = this->displayHeight;
	int wp = this->drcScreen.right - this->drcScreen.left;
	int hp = this->drcScreen.bottom - this->drcScreen.top;

	left = (float)(rcVicCycle.left - s) * (float)wp / (float)wc;
	float right = (float)(rcVicCycle.right - s) * (float)wp / (float)wc;
	width = right - left;

	top = (float)(rcVicCycle.top - st) * (float)hp / (float)hc;
	float bottom = (float)(rcVicCycle.bottom - st) * (float)hp / (float)hc;
	height = bottom - top;

	left += (float)this->drcScreen.left;
	top += (float)this->drcScreen.top;
}

void C64Display::DrawCursorAtVicPosition(const XMMATRIX& orthoMatrix, int cycle, int line)
{
	RECT rcVicCycle;
	
	float left = 0.0f;
	float top = 0.0f;
	float width = 0.0f;
	float height = 0.0f;

	SetRect(&rcVicCycle, (cycle - 1) * 8, line, (cycle) * 8, line + 1);
	GetDisplayRectFromVicRect(rcVicCycle, left, top, width, height);

	this->spriteVicCursorBar.SetScale(width, height);
	this->spriteVicCursorBar.SetPosition(left, top, 0.0f);
	this->spriteVicCursorBar.Draw(orthoMatrix);
}

void C64Display::EnableVicCursorSprites(bool enabled)
{
	enableVicCursorSprites = enabled;
}

void C64Display::ClearVicCursorSprites()
{
	this->listVicCursorPosition.clear();
}

void C64Display::InsertCursorSprite(const VicCursorPosition&& viccursor)
{
	this->listVicCursorPosition.push_back(viccursor);
}


VicCursorPosition::VicCursorPosition(int VicCycle, int VicLine) noexcept
	:VicCycle(VicCycle), VicLine(VicLine)
{

}