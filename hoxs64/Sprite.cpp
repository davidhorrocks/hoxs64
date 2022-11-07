#include "Sprite.h"
#include <DirectXTK/WICTextureLoader.h>

void Sprite::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, const Color& color, aiTextureType type, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d)
{
	this->texture = std::make_unique<Texture>(device, color, aiTextureType::aiTextureType_DIFFUSE);
	Init(device, deviceContext, width, height, cb_vs_vertexshader_2d);
}

void Sprite::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, std::wstring spritePath, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d)
{
	this->texture = std::make_unique<Texture>(device, spritePath, aiTextureType::aiTextureType_DIFFUSE);
	Init(device, deviceContext, width, height, cb_vs_vertexshader_2d);
}

void Sprite::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d)
{
	this->deviceContext = deviceContext;
	if (deviceContext == nullptr)
	{
		COM_ERROR_IF_FAILED(E_FAIL, "Failed to initialize sprite.");
	}

	this->cb_vs_vertexshader_2d = &cb_vs_vertexshader_2d;

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

	HRESULT hr = vertices.Initialize(device, vertexData.data(), (UINT)vertexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for sprite.");

	hr = indices.Initialize(device, indexData.data(), (UINT)indexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	SetScale(width, height);
}

void Sprite::Cleanup()
{
	indices.Cleanup();
	vertices.Cleanup();
	texture.reset();
	deviceContext.Reset();
}

void Sprite::Draw(const XMMATRIX& orthoMatrix)
{
	if (deviceContext == nullptr)
	{
		return;
	}

	XMMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader_2d->GetAddressOf());
	cb_vs_vertexshader_2d->data.wvpMatrix = wvpMatrix;
	cb_vs_vertexshader_2d->ApplyChanges();

	deviceContext->PSSetShaderResources(0, 1, texture->GetTextureResourceViewAddress());

	const UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), vertices.StridePtr(), &offsets);
	deviceContext->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indices.IndexCount(), 0, 0);
}

float Sprite::GetWidth()
{
	return scale.x;
}

float Sprite::GetHeight()
{
	return scale.y;
}

void Sprite::UpdateMatrix()
{
	worldMatrix = XMMatrixScaling(scale.x, scale.y, 1.0f) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x + scale.x / 2.0f, pos.y + scale.y / 2.0f, pos.z);
}