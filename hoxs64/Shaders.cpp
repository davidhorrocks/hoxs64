#include "Shaders.h"
#include "ErrorLogger.h"

bool VertexShader::Initialize(ID3D11Device* device, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
	Cleanup();
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateVertexShader(this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(), NULL, this->shader.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateInputLayout(layoutDesc, numElements, this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(), this->inputLayout.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create input layout.";
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

void VertexShader::Cleanup()
{
	this->shader_buffer.Reset();
	this->shader.Reset();
	this->inputLayout.Reset();
}

ID3D11VertexShader* VertexShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return this->shader_buffer.Get();
}

ID3D11InputLayout* VertexShader::GetInputLayout()
{
	return this->inputLayout.Get();
}

bool PixelShader::Initialize(ID3D11Device *device, std::wstring shaderpath)
{
	Cleanup();
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreatePixelShader(this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(), NULL, this->shader.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create pixel shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

void PixelShader::Cleanup()
{
	this->shader_buffer.Reset();
	this->shader.Reset();
}

ID3D11PixelShader* PixelShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* PixelShader::GetBuffer()
{
	return this->shader_buffer.Get();
}