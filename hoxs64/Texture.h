#pragma once
#include "dx_version.h"
#include <wrl/client.h>
#include "Color.h"
#include <assimp/material.h>

enum class TextureStorageType
{
	Invalid,
	None,
	EmbeddedIndexCompressed,
	EmbeddedIndexNonCompressed,
	EmbeddedCompressed,
	EmbeddedNonCompressed,
	Disk
};

class Texture
{
public:
	Texture() = default;
	Texture(const Texture&) = default;
	Texture& operator=(const Texture&) = default;
	Texture(Texture&&) = default;
	Texture& operator=(Texture&&) = default;

	~Texture();	
	Texture(ID3D11Device* device, const Color& color, aiTextureType type);
	Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type); //Generate texture of specific color data
	Texture(ID3D11Device* device, const std::string& filePath, aiTextureType type);
	Texture(ID3D11Device* device, const uint8_t* pData, size_t size, aiTextureType type);
	aiTextureType GetType();
	ID3D11ShaderResourceView* GetTextureResourceView();
	ID3D11ShaderResourceView** GetTextureResourceViewAddress();
	void Cleanup() noexcept;

private:
	void Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type);
	void InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type);
	Microsoft::WRL::ComPtr<ID3D11Resource> texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
	aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
};
