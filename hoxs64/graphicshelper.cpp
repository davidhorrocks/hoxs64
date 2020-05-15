#include <Windows.h>
#include <math.h>
#include "wfs.h"
#include "graphicshelper.h"
#include "../dxsdk/d3dx_dxgiformatconvert.inl"

DWORD GraphicsHelper::DDColorMatch(IDXGISurface1* pdds, COLORREF rgb)
{
	COLORREF                rgbT = 0;
	HDC                     hdc;
	DWORD                   dw = CLR_INVALID;
	DXGI_SURFACE_DESC       ddsd;
	HRESULT                 hr;
	DXGI_MAPPED_RECT        rectLocked;
	DWORD                   bpp;
	//
	//  Use GDI SetPixel to color match for us
	//

	dw = 0;
	hr = pdds->GetDesc(&ddsd);
	if (FAILED(hr))
	{
		return 0;
	}

	RECT rcChanged = { 0, 0, 1, 1 };
	if (rgb != CLR_INVALID && pdds->GetDC(TRUE, &hdc) == S_OK)
	{
		rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
		SetPixel(hdc, 0, 0, rgb);       // Set our value
		pdds->ReleaseDC(&rcChanged);
	}

	//
	// Now lock the surface so we can read back the converted color
	//
	hr = pdds->Map(&rectLocked, DXGI_MAP_READ);
	if (hr == S_OK)
	{
		dw = *(DWORD*)rectLocked.pBits;                 // Get DWORD
		bpp = GetBitsPerPixel(ddsd.Format);
		if (bpp < 32)
		{
			dw &= (1 << bpp) - 1;  // Mask it to bpp
		}

		pdds->Unmap();

	}

	//
	//  Now put the color that was there back.
	//
	if (rgb != CLR_INVALID && pdds->GetDC(TRUE, &hdc) == S_OK)
	{
		SetPixel(hdc, 0, 0, rgbT);
		pdds->ReleaseDC(&rcChanged);
	}

	return dw;
}

