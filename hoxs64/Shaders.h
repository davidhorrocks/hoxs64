#pragma once
#include "dx_version.h"
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "ErrorLogger.h"

class VertexShader
{
public:
	bool Initialize(ID3D11Device* device, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
	void Cleanup();
	ID3D11VertexShader* GetShader();
	ID3D10Blob* GetBuffer();
	ID3D11InputLayout* GetInputLayout();
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

class PixelShader
{
public:
	bool Initialize(ID3D11Device* device, std::wstring shaderpath);
	void Cleanup();
	ID3D11PixelShader* GetShader();
	ID3D10Blob* GetBuffer();
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
};