#include "C64ScreenTexture.h"

C64ScreenTexture::C64ScreenTexture() noexcept
{
	pbuffer = nullptr;
	width = 0;
	height = 0;
}

C64ScreenTexture::~C64ScreenTexture()
{
	Cleanup();
}

HRESULT C64ScreenTexture::Initialize(ID3D11Device* device, UINT width, UINT height)
{
	FreeSmallSurface();
	HRESULT hr = CreateSmallSurface(device, width, height);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT C64ScreenTexture::ResizeOrKeep(ID3D11Device* device, UINT width, UINT height)
{
	if (width == 0 || height == 0)
	{
		return E_FAIL;
	}

	if (this->width == width && this->height == height)
	{
		return S_OK;
	}

	return Initialize(device, width, height);
}

UINT C64ScreenTexture::GetWidth()
{
	return width;
}

UINT C64ScreenTexture::GetHeight()
{
	return height;
}

void C64ScreenTexture::Cleanup()
{
	FreeSmallSurface();
}

HRESULT C64ScreenTexture::CreateSmallSurface(ID3D11Device* device, int width, int height)
{
	HRESULT hr;
	FreeSmallSurface();
	this->width = width;
	this->height = height;
	int widthBytes = width * sizeof(Color);
	widthBytes = width * 4;
	int widthPadding = widthBytes % 64;
	int widthTotalBytes = widthBytes + widthPadding;
	pbuffer = (unsigned char*)GlobalAlloc(GPTR, widthTotalBytes * height);
	if (pbuffer == nullptr)
	{
		return E_FAIL;
	}

	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = pbuffer;
	initialData.SysMemPitch = widthTotalBytes;
	initialData.SysMemSlicePitch = 0;


	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.Format = GraphicsHelper::DefaultPixelFormat;
	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	if (hr != S_OK)
	{
		if (FAILED(hr))
		{
			return hr;
		}
		else
		{
			return E_FAIL;
		}
	}

	texture = p2DTexture;
	p2DTexture->Release();
	p2DTexture = nullptr;

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView.GetAddressOf());
	if (hr != S_OK)
	{
		if (FAILED(hr))
		{
			return hr;
		}
		else
		{
			return E_FAIL;
		}
	}

	return hr;
}


void C64ScreenTexture::FreeSmallSurface()
{
	textureView.Reset();
	texture.Reset();
	if (pbuffer)
	{
		GlobalFree(pbuffer);
		pbuffer = nullptr;
	}

	width = 0;
	height = 0;
}

ID3D11ShaderResourceView* C64ScreenTexture::GetTextureResourceView()
{
	return this->textureView.Get();
}

ID3D11ShaderResourceView** C64ScreenTexture::GetTextureResourceViewAddress()
{
	return this->textureView.GetAddressOf();
}

HRESULT C64ScreenTexture::GetTextureResource(Microsoft::WRL::ComPtr<ID3D11Resource>* pTextureResource)
{
	if (pTextureResource == nullptr)
	{
		return E_INVALIDARG;
	}

	*pTextureResource = texture;
	return S_OK;
}