DWORD GraphicsHelper::ConvertColour(DXGI_FORMAT format, COLORREF rgb)
{
	DWORD v;
	v = 0;
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	{
		DirectX::XMFLOAT4 float4(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), 1.0);
		v = D3DX::D3DX_FLOAT4_to_R8G8B8A8_UNORM_SRGB(float4);
		break;
	}
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	{
		DirectX::XMFLOAT4 float4(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb), 1.0);
		v = D3DX::D3DX_FLOAT4_to_R8G8B8A8_UNORM_SRGB(float4);
		break;
	}
	case DXGI_FORMAT_R8G8B8A8_SINT:
	{
		DirectX::XMINT4 xmint4(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), 1);
		v = D3DX::D3DX_INT4_to_R8G8B8A8_SINT(xmint4);
		break;
	}
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	{
		DirectX::XMFLOAT4 float4(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), 1);
		v = D3DX::D3DX_FLOAT4_to_R8G8B8A8_SNORM(float4);
		break;
	}
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		//case D3DFMT_A8R8G8B8:
		v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		v |= 0xff000000;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
		//case D3DFMT_A8B8G8R8:
		v = GetBValue(rgb) << 16 | GetGValue(rgb) << 8 | GetRValue(rgb);
		v |= 0xff000000;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
		//case D3DFMT_A2B10G10R10:
		v = GetBValue(rgb) << 20 | GetGValue(rgb) << 10 | GetRValue(rgb);
		v |= 0xc0000000;
		break;
		//case D3DFMT_A2R10G10B10:
		//	v = GetRValue(rgb) << 20 | GetGValue(rgb) << 10 | GetBValue(rgb);
		//	v |= 0xc0000000;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		//case D3DFMT_X8R8G8B8:
		v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		break;
		//case D3DFMT_X8B8G8R8:
		//	v = GetBValue(rgb) << 16 | GetGValue(rgb) << 8 | GetRValue(rgb);
		//	break;
		//case D3DFMT_R8G8B8:
		//	v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		//	break;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		//case D3DFMT_A1R5G5B5:
		v = (ReduceBits(GetRValue(rgb), 5)) << 10 | (ReduceBits(GetGValue(rgb), 5)) << 8 | (ReduceBits(GetBValue(rgb), 5));
		v |= 0x800000;
		break;
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		//case D3DFMT_A4R4G4B4:
		v = (ReduceBits(GetRValue(rgb), 4)) << 8 | (ReduceBits(GetGValue(rgb), 4)) << 4 | (ReduceBits(GetBValue(rgb), 4));
		v |= 0xf000;
		break;
		//case D3DFMT_A8R3G3B2:
		//	v = (ReduceBits(GetRValue(rgb), 3)) << 5 | (ReduceBits(GetGValue(rgb), 3)) << 2 | (ReduceBits(GetBValue(rgb), 2));
		//	v |= 0xff00;
		//	break;
		//case D3DFMT_X1R5G5B5:
		//	v = (ReduceBits(GetRValue(rgb), 5)) << 10 | (ReduceBits(GetGValue(rgb), 5)) << 5 | (ReduceBits(GetBValue(rgb), 5));
		//	break;
		//case D3DFMT_X4R4G4B4:
		//	v = (ReduceBits(GetRValue(rgb), 4)) << 8 | (ReduceBits(GetGValue(rgb), 4)) << 4 | (ReduceBits(GetBValue(rgb), 4));
		//	break;
	case DXGI_FORMAT_B5G6R5_UNORM:
		//case D3DFMT_R5G6B5:
		v = (ReduceBits(GetRValue(rgb), 5)) << 11 | (ReduceBits(GetGValue(rgb), 6)) << 5 | (ReduceBits(GetBValue(rgb), 5));
		break;
		//case D3DFMT_R3G3B2:
		//	v = (ReduceBits(GetRValue(rgb), 3)) << 5 | (ReduceBits(GetGValue(rgb), 3)) << 2 | (ReduceBits(GetBValue(rgb), 2));
		//	break;
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_P8:
		//case D3DFMT_P8:
		v = 0;
		break;
	default:
		v = 0;
	}

	return v;
}

DWORD GraphicsHelper::ReduceBits(BYTE v, BYTE bits) noexcept
{
	return ((DWORD)ceil((((double)v) / 255.0) * ((double)((1L << bits) - 1)))) & ((1L << bits) - 1);
}

unsigned int GraphicsHelper::GetBitsPerPixel(DXGI_FORMAT fmt) noexcept
{
	switch (static_cast<int>(fmt))
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		//case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
		//case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
		//case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 0;

	//case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
	//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
	//case WIN10_DXGI_FORMAT_V408:

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		//case WIN10_DXGI_FORMAT_P208:
		//case WIN10_DXGI_FORMAT_V208:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 0;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		//case XBOX_DXGI_FORMAT_R4G4_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 0;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 0;

	default:
		return 0;
	}
}

unsigned int GraphicsHelper::GetDisplayResolutionText(unsigned int width, unsigned int height, LPWSTR buffer, unsigned int charBufferLen)
{
WCHAR sMode[50];

	unsigned int r = _snwprintf_s(sMode, _countof(sMode), _TRUNCATE, TEXT("%d x %d"), (int)width, (int)height);
	if (charBufferLen == 0)
	{
		charBufferLen = r + 1;
	}
	else if ((r + 1) > charBufferLen)
	{
		charBufferLen = r + 1;
	}

	if (buffer!=NULL)
	{
		wcscpy_s(buffer, charBufferLen, &sMode[0]);
	}

	return charBufferLen;
}

