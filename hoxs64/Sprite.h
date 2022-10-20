#pragma once
#include "GameObject2D.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <string>
#include <memory>
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Vertex.h"

class Sprite : public GameObject2D
{
public:
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, const Color& color, aiTextureType type, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d);
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, std::wstring spritePath, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d);
	void Cleanup();
	void Draw(const XMMATRIX& orthoMatrix); //2d camera orthogonal matrix
	float GetWidth();
	float GetHeight();
private:
	void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d);
	void UpdateMatrix() override;

	ConstantBuffer<CB_VS_vertexshader_2d>* cb_vs_vertexshader_2d = nullptr;
	XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

	std::unique_ptr<Texture> texture;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

	IndexBuffer indices;
	VertexBuffer<Vertex2D> vertices;
};