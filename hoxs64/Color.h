#pragma once
#include "windows.h"

class Color
{
public:
	Color() noexcept;
	~Color() = default;
	Color(Color&&) = default;
	Color& operator=(Color&&) = default;
	Color(unsigned __int32 val) noexcept;
	Color(BYTE r, BYTE g, BYTE b) noexcept;
	Color(BYTE r, BYTE g, BYTE b, BYTE a) noexcept;
	Color(const Color& src) noexcept;

	Color& operator=(const Color& src) noexcept;
	bool operator==(const Color& rhs) const noexcept;
	bool operator!=(const Color& rhs) const noexcept;


	constexpr BYTE GetR() const noexcept;
	void SetR(BYTE r) noexcept;

	constexpr BYTE GetG() const noexcept;
	void SetG(BYTE g) noexcept;

	constexpr BYTE GetB() const noexcept;
	void SetB(BYTE b) noexcept;

	constexpr BYTE GetA() const noexcept;
	void SetA(BYTE a) noexcept;

	COLORREF GetColorRef() const noexcept;

private:
	union
	{
		BYTE rgba[4];
		unsigned __int32 color;
	};
};

namespace Colors
{
	constexpr unsigned __int32 UnloadedTextureColor = 0xff646464;
	constexpr unsigned __int32 UnhandledTextureColor = 0xfffa0000;
}
