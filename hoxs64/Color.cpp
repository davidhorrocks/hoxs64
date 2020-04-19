#include "Color.h"

Color::Color() noexcept
	:color(0)
{}

Color::Color(unsigned __int32 val) noexcept
	: color(val)
{}

Color::Color(BYTE r, BYTE g, BYTE b) noexcept
	: Color(r, g, b, 255)
{
}

Color::Color(BYTE r, BYTE g, BYTE b, BYTE a) noexcept
{
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

Color::Color(const Color& src) noexcept
	:color(src.color)
{}

Color& Color::operator=(const Color& src) noexcept
{
	this->color = src.color;
	return *this;
}

bool Color::operator==(const Color& rhs) const noexcept
{
	return (this->color == rhs.color);
}

bool Color::operator!=(const Color& rhs) const noexcept
{
	return !(*this == rhs);
}

constexpr BYTE Color::GetR() const noexcept
{
	return this->rgba[0];
}
void Color::SetR(BYTE r) noexcept
{
	this->rgba[0] = r;
}

constexpr BYTE Color::GetG() const noexcept
{
	return this->rgba[1];
}
void Color::SetG(BYTE g) noexcept
{
	this->rgba[1] = g;
}

constexpr BYTE Color::GetB() const noexcept
{
	return this->rgba[2];
}
void Color::SetB(BYTE b) noexcept
{
	this->rgba[2] = b;
}

constexpr BYTE Color::GetA() const noexcept
{
	return this->rgba[3];
}

void Color::SetA(BYTE a) noexcept
{
	this->rgba[3] = a;
}

COLORREF Color::GetColorRef() const noexcept
{
	BYTE r = rgba[0];
	BYTE g = rgba[1];
	BYTE b = rgba[2];
	BYTE a = rgba[3];
	return ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16) | (((DWORD)(BYTE)(a)) << 24)));
}