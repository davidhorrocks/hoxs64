#pragma once
#include <DirectXMath.h>

struct Vertex2D
{
	Vertex2D() noexcept : pos(0, 0, 0), texCoord(0, 0) {}
	Vertex2D(float x, float y, float z, float u, float v)
		: pos(x, y, z), texCoord(u, v) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
};

struct Vertex3D
{
	Vertex3D() noexcept : pos(0, 0, 0), texCoord(0, 0), normal(0, 0, 0) {}
	Vertex3D(float x, float y, float z, float u, float v, float nx, float ny, float nz) noexcept
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
};