unsigned int GraphicsHelper::GetDisplayFormatText(DXGI_FORMAT format, LPWSTR buffer, unsigned int charBufferLen)
{
WCHAR sMode[20];

	switch(format)
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("32 bit B8G8R8A8"));
			break;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("32 bit B8G8R8X8"));
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("32 bit R8G8B8A8"));
			break;
		//case D3DFMT_X8B8G8R8:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("32 bit X8B8G8R8"));
		//	break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("32 bit R10G10B10A2"));
			break;
		//case D3DFMT_A2R10G10B10:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("32 bit A2R10G10B10"));
		//	break;
		//case D3DFMT_R8G8B8:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("24 bit R8G8B8"));
		//	break;
		//case D3DFMT_X1R5G5B5:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("16 bit X1R5G5B5"));
		//	break;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("16 bit B5G5R5A1"));
			break;
		case DXGI_FORMAT_B5G6R5_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("16 bit B5G6R5"));
			break;
		//case D3DFMT_X4R4G4B4:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("16 bit X4R4G4B4"));
		//	break;
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			wcscpy_s(sMode, _countof(sMode), TEXT("16 bit B4G4R4A4"));
			break;
		//case D3DFMT_A8R3G3B2:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("16 bit A8R3G3B2"));
		//	break;
		//case D3DFMT_R3G3B2:
		//	wcscpy_s(sMode, _countof(sMode), TEXT("8 bit R3G3B2"));
		//	break;
		case DXGI_FORMAT_P8:
			wcscpy_s(sMode, _countof(sMode), TEXT("8 bit P8"));
			break;
		case DXGI_FORMAT_A8P8:
			wcscpy_s(sMode, _countof(sMode), TEXT("16 bit A8P8"));
			break;
		default:
			wcscpy_s(sMode, _countof(sMode), TEXT("?"));
			break;
	}

	unsigned int r = lstrlenW(sMode);
	if (charBufferLen == 0)
	{
		charBufferLen = r + 1;
	}
	else if ((r + 1) > charBufferLen)
	{
		charBufferLen = r + 1;
	}

	if (buffer != NULL)
	{
		wcscpy_s(buffer, charBufferLen, &sMode[0]);
	}

	return charBufferLen;
}

unsigned int GraphicsHelper::GetDisplayRefreshText(unsigned int refreshRateNumerator, unsigned int refreshRateDenominator, LPWSTR buffer, unsigned int charBufferLen)
{
WCHAR sMode[100];
double rate = 0.0;
	if (refreshRateDenominator != 0)
	{
		rate = (double)refreshRateNumerator / (double)refreshRateDenominator;
	}

	unsigned int r = _snwprintf_s(sMode, _countof(sMode), _TRUNCATE, TEXT("%.2f Hz (%d / %d)"), (double)rate, refreshRateNumerator, refreshRateDenominator);
	r = lstrlenW(sMode);
	if (charBufferLen == 0)
	{
		charBufferLen = r + 1;
	}
	else if ((r + 1) > charBufferLen)
	{
		charBufferLen = r + 1;
	}

	if (buffer!=NULL)
	{
		wcscpy_s(buffer, charBufferLen, &sMode[0]);
	}

	return charBufferLen;
}

const DXGI_FORMAT GraphicsHelper::Formats2[] = {
	DXGI_FORMAT_B8G8R8A8_UNORM, // D3DFMT_A8R8G8B8
	DXGI_FORMAT_B8G8R8X8_UNORM, // D3DFMT_X8R8G8B8
	DXGI_FORMAT_R8G8B8A8_UNORM, // D3DFMT_A8B8G8R8
	// D3DFMT_X8B8G8R8
	DXGI_FORMAT_R10G10B10A2_UNORM,// D3DFMT_A2B10G10R10,
	// D3DFMT_A2R10G10B10,
	// D3DFMT_R8G8B8,
	// D3DFMT_X1R5G5B5,
	DXGI_FORMAT_B5G5R5A1_UNORM,// D3DFMT_A1R5G5B5,
	DXGI_FORMAT_B5G6R5_UNORM,// D3DFMT_R5G6B5,
	// D3DFMT_X4R4G4B4,
	DXGI_FORMAT_B4G4R4A4_UNORM,// D3DFMT_A4R4G4B4,
	// D3DFMT_A8R3G3B2,
	DXGI_FORMAT_A8P8,// D3DFMT_A8P8,
	// D3DFMT_R3G3B2,
	DXGI_FORMAT_P8// D3DFMT_P8,
};

