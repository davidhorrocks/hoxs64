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

	HRESULT Initialize(ID3D11Device* device, UINT width, UINT height);
	void Cleanup();
	HRESULT ResizeOrKeep(ID3D11Device* device, UINT width, UINT height);
	ID3D11ShaderResourceView* GetTextureResourceView();
	ID3D11ShaderResourceView** GetTextureResourceViewAddress();
	HRESULT GetTextureResource(Microsoft::WRL::ComPtr<ID3D11Resource>* pTextureResource);
	UINT GetWidth();
	UINT GetHeight();

private:
	HRESULT CreateSmallSurface(ID3D11Device* device, int width, int height);
	void FreeSmallSurface();
	Microsoft::WRL::ComPtr<ID3D11Resource> texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
	UINT width = 0;
	UINT height = 0;
	unsigned char* pbuffer = nullptr;
};


