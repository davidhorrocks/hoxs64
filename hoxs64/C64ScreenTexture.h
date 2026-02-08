#pragma once
#include "dx_version.h"
#include <wrl/client.h>
#include "Color.h"
#include <assimp/material.h>
#include "graphicshelper.h"

class C64ScreenTexture
{
public:
	C64ScreenTexture() noexcept;
	virtual  ~C64ScreenTexture();
	C64ScreenTexture(const C64ScreenTexture&) = delete;
	C64ScreenTexture& operator=(const C64ScreenTexture&) = delete;
	C64ScreenTexture(C64ScreenTexture&&) = delete;
	C64ScreenTexture& operator=(C64ScreenTexture&&) = delete;

	HRESULT Initialize(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);
	void Cleanup() noexcept;
	HRESULT ResizeOrKeep(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);
	ID3D11ShaderResourceView* GetTextureResourceView();
	ID3D11ShaderResourceView** GetTextureResourceViewAddress();
	HRESULT GetTextureResource(Microsoft::WRL::ComPtr<ID3D11Resource>* pTextureResource);
	UINT GetWidth() const;
	UINT GetHeight() const;
	DXGI_FORMAT C64ScreenTexture::GetFormat() const;

private:
	HRESULT CreateSmallSurface(ID3D11Device* device, int width, int height, DXGI_FORMAT format);
	void FreeSmallSurface() noexcept;
	Microsoft::WRL::ComPtr<ID3D11Resource> texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
	UINT width = 0;
	UINT height = 0;
	DXGI_FORMAT format;
	unsigned char* pbuffer = nullptr;
};


