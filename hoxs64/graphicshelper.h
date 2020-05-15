#pragma once
#include "dx_version.h"
#include <tchar.h>
#include <string>
#include <wrl.h>

struct GraphicsDisplayMode
{
	unsigned int Width;
	unsigned int Height;
	unsigned int RefreshRateNumerator;
	unsigned int RefreshRateDenominator;
	DXGI_FORMAT Format;
	DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	DXGI_MODE_SCALING Scaling;
};

class GraphicsHelper
{
public:
	static DWORD ConvertColour(DXGI_FORMAT format, COLORREF rgb);
	static DWORD DDColorMatch(IDXGISurface1* pdds, COLORREF rgb);
	static unsigned int GetBitsPerPixel(DXGI_FORMAT fmt) noexcept;
	static DWORD ReduceBits(BYTE v, BYTE bits) noexcept;
	static unsigned int GetDisplayResolutionText(unsigned int width, unsigned int height, LPWSTR buffer, unsigned int charBufferLen);
	static unsigned int GetDisplayFormatText(DXGI_FORMAT format, LPWSTR buffer, unsigned int charBufferLen);
	static unsigned int GetDisplayRefreshText(unsigned int refreshRateNumerator, unsigned int refreshRateDenominator, LPWSTR buffer, unsigned int charBufferLen);
	static const DXGI_FORMAT Formats[1];
	static const DXGI_FORMAT Formats2[9];
	static const DXGI_FORMAT DefaultPixelFormat = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	static bool IsAcceptableMode(UINT Width, UINT Height) noexcept;
	static bool IsAcceptableMode(UINT Width, UINT Height, DXGI_FORMAT Format) noexcept;
	static bool IsAcceptableFormat(DXGI_FORMAT format) noexcept;
	static bool GetAppDir(std::wstring& str);
	static bool GetAppFilename(std::wstring& str);
	static bool GetAppFileFullPath(const wchar_t* filename, std::wstring& str);
    static HRESULT GetPreferredMode(ID3D11Device *device, IDXGIOutput* pOutput, const DXGI_MODE_DESC* pRequestedMode, DXGI_MODE_DESC* pPreferredMode);
private:
	static const int MinNonInterlacedScanRating = 3;
	static int GetScanRating(const DXGI_MODE_SCANLINE_ORDER& scanOrder);
	static int GetScaleRating(const DXGI_MODE_SCALING& scaling);
};