const DXGI_FORMAT GraphicsHelper::Formats[] = {
	GraphicsHelper::DefaultPixelFormat
};

bool GraphicsHelper::IsAcceptableMode(UINT Width, UINT Height) noexcept
{
	return Width >= 320 && Height >= 200;
}

bool GraphicsHelper::IsAcceptableMode(UINT Width, UINT Height, DXGI_FORMAT Format) noexcept
{
	return IsAcceptableMode(Width, Height) && IsAcceptableFormat(Format);
}

bool GraphicsHelper::IsAcceptableFormat(DXGI_FORMAT format) noexcept
{
	for (int i = 0; i < _countof(Formats); i++)
	{
		if (Formats[i] == format)
		{
			return true;
		}
	}

	return false;
}

bool GraphicsHelper::GetAppFilename(std::wstring& str)
{
	bool r = true;
	wchar_t* pszAppFullPath = nullptr;
	try
	{
		str.clear();
		wchar_t buffer[MAX_PATH + 1];
		DWORD dw;
		DWORD len = _countof(buffer);
		wchar_t* psz;
		for (int i = 0;;i++)
		{
			if (i == 0)
			{
				len = _countof(buffer);
				psz = &buffer[0];
			}
			else
			{
				pszAppFullPath = (wchar_t*)malloc(len * sizeof(wchar_t));
				psz = pszAppFullPath;
			}

			dw = GetModuleFileNameW(GetModuleHandleW(0), psz, len);
			if (dw < len || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				len = dw + 1;
				break;
			}
		}

		str = psz;
		return true;
	}
	catch (std::exception)
	{
		r = false;
	}

	if (pszAppFullPath)
	{
		free(pszAppFullPath);
		pszAppFullPath = nullptr;
	}

	return r;
}

bool GraphicsHelper::GetAppDir(std::wstring &str)
{
	bool r = true;
	wchar_t* pszAppFullPath = nullptr;
	try
	{
		str.clear();
		wchar_t buffer[MAX_PATH + 1];
		DWORD dw;
		DWORD len = _countof(buffer);
		wchar_t* psz;
		for (int i = 0;; i++)
		{
			if (i == 0)
			{
				len = _countof(buffer);
				psz = &buffer[0];
			}
			else
			{
				pszAppFullPath = (wchar_t*)malloc(len * sizeof(wchar_t));
				psz = pszAppFullPath;
			}

			dw = GetModuleFileNameW(GetModuleHandleW(0), psz, len);
			if (dw < len || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				break;
			}

			len = dw + 1;
		}

		std::wstring path = psz;
		std::wstring root;
		std::wstring directorypath;
		std::wstring filename;
		Wfs::SplitRootPath(path, root, directorypath, filename);
		str = root + directorypath;
		return true;
	}
	catch (std::exception)
	{
		r= false;
	}

	if (pszAppFullPath)
	{
		free(pszAppFullPath);
		pszAppFullPath = nullptr;
	}

	return r;
}

bool GraphicsHelper::GetAppFileFullPath(const wchar_t *filename, std::wstring& str)
{
	if (GetAppDir(str))
	{
		if (str.length() > 0 && str[str.length() - 1] != L'\\')
		{
			str.append(1, L'\\');
		}

		str.append(filename);
		return true;
	}

	return false;
}

int GraphicsHelper::GetScanRating(const DXGI_MODE_SCANLINE_ORDER& scanOrder)
{
	switch (scanOrder)
	{
	case DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED:
		return 3;
	case DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE:
		return 4;
	case DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST:
		return 2;
	case DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST:
		return 1;
	default:
		return 0;
	}
}

