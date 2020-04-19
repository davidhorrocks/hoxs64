#include "Texture.h"
#include "ErrorLogger.h"
#include "StringHelper.h"
#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/DDSTextureLoader.h>
#include "graphicshelper.h"

Texture::Texture(ID3D11Device* device, const Color& color, aiTextureType type)
{
	this->Initialize1x1ColorTexture(device, color, type);
}

Texture::Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	this->InitializeColorTexture(device, colorData, width, height, type);
}

Texture::Texture(ID3D11Device* device, const std::string& filePath, aiTextureType type)
{
	this->type = type;
	if (StringHelper::GetFileExtension(filePath) == ".dds")
	{
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device, StringConverter::StringToWideString(filePath).c_str(), texture.GetAddressOf(), this->textureView.GetAddressOf());
		if (FAILED(hr))
		{
			this->Initialize1x1ColorTexture(device, Color(Colors::UnloadedTextureColor), type);
		}
		return;
	}
	else
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(device, StringConverter::StringToWideString(filePath).c_str(), texture.GetAddressOf(), this->textureView.GetAddressOf());
		if (FAILED(hr))
		{
			this->Initialize1x1ColorTexture(device, Color(Colors::UnloadedTextureColor), type);
		}
		return;
	}
}

Texture::Texture(ID3D11Device* device, const uint8_t* pData, size_t size, aiTextureType type)
{
	this->type = type;
	HRESULT hr = DirectX::CreateWICTextureFromMemory(device, pData, size, this->texture.GetAddressOf(), this->textureView.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create Texture from memory.");
}

aiTextureType Texture::GetType()
{
	return this->type;
}

ID3D11ShaderResourceView* Texture::GetTextureResourceView()
{
	return this->textureView.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceViewAddress()
{
	return this->textureView.GetAddressOf();
}

void Texture::Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type)
{
	Color cl(GraphicsHelper::ConvertColour(GraphicsHelper::DefaultPixelFormat, colorData.GetColorRef()));
	InitializeColorTexture(device, &cl, 1, 1, type);
}

void Texture::InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	this->type = type;
	CD3D11_TEXTURE2D_DESC textureDesc(GraphicsHelper::DefaultPixelFormat, width, height);
	textureDesc.MipLevels = 1;
	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = colorData;
	initialData.SysMemPitch = width * sizeof(Color);
	HRESULT hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	texture = p2DTexture;
	p2DTexture->Release();
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture generated from color data.");
}

void Texture::Cleanup() noexcept
{
	try
	{
		textureView.Reset();
		texture.Reset();
	}
	catch (...)
	{

	}
}

Texture::~Texture()
{
	Cleanup();
}