int GraphicsHelper::GetScaleRating(const DXGI_MODE_SCALING& scaling)
{
	switch (scaling)
	{
	case DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED:
		return 2;
	case DXGI_MODE_SCALING::DXGI_MODE_SCALING_STRETCHED:
		return 1;
	case DXGI_MODE_SCALING::DXGI_MODE_SCALING_CENTERED:
		return 3;
	default:
		return 0;
	}
}

HRESULT GraphicsHelper::GetPreferredMode(ID3D11Device* device, IDXGIOutput* pOutput, const DXGI_MODE_DESC* pRequestedMode, DXGI_MODE_DESC* pPreferredMode)
{
	HRESULT hr;
	if (pRequestedMode == nullptr || pPreferredMode == nullptr)
	{
		return E_POINTER;
	}

	DXGI_MODE_DESC selectedDesc = {};
	bool foundExactModeMatch = false;
	bool foundBestModeMatch = false;
	double refreshRateRequested = 0.0;
	if (pRequestedMode->RefreshRate.Numerator != 0 && pRequestedMode->RefreshRate.Denominator != 0)
	{
		refreshRateRequested = (double)pRequestedMode->RefreshRate.Numerator / (double)pRequestedMode->RefreshRate.Denominator;
	}

	DXGI_OUTPUT_DESC outdesc;
	hr = pOutput->GetDesc(&outdesc);
	if (SUCCEEDED(hr))
	{
		UINT num = 0;
		hr = pOutput->GetDisplayModeList(GraphicsHelper::DefaultPixelFormat, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &num, nullptr);
		if (SUCCEEDED(hr))
		{
			DXGI_MODE_DESC* pDescs = new DXGI_MODE_DESC[num];
			try
			{
				hr = pOutput->GetDisplayModeList(GraphicsHelper::DefaultPixelFormat, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &num, pDescs);
				if (SUCCEEDED(hr))
				{
					constexpr double MIN_REFRESH_RATE = 50.2;
					double selectedRefreshRate = 0.0;
					for (unsigned int modeindex = 0; modeindex < num; modeindex++)
					{
						DXGI_MODE_DESC* pdesc = &pDescs[modeindex];
						if (pdesc->Format == GraphicsHelper::DefaultPixelFormat)
						{
							double refreshRate = 0.0;
							if (pdesc->RefreshRate.Numerator != 0 && pdesc->RefreshRate.Denominator != 0)
							{
								refreshRate = (double)pdesc->RefreshRate.Numerator / (double)pdesc->RefreshRate.Denominator;
							}

							if (pRequestedMode->Width != 0 && pRequestedMode->Height != 0 && pRequestedMode->RefreshRate.Numerator != 0 && pRequestedMode->RefreshRate.Denominator != 0 && pRequestedMode->Width == pdesc->Width && pRequestedMode->Height == pdesc->Height && pRequestedMode->RefreshRate.Numerator == pdesc->RefreshRate.Numerator && pRequestedMode->RefreshRate.Denominator == pdesc->RefreshRate.Denominator && pRequestedMode->ScanlineOrdering == pdesc->ScanlineOrdering && pRequestedMode->Scaling == pdesc->Scaling)
							{
								selectedDesc = *pdesc;
								foundExactModeMatch = true;
								foundBestModeMatch = true;
								break;
							}
							else if (refreshRate >= MIN_REFRESH_RATE && pdesc->Width >= 640 && pdesc->Height >= 300)
							{
								bool chooseMode = false;
								if (!foundBestModeMatch)
								{
									chooseMode = true;
								}
								else
								{
									if (pRequestedMode->Width == 0 || pRequestedMode->Height == 0)
									{
										if (GetScanRating(selectedDesc.ScanlineOrdering) < MinNonInterlacedScanRating && GetScanRating(pdesc->ScanlineOrdering) >= MinNonInterlacedScanRating)
										{
											chooseMode = true;
										}
										else if (GetScanRating(pdesc->ScanlineOrdering) >= MinNonInterlacedScanRating || GetScanRating(pdesc->ScanlineOrdering) >= GetScanRating(selectedDesc.ScanlineOrdering))
										{
											if (pdesc->Width > selectedDesc.Width)
											{
												chooseMode = true;
											}
											else if (pdesc->Width == selectedDesc.Width)
											{
												if (pdesc->Height > selectedDesc.Height)
												{
													chooseMode = true;
												}
												else if (pdesc->Height == selectedDesc.Height)
												{
													if (refreshRate > selectedRefreshRate)
													{
														chooseMode = true;
													}
													else if (refreshRate == selectedRefreshRate)
													{
														if (GetScaleRating(pdesc->Scaling) > GetScaleRating(selectedDesc.Scaling))
														{
															chooseMode = true;
														}
														else if (GetScaleRating(pdesc->Scaling) == GetScaleRating(selectedDesc.Scaling))
														{
															chooseMode = true;
														}
													}
												}
											}
										}
									}
									else if (pRequestedMode->Width == pdesc->Width && pRequestedMode->Height == pdesc->Height && pRequestedMode->ScanlineOrdering == pdesc->ScanlineOrdering && pRequestedMode->Scaling == pdesc->Scaling)
									{
										if (refreshRate > selectedRefreshRate)
										{
											chooseMode = true;
										}
										else if (refreshRate == selectedRefreshRate)
										{
											chooseMode = true;
										}
									}
								}

								if (chooseMode)
								{
									selectedDesc = *pdesc;
									selectedRefreshRate = 0.0;
									if (pdesc->RefreshRate.Numerator != 0 && pdesc->RefreshRate.Denominator != 0)
									{
										selectedRefreshRate = (double)selectedDesc.RefreshRate.Numerator / (double)selectedDesc.RefreshRate.Denominator;
									}

									foundBestModeMatch = true;
								}
							}
						}
					}
				}
			}
			catch (std::bad_alloc)
			{
				delete[] pDescs;
				pDescs = nullptr;
				throw;
			}

			delete[] pDescs;
			pDescs = nullptr;
		}
	}

	if (!foundBestModeMatch)
	{
		DXGI_MODE_DESC modeToMatchDesc = {};
		modeToMatchDesc.Width = pRequestedMode->Width;
		modeToMatchDesc.Height = pRequestedMode->Height;
		modeToMatchDesc.Format = GraphicsHelper::DefaultPixelFormat;
		modeToMatchDesc.Scaling = pRequestedMode->Scaling;
		modeToMatchDesc.ScanlineOrdering = pRequestedMode->ScanlineOrdering;
		if (pRequestedMode->RefreshRate.Numerator == 0 || pRequestedMode->RefreshRate.Denominator == 0)
		{
			modeToMatchDesc.RefreshRate.Numerator = 0;
			modeToMatchDesc.RefreshRate.Denominator = 0;
		}
		else
		{
			modeToMatchDesc.RefreshRate.Numerator = pRequestedMode->RefreshRate.Numerator;
			modeToMatchDesc.RefreshRate.Denominator = pRequestedMode->RefreshRate.Denominator;
		}

		hr = pOutput->FindClosestMatchingMode(&modeToMatchDesc, &selectedDesc, device);
		if (FAILED(hr))
		{
			ZeroMemory(&modeToMatchDesc, sizeof(modeToMatchDesc));
			modeToMatchDesc.Format = GraphicsHelper::DefaultPixelFormat;
			hr = pOutput->FindClosestMatchingMode(&modeToMatchDesc, &selectedDesc, device);
		}

		if (SUCCEEDED(hr))
		{
			foundBestModeMatch = true;
		}
	}

	if (foundBestModeMatch)
	{
		*pPreferredMode = selectedDesc;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}
